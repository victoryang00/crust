#ifndef _CRUST_INCLUDE_FUNCTION_HPP
#define _CRUST_INCLUDE_FUNCTION_HPP


#include "utility.hpp"


namespace crust { namespace ops {
namespace _impl_fn {
template <
    class Self,
    class F = decltype(&Self::operator()),
    F f = &Self::operator()>
struct MemFnClosure;

template <class Self, class Ret, class... Args, Ret (Self::*f)(Args...) const>
struct MemFnClosure<Self, Ret (Self::*)(Args...) const, f> {
  using Inner = Ret(Args...);
};

template <class Self, class Ret, class... Args, Ret (Self::*f)(Args...)>
struct MemFnClosure<Self, Ret (Self::*)(Args...), f> {
  using Inner = Ret(Args...);
};

template <class F, F f>
struct RawMemFn;

template <class Self, class Ret, class... Args, Ret (Self::*f)(Args...) const>
struct RawMemFn<Ret (Self::*)(Args...) const, f> {
  constexpr Ret operator()(const Self &self, Args... args) const {
    return (self.*f)(forward<Args>(args)...);
  }
};

template <class Self, class Ret, class... Args, Ret (Self::*f)(Args...)>
struct RawMemFn<Ret (Self::*)(Args...), f> {
  constexpr Ret operator()(Self &self, Args... args) const {
    return (self.*f)(forward<Args>(args)...);
  }
};

template <class F, F *f>
struct RawFn;

template <class Ret, class... Args, Ret (*f)(Args...)>
struct RawFn<Ret(Args...), f> {
  constexpr Ret operator()(Args... args) const {
    return f(forward<Args>(args)...);
  }
};
} // namespace _impl_fn

template <class Self, class F = typename _impl_fn::MemFnClosure<Self>::Inner>
class Fn;

template <class Self, class Ret, class... Args>
class Fn<Self, Ret(Args...)> {
private:
  crust_static_assert(!IsConstOrRef<Self>::result);

  Self self;

public:
  constexpr Fn(Self &&self) : self{move(self)} {}

  crust_cxx14_constexpr Ret operator()(Args... args) const {
    return self(forward<Args>(args)...);
  }
};

template <class F, F *f>
Fn<_impl_fn::RawFn<F, f>> bind(TmplArg<F *, f>) {
  return Fn<_impl_fn::RawFn<F, f>>{_impl_fn::RawFn<F, f>{}};
}

template <class T, class F, F *f>
Fn<_impl_fn::RawFn<F, f>, T> bind(TmplArg<F *, f>) {
  return Fn<_impl_fn::RawFn<F, f>, T>{_impl_fn::RawFn<F, f>{}};
}

template <class F, F f>
Fn<_impl_fn::RawMemFn<F, f>> bind(TmplArg<F, f>) {
  return Fn<_impl_fn::RawMemFn<F, f>>{_impl_fn::RawMemFn<F, f>{}};
}

template <class T, class F, F f>
Fn<_impl_fn::RawMemFn<F, f>, T> bind(TmplArg<F, f>) {
  return Fn<_impl_fn::RawMemFn<F, f>, T>{_impl_fn::RawMemFn<F, f>{}};
}

template <class T>
constexpr Fn<T> bind(T &&f) {
  return Fn<T>{forward<T>(f)};
}

template <class F, class T>
constexpr Fn<T, F> bind(T &&f) {
  return Fn<T, F>{forward<T>(f)};
}

template <class Self, class F = typename _impl_fn::MemFnClosure<Self>::Inner>
class FnMut;

template <class Self, class Ret, class... Args>
class FnMut<Self, Ret(Args...)> {
private:
  crust_static_assert(!IsConstOrRef<Self>::result);

  Self self;

public:
  constexpr FnMut(Self &&self) : self{move(self)} {}

  crust_cxx14_constexpr Ret operator()(Args... args) {
    return self(forward<Args>(args)...);
  }
};

template <class F, F *f>
FnMut<_impl_fn::RawFn<F, f>> bind_mut(TmplArg<F *, f>) {
  return FnMut<_impl_fn::RawFn<F, f>>{_impl_fn::RawFn<F, f>{}};
}

template <class T, class F, F *f>
FnMut<_impl_fn::RawFn<F, f>, T> bind_mut(TmplArg<F *, f>) {
  return FnMut<_impl_fn::RawFn<F, f>, T>{_impl_fn::RawFn<F, f>{}};
}

template <class F, F f>
FnMut<_impl_fn::RawMemFn<F, f>> bind_mut(TmplArg<F, f>) {
  return FnMut<_impl_fn::RawMemFn<F, f>>{_impl_fn::RawMemFn<F, f>{}};
}

template <class T, class F, F f>
FnMut<_impl_fn::RawMemFn<F, f>, T> bind_mut(TmplArg<F, f>) {
  return FnMut<_impl_fn::RawMemFn<F, f>, T>{_impl_fn::RawMemFn<F, f>{}};
}

template <class T>
constexpr FnMut<T> bind_mut(T &&f) {
  return FnMut<T>{forward<T>(f)};
}

template <class F, class T>
constexpr FnMut<T, F> bind_mut(T &&f) {
  return FnMut<T, F>{forward<T>(f)};
}

namespace _impl_fn {
template <class Base, class Self>
struct DestructorStaticWrapper {
  static crust_cxx14_constexpr void inner(Base *self) {
    reinterpret_cast<Self *>(self)->~Self();
  }
};

template <class Ret, class... Args>
struct FnVTable {
  void (*drop)(void *);
  usize size;
  usize align;
  Ret (*call)(const void *, Args...);
};

template <class Self, class Ret, class... Args>
struct StaticFnVTable {
  static const FnVTable<Ret, Args...> vtable;

  static constexpr Ret call(const void *self, Args &&...args) {
    return (*reinterpret_cast<const Self *>(self))(forward<Args>(args)...);
  }
};

template <class Self, class Ret, class... Args>
const FnVTable<Ret, Args...> StaticFnVTable<Self, Ret, Args...>::vtable{
    DestructorStaticWrapper<void, Self>::inner,
    sizeof(Self),
    alignof(Self),
    call,
};

template <class Ret, class... Args>
struct FnMutVTable {
  void (*drop)(void *);
  usize size;
  usize align;
  Ret (*call)(void *, Args...);
};

template <class Self, class Ret, class... Args>
struct StaticFnMutVTable {
  static const FnMutVTable<Ret, Args...> vtable;

  static crust_cxx14_constexpr Ret call(void *self, Args... args) {
    return (*reinterpret_cast<Self *>(self))(forward<Args>(args)...);
  }
};

template <class Self, class Ret, class... Args>
const FnMutVTable<Ret, Args...> StaticFnMutVTable<Self, Ret, Args...>::vtable{
    DestructorStaticWrapper<void, Self>::inner,
    sizeof(Self),
    alignof(Self),
    call,
};
} // namespace _impl_fn

template <class F>
class DynFn;

template <class F>
class DynFnMut;

template <class Ret, class... Args>
class DynFn<Ret(Args...)> { // todo: optimize for zero sized type
private:
  friend DynFnMut<Ret(Args...)>;

  void *self;
  const _impl_fn::FnVTable<Ret, Args...> *vtable;

  void drop() {
    vtable->drop(self);
    delete reinterpret_cast<char *>(self);
    self = nullptr;
  }

public:
  template <class Self>
  constexpr DynFn(Self &&self) :
      self{new Self{forward<Self>(self)}},
      vtable{&_impl_fn::StaticFnVTable<Self, Ret, Args...>::vtable} {}

  template <class F, F *f>
  constexpr DynFn(TmplArg<F *, f>) : DynFn{_impl_fn::RawFn<F, f>{}} {}

  DynFn(DynFn &&other) noexcept {
    if (this != &other) {
      self = other.self;
      vtable = other.vtable;
      other.self = nullptr;
    }
  }

  DynFn &operator=(DynFn &&other) noexcept {
    if (this != &other) {
      if (self != nullptr) {
        drop();
      }
      self = other.self;
      vtable = other.vtable;
      other.self = nullptr;
    }

    return *this;
  }

  constexpr Ret operator()(Args &&...args) const {
    return vtable->call(self, forward<Args>(args)...);
  }

  ~DynFn() {
    if (self != nullptr) {
      drop();
    }
  }
};

template <class Ret, class... Args>
class DynFnMut<Ret(Args...)> {
private:
  void *self;
  const _impl_fn::FnMutVTable<Ret, Args...> *vtable;

  void drop() {
    vtable->drop(self);
    delete reinterpret_cast<char *>(self);
    self = nullptr;
  }

public:
  template <class Self>
  constexpr DynFnMut(Self &&self) :
      self{new Self{forward<Self>(self)}},
      vtable{&_impl_fn::StaticFnMutVTable<Self, Ret, Args...>::vtable} {}

  template <class F, F *f>
  constexpr DynFnMut(TmplArg<F *, f>) : DynFnMut{_impl_fn::RawFn<F, f>{}} {}

  DynFnMut(DynFnMut &&other) noexcept {
    if (this != &other) {
      self = other.self;
      vtable = other.vtable;
      other.self = nullptr;
    }
  }

  DynFnMut &operator=(DynFnMut &&other) noexcept {
    if (this != &other) {
      if (self != nullptr) {
        drop();
      }
      self = other.self;
      vtable = other.vtable;
      other.self = nullptr;
    }

    return *this;
  }

  crust_cxx14_constexpr Ret operator()(Args... args) {
    return vtable->call(self, forward<Args>(args)...);
  }

  ~DynFnMut() {
    if (self != nullptr) {
      drop();
    }
  }
};
}} // namespace crust::ops


#endif //_CRUST_INCLUDE_FUNCTION_HPP

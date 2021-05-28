#pragma once

namespace Moe {

template<typename T>
class Opt_ptr
{
  l4_umword_t _e;

  explicit Opt_ptr(int err)
  : _e((l4_umword_t)err)
  { /*assert(err < 0 && err > -L4_ERRNOMAX);*/ }

public:
  Opt_ptr() = default;
  constexpr Opt_ptr(T* e) : _e(reinterpret_cast<l4_umword_t>(e)) {}

  constexpr Opt_ptr(Opt_ptr const &other) : _e(other._e) {}
  inline Opt_ptr& operator=(Opt_ptr const &other)
  {
    _e = other._e;
    return *this;
  }

  static inline Opt_ptr err(int e)
  { return Opt_ptr(e); }

  explicit operator bool() const
  { return _e < (l4_umword_t)-L4_ERRNOMAX; }

  inline T* unwrap() const
  {
    assert(_e < (l4_umword_t)-L4_ERRNOMAX);
    return reinterpret_cast<T*>(_e);
  }

  inline int err() const
  {
    return (int)_e;
  }
};

}

/**
 * \file
 * Type information handling.
 */
/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Alexander Warg <alexander.warg@kernkonzept.com>
 */
/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once
#pragma GCC system_header

#include "cxx/types"
#include "cxx/ipc_basics"
#include "cxx/capability.h"

#if defined(__GXX_RTTI) && !defined(L4_NO_RTTI)
#  include <typeinfo>
   typedef std::type_info const *L4_std_type_info_ptr;
#  define L4_KOBJECT_META_RTTI(type) (&typeid(type))
   inline char const *L4_kobject_type_name(L4_std_type_info_ptr n)
   { return n ? n->name() : 0; }
#else
   typedef void const *L4_std_type_info_ptr;
#  define L4_KOBJECT_META_RTTI(type) (0)
   inline char const *L4_kobject_type_name(L4_std_type_info_ptr)
   { return 0; }
#endif

namespace L4 {
  typedef int Opcode;
// internal max helpers
namespace __I {
  // internal max of A nd B helper
  template< unsigned char A, unsigned char B>
  struct Max { enum { Res = A > B ? A : B }; };
} // namespace __I

enum
{
  /// Default protocol used by Kobject_t and Kobject_x
  PROTO_ANY   = 0,
  /// Empty protocol for empty APIs
  PROTO_EMPTY = -19,
};

/**
 * \defgroup l4_cxx_ipc_ifaces C++ IPC Interface Definition.
 * \ingroup l4_api
 *
 * APIs for defining IPC interfaces using C++ as language.
 */

/**
 * \ingroup l4_cxx_ipc_ifaces
 * Definition of interface data-type helpers.
 * \note These type helpers are intended for internal use, if you look for
 *       standard C++ type traits use the `<type_traits>` header for the
 *       standard C++ library or use `<l4/cxx/type_traits>`.
 */
namespace Typeid {
  /**
   * \defgroup l4_cxx_ipc_internal Internal Helpers
   * \ingroup l4_cxx_ipc_ifaces
   */

    using namespace L4::Types;

  /*********************/
  /**
   * \internal
   * Pair of a protocol number and a C++ interface class describing an interface.
   * \tparam P  The protocol number.
   * \tparam T  The C++ class describing the IPC interface.
   * \ingroup l4_cxx_ipc_internal
   */
  template<long P, typename T>
  struct Iface
  {
    typedef Iface type;
    typedef T iface_type;
    enum { Proto = P };
  };


  /*********************/
  /**
   * \internal
   * End marker for an interface list.
   * \ingroup l4_cxx_ipc_internal
   */
  struct Iface_list_end
  {
    typedef Iface_list_end type;
    static bool contains(long) { return false; }
  };


  /**
   * \internal
   * List of IPC interfaces.
   * \tparam I  The first interface
   * \tparam N  The list tail
   * \ingroup l4_cxx_ipc_internal
   */
  template<typename I, typename N = Iface_list_end>
  struct Iface_list
  {
    typedef Iface_list<I, N> type;

    typedef typename I::iface_type iface_type;
    typedef N Next;

    enum { Proto = I::Proto };

    static bool contains(long proto)
    { return (proto == Proto) || Next::contains(proto); }
  };

  // do not insert PROTO_EMPTY interfaces
  template<typename I, typename N>
  struct Iface_list<Iface<PROTO_EMPTY, I>, N> : N {};

  // do not insert 'void' type interfaces
  template<long P, typename N>
  struct Iface_list<Iface<P, void>, N> : N {};


  /*********************/
  /*
   * \internal
   * Test if an interface I is in list L
   * \tparam I  Interface for lookup
   * \tparam L  Iface_list for search
   */
  template< typename I, typename L >
  struct _In_list;

  template< typename I >
  struct _In_list<I, Iface_list_end> : False {};

  template< typename I, typename N >
  struct _In_list<I, Iface_list<I, N> > : True {};

  template< typename I, typename I2, typename N >
  struct _In_list<I, Iface_list<I2, N> > : _In_list<I, typename N::type> {};

  template<typename I, typename L>
  struct In_list : _In_list<typename I::type, typename L::type> {};


  /************/
  /*
   * \internal
   * Add Helper: add I to interface list L if ADD is true
   * \ingroup l4_cxx_ipc_internal
   */
  template< bool ADD, typename I, typename L>
  struct _Iface_list_add;

  template< typename I, typename L>
  struct _Iface_list_add<false, I, L> : L {};

  template< typename I, typename L>
  struct _Iface_list_add<true, I, L> : Iface_list<I, L> {};

  /*
   * \internal
   * Add Helper: add I to interface list L if not already in L.
   * \ingroup l4_cxx_ipc_internal
   */
  template< typename I, typename L >
  struct Iface_list_add :
    _Iface_list_add<
      !In_list<I, typename L::type>::value, I, typename L::type>
  {};

  /************/
  /*
   * \internal
   * Helper: checking for a conflict between I2 and I2.
   * A conflict means I1 and I2 have the same protocol ID but a different
   * iface_type.
   */
  template< typename I1, typename I2 >
  struct __Iface_conflict : Bool<I1::Proto != PROTO_EMPTY && I1::Proto == I2::Proto> {};

  template< typename I >
  struct __Iface_conflict<I, I> : False {};

  /*
   * \internal
   * Helper: checking for a conflict between I and any interface in LIST.
   */
  template< typename I, typename LIST >
  struct _Iface_conflict;

  template< typename I >
  struct _Iface_conflict<I, Iface_list_end> : False {};

  template< typename I, typename I2, typename LIST >
  struct _Iface_conflict<I, Iface_list<I2, LIST> > :
    Bool<__Iface_conflict<I, I2>::value || _Iface_conflict<I, typename LIST::type>::value>
  {};

  /**
   * \internal
   * check for a conflict between I and any interface in LIST.
   */
  template< typename I, typename LIST >
  struct Iface_conflict : _Iface_conflict<typename I::type, typename LIST::type> {};

  /**************/
  /*
   * \internal
   * Helper: merge two interface lists
   */
  template< typename L1, typename L2 >
  struct _Merge_list;

  template< typename L >
  struct _Merge_list<Iface_list_end, L> : L {};

  template< typename I, typename L1, typename L2 >
  struct _Merge_list<Iface_list<I, L1>, L2> :
    _Merge_list<typename L1::type, typename Iface_list_add<I, L2>::type> {};

  template<typename L1, typename L2>
  struct Merge_list : _Merge_list<typename L1::type, typename L2::type> {};

  /**************/
  /*
   * \internal
   * check for conflicts among all interfaces in L1 with any interfaces in L2.
   */
  template< typename L1, typename L2 >
  struct _Conflict;

  template< typename L >
  struct _Conflict<Iface_list_end, L> : False {};

  template< typename I, typename L1, typename L2 >
  struct _Conflict<Iface_list<I, L1>, L2> :
    Bool<Iface_conflict<I, typename L2::type>::value
           || _Conflict<typename L1::type, typename L2::type>::value> {};

  template< typename L1, typename L2 >
  struct Conflict : _Conflict<typename L1::type, typename L2::type> {};

  // to be removed ---------------------------------------
  // p_dispatch code -- for legacy dispatch ------------------------------
  /**********************/
  /*
   * \internal
   * helper: Dispatch helper for calling server-side p_dispatch() functions.
   */
  template<typename LIST>
  struct _P_dispatch;

  // No matching dispatcher found
  template<>
  struct _P_dispatch<Iface_list_end>
  {
    template< typename THIS, typename A1, typename A2 >
    static int f(THIS *, long, A1, A2 &)
    { return -L4_EBADPROTO; }
  };


  // call matching p_dispatch() function
  template< typename I, typename LIST >
  struct _P_dispatch<Iface_list<I, LIST> >
  {
    // special handling for the meta protocol, to avoid 'using' murx
    template< typename THIS, typename A1, typename A2 >
    static int _f(THIS self, A1, A2 &a2, True::type)
    {
      return self->dispatch_meta_request(a2);
    }

    // normal p_dispatch() dispatching
    template< typename THIS, typename A1, typename A2 >
    static int _f(THIS self, A1 a1, A2 &a2, False::type)
    {
      return self->p_dispatch(reinterpret_cast<typename I::iface_type *>(0),
                              a1, a2);
    }

    // dispatch function with switch for meta protocol
    template< typename THIS, typename A1, typename A2 >
    static int f(THIS *self, long proto, A1 a1, A2 &a2)
    {
      if (I::Proto == proto)
        return _f(self, a1, a2, Bool<I::Proto == (long)L4_PROTO_META>());

      return _P_dispatch<typename LIST::type>::f(self, proto, a1, a2);
    }
  };

  /// Use for protocol based dispatch stage
  template<typename LIST>
  struct P_dispatch : _P_dispatch<typename LIST::type> {};
  // end: p_dispatch -------------------------------------------------------
  // end: to be removed ---------------------------------------

  template<typename RPC> struct Default_op;

  namespace Detail {

  /// Internal end-of-list marker
  struct Rpcs_end
  {
    typedef void opcode_type;
    typedef Rpcs_end rpc;
    typedef Rpcs_end type;
  };

  /// \cond
  template<typename O1, typename O2, typename RPCS>
  struct _Rpc : _Rpc<typename RPCS::next::rpc, O2, typename RPCS::next>::type {};
  /// \endcond

  template<typename O1, typename O2>
  struct _Rpc<O1, O2, Rpcs_end> {};

  template<typename OP, typename RPCS>
  struct _Rpc<OP, OP, RPCS> : RPCS
  {
    typedef _Rpc type;
  };

  template<typename OP, typename RPCS>
  struct Rpc : _Rpc<typename RPCS::rpc, OP, RPCS> {};

  template<typename T, unsigned CODE>
  struct _Get_opcode
  {
    template<bool, typename> struct Invalid_opcode {};
    template<typename X> struct Invalid_opcode<true, X>;

  private:
    template<typename U, U> struct _chk;
    template<typename U> static long _opc(_chk<int, U::Opcode> *);
    template<typename U> static char _opc(...);

    template<unsigned SZ, typename U>
    struct _Opc { enum { value = CODE }; };

    template<typename U>
    struct _Opc<sizeof(long), U> { enum { value = U::Opcode }; };

  public:
    enum { value = _Opc<sizeof(_opc<T>(0)), T>::value };
    Invalid_opcode<(value < CODE), T> invalid_opcode;
  };

  /// Empty list of RPCs
  template<typename OPCODE, unsigned O, typename ...X>
  struct _Rpcs : Rpcs_end {};

  /// Non-empty list of RPCs
  template<typename OPCODE, unsigned O, typename R, typename ...X>
  struct _Rpcs<OPCODE, O, R, X...>
  {
    /// The list element itself
    typedef _Rpcs type;
    /// The data type for the opcode
    typedef OPCODE opcode_type;
    /// The RPC type L4::Ipc::Msg::Rpc_call or L4::Ipc::Msg::Rpc_inline_call
    typedef R rpc;
    /// The next RPC in the list or Rpcs_end if this is the last
    typedef typename _Rpcs<OPCODE, _Get_opcode<R, O>::value + 1, X...>::type next;
    /// The opcode value to use for this RPC, may be bogus if the opcode_type is void
    enum { Opcode = _Get_opcode<R, O>::value };
    /// Find the given RPC in the list
    template<typename Y> struct Rpc : Typeid::Detail::Rpc<Y, _Rpcs> {};
  };

  template<typename OPCODE, unsigned O, typename R>
  struct _Rpcs<OPCODE, O, Default_op<R> >
  {
    /// The list element itself
    typedef _Rpcs type;
    /// The data type for the opcode
    typedef void opcode_type;
    /// The RPC type L4::Ipc::Msg::Rpc_call or L4::Ipc::Msg::Rpc_inline_call
    typedef R rpc;
    /// The next RPC in the list or Rpcs_end if this is the last
    typedef Rpcs_end next;
    /// The opcode value to use for this RPC, may be bogus if the opcode_type is void
    enum { Opcode = -99 };
    /// Find the given RPC in the list
    template<typename Y> struct Rpc : Typeid::Detail::Rpc<Y, _Rpcs> {};
  };

  } // namespace Detail

  /**
   * RPCs list for passing raw incoming IPC to the server object.
   * \tparam CLASS  The type of the interface (e.g., L4::Icu)
   * \headerfile l4/sys/capability
   *
   * This template allows to have fully handcrafted IPC protocols.
   */
  template<typename CLASS>
  struct Raw_ipc
  {
    typedef Raw_ipc type;
    typedef Detail::Rpcs_end next;
    typedef void opcode_type;
  };

  /**
   * Standard list of RPCs of an interface.
   * \tparam RPCS  list of RPC types as defined by L4_RPC etc.
   * \headerfile l4/sys/capability
   *
   * This is the default list for RPC functions of an interface, it uses
   * L4::Opcode as opcode type and uses opcodes starting from 0.
   */
  template<typename ...RPCS>
  struct Rpcs : Detail::_Rpcs<L4::Opcode, 0, RPCS...> {};

  /**
   * List of RPCs of an interface using a special opcode type
   * \tparam OPCODE_TYPE  The data type of the opcode.
   * \headerfile l4/sys/capability
   *
   * List for RPC functions of an interface, using OPCODE_TYPE
   * as data type for the opcode, opcodes starting from 0.
   */
  template<typename OPCODE_TYPE>
  struct Rpcs_code
  {
   /**
    * \tparam RPCS  list of RPC types as defined by L4_RPC etc.
    */
    template<typename ...RPCS>
    struct F : Detail::_Rpcs<OPCODE_TYPE, 0, RPCS...> {};
  };

  /**
   * List of RPCs of an interface using a single operation without an opcode.
   * \tparam OPERATION  The RPC operation as defined by L4_RPC etc.
   * \headerfile l4/sys/capability
   */
  template<typename OPERATION>
  struct Rpc_nocode : Detail::_Rpcs<void, 0, OPERATION> {};

  /**
   * List of RPCs typically used for kernel interfaces.
   * \tparam RPCS  list of RPC types as defined by L4_RPC etc.
   * \headerfile l4/sys/capability
   *
   * This list of RPC functions uses l4_umword_t as type for the opcode as
   * most kernel protocol do.
   */
  template<typename ...ARG>
  struct Rpcs_sys : Detail::_Rpcs<l4_umword_t, 0, ARG...> {};

  template<typename CLASS>
  struct Rights
  {
    unsigned rights;
    Rights(unsigned rights) : rights(rights) {}
    unsigned operator & (unsigned rhs) const { return rights & rhs; }
  };

} // namespace Typeid

/**
 * Type information for L4 server objects that can be called via IPC.
 * \defgroup l4_kobject_rtti L4 kernel object type information
 * \ingroup l4_kernel_object_api
 *
 * This type information consists of inheritance information, the protocol
 * number assigned to an interface as well as the demand on server-side
 * resources.
 */
/**
 * Dynamic Type Information for L4Re Interfaces.
 * \ingroup l4_kobject_rtti
 * \headerfile l4/sys/capability
 *
 * This class represents the runtime-dynamic type information for
 * L4Re interfaces, and is not intended to be used directly by applications.
 * \note The interface of is subject to changes.
 *
 * The main use for this info is to be used by the implementation of the
 * L4::cap_dynamic_cast() function.
 *
 */
struct L4_EXPORT Type_info
{
  /**
   * Data type for expressing the needed receive buffers at the server-side
   * of an interface.
   * \headerfile l4/sys/capability
   */
  class L4_EXPORT Demand
  {
  private:
    /// internal max helper
    static unsigned char max(unsigned char a, unsigned char b)
    { return a > b ? a : b; }

  public:
    unsigned char caps;  ///< number of capability receive buffers.
    unsigned char flags; ///< flags, such as the need for timeouts (TBD).
    unsigned char mem;   ///< number of memory receive buffers.
    unsigned char ports; ///< number of IO-port receive buffers.

    /**
     * Make Demand object.
     * \param caps    number of capability receive buffers
     * \param flags   flags, such as the need for timeouts (TBD).
     * \param mem     number of memory receive windows.
     * \param ports   number of IO-port receive windows.
     */
    explicit
    Demand(unsigned char caps = 0, unsigned char flags = 0,
           unsigned char mem = 0,  unsigned char ports = 0)
    : caps(caps), flags(flags), mem(mem), ports(ports) {}

    /// \return true if there is no demand at all
    bool no_demand() const
    { return caps == 0 && mem == 0 && ports == 0 && flags == 0; }

    /// get the combined demand of this and rhs
    Demand operator | (Demand const &rhs) const
    {
      return Demand(max(caps, rhs.caps), flags | rhs.flags,
                    max(mem, rhs.mem), max(ports, rhs.ports));
    }
  };

  /**
   * Template type statically describing demand of receive buffers.
   * \tparam CAPS   number of capability receive buffers needed.
   * \tparam FLAGS  flags, such as the need for timeouts (TBD).
   * \tparam MEM    number of memory receive windows needed.
   * \tparam PORTS  number of IO-port receive windwows needed.
   * \headerfile l4/sys/capability
   */
  template<unsigned char CAPS = 0, unsigned char FLAGS = 0,
           unsigned char MEM  = 0, unsigned char PORTS = 0>
  struct Demand_t : Demand
  {
    enum
    {
      Caps  = CAPS,  ///< number of capability receive buffers.
      Flags = FLAGS, ///< flags, such as the need for timeouts.
      Mem   = MEM,   ///< number of memory receive windows.
      Ports = PORTS  ///< number of IO-port receive windows.
    };
    Demand_t() : Demand(CAPS, FLAGS, MEM, PORTS) {}
  };

  /**
   * Template type statically describing the combination of two
   * Demand object.
   * \tparam D1  first demand object.
   * \tparam D2  second demand object.
   * \headerfile l4/sys/capability
   */
  template<typename D1, typename D2>
  struct Demand_union_t : Demand_t<__I::Max<D1::Caps,  D2::Caps>::Res,
                                   D1::Flags | D2::Flags,
                                   __I::Max<D1::Mem,   D2::Mem>::Res,
                                   __I::Max<D1::Ports, D2::Ports>::Res>
  {};

  L4_std_type_info_ptr _type;
  Type_info const *const *_bases;
  unsigned _num_bases;
  long _proto;

  L4_std_type_info_ptr type() const { return _type; }
  Type_info const *base(unsigned idx) const { return _bases[idx]; }
  unsigned num_bases() const { return _num_bases; }
  long proto() const { return _proto; }
  char const *name() const { return L4_kobject_type_name(type()); }
  bool has_proto(long proto) const
  {
    if (_proto && _proto == proto)
      return true;

    if (!proto)
      return false;

    for (unsigned i = 0; i < _num_bases; ++i)
      if (base(i)->has_proto(proto))
        return true;

    return false;
  }
};

/**
 * Meta object for handling access to type information of Kobjects.
 * \tparam T  The data type derived from Kobject, usually using Kobject_t.
 * \ingroup l4_kobject_rtti
 */
template<typename T> struct Kobject_typeid
{
  /**
   * Data type expressing the static demand of receive buffers in a server.
   * \headerfile l4/sys/capability
   *
   * This information is the combined demand of all base interfaces for T and
   * the buffer demand of T itself.  The buffer demand of T is usually
   * specified as the S_DEMAND argument of the Kobject_t or Kobject_2t
   * inheritance helpers.  S_DEMAND is usually of type L4::Type_info::Demand_t,
   * or L4::Type_info::Demand_union_t.
   */
  typedef typename T::__Kobject_typeid::Demand Demand;
  typedef typename T::__Iface::iface_type Iface;
  typedef typename T::__Iface_list Iface_list;

  /**
   * Get a pointer to teh Kobject type information of T.
   * \return a pointer to the Kobject typeinfor of T.
   */
  static Type_info const *id() { return &T::__Kobject_typeid::_m; }

  /**
   * Get the receive-buffer demand for the server providing the
   * interface T.
   *
   * \return A demand value describing the minimum receive buffers
   *         needed for handling server side requests for interface T.
   */
  static Type_info::Demand demand()
  { return T::__Kobject_typeid::Demand(); }

  // to be removed ---------------------------------------
  // p_dispatch -----------------------------------------------------------
  /**
   * Protocol based server-side dispatch function.
   * \tparam THIS  Data type of the server-side object implementing the
   *               interface T.
   * \tparam A1    Data type of second argument for p_dispatch()
   * \tparam A2    Data type of third argument for p_dispatch()
   * \param self   The pointer to the server object
   * \param proto  The protocol number used by the caller
   * \param a1     The second argument passed to self->p_dispatch()
   * \param a2     The third argument passed to self->p_dispatch()
   *
   * This function forwards the call to the overloaded p_dispatch() function
   * of self. The data type of the first argument for p_dispatch is determined
   * by the given protocol number.
   */
  template<typename THIS, typename A1, typename A2>
  static int proto_dispatch(THIS *self, long proto, A1 a1, A2 &a2)
  { return Typeid::P_dispatch<typename T::__Iface_list>::f(self, proto, a1, a2);  }
  // p_dispatch -----------------------------------------------------------
  // end: to be removed ---------------------------------------
};

/**
 * Get the L4::Type_info for the L4Re interface given in `T`.
 * \ingroup l4_kobject_rtti
 * \tparam T  The type (L4Re interface) for which the information shall be
 *            returned.
 *
 * \return A pointer to the L4::Type_info structure for `T`.
 */
template<typename T>
inline
Type_info const *kobject_typeid()
{ return Kobject_typeid<T>::id(); }

/**
 * \internal
 * \ingroup l4_kobject_rtti
 */
#define L4____GEN_TI(t...)                                               \
Type_info const t::__Kobject_typeid::_m =                                \
{                                                                        \
  L4_KOBJECT_META_RTTI(Derived),                                         \
  &t::__Kobject_typeid::_b[0],                                           \
  sizeof(t::__Kobject_typeid::_b) / sizeof(t::__Kobject_typeid::_b[0]),  \
  PROTO                                                                  \
}

/**
 * \internal
 * \ingroup l4_kobject_rtti
 */
#define L4____GEN_TI_MEMBERS(BASE_DEMAND...)                             \
private:                                                                 \
  template< typename T > friend struct Kobject_typeid;                   \
protected:                                                               \
  struct __Kobject_typeid {                                              \
    typedef Type_info::Demand_union_t<S_DEMAND, BASE_DEMAND> Demand;     \
    static Type_info const *const _b[];                                  \
    static Type_info const _m;                                           \
  };                                                                     \
public:                                                                  \
  static long const Protocol = PROTO;                                    \
  typedef L4::Typeid::Rights<Class> Rights;

/**
 * \ingroup l4_kobject_rtti
 * \headerfile l4/sys/capability
 * Helper class to create an L4Re interface class that is derived
 *        from a single base class.
 *
 * \tparam Derived   is the name of the new interface.
 * \tparam Base      is the name of the interfaces single base class.
 * \tparam PROTO     may be set to the statically assigned protocol number
 *                   used to communicate with this interface.
 * \tparam S_DEMAND  type defining the demand on server-side resources for
 *                   this interface, usually a L4::Type_info::Demand_t.  This
 *                   value must describe the server-side resources needed by
 *                   the interface itself, the resource demand of the base
 *                   interface `Base` is automatically included.
 *
 * The typical usage pattern is shown in the following code snippet. The
 * semantics of this example is an interface My_iface that is derived from
 * L4::Kobject.
 *
 * \code
 * class My_iface : public L4::Kobject_t<My_iface, L4::Kobject>
 * {
 *   ...
 * };
 * \endcode
 *
 */
template<
  typename Derived,
  typename Base,
  long PROTO = PROTO_ANY,
  typename S_DEMAND = Type_info::Demand_t<>
>
class Kobject_t : public Base
{
protected:
  /// The target interface type (inheriting from Kobject_t)
  typedef Derived Class;
  /// The interface description for the derived class
  typedef Typeid::Iface<PROTO, Derived> __Iface;
  /// The list of all RPC interfaces provided directly or through inheritance
  typedef Typeid::Merge_list<
    Typeid::Iface_list<__Iface>, typename Base::__Iface_list
  > __Iface_list;

  /// Helper to check for protocol conflicts
  static void __check_protocols__()
  {
    typedef Typeid::Iface_conflict<__Iface, typename Base::__Iface_list> Base_conflict;
    static_assert(!Base_conflict::value, "ambiguous protocol ID: protocol also used by Base");
  }

  /// Get the capability to ourselves
  L4::Cap<Class> c() const { return L4::Cap<Class>(this->cap()); }

  // Generate the remaining type information
  L4____GEN_TI_MEMBERS(typename Base::__Kobject_typeid::Demand)
};


template< typename Derived, typename Base, long PROTO, typename S_DEMAND>
Type_info const *const
Kobject_t<Derived, Base, PROTO, S_DEMAND>::
  __Kobject_typeid::_b[] = { &Base::__Kobject_typeid::_m };

/**
 * \internal
 * \ingroup l4_kobject_rtti
 */
template< typename Derived, typename Base, long PROTO, typename S_DEMAND>
L4____GEN_TI(Kobject_t<Derived, Base, PROTO, S_DEMAND>);


/**
 * \ingroup l4_kobject_rtti
 * \headerfile l4/sys/capability
 * Helper class to create an L4Re interface class that is derived
 *        from two base classes (see `L4::Kobject_t`).
 *
 * \tparam Derived   is the name of the new interface.
 * \tparam Base1     is the name of the interface's first base class.
 * \tparam Base2     is the name of the interface's second base class.
 * \tparam PROTO     may be set to the statically assigned protocol number
 *                   used to communicate with this interface.
 * \tparam S_DEMAND  type defining the demand of server-side resources for
 *                   this interface, usually a L4::Type_info::Demand_t.  This
 *                   value must describe the server-side resources needed by
 *                   the interface itself, the resource demand of the base
 *                   interfaces (Base1 and Base2) are automatically included.
 *
 * The typical usage pattern is shown in the following code snippet. The
 * semantics of this example is an interface My_iface that is derived from
 * L4::Icu and L4Re::Dataspace.
 *
 * \code
 * class My_iface : public L4::Kobject_2t<My_iface, L4::Icu, L4Re::Dataspace>
 * {
 *   ...
 * };
 * \endcode
 *
 */
template<
  typename Derived,
  typename Base1,
  typename Base2,
  long PROTO = PROTO_ANY,
  typename S_DEMAND = Type_info::Demand_t<>
>
class Kobject_2t : public Base1, public Base2
{
protected:
  /// \copydoc L4::Kobject_t::Class
  typedef Derived Class;
  /// \copydoc L4::Kobject_t::__Iface
  typedef Typeid::Iface<PROTO, Derived> __Iface;
  /// \copydoc L4::Kobject_t::__Iface_list
  typedef Typeid::Merge_list<
    Typeid::Iface_list<__Iface>,
    Typeid::Merge_list<
      typename Base1::__Iface_list,
      typename Base2::__Iface_list
    >
  > __Iface_list;

  /// \copydoc L4::Kobject_t::__check_protocols__
  static void __check_protocols__()
  {
    typedef typename Base1::__Iface_list Base1_proto_list;
    typedef typename Base2::__Iface_list Base2_proto_list;

    typedef Typeid::Iface_conflict<__Iface, Base1_proto_list> Base1_conflict;
    typedef Typeid::Iface_conflict<__Iface, Base2_proto_list> Base2_conflict;
    static_assert(!Base1_conflict::value, "ambiguous protocol ID, also in Base1");
    static_assert(!Base2_conflict::value, "ambiguous protocol ID, also in Base2");

    typedef Typeid::Conflict<Base1_proto_list, Base2_proto_list> Bases_conflict;
    static_assert(!Bases_conflict::value, "ambiguous protocol IDs in base classes");
  }

  // disambiguate cap()
  l4_cap_idx_t cap() const throw()
  { return Base1::cap(); }

  /// \copydoc L4::Kobject_t::c()
  L4::Cap<Class> c() const { return L4::Cap<Class>(this->cap()); }

  L4____GEN_TI_MEMBERS(Type_info::Demand_union_t<
    typename Base1::__Kobject_typeid::Demand,
    typename Base2::__Kobject_typeid::Demand>
  )

public:
  // Provide non-ambiguous conversion to Kobject
  operator Kobject const & () const
  { return *static_cast<Base1 const *>(this); }

  // Provide non-ambiguous access of dec_refcnt()
  l4_msgtag_t dec_refcnt(l4_mword_t diff, l4_utcb_t *utcb = l4_utcb())
  { return Base1::dec_refcnt(diff, utcb); }
};


template< typename Derived, typename Base1, typename Base2,
          long PROTO, typename S_DEMAND >
Type_info const *const
Kobject_2t<Derived, Base1, Base2, PROTO, S_DEMAND>::__Kobject_typeid::_b[] =
{
  &Base1::__Kobject_typeid::_m,
  &Base2::__Kobject_typeid::_m
};

/**
 * \internal
 * \ingroup l4_kobject_rtti
 */
template< typename Derived, typename Base1, typename Base2,
          long PROTO, typename S_DEMAND >
L4____GEN_TI(Kobject_2t<Derived, Base1, Base2, PROTO, S_DEMAND>);



/**
 * \ingroup l4_kobject_rtti
 * \headerfile l4/sys/capability
 * Helper class to create an L4Re interface class that is derived
 *        from three base classes (see `L4::Kobject_t`).
 *
 * \tparam Derived   is the name of the new interface.
 * \tparam Base1     is the name of the interface's first base class.
 * \tparam Base2     is the name of the interface's second base class.
 * \tparam Base3     is the name of the interfaces third base class.
 * \tparam PROTO     may be set to the statically assigned protocol number
 *                   used to communicate with this interface.
 * \tparam S_DEMAND  type defining the demand on server-side resources for
 *                   this interface, usually a L4::Type_info::Demand_t.  This
 *                   value must describe the server-side resources needed by
 *                   the interface itself, the resource demand of the base
 *                   interfaces (Base1 and Base2) are automatically included.
 * \sa L4::Kobject_t, L4::Kobject_2t, L4::Kobject_0t, L4::Kobject_x
 */
template<
  typename Derived,
  typename Base1,
  typename Base2,
  typename Base3,
  long PROTO = PROTO_ANY,
  typename S_DEMAND = Type_info::Demand_t<>
>
struct Kobject_3t : Base1, Base2, Base3
{
protected:
  /// \copydoc L4::Kobject_t::Class
  typedef Derived Class;
  /// \copydoc L4::Kobject_t::__Iface
  typedef Typeid::Iface<PROTO, Derived> __Iface;
  /// \copydoc L4::Kobject_t::__Iface_list
  typedef Typeid::Merge_list<
    Typeid::Iface_list<__Iface>,
    Typeid::Merge_list<
      typename Base1::__Iface_list,
      Typeid::Merge_list<
        typename Base2::__Iface_list,
        typename Base3::__Iface_list
      >
    >
  > __Iface_list;

  /// \copydoc L4::Kobject_t::__check_protocols__
  static void __check_protocols__()
  {
    typedef typename Base1::__Iface_list Base1_proto_list;
    typedef typename Base2::__Iface_list Base2_proto_list;
    typedef typename Base3::__Iface_list Base3_proto_list;

    typedef Typeid::Iface_conflict<__Iface, Base1_proto_list> Base1_conflict;
    typedef Typeid::Iface_conflict<__Iface, Base2_proto_list> Base2_conflict;
    typedef Typeid::Iface_conflict<__Iface, Base3_proto_list> Base3_conflict;

    static_assert(!Base1_conflict::value, "ambiguous protocol ID, also in Base1");
    static_assert(!Base2_conflict::value, "ambiguous protocol ID, also in Base2");
    static_assert(!Base3_conflict::value, "ambiguous protocol ID, also in Base3");

    typedef Typeid::Conflict<Base1_proto_list, Base2_proto_list> Conflict_bases12;
    typedef Typeid::Conflict<Base1_proto_list, Base3_proto_list> Conflict_bases13;
    typedef Typeid::Conflict<Base2_proto_list, Base3_proto_list> Conflict_bases23;

    static_assert(!Conflict_bases12::value, "ambiguous protocol IDs in base classes: Base1 and Base2");
    static_assert(!Conflict_bases13::value, "ambiguous protocol IDs in base classes: Base1 and Base3");
    static_assert(!Conflict_bases23::value, "ambiguous protocol IDs in base classes: Base2 and Base3");
  }

  // disambiguate cap()
  l4_cap_idx_t cap() const throw()
  { return Base1::cap(); }

  /// \copydoc L4::Kobject_t::c()
  L4::Cap<Class> c() const { return L4::Cap<Class>(this->cap()); }

  L4____GEN_TI_MEMBERS(Type_info::Demand_union_t<Type_info::Demand_union_t<
    typename Base1::__Kobject_typeid::Demand,
    typename Base2::__Kobject_typeid::Demand>,
    typename Base3::__Kobject_typeid::Demand>
  )

public:
  // Provide non-ambiguous conversion to Kobject
  operator Kobject const & () const
  { return *static_cast<Base1 const *>(this); }

  // Provide non-ambiguous access of dec_refcnt()
  l4_msgtag_t dec_refcnt(l4_mword_t diff, l4_utcb_t *utcb = l4_utcb())
  { return Base1::dec_refcnt(diff, utcb); }
};


template< typename Derived, typename Base1, typename Base2, typename Base3,
          long PROTO, typename S_DEMAND >
Type_info const *const
Kobject_3t<Derived, Base1, Base2, Base3, PROTO, S_DEMAND>::__Kobject_typeid::_b[] =
{
  &Base1::__Kobject_typeid::_m,
  &Base2::__Kobject_typeid::_m,
  &Base3::__Kobject_typeid::_m
};

/**
 * \internal
 * \ingroup l4_kobject_rtti
 */
template< typename Derived, typename Base1, typename Base2, typename Base3,
          long PROTO, typename S_DEMAND >
L4____GEN_TI(Kobject_3t<Derived, Base1, Base2, Base3, PROTO, S_DEMAND>);

}

#if __cplusplus >= 201103L

namespace L4 {

/**
 * \headerfile l4/sys/capability
 * Get the combined server-side resource requirements for all type T...
 * \tparam T   List of IPC interface types for which the combined server-side
 *             resource requirements shall be calculated.
 */
template< typename ...T >
struct Kobject_demand;

template<>
struct Kobject_demand<> : Type_info::Demand_t<> {};

template<typename T>
struct Kobject_demand<T> : Kobject_typeid<T>::Demand {};

template<typename T1, typename ...T2>
struct Kobject_demand<T1, T2...> :
  Type_info::Demand_union_t<typename Kobject_typeid<T1>::Demand,
                            Kobject_demand<T2...> >
{};

namespace Typeid_xx {

  template<typename ...LISTS>
  struct Merge_list;

  template<typename L>
  struct Merge_list<L> : L {};

  template<typename L1, typename L2>
  struct Merge_list<L1, L2> : Typeid::Merge_list<L1, L2> {};

  template<typename L1, typename L2, typename ...LISTS>
  struct Merge_list<L1, L2, LISTS...> :
    Merge_list<typename Typeid::Merge_list<L1, L2>::type, LISTS...> {};

  template< typename I, typename ...LIST >
  struct Iface_conflict;

  template< typename I >
  struct Iface_conflict<I> : Typeid::False {};

  template< typename I, typename L, typename ...LIST >
  struct Iface_conflict<I, L, LIST...> :
    Typeid::Bool<Typeid::Iface_conflict<typename I::type, typename L::type>::value
                 || Iface_conflict<I, LIST...>::value>
  {};

  template< typename ...LIST >
  struct Conflict;

  template< typename L >
  struct Conflict<L> : Typeid::False {};

  template< typename L1, typename L2, typename ...LIST >
  struct Conflict<L1, L2, LIST...> :
    Typeid::Bool<Typeid::Conflict<typename L1::type, typename L2::type>::value
                 || Conflict<L1, LIST...>::value
                 || Conflict<L2, LIST...>::value>
  {};

  template< typename T >
  struct Is_demand
  {
    static long test(Type_info::Demand const *);
    static char test(...);
    enum { value = sizeof(test((T*)0)) == sizeof(long) };
  };

  template< typename T, typename ... >
  struct First : T { typedef T type; };
} // Typeid

/**
 * \internal
 * Internal base class for Kobject inheritance, users must use Kobject_x
 * in their software.
 */
template< typename Derived, long PROTO, typename S_DEMAND, typename ...BASES>
struct __Kobject_base : BASES...
{
protected:
  typedef Derived Class;
  typedef Typeid::Iface<PROTO, Derived> __Iface;
  typedef Typeid_xx::Merge_list<
    Typeid::Iface_list<__Iface>,
    typename BASES::__Iface_list...
  > __Iface_list;

  static void __check_protocols__()
  {
    typedef Typeid_xx::Iface_conflict<__Iface, typename BASES::__Iface_list...> Conflict;
    static_assert(!Conflict::value, "ambiguous protocol ID, protocol also used in base class");

    typedef Typeid_xx::Conflict<typename BASES::__Iface_list...> Base_conflict;
    static_assert(!Base_conflict::value, "ambiguous protocol IDs in base classes");
  }

  // disambiguate cap()
  l4_cap_idx_t cap() const throw()
  { return Typeid_xx::First<BASES...>::type::cap(); }

  L4::Cap<Class> c() const { return L4::Cap<Class>(this->cap()); }

  L4____GEN_TI_MEMBERS(Kobject_demand<BASES...>)

private:
  // This function returns the first base class (used below)
  template<typename B1, typename ...> struct Base1 { typedef B1 type; };

public:
  // Provide non-ambiguous conversion to Kobject
  operator Kobject const & () const
  { return *static_cast<typename Base1<BASES...>::type const *>(this); }

  // Provide non-ambiguous access of dec_refcnt()
  l4_msgtag_t dec_refcnt(l4_mword_t diff, l4_utcb_t *utcb = l4_utcb())
  { return Base1<BASES...>::type::dec_refcnt(diff, utcb); }
};

template< typename Derived, long PROTO, typename S_DEMAND, typename ...BASES>
Type_info const *const
__Kobject_base<Derived, PROTO, S_DEMAND, BASES...>::__Kobject_typeid::_b[] =
{
  (&BASES::__Kobject_typeid::_m)...
};

template< typename Derived, long PROTO, typename S_DEMAND, typename ...BASES>
L4____GEN_TI(__Kobject_base<Derived, PROTO, S_DEMAND, BASES...>);


// Test if the there is a Demand argument to Kobject_x
template< typename Derived, long PROTO, bool HAS_DEMAND, typename DEMAND, typename ...ARGS >
struct __Kobject_x_proto;

// YES: pass it to __Kobject_base
template< typename Derived, long PROTO, typename DEMAND, typename ...BASES>
struct __Kobject_x_proto<Derived, PROTO, true, DEMAND, BASES...> :
  __Kobject_base<Derived, PROTO, DEMAND, BASES...> {};

// NO: pass it empty Type_info::Demand_t
template< typename Derived, long PROTO, typename B1, typename ...BASES>
struct __Kobject_x_proto<Derived, PROTO, false, B1, BASES...> :
  __Kobject_base<Derived, PROTO, Type_info::Demand_t<>, B1, BASES...> {};

/**
 * Data type for defining protocol numbers
 * \tparam P  The protocol number itself
 *
 * This type must be used when specifying a protocol number with
 * Kobject_x.
 */
template< long P = PROTO_EMPTY >
struct Proto_t {};

/**
 * Generic Kobject inheritance template.
 *
 * \tparam Derived  The class name that derives from Kobject_x.
 * \tparam ARGS     An optional protocol number via L4::Proto_t, followed by
 *                  an optional server-side requirement passed as
 *                  L4::Type_info::Demand_t, followed by the list of base
 *                  classes.
 *
 * \headerfile l4/sys/capability
 * \ingroup l4_kobject_rtti
 *
 */
template< typename Derived, typename ...ARGS >
struct Kobject_x;

template< typename Derived, typename A, typename ...ARGS >
struct Kobject_x<Derived, A, ARGS...> :
  __Kobject_x_proto<Derived, PROTO_ANY, Typeid_xx::Is_demand<A>::value, A, ARGS...>
{};

template< typename Derived, long PROTO, typename A, typename ...ARGS >
struct Kobject_x<Derived, Proto_t<PROTO>, A, ARGS...> :
  __Kobject_x_proto<Derived, PROTO, Typeid_xx::Is_demand<A>::value, A, ARGS...>
{};

}
#endif

#undef L4____GEN_TI
#undef L4____GEN_TI_MEMBERS


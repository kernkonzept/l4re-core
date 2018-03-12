/* Note, automatically generated from Fiasco binary */
#pragma once

enum L4_ktrace_tbuf_entry_fixed
{
  l4_ktrace_tbuf_unused = 0,
  l4_ktrace_tbuf_pf = 1,
  l4_ktrace_tbuf_ipc = 2,
  l4_ktrace_tbuf_ipc_res = 3,
  l4_ktrace_tbuf_ipc_trace = 4,
  l4_ktrace_tbuf_ke = 5,
  l4_ktrace_tbuf_ke_reg = 6,
  l4_ktrace_tbuf_breakpoint = 7,
  l4_ktrace_tbuf_ke_bin = 8,
  l4_ktrace_tbuf_dynentries = 9,
  l4_ktrace_tbuf_max = 128,
  l4_ktrace_tbuf_hidden = 128,
};

typedef unsigned long L4_ktrace_t__Address;
typedef unsigned long L4_ktrace_t__Cap_index;
typedef void L4_ktrace_t__Context;
typedef void L4_ktrace_t__Context__Drq;
typedef unsigned L4_ktrace_t__Context__Drq_log__Type;
typedef unsigned L4_ktrace_t__Cpu_number;
typedef void L4_ktrace_t__Irq_base;
typedef void L4_ktrace_t__Irq_chip;
typedef void L4_ktrace_t__Kobject;
typedef unsigned long L4_ktrace_t__L4_error;
typedef unsigned long L4_ktrace_t__L4_msg_tag;
typedef unsigned long L4_ktrace_t__L4_obj_ref;
typedef unsigned L4_ktrace_t__L4_timeout_pair;
typedef unsigned long L4_ktrace_t__Mword;
typedef void L4_ktrace_t__Rcu_item;
typedef void L4_ktrace_t__Sched_context;
typedef long L4_ktrace_t__Smword;
typedef void L4_ktrace_t__Space;
typedef unsigned int L4_ktrace_t__Unsigned32;
typedef unsigned long long L4_ktrace_t__Unsigned64;
typedef unsigned char L4_ktrace_t__Unsigned8;
typedef void L4_ktrace_t__cxx__Type_info;

typedef struct __attribute__((packed))
{
  L4_ktrace_t__Mword _number; /* 0+4 */
  L4_ktrace_t__Address _ip; /* 4+4 */
  L4_ktrace_t__Unsigned64 _tsc; /* 8+8 */
  L4_ktrace_t__Context *_ctx; /* 16+4 */
  L4_ktrace_t__Unsigned32 _pmc1; /* 20+4 */
  L4_ktrace_t__Unsigned32 _pmc2; /* 24+4 */
  L4_ktrace_t__Unsigned32 _kclock; /* 28+4 */
  L4_ktrace_t__Unsigned8 _type; /* 32+1 */
  L4_ktrace_t__Unsigned8 _cpu; /* 33+1 */
  union __attribute__((__packed__))
  {
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      void *func; /* 36+4 */
      L4_ktrace_t__Context *thread; /* 40+4 */
      L4_ktrace_t__Context__Drq *rq; /* 44+4 */
      L4_ktrace_t__Cpu_number target_cpu; /* 48+4 */
      L4_ktrace_t__Context__Drq_log__Type type; /* 52+4 */
      char wait; /* 56+1 */
    } drq; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword state; /* 36+4 */
      L4_ktrace_t__Mword ip; /* 40+4 */
      L4_ktrace_t__Mword sp; /* 44+4 */
      L4_ktrace_t__Mword space; /* 48+4 */
      L4_ktrace_t__Mword err; /* 52+4 */
      unsigned char type; /* 56+1 */
      unsigned char trap; /* 57+1 */
    } vcpu; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Smword op; /* 36+4 */
      L4_ktrace_t__Cap_index buffer; /* 40+4 */
      L4_ktrace_t__Mword id; /* 44+4 */
      L4_ktrace_t__Mword ram; /* 48+4 */
      L4_ktrace_t__Mword newo; /* 52+4 */
    } factory; /* 56 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword gate_dbg_id; /* 36+4 */
      L4_ktrace_t__Mword thread_dbg_id; /* 40+4 */
      L4_ktrace_t__Mword label; /* 44+4 */
    } gate; /* 48 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Irq_base *obj; /* 36+4 */
      L4_ktrace_t__Irq_chip *chip; /* 40+4 */
      L4_ktrace_t__Mword pin; /* 44+4 */
    } irq; /* 48 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Kobject *obj; /* 36+4 */
      L4_ktrace_t__Mword id; /* 40+4 */
      L4_ktrace_t__cxx__Type_info *type; /* 44+4 */
      L4_ktrace_t__Mword ram; /* 48+4 */
    } destroy; /* 56 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Cpu_number cpu; /* 36+4 */
      L4_ktrace_t__Rcu_item *item; /* 40+4 */
      void *cb; /* 44+4 */
      unsigned char event; /* 48+1 */
    } rcu; /* 56 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword id; /* 36+4 */
      L4_ktrace_t__Mword mask; /* 40+4 */
      L4_ktrace_t__Mword fpage; /* 44+4 */
    } tunmap; /* 48 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Address _address; /* 36+4 */
      int _len; /* 40+4 */
      L4_ktrace_t__Mword _value; /* 44+4 */
      int _mode; /* 48+4 */
    } bp; /* 56 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Context *dst; /* 36+4 */
      L4_ktrace_t__Context *dst_orig; /* 40+4 */
      L4_ktrace_t__Address kernel_ip; /* 44+4 */
      L4_ktrace_t__Mword lock_cnt; /* 48+4 */
      L4_ktrace_t__Space *from_space; /* 52+4 */
      L4_ktrace_t__Sched_context *from_sched; /* 56+4 */
      L4_ktrace_t__Mword from_prio; /* 60+4 */
    } context_switch; /* 64 */
    struct __attribute__((__packed__))
    {
    } generic; /* 40 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__L4_msg_tag _tag; /* 36+4 */
      unsigned long _dword[2]; /* 40+8 */
      L4_ktrace_t__L4_obj_ref _dst; /* 48+4 */
      L4_ktrace_t__Mword _dbg_id; /* 52+4 */
      L4_ktrace_t__Mword _label; /* 56+4 */
      L4_ktrace_t__L4_timeout_pair _timeout; /* 60+4 */
    } ipc; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__L4_msg_tag _tag; /* 36+4 */
      unsigned long _dword[2]; /* 40+8 */
      L4_ktrace_t__L4_error _result; /* 48+4 */
      L4_ktrace_t__Mword _from; /* 52+4 */
      L4_ktrace_t__Mword _pair_event; /* 56+4 */
      L4_ktrace_t__Unsigned8 _have_snd; /* 60+1 */
      L4_ktrace_t__Unsigned8 _is_np; /* 61+1 */
    } ipc_res; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[6];
      L4_ktrace_t__Unsigned64 _snd_tsc; /* 40+8 */
      L4_ktrace_t__L4_msg_tag _result; /* 48+4 */
      L4_ktrace_t__L4_obj_ref _snd_dst; /* 52+4 */
      L4_ktrace_t__Mword _rcv_dst; /* 56+4 */
      L4_ktrace_t__Unsigned8 _snd_desc; /* 60+1 */
      L4_ktrace_t__Unsigned8 _rcv_desc; /* 61+1 */
    } ipc_trace; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      union __attribute__((__packed__)) {
        char msg[24]; /* 0+24 */
        struct __attribute__((__packed__)) {
          char tag[2]; /* 0+2 */
          char __pad_1[2];
          char *ptr; /* 4+4 */
        } mptr; /* 0+8 */
      } _msg; /* 36+24 */
    } ke; /* 64 */
    struct __attribute__((__packed__))
    {
      char _msg[24]; /* 34+24 */
    } ke_bin; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      unsigned long v[3]; /* 36+12 */
      union __attribute__((__packed__)) {
        char msg[12]; /* 0+12 */
        struct __attribute__((__packed__)) {
          char tag[2]; /* 0+2 */
          char __pad_1[2];
          char *ptr; /* 4+4 */
        } mptr; /* 0+8 */
      } _msg; /* 48+12 */
    } ke_reg; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Address _pfa; /* 36+4 */
      L4_ktrace_t__Mword _error; /* 40+4 */
      L4_ktrace_t__Space *_space; /* 44+4 */
    } pf; /* 48 */
    struct __attribute__((__packed__))
    {
      unsigned short mode; /* 34+2 */
      L4_ktrace_t__Context *owner; /* 36+4 */
      unsigned short id; /* 40+2 */
      unsigned short prio; /* 42+2 */
      long left; /* 44+4 */
      unsigned long quantum; /* 48+4 */
    } sched; /* 56 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Unsigned32 _error; /* 36+4 */
      L4_ktrace_t__Mword _cpsr; /* 40+4 */
      L4_ktrace_t__Mword _sp; /* 44+4 */
    } trap; /* 48 */
    struct __attribute__((__packed__))
    {
      char _padding[24]; /* 34+24 */
      char __post_pad[6]; /* 58+6 */
    } fullsize; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Cap_index cap_idx; /* 36+4 */
    } invexchdl; /* 40 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword pfa; /* 36+4 */
      L4_ktrace_t__Cap_index cap_idx; /* 40+4 */
      L4_ktrace_t__Mword err; /* 44+4 */
    } invpfhdl; /* 48 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword id; /* 36+4 */
      L4_ktrace_t__Mword ip; /* 40+4 */
      L4_ktrace_t__Mword sp; /* 44+4 */
      L4_ktrace_t__Mword op; /* 48+4 */
    } exregs; /* 56 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword state; /* 36+4 */
      L4_ktrace_t__Address user_ip; /* 40+4 */
      L4_ktrace_t__Cpu_number src_cpu; /* 44+4 */
      L4_ktrace_t__Cpu_number target_cpu; /* 48+4 */
    } migration; /* 56 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Irq_base *obj; /* 36+4 */
      L4_ktrace_t__Address user_ip; /* 40+4 */
    } timer; /* 48 */
  } m;
} l4_tracebuffer_entry_t;

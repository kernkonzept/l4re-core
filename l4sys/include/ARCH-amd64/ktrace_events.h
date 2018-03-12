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
typedef unsigned short L4_ktrace_t__Unsigned16;
typedef unsigned int L4_ktrace_t__Unsigned32;
typedef unsigned long long L4_ktrace_t__Unsigned64;
typedef unsigned char L4_ktrace_t__Unsigned8;
typedef void L4_ktrace_t__cxx__Type_info;

typedef struct __attribute__((packed))
{
  L4_ktrace_t__Mword _number; /* 0+8 */
  L4_ktrace_t__Address _ip; /* 8+8 */
  L4_ktrace_t__Unsigned64 _tsc; /* 16+8 */
  L4_ktrace_t__Context *_ctx; /* 24+8 */
  L4_ktrace_t__Unsigned32 _pmc1; /* 32+4 */
  L4_ktrace_t__Unsigned32 _pmc2; /* 36+4 */
  L4_ktrace_t__Unsigned32 _kclock; /* 40+4 */
  L4_ktrace_t__Unsigned8 _type; /* 44+1 */
  L4_ktrace_t__Unsigned8 _cpu; /* 45+1 */
  union __attribute__((__packed__))
  {
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      void *func; /* 48+8 */
      L4_ktrace_t__Context *thread; /* 56+8 */
      L4_ktrace_t__Context__Drq *rq; /* 64+8 */
      L4_ktrace_t__Cpu_number target_cpu; /* 72+4 */
      L4_ktrace_t__Context__Drq_log__Type type; /* 76+4 */
      char wait; /* 80+1 */
    } drq; /* 88 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword state; /* 48+8 */
      L4_ktrace_t__Mword ip; /* 56+8 */
      L4_ktrace_t__Mword sp; /* 64+8 */
      L4_ktrace_t__Mword space; /* 72+8 */
      L4_ktrace_t__Mword err; /* 80+8 */
      unsigned char type; /* 88+1 */
      unsigned char trap; /* 89+1 */
    } vcpu; /* 96 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Smword op; /* 48+8 */
      L4_ktrace_t__Cap_index buffer; /* 56+8 */
      L4_ktrace_t__Mword id; /* 64+8 */
      L4_ktrace_t__Mword ram; /* 72+8 */
      L4_ktrace_t__Mword newo; /* 80+8 */
    } factory; /* 88 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword gate_dbg_id; /* 48+8 */
      L4_ktrace_t__Mword thread_dbg_id; /* 56+8 */
      L4_ktrace_t__Mword label; /* 64+8 */
    } gate; /* 72 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Irq_base *obj; /* 48+8 */
      L4_ktrace_t__Irq_chip *chip; /* 56+8 */
      L4_ktrace_t__Mword pin; /* 64+8 */
    } irq; /* 72 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Kobject *obj; /* 48+8 */
      L4_ktrace_t__Mword id; /* 56+8 */
      L4_ktrace_t__cxx__Type_info *type; /* 64+8 */
      L4_ktrace_t__Mword ram; /* 72+8 */
    } destroy; /* 80 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Cpu_number cpu; /* 48+4 */
      char __pad_1[4];
      L4_ktrace_t__Rcu_item *item; /* 56+8 */
      void *cb; /* 64+8 */
      unsigned char event; /* 72+1 */
    } rcu; /* 80 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword id; /* 48+8 */
      L4_ktrace_t__Mword mask; /* 56+8 */
      L4_ktrace_t__Mword fpage; /* 64+8 */
    } tunmap; /* 72 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Address _address; /* 48+8 */
      int _len; /* 56+4 */
      char __pad_1[4];
      L4_ktrace_t__Mword _value; /* 64+8 */
      int _mode; /* 72+4 */
    } bp; /* 80 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Context *dst; /* 48+8 */
      L4_ktrace_t__Context *dst_orig; /* 56+8 */
      L4_ktrace_t__Address kernel_ip; /* 64+8 */
      L4_ktrace_t__Mword lock_cnt; /* 72+8 */
      L4_ktrace_t__Space *from_space; /* 80+8 */
      L4_ktrace_t__Sched_context *from_sched; /* 88+8 */
      L4_ktrace_t__Mword from_prio; /* 96+8 */
    } context_switch; /* 104 */
    struct __attribute__((__packed__))
    {
    } generic; /* 48 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__L4_msg_tag _tag; /* 48+8 */
      unsigned long _dword[2]; /* 56+16 */
      L4_ktrace_t__L4_obj_ref _dst; /* 72+8 */
      L4_ktrace_t__Mword _dbg_id; /* 80+8 */
      L4_ktrace_t__Mword _label; /* 88+8 */
      L4_ktrace_t__L4_timeout_pair _timeout; /* 96+4 */
    } ipc; /* 104 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__L4_msg_tag _tag; /* 48+8 */
      unsigned long _dword[2]; /* 56+16 */
      L4_ktrace_t__L4_error _result; /* 72+8 */
      L4_ktrace_t__Mword _from; /* 80+8 */
      L4_ktrace_t__Mword _pair_event; /* 88+8 */
      L4_ktrace_t__Unsigned8 _have_snd; /* 96+1 */
      L4_ktrace_t__Unsigned8 _is_np; /* 97+1 */
    } ipc_res; /* 104 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Unsigned64 _snd_tsc; /* 48+8 */
      L4_ktrace_t__L4_msg_tag _result; /* 56+8 */
      L4_ktrace_t__L4_obj_ref _snd_dst; /* 64+8 */
      L4_ktrace_t__Mword _rcv_dst; /* 72+8 */
      L4_ktrace_t__Unsigned8 _snd_desc; /* 80+1 */
      L4_ktrace_t__Unsigned8 _rcv_desc; /* 81+1 */
    } ipc_trace; /* 88 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      union __attribute__((__packed__)) {
        char msg[80]; /* 0+80 */
        struct __attribute__((__packed__)) {
          char tag[2]; /* 0+2 */
          char __pad_1[6];
          char *ptr; /* 8+8 */
        } mptr; /* 0+16 */
      } _msg; /* 48+80 */
    } ke; /* 128 */
    struct __attribute__((__packed__))
    {
      char _msg[80]; /* 46+80 */
    } ke_bin; /* 128 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      unsigned long v[3]; /* 48+24 */
      union __attribute__((__packed__)) {
        char msg[56]; /* 0+56 */
        struct __attribute__((__packed__)) {
          char tag[2]; /* 0+2 */
          char __pad_1[6];
          char *ptr; /* 8+8 */
        } mptr; /* 0+16 */
      } _msg; /* 72+56 */
    } ke_reg; /* 128 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Address _pfa; /* 48+8 */
      L4_ktrace_t__Mword _error; /* 56+8 */
      L4_ktrace_t__Space *_space; /* 64+8 */
    } pf; /* 72 */
    struct __attribute__((__packed__))
    {
      unsigned short mode; /* 46+2 */
      L4_ktrace_t__Context *owner; /* 48+8 */
      unsigned short id; /* 56+2 */
      unsigned short prio; /* 58+2 */
      long left; /* 60+8 */
      unsigned long quantum; /* 68+8 */
    } sched; /* 80 */
    struct __attribute__((__packed__))
    {
      char _trapno; /* 46+1 */
      L4_ktrace_t__Unsigned16 _error; /* 47+2 */
      L4_ktrace_t__Mword _rbp; /* 49+8 */
      L4_ktrace_t__Mword _cr2; /* 57+8 */
      L4_ktrace_t__Mword _rax; /* 65+8 */
      L4_ktrace_t__Mword _rflags; /* 73+8 */
      L4_ktrace_t__Mword _rsp; /* 81+8 */
      L4_ktrace_t__Unsigned16 _cs; /* 89+2 */
      L4_ktrace_t__Unsigned16 _ds; /* 91+2 */
    } trap; /* 96 */
    struct __attribute__((__packed__))
    {
      char _padding[80]; /* 46+80 */
      char __post_pad[2]; /* 126+2 */
    } fullsize; /* 128 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Cap_index cap_idx; /* 48+8 */
    } invexchdl; /* 56 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword pfa; /* 48+8 */
      L4_ktrace_t__Cap_index cap_idx; /* 56+8 */
      L4_ktrace_t__Mword err; /* 64+8 */
    } invpfhdl; /* 72 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword id; /* 48+8 */
      L4_ktrace_t__Mword ip; /* 56+8 */
      L4_ktrace_t__Mword sp; /* 64+8 */
      L4_ktrace_t__Mword op; /* 72+8 */
    } exregs; /* 80 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword state; /* 48+8 */
      L4_ktrace_t__Address user_ip; /* 56+8 */
      L4_ktrace_t__Cpu_number src_cpu; /* 64+4 */
      L4_ktrace_t__Cpu_number target_cpu; /* 68+4 */
    } migration; /* 72 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Irq_base *obj; /* 48+8 */
      L4_ktrace_t__Address user_ip; /* 56+8 */
    } timer; /* 64 */
    struct __attribute__((__packed__))
    {
      char __pre_pad[2];
      L4_ktrace_t__Mword exitcode; /* 48+8 */
      L4_ktrace_t__Mword exitinfo1; /* 56+8 */
      L4_ktrace_t__Mword exitinfo2; /* 64+8 */
      L4_ktrace_t__Mword rip; /* 72+8 */
    } svm; /* 80 */
  } m;
} l4_tracebuffer_entry_t;

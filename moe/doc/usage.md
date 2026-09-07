# Moe {#l4re_servers_moe}

[comment]: # (This is a generated file. Do not change it.)
[comment]: # (Instead, change capdb.yml.)


## Description {#l4re_servers_moe_description}

Moe is the default root-task implementation for L4Re-based systems.

*Moe* is the first task which is usually started in L4Re-based systems. The
microkernel starts *Moe* as the Root-Task.


## Moe objects {#l4re_moe_objects}

Moe provides a default implementation for the basic L4Re abstractions, such as
data spaces (L4Re::Dataspace), region maps (L4Re::Rm), memory allocators
(L4::Factory, L4Re::Mem_alloc), name spaces (L4Re::Namespace) and so on (see
[L4Re Interface](#api_l4re)). These are described in the following subsections.

### Factory {#l4re_moe_factory}

The factory in Moe is responsible for all kinds of dynamic object allocation.

Moe's factory allows allocation of the following objects:
- L4Re::Namespace
- L4Re::Dataspace, RAM allocation
- L4Re::Dma_space, memory management for DMA-capable devices
- L4Re::Rm, virtual memory management for application tasks
- L4::Vcon (output only)
- L4::Scheduler, to provide a restricted priority / CPU range for clients
- L4::Factory, to provide a quota limited allocation for clients

> [!note]
> Creating L4::Scheduler objects requires the `s` permission of the factory (see
> [Factory configuration](#l4re_moe_factory_config)). By default only the
> factory that Moe provides to the initial application holds this permission.

#### Passing parameters to the create stream {#l4re_moe_memory_alloc_factory}

L4::Factory.create() returns a [create stream](@ref L4::Factory::S) that allows
arguments to be forwarded to the object creation in Moe.

#### Factory configuration {#l4re_moe_factory_config}

Every factory in Moe carries a configuration that determines the physical memory
the factory may hand out and the operations it is allowed to perform. The
configuration of the root factory, which Moe provides to the init process, is
set with the [`--root-factory`](#l4re_servers_moe_cmdline_options) command line
option. The configuration of a sub-factory is provided when [creating the
factory](#l4re_servers_moe_param_factory).

A configuration consists of a list of physical memory regions and a set of
permissions. It is described by the following arguments:

- `untyped=<size>@<base>`, or short `ut=<size>@<base>`

  Adds an untyped memory region. Untyped memory backs ordinary dataspace
  allocations as well as Moe's internal allocations. The root factory
  configuration must contain at least one untyped region.

- `typed=<name>=<size>@<base>`, or short `t=<name>=<size>@<base>`

  Adds a typed memory region named `<name>`. Typed memory is never used for
  ordinary allocations. It is handed out only to clients that request it
  explicitly by passing `type=<name>` when [creating a
  dataspace](#l4re_servers_moe_param_dataspace). The name must consist of
  alphanumeric characters only and must not be longer than 23 characters.
  Multiple regions may carry the same name, in which case they form a single
  pool.

- `allow=<permissions>`, or short `a=<permissions>`

  Grants permissions to the factory. `<permissions>` is a string built from the
  following characters:
    * `p`: The factory may create sub-factories that specify their own memory
    regions.
    * `d`: Reserved for DMA constraints. Currently without effect.
    * `s`: The factory may create L4::Scheduler proxy objects.

  This argument must be given at most once.

`<size>` and `<base>` are unsigned numbers, written either in decimal, in
hexadecimal with a `0x` prefix or in octal with a leading `0`. Both may carry a
single `K`, `M` or `G` suffix, which multiplies the value by 1024, 1024^2 or
1024^3 respectively.

`<size>` must not be zero and a region must not extend beyond the end of the
physical address space. Regions carrying the same name must neither overlap nor
exceed the number of 15 regions per configuration. Adjacent regions of the same
name are merged.

The configuration of a sub-factory must be covered by the configuration of the
factory that creates it:
- Specifying any memory region requires the `p` permission in the creating
factory.
- Each specified region must be contained in a region of the creating factory.
- Only permissions held by the creating factory can be granted.

If a sub-factory does not specify any memory region, it inherits the untyped
regions of the creating factory. Typed regions and permissions are never
inherited implicitly.

### Namespace
Moe provides a name space conforming to the L4Re::Namespace interface (see
[Namespace](#api_l4re_namespace). Per default Moe creates a single name space
for the [Boot FS](#l4re_moe_bootfs). That is available as `rom` in the initial
objects of the init process.

#### Boot FS {#l4re_moe_bootfs}

The Boot FS subsystem provides access to the files loaded during the platform
boot (or available in ROM). These files are either linked into the boot image or
loaded via a flexible boot loader, such as GRUB.

The subsystem provides an L4Re::Namespace object as directory and an
L4Re::Dataspace object for each file.

By default all files are read only and visible in the namespace *`rom`*. As an
option, files can be supplied with the argument `:rw` to mark them as writable
modules. Moe will allow read and write access to these dataspaces and make them
visible in a different namespace called *`rwfs`*.

An example entry in 'modules.list' would look like this:

```
module somemodule :rw
```

> [!note]
> In order for a client to receive write permissions to the dataspace, the
> corresponding cap also needs write permissions.


<hr>
## Capabilities {#l4re_servers_moe_capabilities}

* `svr`

  Server Capability of application. Endpoint for IPC calls

  Mandatory capability.


<hr>
## Command Line Options {#l4re_servers_moe_cmdline_options}

* `--debug=<flags>`

  This option enables debug messages from Moe itself.

  Multiple Flags in one string separated through '|', '+' or ','.

  Possible values for `<flags>` are
    * info
    * warn
    * boot
    * server
    * exceptions
    * loader
    * ns
    * all

* `--init=<init_process>`

  This options allows to set the init process binary.

  String value.

  Default: `rom/ned`

* `--root-factory=<config>`

  This option provides the configuration of the root factory, that is the
  physical memory regions the factory may allocate from and the permissions it
  holds. The value is a comma separated list of configuration arguments, see
  [Factory configuration](#l4re_moe_factory_config).

  At least one untyped memory region must be specified. Note that no permission
  is granted implicitly, in particular `allow=s` is required to let the init
  process create L4::Scheduler proxy objects.

  The option must be given at most once and is mutually exclusive with the
  `--brk` option.

  Example: `--root-factory=ut=256M@0x40000000,t=vram=16M@0x80000000,a=ps`

  If the option is not given, the root factory may allocate from the whole
  physical address space and holds the `s` permission.

  String value.

* `--l4re-dbg=<flags>`

  This option allows to set the debug options for the L4Re runtime environment
  of the init process.

  Multiple Flags in one string separated through '|', '+' or ','.

  Possible values for `<flags>` are
    * info
    * warn
    * boot
    * server
    * exceptions
    * loader
    * ns
    * all

* `--ldr-flags=<flags>`

  This option allows setting some loader options for the L4Re runtime
  environment.

  Multiple Flags in one string separated through '|', '+' or ','.

  Possible values for `<flags>` are
    * pre_alloc
    * all_segs_cow
    * pinned_segs

* `--brk=<address>`

  This option is only present on systems without MMU. It restricts dynamic
  allocations to addresses equal or higher than `<address>`. Use it to prevent
  moe from allocating memory in regions that shall later be used by other
  applications or virtual machines.

  This option is obsolete and only kept for backwards compatibility. Use the
  `--root-factory` option instead, which is mutually exclusive with this option.

  Hexadecimal number without '0x' prefix.

* `-- <init_options>`

  All command-line parameters after the special `--` option are passed directly
  to the init process.

## Namespace {#l4re_servers_moe_param_namespace}

Call:   `create(L4.Proto.Namespace)`





## Dataspace {#l4re_servers_moe_param_dataspace}

Dataspaces can be allocated with an arbitrary size. The granularity for memory
allocation however is the machine page size (#L4_PAGESIZE). A dataspace user
must be aware that, as a result of this page-size granularity, there may be
padding memory at the end of a dataspace which is accessible to each client. Moe
currently allows most dataspace operations on this padding area. Nonetheless,
the client must not make any assumptions about the size or content of the
padding area, as this behaviour might change in the future.

The provided data spaces can have different characteristics:
- Physically contiguous and pre-allocated
- Non contiguous and on-demand allocated with possible copy on write (COW)

Dataspaces allocated via the Moe's factory allow mappings with any combination
of the read-write-execute (RWX) rights, subject to a possible restriction of the
writable right for client capabilities lacking the 'W' right.

The physical memory backing a dataspace is taken from the memory regions of the
factory the dataspace is created by, see [Factory
configuration](#l4re_moe_factory_config). By default the untyped regions of the
factory are used. The optional `type` argument selects the typed regions of the
given name instead.

Allocations can be further constrained to a range of physical addresses, as
required by devices with a limited DMA addressing capability. Use
`L4Re::Mem_alloc::Dma_mask()` to construct the `flags` value that restricts the
allocation to physical addresses that fit into the given number of bits.
Together with the `L4Re::Mem_alloc::Invert_dma_mask` flag the allocation is
restricted to the addresses above that limit instead. Both require the
`Continuous` flag to be set. A DMA mask of less than
#L4_PAGESHIFT bits is rejected with -L4_EINVAL.

Call:   `create(L4.Proto.Dataspace, size [, flags, align, base, type])`

* `size`

  Size in bytes.

  Numerical value.

* `flags`

  Special dataspace properties

  Bitmap of length 4 with the single bits holding the following meaning:
    * 0x01: Continuous Allocate physically contiguous memory.
    * 0x02: Pinned Deprecated, use L4Re::Dma_space instead.
    * 0x04: Super pages Allocate super pages.
    * 0x08: Fixed physical Address Allocate at fixed physical address. Only
    honored on no-MMU systems. Will fail on MMU systems.



* `align`

  Log2 alignment of dataspace if supported by allocator

  Numerical value.

* `base`

  Physical base address of the dataspace. Only consumed if the `Fixed physical
  Address` flag is set. The requested address range must be covered by the
  memory regions of the factory.

  Numerical value.

* `type`

  Allocate from typed memory. The argument is the string `type=<name>`, where
  `<name>` refers to the typed memory regions of the factory. Without this
  argument the dataspace is allocated from untyped memory.

  String value.



## DMA space {#l4re_servers_moe_param_dma_space}

Call:   `create(L4.Proto.Dma_space)`





## Region Map {#l4re_servers_moe_param_region_map}

Call:   `create(L4.Proto.Rm)`





## Virtual console {#l4re_servers_moe_param_virtual_console}

The logging facility of Moe provides per application tagged and synchronized log
output.

Call:   `create(L4.Proto.Log [, label, "color=(string|int)"])`

* `label`

  Label used as prefix for the console output.

  String value.

* `"color=(string|int)"`

  Color of client's output

  The Value for this parameter can be one of the following:

  * `string`

    Define the color by the first character of the given string

    Possible values are
      * `n`: gray
      * `r`: red
      * `g`: green
      * `y`: yellow
      * `b`: blue
      * `m`: magenta
      * `c`: cyan
      * `w`: white
      * `N`: black
      * `R`: light red
      * `G`: light green
      * `Y`: light yellow
      * `B`: light blue
      * `M`: light magenta
      * `C`: light cyan
      * `W`: bright white

  * `int`

    Define the color by integer.

    Possible values are
      * `0`: gray
      * `1`: red
      * `2`: green
      * `3`: yellow
      * `4`: blue
      * `5`: magenta
      * `6`: cyan
      * `7`: white
      * `8`: black
      * `9`: light red
      * `10`: light green
      * `11`: light yellow
      * `12`: light blue
      * `13`: light magenta
      * `14`: light cyan
      * `15`: bright white

  Default: `white`



## Scheduler {#l4re_servers_moe_param_scheduler}

The scheduler subsystem provides a simple scheduler proxy for scheduling policy
enforcement.

The priority offset provided on the creation of a scheduler proxy defines the
minimum priority assigned to threads which are scheduled by that instance of the
scheduler proxy. The offset is implicitly added to priorities provided to
L4::Scheduler.run_thread().

Call:   `create(L4.Proto.Scheduler, limit, offset [, bitmap])`

* `limit`

  Maximum priority.

  Numerical value.

* `offset`

  Priority offset.

  Numerical value.

* `bitmap`

  Bitmap of CPUs - can be repeated to address higher order CPUs

  Numerical value.



## Factory {#l4re_servers_moe_param_factory}

Creates a sub-factory with a restricted quota and, optionally, a restricted
[factory configuration](#l4re_moe_factory_config).

The configuration of the sub-factory must be covered by the configuration of the
factory that creates it. Otherwise the creation fails with -L4_EPERM. Malformed
configuration arguments are rejected with -L4_EINVAL.

If no memory region is specified, the sub-factory inherits the untyped memory
regions of the creating factory. Typed memory regions and permissions are never
inherited implicitly.

Call:   `create(L4.Proto.Factory [, quota, config])`

* `quota`

  Limit in bytes. The limit is deducted from the limit of the factory that
  creates the new factory.

  Numerical value.
    * Must not be zero.

* `config`

  Factory configuration argument, either `untyped=<size>@<base>`,
  `typed=<name>=<size>@<base>` or `allow=<permissions>` - can be repeated to
  provide multiple arguments.

  String value.




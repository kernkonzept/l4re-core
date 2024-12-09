# Moe {#l4re_servers_moe}

[comment]: # (This is a generated file. Do not change it.)
[comment]: # (Instead, change capdb.yml.)

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
> L4::Scheduler objects can be only created through the user factory provided by
> Moe to the initial application. Other factory instances cannot create this
> object.

#### Passing parameters to the create stream {#l4re_moe_memory_alloc_factory}

L4::Factory.create() returns a [create stream](@ref L4::Factory::S) that allows
arguments to be forwarded to the object creation in Moe.

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


## Capabilities

* `svr`

  Server Capability of application. Endpoint for IPC calls

  Mandatory capability.


## Command Line Options

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

  Hexadecimal number without '0x' prefix.

* `-- <init_options>`

  All command-line parameters after the special `--` option are passed directly
  to the init process.

## Namespace

Call:   `create(L4.Proto.Namespace)`





## Dataspace

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

Call:   `create(L4.Proto.Dataspace, size [, flags, align])`

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



## DMA space

Call:   `create(L4.Proto.Dma_space)`





## Region Map

Call:   `create(L4.Proto.Rm)`





## Virtual console

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



## Scheduler

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



## Factory

Call:   `create(L4.Proto.Factory [, quota])`

* `quota`

  Limit in bytes. The limit is deducted from the limit of the factory that
  creates the new factory.

  Numerical value.
    * Must not be zero.




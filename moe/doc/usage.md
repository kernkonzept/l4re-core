# Moe, the Root-Task {#l4re_servers_moe}

Moe is the default root-task implementation for L4Re-based systems.

Moe is the first task which is usually started in L4Re-based systems.
The micro kernel starts Moe as the Root-Task.

## Moe objects {#l4re_moe_objects}

Moe provides a default implementation for the basic L4Re abstractions, such as
data spaces (L4Re::Dataspace), region maps (L4Re::Rm), memory allocators
(L4::Factory, L4Re::Mem_alloc), name spaces (L4Re::Namespace) and so on
(see [L4Re Interface](#api_l4re)). These are described in the
following subsections.

### Factory {#l4re_moe_factory}

The factory in Moe is responsible for all kinds of dynamic object
allocation.

Moe's factory allows allocation of the following objects:
- L4Re::Namespace
- L4Re::Dataspace, RAM allocation
- L4Re::Dma_space, memory management for DMA-capable devices
- L4Re::Rm, virtual memory management for application tasks
- L4::Vcon (output only)
- L4::Scheduler, to provide a restricted priority / CPU range for clients
- L4::Factory, to provide a quota limited allocation for clients

> [!note]
> L4::Scheduler objects can be only created through the user factory
> provided by Moe to the initial application. Other factory instances
> cannot create this object.

#### Passing parameters to the create stream {#l4re_moe_memory_alloc_factory}

L4::Factory.create() returns a [create stream](@ref L4::Factory::S) that
allows arguments to be forwarded to the object creation in Moe.

Objects that support additional parameters on their creation are presented
next with a non-empty list of parameters. The parameters are listed in the
order they should be provided to a create stream. Optional parameters are
identified by their default values. Multiple entries in the next list
denote different ways of initializing an object.

- L4Re::Namespace `()`
   - For more details see @ref l4re_moe_names
- L4Re::Dataspace `(l4_mword_t size, l4_umword_t flags = 0,
                    l4_umword_t align = 0)`
  - Argument `size`: size in bytes (mandatory)
  - Argument `flags`: special dataspace properties, see
    #L4Re::Mem_alloc::Mem_alloc_flags
  - Argument `align`: Log2 alignment of dataspace if supported by allocator
  - See detailed description of the parameters in L4Re::Mem_alloc::alloc()
  - For details on the types of dataspaces provided by Moe, see
    @ref l4re_moe_dataspace
- L4Re::Dma_space `()`
  - For more details see @ref l4re_moe_dma_space
- L4Re::Rm `()`
  - For more details see @ref l4re_moe_rm
- L4::Vcon `(char const *label, l4_mword_t color = 7)`
  - Argument `label`: label used as prefix for the console output
  - Argument `color`: color code `0..15`
  - For more details see @ref l4re_moe_log
- L4::Vcon `(char const *label, char const *color = "w")`
  - Argument `label`: label used as prefix for the console output
  - Argument `color`: color code
    - The color is identified by a single character
    - Supported colors: `N`, `n`, `R`, `r`, `G`, `g`, `Y`, `y`, `B`, `b`,
                        `M`, `m`, `C`, `c`, `W`, `w`
  - For more details see @ref l4re_moe_log
- L4::Scheduler `(l4_mword_t limit, l4_mword_t offset,
                  l4_umword_t bitmap = ~0UL, ...)`
  - Argument `limit`: maximum priority
  - Argument `offset`: priority offset
  - Argument `bitmap`: bitmap of CPUs - can be repeated to address higher
    order CPUs
  - Argument `limit` must be greater than `offset`
  - For more details see @ref l4re_moe_scheduler
- L4::Factory `(l4_mword_t quota)`
  - Argument `quota`: limit in bytes (not zero)
  - The limit is deducted from the limit of the factory that creates
    the new factory

### l4re_moe_names Namespace

Moe provides a name space conforming to the L4Re::Namespace
interface (see @ref api_l4re_namespace). Per default Moe creates a single
name space for the @ref l4re_moe_bootfs. That is available as rom
in the initial objects of the init process.

#### l4re_moe_bootfs Boot FS

The Boot FS subsystem provides access to the files loaded
during the platform boot (or available in ROM). These files are either
linked into the boot image or loaded via a flexible boot loader,
such as GRUB.

The subsystem provides an L4Re::Namespace object as directory and an
L4Re::Dataspace object for each file.

By default all files are read only and visible in the namespace
`rom`. As an option, files can be supplied with the argument `:rw` to mark
them as writable modules. Moe will allow read and write access to these
dataspaces and make them visible in a different namespace called `rwfs`.

An example entry in 'modules.list' would look like this:

~~~~~~~~~~~~~~~~~~~~~~
module somemodule :rw
~~~~~~~~~~~~~~~~~~~~~~

In order for a client to receive write permissions to the dataspace,
the corresponding cap also needs write permissions.

### l4re_moe_dataspace Dataspace

Dataspaces can be allocated with an arbitrary size. The granularity
for memory allocation however is the machine page size (#L4_PAGESIZE).
A dataspace user must be aware that, as a result of this page-size
granularity, there may be padding memory at the end of a dataspace
which is accessible to each client. Moe currently allows most dataspace
operations on this padding area. Nonetheless, the client must not make any
assumptions about the size or content of the padding area, as this
behaviour might change in the future.

The provided data spaces can have different characteristics:
- Physically contiguous and pre-allocated
- Non contiguous and on-demand allocated with possible copy on write (COW)

Dataspaces allocated via the Moe's factory allow mappings with any
combination of the read-write-execute (RWX) rights, subject to a possible
restriction of the writable right for client capabilities lacking the 'W'
right.

### l4re_moe_log Log Subsystem

The logging facility of Moe provides per application tagged and
synchronized log output.

### l4re_moe_dma_space DMA Space

### l4re_moe_scheduler Scheduler subsystem

The scheduler subsystem provides a simple scheduler proxy for scheduling
policy enforcement.

The priority offset provided on the creation of a scheduler proxy defines
the minimum priority assigned to threads which are scheduled by that
instance of the scheduler proxy. The offset is implicitly added to priorities
provided to L4::Scheduler.run_thread().

### l4re_moe_rm Region Map

## l4re_moe_options Command Line Options

Moe's command-line syntax is:

    moe [--debug=<flags>] [--init=<binary>] [--l4re-dbg=<flags>] [--ldr-flags=<flags>] [-- <init options>]

* `--debug=<debug flags>`
This option enables debug messages from Moe itself, the `<debug flags>`
values are a combination of `info`, `warn`, `boot`, `server`, `loader`,
`exceptions`, and `ns` (or `all` for full verbosity).

* `--init=<init process>`
This options allows to override the default init process binary, which is
'rom/ned'.

* `--l4re-dbg=<debug flags>`
This option allows to set the debug options for the L4Re runtime environment
of the init process. The flags are the same as for `--debug=`.

* `--ldr-flags=<loader flags>`
This option allows setting some loader options for the L4Re runtime
environment. The flags are `pre_alloc`, `all_segs_cow`,and `pinned_segs`.

* `--brk=<address>`
This option is only present on systems without MMU. It restricts dynamic
allocations to addresses equal or higher than `<address>`. The argument is
parsed as hexadecimal number without any `0x` prefix. Use it to prevent moe
from allocating memory in regions that shall later be used by other
applications or virtual machines.

* `-- <init options>`
All command-line parameters after the special `--` option are passed
directly to the init process.

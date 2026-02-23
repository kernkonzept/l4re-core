#include <l4/re/dma_space>
#include <l4/sys/cxx/ipc_client>
#include <l4/sys/cxx/consts>

L4_RPC_DEF(L4Re::Dma_space::map);
L4_RPC_DEF(L4Re::Dma_space::unmap);
L4_RPC_DEF(L4Re::Dma_space_mgr::block_area);

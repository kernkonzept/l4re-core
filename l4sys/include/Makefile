PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../../..

PKGNAME		= sys
EXTRA_TARGET	+= capability kip task factory irq icu thread vcon \
                   smart_capability scheduler meta typeinfo_svr ipc_gate \
                   __vm ARCH-x86/vm ARCH-amd64/vm ARCH-arm/vm debugger \
                   platform_control \
                   exception           \
                   kobject             \
                   pager               \
                   semaphore           \
                   iommu               \
                   cxx/ipc_array       \
                   cxx/ipc_basics      \
                   cxx/ipc_client      \
                   cxx/ipc_epiface     \
                   cxx/ipc_iface       \
                   cxx/ipc_ret_array   \
                   cxx/ipc_string      \
                   cxx/ipc_server      \
                   cxx/ipc_server_loop \
                   cxx/ipc_types       \
                   cxx/ipc_varg        \
                   cxx/ipc_legacy      \
                   cxx/types           \


include $(L4DIR)/mk/include.mk

FIASCO_CONFIG_amd64 = amd64-mp
FIASCO_CONFIG_arm   = arm-omap5
FIASCO_CONFIG_ppc32 = ppc32-1
FIASCO_CONFIG_sparc = sparc-1
FIASCO_CONFIG_x86   = ia32-big

.PHONY: update_ktrace_events.h
update_ktrace_events.h:
	$(VERBOSE)if [ -z "$(FIASCO_CHECKALL_ROOT)" ]; then \
	  echo "Need to specify FIASCO_CHECKALL_ROOT"; \
          exit 1; \
	fi
	$(foreach a,$(SYSTEMS_PLAIN),\
	  scp $(FIASCO_CHECKALL_ROOT)/build-all/$(FIASCO_CONFIG_$(a))/ktrace_events.h ARCH-$(a);)

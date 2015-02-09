#
# For a description of the syntax of this configuration file,
# see extra/config/Kconfig-language.txt
#
config TARGET_ARCH
	default "arc"

config FORCE_OPTIONS_FOR_ARCH
	bool
	default y
	select ARCH_ANY_ENDIAN

choice
	prompt "Target Processor Type"
	default CONFIG_ARC_CPU_700

config CONFIG_ARC_CPU_700
	bool "ARC700"
	select ARCH_HAS_MMU
	help
	  ARCompact ISA based ARC CPU

config CONFIG_ARC_CPU_HS
	bool "ARC-HS"
	select ARCH_HAS_MMU
	help
	  Next Generation ARCv2 ISA based Processors

endchoice

choice
	prompt "MMU Page Size"
	default CONFIG_ARC_PAGE_SIZE_8K

config CONFIG_ARC_PAGE_SIZE_8K
	bool "8KB"
	help
	  Choose between 4k, 8k (default) or 16k

config CONFIG_ARC_PAGE_SIZE_16K
	bool "16KB"

config CONFIG_ARC_PAGE_SIZE_4K
	bool "4KB"

endchoice

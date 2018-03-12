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

endchoice

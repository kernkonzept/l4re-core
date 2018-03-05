PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

MODE = static

TEST_GROUP := l4re-core/ned

REQUIRES_LIBS  := libpthread atkins
DEPENDS_PKGS   := atkins

EXTRA_TEST := ipc_cmd_direct_loop

NED_CFG_test_arg := arg.cfg
NED_CFG_test_exit_wait := exit_wait.cfg
NED_CFG_test_ipc_command := ipc_command.cfg
NED_CFG_test_namespace_composition := namespace_composition.cfg
NED_CFG_test_start := start.cfg
NED_CFG_test_rom_overwrite := rom_overwrite.cfg

NED_CFG_ipc_cmd_direct_loop := ipc_command_as_server.cfg
TEST_TARGET_ipc_cmd_direct_loop := test_ipc_command

REQUIRED_MODULES_test_ipc_command += ipc_command_x.cfg
NED_CFG_test_rw_module := rw_module.cfg
REQUIRED_MODULES_test_rw_module += test_namespace_composition:rw

include $(L4DIR)/mk/test.mk

PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

MODE = static

TEST_GROUP := l4re-core/ned

REQUIRES_LIBS  := libpthread atkins
DEPENDS_PKGS   := atkins

MOE_CFG_test_exit_wait := exit_wait.cfg
MOE_CFG_test_ipc_command := ipc_command.cfg
MOE_CFG_test_namespace_composition := namespace_composition.cfg
REQUIRED_MODULES_test_ipc_command += ipc_command_x.cfg

include $(L4DIR)/mk/test.mk

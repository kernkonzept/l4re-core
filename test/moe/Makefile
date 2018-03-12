PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

TEST_GROUP := l4re-core/moe

REQUIRES_LIBS  := libpthread atkins
DEPENDS_PKGS   := atkins

REQUIRED_MODULES  := moe_bootfs_example.txt

EXTRA_TEST := loop_moe_test_seq loop_moe_test_par

ALL_MODS := test_dataspace test_dma_space test_factory test_mem_alloc test_namespace test_region_mapper moe_bootfs_example.txt

TEST_TARGET_loop_moe_test_seq := test_bootfs
REQUIRED_MODULES_loop_moe_test_seq := $(ALL_MODS) test_exhaust
MOE_CFG_loop_moe_test_seq := loop_sequential.cfg
TEST_EXPECTED_loop_moe_test_seq := tap_test_alive.output
TEST_EXPECTED_REPEAT_loop_moe_test_seq := 0

TEST_TARGET_loop_moe_test_par := test_bootfs
REQUIRED_MODULES_loop_moe_test_par := $(ALL_MODS)
MOE_CFG_loop_moe_test_par := loop_parallel.cfg
TEST_EXPECTED_loop_moe_test_par := tap_test_alive.output
TEST_EXPECTED_REPEAT_loop_moe_test_par := 0
TEST_TIMEOUT_loop_moe_test_par := 100

# needs to map plenty of memory, which might take some time
TEST_TIMEOUT_test_exhaust := 60
TEST_TIMEOUT_test_dataspace := 60

include $(L4DIR)/mk/test.mk

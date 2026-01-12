#pragma once

// uclibc-ng specific scheduler details
#include <l4/sys/scheduler.h>
#define __need_schedparam
#include <bits/sched.h>

#pragma once

#define close_not_cancel(fd) \
        close(fd)

#define open_not_cancel_2(name, flags) \
        open(name, flags)

#define fcntl_not_cancel(fd, cmd, val) \
        fcntl(fd, cmd, val)

#define close_not_cancel_no_status(fd) \
        close(fd)

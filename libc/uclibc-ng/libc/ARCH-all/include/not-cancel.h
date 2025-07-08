#pragma once

#define close_not_cancel(fd) \
        close(fd)

#define open_not_cancel_2(name, flags) \
        open(name, flags)

#define fcntl_not_cancel(fd, cmd, val) \
        fcntl(fd, cmd, val)

#define close_not_cancel_no_status(fd) \
        close(fd)

#define read_not_cancel(fd, buf, n) \
        read(fd, buf, n)

#define write_not_cancel(fd, buf, n) \
        write(fd, buf, n)

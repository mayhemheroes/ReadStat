
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "readstat.h"
#include "readstat_io_unistd.h"

#if defined _WIN32 || defined __CYGWIN__
#define UNISTD_OPEN_OPTIONS O_RDONLY | O_BINARY
#elif defined _AIX
#define UNISTD_OPEN_OPTIONS O_RDONLY | O_LARGEFILE
#else
#define UNISTD_OPEN_OPTIONS O_RDONLY
#endif

#if defined _WIN32 || defined __CYGWIN__ || defined _AIX
#define lseek lseek64
#endif


int unistd_open_handler(void *io_ctx) {
    const char *filename = ((unistd_io_ctx_t*) io_ctx)->filename;
    int fd = open(filename, UNISTD_OPEN_OPTIONS);
    ((unistd_io_ctx_t*) io_ctx)->fd = fd;
    return fd;
}

int unistd_close_handler(void *io_ctx) {
    int fd = ((unistd_io_ctx_t*) io_ctx)->fd;
    if (fd != -1)
        return close(fd);
    else
        return 0;
}

readstat_off_t unistd_seek_handler(readstat_off_t offset,
        readstat_io_flags_t whence, void *io_ctx) {
    int flag;
    switch(whence) {
    case READSTAT_SEEK_SET:
        flag = SEEK_SET;
        break;
    case READSTAT_SEEK_CUR:
        flag = SEEK_CUR;
        break;
    case READSTAT_SEEK_END:
        flag = SEEK_END;
        break;
    }
    int fd = ((unistd_io_ctx_t*) io_ctx)->fd;
    return lseek(fd, offset, flag);
}

ssize_t unistd_read_handler(void *buf, size_t nbyte, void *io_ctx) {
    int fd = ((unistd_io_ctx_t*) io_ctx)->fd;
    ssize_t out = read(fd, buf, nbyte);
    return out;
}

readstat_error_t unistd_update_handler(long file_size, 
        readstat_progress_handler progress_handler, void *user_ctx,
        void *io_ctx) {
    if (!progress_handler)
        return READSTAT_OK;

    int fd = ((unistd_io_ctx_t*) io_ctx)->fd;
    long current_offset = lseek(fd, 0, SEEK_CUR);

    if (current_offset == -1)
        return READSTAT_ERROR_READ;

    if (progress_handler(1.0 * current_offset / file_size, user_ctx))
        return READSTAT_ERROR_USER_ABORT;

    return READSTAT_OK;
}

readstat_io_t *unistd_io_init(const char *filename) {
    unistd_io_ctx_t *io_ctx = calloc(1, sizeof(unistd_io_ctx_t));
    io_ctx->filename = filename;
    io_ctx->fd = -1;

    readstat_io_t *io = readstat_io_init();
    readstat_set_open_handler(io, unistd_open_handler);
    readstat_set_close_handler(io, unistd_close_handler);
    readstat_set_seek_handler(io, unistd_seek_handler);
    readstat_set_read_handler(io, unistd_read_handler);
    readstat_set_update_handler(io, unistd_update_handler);
    readstat_set_io_ctx(io, (void*) io_ctx);

    return io;
}

void unistd_io_free(readstat_io_t *io) {
    if (io) {
        free(io->io_ctx);
        free(io);
    }
}

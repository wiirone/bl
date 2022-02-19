#ifndef BL_BL_PORT_H
#define BL_BL_PORT_H
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stddef.h>

inline static int fd_open(const char* path) {
    return open(path, O_RDWR);
}

inline static int fd_close(int fd) {
    return close(fd);
}

inline static int fd_write(int fd, void* buffer, size_t size) {
    return write(fd, buffer, size);
}

int fd_read(int fd, void* buffer, size_t size) {
    return read(fd, buffer, size);
}

int fd_len(int fd) {
    int res;
    struct stat stat_buffer;
    res = fstat(fd, &stat_buffer);
    if (res < 0) {
        return res;
    }
    return stat_buffer.st_size;
}

inline static void bl_delay_ms(unsigned int ms) {
    usleep(ms*1000);
}



#endif //BL_BL_PORT_H

#include <stddef.h>
#include "library.h"

#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    if (argc != 3 ) return -1;
    int uart_fd = open(argv[1], O_RDWR);
    if (uart_fd < 0) {
        return -1;
    }
    int res;
    res = bl_start_upgrade(uart_fd, argv[2], false);
    if (res) {
        close(uart_fd);
        return -1;
    }
    close(uart_fd);
    return 0;
}
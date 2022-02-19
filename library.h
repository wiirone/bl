#ifndef BL_LIBRARY_H
#define BL_LIBRARY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BL_ERASE_SIZE 256
#define BL_SIZE  2048
#define BL_APP_START_ADDRESS 0x800

#define BL_ERR_NONE 0
#define BL_ERR_IO -1
#define BL_ERR_NO_RESPONSE -2
#define BL_ERR_CONFIG -3

#define BTL_GUARD               (0x5048434DU)
#define BL_INIT_CRC (0xFFFFFFFFU)

typedef enum
{
    BL_CMD_UNLOCK       = 0xa0,
    BL_CMD_DATA         = 0xa1,
    BL_CMD_VERIFY       = 0xa2,
    BL_CMD_RESET        = 0xa3,
    BL_CMD_BKSWAP_RESET = 0xa4,
} bl_cmd_t;

typedef enum
{
    BL_RESP_OK          = 0x50,
    BL_RESP_ERROR       = 0x51,
    BL_RESP_INVALID     = 0x52,
    BL_RESP_CRC_OK      = 0x53,
    BL_RESP_CRC_FAIL    = 0x54,
} bl_resp_t;

int32_t bl_start_upgrade(const int uart_fd, const char* const upgrade_file_path, const bool swap_reset);

#endif //BL_LIBRARY_H

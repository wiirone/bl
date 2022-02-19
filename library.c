#include <string.h>
#include "bl_port.h"
#include "library.h"

static void bl_crc_tab_gen(uint32_t crc_tab[BL_ERASE_SIZE]) {
    for(int i = 0; i< BL_ERASE_SIZE; ++i) {
        uint32_t value = i;
        for (int j= 0 ;j <8; ++j) {
            if (value & 1) {
                value = (value >> 1)^0xedb88320;
            } else {
                value >>= 1;
            }
        }
        crc_tab[i] = value;
    }
}

static uint32_t bl_crc32(uint32_t crc, uint32_t crc_tab[BL_ERASE_SIZE], uint8_t* const data, size_t data_size) {
    //uint32_t crc = 0xFFFFFFFFU;
    for(int i = 0; i < data_size ; ++i) {
        crc = crc_tab[ (crc^data[i]) & 0xFFU] ^ (crc >> 8);
    }
    return crc;
}

inline static void bl_uint32_swap(uint32_t u, uint8_t buffer[4]) {
    buffer[0] = (u>>0) & 0xFF;
    buffer[1] = (u>>8) & 0xFF;
    buffer[2] = (u>>16) & 0xFF;
    buffer[3] = (u>>24) & 0xFF;
}

int32_t bl_send_request(const int fd, const bl_cmd_t cmd, const size_t size, const uint8_t* const data, const size_t data_size) {

    uint8_t buffer[4];
    bl_uint32_swap(BTL_GUARD, buffer);
    if (fd_write(fd, buffer, 4) != 4) {
        return BL_ERR_IO;
    }

    bl_uint32_swap(size, buffer);
    if (fd_write(fd, buffer, 4) != 4) {
        return BL_ERR_IO;
    }

    buffer[0] = cmd;
    if (fd_write(fd, buffer, 1) != 1) {
        return BL_ERR_IO;
    }

    if (fd_write(fd, data, data_size) != data_size) {
        return BL_ERR_IO;
    }

    for(int i = 0; i < 3; ++i) {
        if (fd_read(fd ,buffer, 1) == 1) {
            return buffer[0];
        }
        bl_delay_ms(200);
    }
    return BL_ERR_NO_RESPONSE;
}

//WARNING: MCU counterpart should have pulled down the upgrade pin and informed the MCU application firmware to reset
//so that mcu can enter bootloader upgrade mode.
//Before leave this function, the upgrade pin should be in pull-up state
int32_t bl_start_upgrade(const int uart_fd, const char* const upgrade_file_path, const bool swap_reset) {
    int fd = fd_open(upgrade_file_path);
    if (fd < 0) {
        return BL_ERR_CONFIG;
    }
    size_t file_size = fd_len(fd);
    while(file_size % BL_ERASE_SIZE > 0) {
        file_size++;
    }

    //generate crc tab
    uint32_t crc_tab[BL_ERASE_SIZE];

    bl_crc_tab_gen(crc_tab);

    int res;
    uint8_t buffer[BL_ERASE_SIZE+4];
    //unlocking
    bl_uint32_swap(BL_APP_START_ADDRESS, buffer);
    bl_uint32_swap(file_size, buffer+4);

    res = bl_send_request(uart_fd, BL_CMD_UNLOCK, 8, buffer, 8);

    if (res != BL_RESP_OK) {
        fd_close(fd);
        return BL_ERR_IO;
    }

    uint32_t crc = BL_INIT_CRC;
    // create data block for each ERASE_SIZE
    for (uint32_t addr = BL_APP_START_ADDRESS; addr < (BL_APP_START_ADDRESS+ file_size); addr += BL_ERASE_SIZE) {
        res = fd_read(fd, buffer+4, BL_ERASE_SIZE);
        if (res < 0) {
            fd_close(fd);
            return BL_ERR_IO;
        }
        while(res < BL_ERASE_SIZE) {
            buffer[4+res] = 0xFF;
            res++;
        }
        bl_uint32_swap(addr, buffer);
        res = bl_send_request(uart_fd, BL_CMD_DATA, BL_ERASE_SIZE+4, buffer, BL_ERASE_SIZE+4);
        if (res != BL_RESP_OK) {
            fd_close(fd);
            return BL_ERR_IO;
        }
        crc = bl_crc32(crc,crc_tab,buffer+4, BL_ERASE_SIZE);
    }

    //Verification
    bl_uint32_swap(crc, buffer);
    res = bl_send_request(uart_fd, BL_CMD_VERIFY, 4, buffer, 4);

    if (res != BL_RESP_CRC_OK) {
        fd_close(fd);
        return BL_ERR_IO;
    }

    memset(buffer, 16, 0);

    //WARNING: if here MCU counterpart should have pulled up the upgrade pin;
    //otherwise, mcu will enter bootloader upgrade mode infinitely or can not start the application
    res = bl_send_request(uart_fd, swap_reset?BL_CMD_BKSWAP_RESET:BL_CMD_RESET, 16, buffer, 16);
    if (res != BL_RESP_OK) {
        fd_close(fd);
        return BL_ERR_IO;
    }

    fd_close(fd);
    return BL_ERR_NONE;

}
// eeprom_loadsave.c

#include "eeprom_loadsave.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "st25dv64k.h"
#include "cmsis_os.h"
#include "crc32.h"
#include "log.h"

LOG_COMPONENT_REF(EEPROM);
// public functions for load/save

int eeprom_load_bin_from_usb(const char *fn) {
    int fd;
    uint8_t buff[128];
    uint16_t size = 0x800;
    uint16_t addr = 0x000;
    int bs = sizeof(buff);
    int br;
    fd = open(fn, O_RDONLY);
    if (fd >= 0) {
        while (size) {
            if (size < bs)
                bs = size;
            br = read(fd, buff, bs);
            if (br != bs)
                break;
            st25dv64k_user_write_bytes(addr, buff, bs);
            size -= bs;
            addr += bs;
        }
        close(fd);
        // size is decreased in loop
        // 0 == successful read of all elements
        if (size == 0) {
            log_info(EEPROM, "bin %s loaded successfully", fn);
        } else {
            log_error(EEPROM, "unable to read %d bytes from bin %s", fn, size);
        }
        return (size == 0) ? 1 : 0;
    }
    log_error(EEPROM, "unable to open (rd) bin %s", fn);
    return 0;
}

int eeprom_save_bin_to_usb(const char *fn) {
    int fd;
    uint8_t buff[128];
    uint16_t size = 0x800;
    uint16_t addr = 0x000;
    int bs = sizeof(buff);
    int bw;
    fd = open(fn, O_WRONLY | O_TRUNC);
    if (fd >= 0) {
        while (size) {
            if (size < bs)
                bs = size;
            st25dv64k_user_read_bytes(addr, buff, bs);
            bw = write(fd, buff, bs);
            if (bw != bs)
                break;
            size -= bs;
            addr += bs;
        }
        close(fd);
        // size is decreased in loop
        // 0 == successful write of all elements
        if (size == 0) {
            log_info(EEPROM, "bin %s saved successfully", fn);
        } else {
            log_error(EEPROM, "unable to write %d bytes from bin %s", fn, size);
        }
    } else {
        log_error(EEPROM, "unable to open (wr) bin %s", fn);
    }
    return 0;
}
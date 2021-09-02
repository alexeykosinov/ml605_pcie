#include <stdio.h>
#include <string.h>
#include "xparameters.h"

#define XCF128

#ifdef XCF128
#include "xio.h"
#endif

#define FLASH_BASEADDR          XPAR_LINEAR_FLASH_S_AXI_MEM0_BASEADDR
#define FLASH_IMAGE_BASEADDR  	(FLASH_BASEADDR + 0x00F00000)
#define SREC_MAX_BYTES          255  /* Maximum record length */
#define SREC_DATA_MAX_BYTES     123  /* Maximum of 123 data bytes */

typedef struct srec_info_s {
    char    type;
    unsigned char*  addr;
    unsigned char*  sr_data;
    unsigned char   dlen;
} srec_info_t;

static srec_info_t srinfo;

static unsigned char load_exec();
static unsigned char flash_get_srec_line(unsigned char *buf);
static unsigned char sr_buf[SREC_MAX_BYTES];
static unsigned char sr_data_buf[SREC_DATA_MAX_BYTES];
static unsigned char *flbuf;

unsigned char  decode_srec_line(unsigned char *sr_buf, srec_info_t *info);
unsigned char  grab_hex_byte(unsigned char *buf);
unsigned short grab_hex_word(unsigned char *buf);
unsigned int   grab_hex_dword(unsigned char *buf);
unsigned int   grab_hex_word24(unsigned char *buf);


int main() {

    unsigned char ret;

#ifdef XCF128
    XIo_Out16(FLASH_BASEADDR+0x17BBE, 0x60);
    XIo_Out16(FLASH_BASEADDR+0x17BBE, 0x03);
#endif

    xil_printf("%cc\r\n", 27); // Reset terminal buffer
    xil_printf("Loading SREC image from flash @ address: %x\n", FLASH_IMAGE_BASEADDR);

    flbuf = (unsigned char*)FLASH_IMAGE_BASEADDR;
    ret = load_exec();

    return ret;
}

static unsigned char load_exec() {

    unsigned char ret;
    void (*laddr)();
    char done = 0;
    
    srinfo.sr_data = sr_data_buf;
    
    while (!done) {

        if ((ret = flash_get_srec_line(sr_buf)) != 0) return ret;
        if ((ret = decode_srec_line(sr_buf, &srinfo)) != 0) return ret;
        
        switch (srinfo.type) {
            case 0: break;
            case 1:
            case 2:
            case 3:
                memcpy ((void*)srinfo.addr, (void*)srinfo.sr_data, srinfo.dlen);
                break;
            case 5: break;
            case 7:
            case 8:
            case 9:
                laddr = (void (*)())srinfo.addr;
                done = 1;
                ret = 0;
                break;
        }
    }

    (*laddr)();
    return 0;
}

static unsigned char flash_get_srec_line(unsigned char *buf) {
    unsigned char c;
    int count = 0;

    while (1) {
        c  = *flbuf++;
        if (c == 0xD) {   
            /* Eat up the 0xA too */
            c = *flbuf++; 
            return 0;
        }
        *buf++ = c;
        count++;
        if (count > SREC_MAX_BYTES) return 2;
    }
}

unsigned char nybble_to_val(char x) {
	if (x >= '0' && x <= '9') {
		return (unsigned char)(x-'0');
	}
	return (unsigned char)((x-'A') + 10);
}

unsigned char grab_hex_byte(unsigned char *buf) {
    return  (unsigned char)((nybble_to_val((char)buf[0]) << 4) + nybble_to_val((char)buf[1]));
}

unsigned short grab_hex_word(unsigned char *buf) {
    return (unsigned short)(((unsigned short)grab_hex_byte(buf) << 8) + grab_hex_byte((unsigned char*)((int)buf + 2)));
}

unsigned int grab_hex_word24(unsigned char *buf) {
    return (unsigned int)(((unsigned int)grab_hex_byte(buf) << 16) + grab_hex_word((unsigned char*)((int)buf + 2)));
}

unsigned int grab_hex_dword(unsigned char *buf) {
    return (unsigned int)(((unsigned int)grab_hex_word(buf) << 16) + grab_hex_word((unsigned char*)((int)buf + 4)));
}

unsigned char decode_srec_data (unsigned char *bufs, unsigned char *bufd, unsigned char count, unsigned char skip) {
    unsigned char cksum = 0, cbyte;
    int i;

    for (i = 0; i < count; i++) {
        cbyte = grab_hex_byte (bufs);
        if ((i >= skip - 1) && (i != count-1)) {
            *bufd++ = cbyte;
        }
        bufs  += 2;
        cksum += cbyte;
    }
    return cksum;
}

unsigned char eatup_srec_line(unsigned char *bufs, unsigned char count) {
    int i;
    unsigned char cksum = 0;

    for (i=0; i < count; i++) {
        cksum += grab_hex_byte(bufs);
        bufs += 2;
    }
    return cksum;
}

unsigned char decode_srec_line(unsigned char *sr_buf, srec_info_t *info){
    unsigned char count;
    unsigned char *bufs;
    unsigned char cksum = 0, skip;
    int type;

    bufs = sr_buf;

    if (*bufs != 'S') return 3;

    type = *++bufs - '0';
    count = grab_hex_byte(++bufs);
    bufs += 2;
    cksum = count;

    switch (type) {
        case 0: 
            info->type = 0;
            info->dlen = count;
            cksum += eatup_srec_line(bufs, count);
            break;
        case 1: 
            info->type = 1;
            skip = 3;
            info->addr = (unsigned char*)(unsigned int)grab_hex_word(bufs);
            info->dlen = count - skip;
            cksum += decode_srec_data(bufs, info->sr_data, count, skip);
            break;
        case 2: 
            info->type = 2;
            skip = 4;
            info->addr = (unsigned char*)(unsigned int)grab_hex_word24(bufs);
            info->dlen = count - skip;
            cksum += decode_srec_data(bufs, info->sr_data, count, skip);
            break;
        case 3: 
            info->type = 3;
            skip = 5;
            info->addr = (unsigned char*)(unsigned int)grab_hex_dword(bufs);
            info->dlen = count - skip;
            cksum += decode_srec_data(bufs, info->sr_data, count, skip);
            break;
        case 5:
            info->type = 5;
            info->addr = (unsigned char*)(unsigned int)grab_hex_word(bufs);
            cksum += eatup_srec_line(bufs, count);
            break;
        case 7:
            info->type = 7;
            info->addr = (unsigned char*)(unsigned int)grab_hex_dword(bufs);
            cksum += eatup_srec_line(bufs, count);
            break;
        case 8:
            info->type = 8;
            info->addr = (unsigned char*)(unsigned int)grab_hex_word24(bufs);
            cksum += eatup_srec_line(bufs, count);
            break;
        case 9:
            info->type = 9;
            info->addr = (unsigned char*)(unsigned int)grab_hex_word(bufs);
            cksum += eatup_srec_line(bufs, count);
            break;
        default:
            return 3;
    }

    if (++cksum) return 4;   
    return 0;
}

#ifndef __RESOURCES_H_
#define __RESOURCES_H_

#include "xbasic_types.h"

#define UART_BASEADDR 		XPAR_UARTLITE_0_BASEADDR
#define UART_INTERRUPT_MASK XPAR_AXI_UARTLITE_0_INTERRUPT_MASK
#define UART_INTERRUPT_INTR	XPAR_INTC_0_UARTLITE_0_VEC_ID
#define INTC_BASEADDR 		XPAR_INTC_0_BASEADDR

#define UART_MAX_LENGTH_BUFFER 8

typedef volatile u32 *U32Ptr;
#define WR_WORD(ADDR, DATA) (*(U32Ptr)(ADDR) = (DATA))
#define RD_WORD(ADDR, DATA) ((DATA) = *(U32Ptr)(ADDR))

int init_platform();
void cleanup_platform();
void ScanAXIBAR(u32 Word, u32 AXIBAR);
void WrAXIBAR(u32 word);
void led_blinker(u8 GpioWidth);

#endif




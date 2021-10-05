#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resources.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xtmrctr.h"
#include "xuartlite_l.h"
#include "xintc_l.h"

#define BUF_LENGTH 	64


volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;

u32 *BramPtr 	= (u32 *) XPAR_BRAM_0_BASEADDR;

u32 BRAM_RX_Buff[BUF_LENGTH];
u32 BRAM_TX_Buff[BUF_LENGTH];


/* Функция обратного вызова UART */
void uart_handler(void *baseaddr_p) {

	u32 i;
	xil_printf ("UART RX IRQ\n");
	while (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {
		Rx_byte = XUartLite_RecvByte(UART_BASEADDR);
		if (Rx_byte == '\r'){
			if (strncmp(Rx_data, "rbram", Rx_indx) == 0) {
				xil_printf (">> BRAM READ\n");
				for (i = 0; i < BUF_LENGTH; i++) {
					BRAM_RX_Buff[i] = BramPtr[i];
					xil_printf(">> BRAM [ADDRESS: %08x] [DATA: %08x]\n", &BramPtr[i], BRAM_RX_Buff[i]);
				}
				xil_printf ("\n");
			}
			else if (strncmp(Rx_data, "wbram", Rx_indx) == 0) {
				xil_printf (">> BRAM WRITE\n");
				for(i = 0; i < BUF_LENGTH; i++){
					BRAM_TX_Buff[i] = i;
				}
				memcpy(BramPtr, BRAM_TX_Buff, sizeof(BRAM_TX_Buff)*BUF_LENGTH);
			}
			else{
				xil_printf ("Wrong command\n");

			}
			Rx_indx = 0;
			Rx_data[0] = 0;
		}
		else{
			Rx_data[Rx_indx++] = Rx_byte;
		}
	}
}

void enable_caches() {
    Xil_ICacheEnable();
    Xil_DCacheEnable();
}

void disable_caches(){
    Xil_DCacheDisable();
    Xil_ICacheDisable();
}

void cleanup_platform() {
    disable_caches();
}

int init_platform(){

    enable_caches();

    /* Конфигурация прерываний для UART */
	XIntc_RegisterHandler(INTC_BASEADDR, UART_INTERRUPT_INTR, (XInterruptHandler)uart_handler, (void *)UART_BASEADDR);
	XIntc_MasterEnable(INTC_BASEADDR);
	XIntc_EnableIntr(INTC_BASEADDR, UART_INTERRUPT_MASK);
	XUartLite_EnableIntr(UART_BASEADDR);

	return XST_SUCCESS;
}

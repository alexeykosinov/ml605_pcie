#include "stdio.h"
#include "stdlib.h"
#include "resources.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xaxidma.h"
#include "xaxipcie.h"
#include "xtmrctr.h"
#include "xuartlite_l.h"
#include "xintc_l.h"

#include "pcie.h"
#include "dma.h"

#define SADDR0 		XPAR_BRAM_0_BASEADDR
#define DADDR0 		XPAR_V6DDR_0_S_AXI_BASEADDR + 0x00000100

#define BUF_LENGTH 	64

static XAxiPcie pcieInst;
static XAxiDma dmaInst;
static XTmrCtr xtmrInst;

volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;

u32 *BramPtr = (u32*)XPAR_BRAM_0_BASEADDR;
u32 *DmaPtr = (u32*)XPAR_AXIDMA_0_BASEADDR;
u32 *PciePtr = (u32*)XPAR_AXIPCIE_0_BASEADDR;

u32 BRAM_RX_Buff[BUF_LENGTH];
u32 BRAM_TX_Buff[BUF_LENGTH];

u32 DDR_RX_Buf[BUF_LENGTH];
u32 DDR_TX_Buf[BUF_LENGTH];

/* Функция обратного вызова UART */
void uart_handler(void *baseaddr_p) {

	u32 i;

	u32 MyAdr;
	u32 StartAdr = DADDR0;


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
			else if (strncmp(Rx_data, "rddr", Rx_indx) == 0) {
				xil_printf (">> READ DDR\n");
				// u32 cnt = 0;
				// MyAdr = 0;
				for (MyAdr = StartAdr; MyAdr < StartAdr + sizeof(u32)*BUF_LENGTH; MyAdr += 4) {
					// RD_WORD(MyAdr, DDR_TX_Buf[cnt]);
					// DDR_TX_Buf[cnt] = Xil_In32(MyAdr);
					xil_printf(">> DDR [ADDRESS: 0x%08x] [DATA: 0x%08x]\n", MyAdr, Xil_In32(MyAdr));
					// cnt++;
				}
				xil_printf ("\n");
			}
			else if (strncmp(Rx_data, "wddr", Rx_indx) == 0) {
				xil_printf (">> WRITE DDR\n");
				// u32 cnt = 0;
				// MyAdr = 0;
				// for(i = 0; i < BUF_LENGTH; i++){
				// 	DDR_TX_Buf[i] = i*2;
				// }
				// for (MyAdr = StartAdr; MyAdr < StartAdr + sizeof(u32)*BUF_LENGTH; MyAdr += 4) {
					// WR_WORD(MyAdr, DDR_TX_Buf[cnt]);
					// Xil_Out32(MyAdr, DDR_TX_Buf[cnt]);
					// Xil_Out32(MyAdr, 0xAAAAAAAA);
					// xil_printf(">> DDR Reading:\n");
					// xil_printf(">> DDR [DATA: 0x%08x]\n", Xil_In32(MyAdr));
					// cnt++;				
				// }

			}
			else if (strncmp(Rx_data, "rpci", Rx_indx) == 0) {
				xil_printf (">> READ PCI Regs\n");
				PCIe_PrintInfo(&pcieInst);
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
	int status;

    enable_caches();

	/* Инициализация PCI Express
	* !ВАЖНО
	* Инициализация писюна должна быть одной из первых т.к.
	* на время конфигурации Root устройством ограничено (100 мс)
	*/
	status = PCIe_Init(&pcieInst);
	if (status != XST_SUCCESS) {
		xil_printf("%c[1;31m[ E ] PCIe: Device is not working properly %c[0m\n", 27, 27); // in red color
		return XST_FAILURE;
	}
	PCIe_PrintInfo(&pcieInst);
	
	/* Инициализация CDMA */
	status = DMA_Init(&dmaInst);
	if (status != XST_SUCCESS){
		xil_printf("%c[1;31m[ E ] DMA: Peripheral is not working properly %c[0m\n", 27, 27);
		return XST_FAILURE;
	}
	DMA_PrintInfo(&dmaInst);

    /* Конфигурация прерываний для UART */
	XIntc_RegisterHandler(INTC_BASEADDR, UART_INTERRUPT_INTR, (XInterruptHandler)uart_handler, (void *)UART_BASEADDR);
	XIntc_MasterEnable(INTC_BASEADDR);
	XIntc_EnableIntr(INTC_BASEADDR, UART_INTERRUPT_MASK);
	XUartLite_EnableIntr(UART_BASEADDR);

	return XST_SUCCESS;
}

#include "stdio.h"
#include "stdlib.h"
#include "resources.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xaxicdma.h"
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
static XAxiCdma cdmaInst;
static XTmrCtr xtmrInst;

volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;

u32 *BramPtr 	= (u32 *) XPAR_BRAM_0_BASEADDR;
u32 *CdmaPtr 	= (u32 *) XPAR_AXICDMA_0_BASEADDR;
u32 *PciePtr 	= (u32 *) XPAR_AXIPCIE_0_BASEADDR;
u32 *AxiBarPtr	= (u32 *) XPAR_AXIPCIE_0_AXIBAR_0;

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
			else if (strncmp(Rx_data, "scan", Rx_indx) == 0) {
				xil_printf (">> WRITE scan\n");
				ScanAXIBAR(0xDEADBEEF, XPAR_AXIPCIE_0_AXIBAR_0);

				ScanAXIBAR(0xDEADBEEF, XPAR_AXIPCIE_0_AXIBAR_1);

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
				xil_printf ("\n");
				DMA_PrintInfo();
			}
			else if (strncmp(Rx_data, "wrbar", Rx_indx) == 0) {
				xil_printf (">> WRITE BAR\n");
				WrAXIBAR(0xAA55AA55);
			}
			else if (strncmp(Rx_data, "reset", Rx_indx) == 0) {
				xil_printf (">> WRITE RESET\n");
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
	xil_printf("%cc\r\n", 27); // Reset terminal buffer if complete
	PCIe_PrintInfo(&pcieInst);
	xil_printf("\n");
	
	/* Инициализация CDMA */
	status = DMA_Init(&cdmaInst);
	if (status != XST_SUCCESS){
		xil_printf("%c[1;31m[ E ] DMA: Peripheral is not working properly %c[0m\n", 27, 27);
		return XST_FAILURE;
	}
	DMA_PrintInfo();
	xil_printf("\n");

    /* Конфигурация прерываний для UART */
	XIntc_RegisterHandler(INTC_BASEADDR, UART_INTERRUPT_INTR, (XInterruptHandler)uart_handler, (void *)UART_BASEADDR);
	XIntc_MasterEnable(INTC_BASEADDR);
	XIntc_EnableIntr(INTC_BASEADDR, UART_INTERRUPT_MASK);
	XUartLite_EnableIntr(UART_BASEADDR);

	return XST_SUCCESS;
}


void ScanAXIBAR(u32 Word, u32 AXIBAR) {
	int i;
	u32 temp;
	u32 cnt_words;

	// for(i = 0; i < 16384; i+=4){
	// 	temp = Xil_In32(XPAR_AXIPCIE_0_AXIBAR_0 + i);
	// 	if (temp == Word){
	// 		cnt_words++;
	// 	}

	for(i = 0; i < 64; i+=4){

		temp = Xil_In32(AXIBAR + i);
		xil_printf("[ I ] SCAN AXI BAR Found a word 0x%08X at 0x%08X\n", temp, AXIBAR + i);
//		if (temp == Word){
//			xil_printf("[ I ] SCAN AXI BAR Found a word 0x%08X at 0x%08X\n", temp, AXIBAR + i);
//			cnt_words++;
//		}
//		else if (temp != 0){
//			xil_printf("[ I ] SCAN AXI BAR Found a word 0x%08X at 0x%08X\n", temp, AXIBAR + i);
//		}
	}
	xil_printf("[ I ] SCAN AXI BAR counting done, found num of words: %d\n", cnt_words);
}

void WrAXIBAR(u32 word) {
	int i;
	for(i = 0; i < 256; i+=4){
		Xil_Out32(XPAR_AXIPCIE_0_AXIBAR_0 + i, word);
	}
	xil_printf("[ I ] WRITE AXI BAR done\n");
}

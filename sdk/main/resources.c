#include "stdio.h"
#include "stdlib.h"
#include "resources.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_printf.h"
// #include "xaxicdma.h"
#include "xaxipcie.h"
#include "xtmrctr.h"
#include "xuartlite_l.h"
#include "xintc_l.h"
#include "xgpio.h"

#include "pcie.h"
// #include "dma.h"

#define SADDR0 		XPAR_BRAM_0_BASEADDR
#define DADDR0 		XPAR_V6DDR_0_S_AXI_BASEADDR + 0x00000100

#define BUF_LENGTH 	64

static XAxiPcie pcieInst;
static XGpio GpioOutput;
static XAxiPcie_BarAddr barAddrPtr;

volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;

u32 *BramPtr 	= (u32 *) XPAR_BRAM_0_BASEADDR;
u32 *PciePtr 	= (u32 *) XPAR_AXIPCIE_0_BASEADDR;
u32 *AxiBarPtr	= (u32 *) XPAR_AXIPCIE_0_AXIBAR_0;
u32 *DdrPtr		= (u32 *) XPAR_V6DDR_0_S_AXI_BASEADDR;
u32 *MsiPtr		= (u32 *) XPAR_AXI_MSI_0_BASEADDR;

u32 BRAM_RX_Buff[BUF_LENGTH];
u32 BRAM_TX_Buff[BUF_LENGTH];

u32 DDR_RX_Buf[BUF_LENGTH];
u32 DDR_TX_Buf[BUF_LENGTH];

u32 msi_status(u32 MSI_Controller);
void msi_request(u32 MSI_Controller, u8 MSI_Num);

/* Функция обратного вызова UART */
void uart_handler(void *baseaddr_p) {

	u32 i;

	while (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {
		Rx_byte = XUartLite_RecvByte(UART_BASEADDR);
		if (Rx_byte == '\r'){
			if (strncmp(Rx_data, "b", Rx_indx) == 0) {
				xil_printf (">> ERROR\n");
			}
			else if (strncmp(Rx_data, "scan", Rx_indx) == 0) {
				xil_printf (">> WRITE scan\n");
				ScanAXIBAR(0xDEADBEEF, XPAR_AXIPCIE_0_AXIBAR_0);

			}
			else if (strncmp(Rx_data, "rpci", Rx_indx) == 0) {
				xil_printf (">> READ PCI Regs\n");
				pci_update_reg(&pcieInst);
				PCIe_PrintInfo(&pcieInst);
				xil_printf ("\n");
				GetMSICapStruct(&pcieInst);
			}
			else if (strncmp(Rx_data, "wrbar", Rx_indx) == 0) {
				xil_printf (">> WRITE BAR\n");
				WrAXIBAR(0xAA55AA55);
			}
			else if (strncmp(Rx_data, "rddr", Rx_indx) == 0) {
				for (i = 0; i < 64; i++){
					xil_printf (">> %d DDR DATA: 0x%08X\n", i, DdrPtr[i]);
				}
			}
			else if (strncmp(Rx_data, "wddr", Rx_indx) == 0) {
				for (i = 0; i < 64; i+=4){
					WR_WORD(XPAR_V6DDR_0_S_AXI_BASEADDR + i, 0x8000000 + (i/4) );
				}
			}
			else if (strncmp(Rx_data, "sbar", Rx_indx) == 0) { // Set address AXIBAR2PCIBAR
				barAddrPtr.UpperAddr = DdrPtr[1];
				barAddrPtr.LowerAddr = DdrPtr[0];
				XAxiPcie_SetLocalBusBar2PcieBar(&pcieInst, 0, &barAddrPtr);
				xil_printf ("AXIBAR are set\n");
			}
			else if (strncmp(Rx_data, "smsi", Rx_indx) == 0) { 
				xil_printf ("MSI STATUS: 0x%08X\n", msi_status(0x7EE00000));
			}

			else if (strncmp(Rx_data, "rmsi", Rx_indx) == 0) { 
				msi_request(0x7EE00001, 0x01);
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

void led_blinker(u8 GpioWidth) {

  u8 LedBit;
  u8 LedLoop;
  unsigned int i;
  int numTimes = 1;

  XGpio_Initialize(&GpioOutput, 0);				// Initialization
  XGpio_SetDataDirection(&GpioOutput, 1, 0x0); 	// Set the direction for all signals to be outputs
  XGpio_DiscreteWrite(&GpioOutput, 1, 0x0); 	// Set the GPIO outputs to low

  while (numTimes > 0) {
      for (LedBit = 0x0; LedBit < GpioWidth; LedBit++) {
          for (LedLoop = 0; LedLoop < 1; LedLoop++) {
			XGpio_DiscreteWrite(&GpioOutput, 1, 1 << LedBit); // Set the GPIO Output to High
			for(i = 0; i < 0x00F0000; i++){}

            XGpio_DiscreteClear(&GpioOutput, 1, 1 << LedBit); //  Clear the GPIO Output
            for(i = 0; i < 0x00F0000; i++){}
          }
      }
      numTimes--;
  }
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
	// status = DMA_Init(&cdmaInst);
	// if (status != XST_SUCCESS){
	// 	xil_printf("%c[1;31m[ E ] DMA: Peripheral is not working properly %c[0m\n", 27, 27);
	// 	return XST_FAILURE;
	// }
	// DMA_PrintInfo();
	// xil_printf("\n");

    /* Конфигурация прерываний для UART */
	XIntc_RegisterHandler(INTC_BASEADDR, UART_INTERRUPT_INTR, (XInterruptHandler)uart_handler, (void *)UART_BASEADDR);
	XIntc_MasterEnable(INTC_BASEADDR);
	XIntc_EnableIntr(INTC_BASEADDR, UART_INTERRUPT_MASK);
	XUartLite_EnableIntr(UART_BASEADDR);

	microblaze_enable_interrupts();


	return XST_SUCCESS;
}

void ScanAXIBAR(u32 Word, u32 AXIBAR) {
	int i;
	u32 temp;

	for(i = 0; i < 64; i+=4){
		temp = Xil_In32(AXIBAR + i);
		xil_printf("[ I ] SCAN AXI BAR Found a word 0x%08X at 0x%08X\n", temp, AXIBAR + i);
	}
}

void WrAXIBAR(u32 word) {
	int i;
	for(i = 0; i < 256; i+=4){
		Xil_Out32(XPAR_AXIPCIE_0_AXIBAR_0 + i, word);
	}
	xil_printf("[ I ] WRITE AXI BAR done\n");
}

u32 msi_status(u32 MSI_Controller){
	return Xil_In32(MSI_Controller);
}

void msi_request(u32 MSI_Controller, u8 MSI_Num){
	u32 temp;
	temp = MsiPtr[1];
	MsiPtr[1] = ((MSI_Num << 1) | temp | 0x1);
	xil_printf ("MSI REG1 set  : 0x%08X\n", MsiPtr[1]);
	MsiPtr[1] = 0x0;
}
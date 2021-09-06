#ifndef __RESOURCES_H_
#define __RESOURCES_H_

#include "xaxicdma.h"		/* AXICDMA interface */

#define UART_BASEADDR 		XPAR_RS232_UART_1_BASEADDR
#define UART_INTERRUPT_MASK XPAR_RS232_UART_1_INTERRUPT_MASK
#define UART_INTERRUPT_INTR	XPAR_MICROBLAZE_0_INTC_RS232_UART_1_INTERRUPT_INTR
#define INTC_BASEADDR 		XPAR_INTC_0_BASEADDR

#define UART_MAX_LENGTH_BUFFER 8
#define LENGTH 128

typedef volatile Xuint32 *U32Ptr;
#define WR_WORD(ADDR, DATA) (*(U32Ptr)(ADDR) = (DATA))
#define RD_WORD(ADDR, DATA) ((DATA) = *(U32Ptr)(ADDR))

#define SADDR0 	XPAR_PCI_EXPRESS_PCIEBAR2AXIBAR_0
#define DADDR0 	XPAR_DDR3_SDRAM_S_AXI_BASEADDR + 0x00000100
#define Length0 0xFFFF

#define PCIE_CFG_ID_REG			0x0000		/* Vendor ID/Device ID offset */
#define PCIE_CFG_CMD_STATUS_REG	0x0001		/* Command/Status Register offset */
#define PCIE_CFG_CAH_LAT_HD_REG	0x0003		/* Cache Line / Latency Timer / Header Type / BIST Register Offset */
#define PCIE_CFG_BAR_ZERO_REG	0x0004		/* Bar 0 offset */
#define PCIE_CFG_CMD_BUSM_EN	0x00000004 	/* Bus master enable */



int init_platform();
void cleanup_platform();

int DmaDataTransfer(XAxiCdma *CdmaInstance);
void CDMA_Transfer();

#endif




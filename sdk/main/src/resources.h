#ifndef __RESOURCES_H_
#define __RESOURCES_H_

#include "xbasic_types.h"

#define UART_BASEADDR 		XPAR_RS232_UART_1_BASEADDR
#define UART_INTERRUPT_MASK XPAR_RS232_UART_1_INTERRUPT_MASK
#define UART_INTERRUPT_INTR	XPAR_MICROBLAZE_0_INTC_RS232_UART_1_INTERRUPT_INTR
#define INTC_BASEADDR 		XPAR_INTC_0_BASEADDR

#define UART_MAX_LENGTH_BUFFER 8

typedef volatile u32 *U32Ptr;
#define WR_WORD(ADDR, DATA) (*(U32Ptr)(ADDR) = (DATA))
#define RD_WORD(ADDR, DATA) ((DATA) = *(U32Ptr)(ADDR))


#define PCIE_CFG_ID_REG			0x0000		/* Vendor ID/Device ID offset */
#define PCIE_CFG_CMD_STATUS_REG	0x0001		/* Command/Status Register offset */
#define PCIE_CFG_CAH_LAT_HD_REG	0x0003		/* Cache Line / Latency Timer / Header Type / BIST Register Offset */
#define PCIE_CFG_BAR_ZERO_REG	0x0004		/* Bar 0 offset */
#define PCIE_CFG_CMD_BUSM_EN	0x00000004 	/* Bus master enable */


// typedef  struct {
// 	u8  BusNum;
// 	u8  DeviceNum;
// 	u8  FunctionNum;
// 	u8  PortNumber;
//     u16 VendorID;
//     u16 DeviceID;
//     u32 CommandStatus;
//     u32 BarAddress0;
//     u32 BarAddress1;
//     u16 HeaderType;
//     u16 LatencyTimer;
// } PCIe_Info;


int init_platform();
void cleanup_platform();


#endif




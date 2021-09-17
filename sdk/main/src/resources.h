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
#define PCIE_CFG_REV_CLS_REG	0x0002		/* Revision/Class Code offset */
#define PCIE_CFG_CAH_LAT_HD_REG	0x0003		/* Cache Line / Latency Timer / Header Type / BIST Register Offset */
#define PCIE_CFG_BAR0_REG	    0x0004		/* Bar 0 offset */
#define PCIE_CFG_BAR1_REG	    0x0005		/* Bar 1 offset */
#define PCIE_CFG_BAR2_REG	    0x0006		/* Bar 2 offset */
#define PCIE_CFG_BAR3_REG	    0x0007		/* Bar 3 offset */
#define PCIE_CFG_BAR4_REG	    0x0008		/* Bar 4 offset */
#define PCIE_CFG_BAR5_REG	    0x0009		/* Bar 5 offset */
#define PCIE_CFG_SUB_ID_REG	    0x000B		/* Subsystem Vendor ID/Subsystem Device ID offset */
#define PCIE_CFG_IRQ_REG	    0x000F		/* IRQ Line/IRQ Pin/Min_Gnt/Max_Lat */
#define PCIE_CFG_CMD_BUSM_EN	0x00000004 	/* Bus master enable */


typedef  struct {
	u8  BusNum;
	u8  DeviceNum;
	u8  FunctionNum;
	u8  PortNumber;
    u16 VID;
    u16 PID;
    u16 Command;
    u16 Status;
    u8  RevisonID;
    u32 ClassCode;
    u8  CashLine;
    u8  LatencyTimer;
    u8  HeaderType;
    u8  BIST;
    u32 Bar0;
    u32 Bar1;
    u32 Bar2;
    u32 Bar3;
    u32 Bar4;
    u32 Bar5;
    u16 SubsystemVID;
    u16 SubsystemPID;
    u8  IRQLine;
    u8  IRQPin;
    u8  Min_Gnt;
    u8  Max_Lat;
} PCIe_Info;


int init_platform();
void cleanup_platform();


#endif




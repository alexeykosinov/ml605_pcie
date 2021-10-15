#ifndef __PCIE_H_
#define __PCIE_H_

#include "xbasic_types.h"
#include "xaxipcie.h"

#define PCIE_CFG_ID_REG			    0x0000		/* Vendor ID/Device ID offset */
#define PCIE_CFG_CMD_STATUS_REG	    0x0001		/* Command/Status Register offset */
#define PCIE_CFG_REV_CLS_REG	    0x0002		/* Revision/Class Code offset */
#define PCIE_CFG_CAH_LAT_HD_REG	    0x0003		/* Cache Line / Latency Timer / Header Type / BIST Register Offset */
#define PCIE_CFG_BAR0_REG	        0x0004		/* Bar 0 offset */
#define PCIE_CFG_BAR1_REG	        0x0005		/* Bar 1 offset */
#define PCIE_CFG_BAR2_REG	        0x0006		/* Bar 2 offset */
#define PCIE_CFG_BAR3_REG	        0x0007		/* Bar 3 offset */
#define PCIE_CFG_BAR4_REG	        0x0008		/* Bar 4 offset */
#define PCIE_CFG_BAR5_REG	        0x0009		/* Bar 5 offset */
#define PCIE_CFG_SUB_ID_REG	        0x000B		/* Subsystem Vendor ID/Subsystem Device ID offset */
#define PCIE_CFG_IRQ_REG	        0x000F		/* IRQ Line/IRQ Pin/Min_Gnt/Max_Lat */

#define PCIE_CFG_MSI_CTL	        0x0012		/* MSI Base */
#define PCIE_CFG_MSI_ADDRL	        0x0013		/* MSI Message Address lower */
#define PCIE_CFG_MSI_ADDRU	        0x0014		/* MSI Message Address upper */
#define PCIE_CFG_MSI_DATA	        0x0015		/* MSI Message Data */

#define PCIE_PHY_STATCTL_REG        0x0051 /* Physical-Side Interface (PHY) Status/Control */
#define PCIE_BRIDGE_INFO_REG        0x004C /* Bridge Info */
#define PCIE_BRIDGE_STATCTL_REG     0x004D /* Bridge Status and Control */
#define PCIE_INTR_DEC_REG           0x004E /* Interrupt Decode */
#define PCIE_INTR_MASK_REG          0x004F /* Interrupt Mask */
#define PCIE_BUSLOC_REG             0x0050 /* Bus Location */
#define PCIE_AXIBAR2PCIEBAR_0U_REG  0x0082 /* AXI Base Address Translation Configuration Registers 1 UPPER*/
#define PCIE_AXIBAR2PCIEBAR_0L_REG  0x0083 /* AXI Base Address Translation Configuration Registers 1 LOWER*/

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
    u16 MSI_Control;
    u8  MSI_NxtCap;
    u8  MSI_Cap;
    u16 MSG_Data;
    u32 MSG_AddrL;
    u32 MSG_AddrU;
} PCIe_Info_t;

typedef  struct {
    u32 phy_ctl_stat;
    u32 bridge_info;
    u32 bridge_ctl_stat;
    u32 intr_dec;
    u32 intr_mask;
    u32 axibar_u;
    u32 axibar_l;
    u32 msi_ctl_stat;
    u32 msi_data;
} PCIe_Ctl_t;

int PCIe_Init(XAxiPcie *XlnxEndPointPtr);
void PCIe_PrintInfo(XAxiPcie *XlnxEndPointPtr);
int GetMSICapStruct(XAxiPcie * XlnxEndPointPtr);
void pci_update_reg(XAxiPcie *XlnxEndPointPtr);

#endif




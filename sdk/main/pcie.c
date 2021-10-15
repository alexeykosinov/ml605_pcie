#include "pcie.h"

PCIe_Info_t pcie_info;
PCIe_Ctl_t pcie_ctl;



int PCIe_Init(XAxiPcie *XlnxEndPointPtr) {
	int Status;
	u32 HeaderData;
	XAxiPcie_Config *ConfigPtr;

	/* Initialization of the driver */
	ConfigPtr = XAxiPcie_LookupConfig(0);
	if (ConfigPtr == NULL) {
		xil_printf("[ E ] PCIe end point instance couldn't be found\n");
		return XST_FAILURE;
	}

	Status = XAxiPcie_CfgInitialize(XlnxEndPointPtr, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("[ E ] Failed to configure PCIe end point instance\n");
		return Status;
	}

	// XAxiPcie_DisableInterrupts(XlnxEndPointPtr, XAXIPCIE_IM_ENABLE_ALL_MASK);
	XAxiPcie_ClearPendingInterrupts(XlnxEndPointPtr, XAXIPCIE_ID_CLEAR_ALL_MASK);	/* Clear the pending interrupt */

	/* Make sure link is up. */
	Status = XAxiPcie_IsLinkUp(XlnxEndPointPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("[ E ] PCIe: Link is down\n");
		return XST_FAILURE;
	}

	/* See if root complex has already configured this end point. */
	while(!(HeaderData & PCIE_CFG_CMD_BUSM_EN)){
		XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG, &HeaderData);
	}

	XAxiPcie_GetRequesterId(XlnxEndPointPtr, &pcie_info.BusNum, &pcie_info.DeviceNum, &pcie_info.FunctionNum, &pcie_info.PortNumber);

	/* Read configuration space */
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_ID_REG, &HeaderData);
	pcie_info.VID = (u16) (HeaderData & 0xFFFF);
	pcie_info.PID = (u16) ((HeaderData >> 16) & 0xFFFF);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG, &HeaderData);
	pcie_info.Command = (u16) (HeaderData & 0xFFFF);
	pcie_info.Status = (u16) ((HeaderData >> 16) & 0xFFFF);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_REV_CLS_REG, &HeaderData);
	pcie_info.RevisonID = (u8) HeaderData & 0xFF;
	pcie_info.ClassCode = (HeaderData >> 8) & 0xFFFFFF;

	xil_printf("dummy\n"); /* Delay coz host need more time to update registers */
	xil_printf("dummy\n"); /* Delay coz host need more time to update registers */
	xil_printf("dummy\n"); /* Delay coz host need more time to update registers */
	xil_printf("dummy\n"); /* Delay coz host need more time to update registers */

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CAH_LAT_HD_REG, &HeaderData);
	pcie_info.CashLine = (u8) (HeaderData & 0xFF);
	pcie_info.LatencyTimer = (u8) ((HeaderData >> 8) & 0xFF);
	pcie_info.HeaderType = (u8) ((HeaderData >> 16) & 0xFF);
	pcie_info.BIST = (u8) ((HeaderData >> 24) & 0xFF);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR0_REG, &HeaderData);
	pcie_info.Bar0 = HeaderData;

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR1_REG, &HeaderData);
	pcie_info.Bar1 = HeaderData;

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR2_REG, &HeaderData);
	pcie_info.Bar2 = HeaderData;

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR3_REG, &HeaderData);
	pcie_info.Bar3 = HeaderData;

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR4_REG, &HeaderData);
	pcie_info.Bar4 = HeaderData;

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR5_REG, &HeaderData);
	pcie_info.Bar5 = HeaderData;

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_IRQ_REG, &HeaderData);
	pcie_info.IRQLine = (u8) (HeaderData & 0xFF);
	pcie_info.IRQPin = (u8) ((HeaderData >> 8) & 0xFF);
	pcie_info.Min_Gnt = (u8) ((HeaderData >> 16) & 0xFF);
	pcie_info.Max_Lat = (u8) ((HeaderData >> 24) & 0xFF);

	XAxiPcie_EnableGlobalInterrupt(XlnxEndPointPtr);

	pci_update_reg(XlnxEndPointPtr);

	return XST_SUCCESS;
}

void PCIe_PrintInfo(XAxiPcie *XlnxEndPointPtr){

	xil_printf("[ I ] PCIe: Root Complex has configured this end point\n");
	xil_printf("[ I ] PCIe: BDF number            - %02X:%02X.%02X\n", pcie_info.BusNum, pcie_info.DeviceNum, pcie_info.FunctionNum);
	xil_printf("[ I ] PCIe: Vendor ID             - 0x%04X\n", pcie_info.VID);
	xil_printf("[ I ] PCIe: Device ID             - 0x%04X\n", pcie_info.PID);
	xil_printf("[ I ] PCIe: Command Register      - 0x%04X\n", pcie_info.Command);
	xil_printf("[ I ] PCIe: Status Register       - 0x%04X\n", pcie_info.Status);
	xil_printf("[ I ] PCIe: Revision ID           - 0x%02X\n", pcie_info.RevisonID);
	xil_printf("[ I ] PCIe: Class Code            - 0x%06X\n", pcie_info.ClassCode);
	xil_printf("[ I ] PCIe: Cash Line             - 0x%02X\n", pcie_info.CashLine);
	xil_printf("[ I ] PCIe: Latency Timer         - 0x%02X\n", pcie_info.LatencyTimer);
	xil_printf("[ I ] PCIe: Header Type           - 0x%02X\n", pcie_info.HeaderType);
	xil_printf("[ I ] PCIe: BIST                  - 0x%02X\n", pcie_info.BIST);
	xil_printf("[ I ] PCIe: BAR0                  - 0x%08X\n", pcie_info.Bar0);
	xil_printf("[ I ] PCIe: BAR1                  - 0x%08X\n", pcie_info.Bar1);
	xil_printf("[ I ] PCIe: BAR2                  - 0x%08X\n", pcie_info.Bar2);
	xil_printf("[ I ] PCIe: BAR3                  - 0x%08X\n", pcie_info.Bar3);
	xil_printf("[ I ] PCIe: BAR4                  - 0x%08X\n", pcie_info.Bar4);
	xil_printf("[ I ] PCIe: BAR5                  - 0x%08X\n", pcie_info.Bar5);
	xil_printf("[ I ] PCIe: IRQ Line              - 0x%02X\n", pcie_info.IRQLine);
	xil_printf("[ I ] PCIe: IRQ Pin               - 0x%02X\n", pcie_info.IRQPin);
	xil_printf("[ I ] PCIe: Min Gnt               - 0x%02X\n", pcie_info.Min_Gnt);
	xil_printf("[ I ] PCIe: Max Latency           - 0x%02X\n", pcie_info.Max_Lat);
	xil_printf("[ I ] PCIe: PHY Status/Control    - 0x%08X\n", pcie_ctl.phy_ctl_stat);
	xil_printf("[ I ] PCIe: Bridge Info           - 0x%08X\n", pcie_ctl.bridge_info);
	xil_printf("[ I ] PCIe: Bridge Status/Control - 0x%08X\n", pcie_ctl.bridge_ctl_stat);
	xil_printf("[ I ] PCIe: Interrupt Decode      - 0x%08X\n", pcie_ctl.intr_dec);
	xil_printf("[ I ] PCIe: Interrupt Mask        - 0x%08X\n", pcie_ctl.intr_mask);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_0U     - 0x%08X\n", pcie_ctl.axibar_u);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_0L     - 0x%08X\n", pcie_ctl.axibar_l);

}


int GetMSICapStruct(XAxiPcie *XlnxEndPointPtr) {
	u32 HeaderData;
	u8 MMCap, MMEn;

	pcie_info.MSI_Cap     = (u8)  (pcie_ctl.msi_ctl_stat & 0xFF);
	pcie_info.MSI_NxtCap  = (u8)  ((pcie_ctl.msi_ctl_stat >> 8) & 0xFF);
	pcie_info.MSI_Control = (u16) ((pcie_ctl.msi_ctl_stat >> 16) & 0xFFFF);

	if (pcie_info.MSI_Cap != 0x05){
		xil_printf("[ E ] PCIe: MSI / Incorrect Capability ID, 0x%02X\n", pcie_info.MSI_Cap);
		return XST_FAILURE;
	}

    if (pcie_info.MSI_Control & 0x0001){
		xil_printf("[ I ] PCIe: MSI / MSI Enable\n");
    }
	else {
		xil_printf("[ I ] PCIe: MSI / MSI Disable\n");
	}

    MMCap = 0x07 & (pcie_info.MSI_Control >> 1);

    switch(MMCap){
        case 0: xil_printf("[ I ] PCIe: MSI / Multiple Message Capable: 1  of vectors requested\n"); break;
        case 1: xil_printf("[ I ] PCIe: MSI / Multiple Message Capable: 2  of vectors requested\n"); break;
        case 2: xil_printf("[ I ] PCIe: MSI / Multiple Message Capable: 4  of vectors requested\n"); break;
        case 3: xil_printf("[ I ] PCIe: MSI / Multiple Message Capable: 8  of vectors requested\n"); break;
        case 4: xil_printf("[ I ] PCIe: MSI / Multiple Message Capable: 16 of vectors requested\n"); break;
        case 5: xil_printf("[ I ] PCIe: MSI / Multiple Message Capable: 32 of vectors requested\n"); break;
    }

    MMEn = 0x07 & (pcie_info.MSI_Control >> 4);

    switch(MMEn){
        case 0: xil_printf("[ I ] PCIe: MSI / Multiple Message Enable: 1  of vectors allocated\n"); break;
        case 1: xil_printf("[ I ] PCIe: MSI / Multiple Message Enable: 2  of vectors allocated\n"); break;
        case 2: xil_printf("[ I ] PCIe: MSI / Multiple Message Enable: 4  of vectors allocated\n"); break;
        case 3: xil_printf("[ I ] PCIe: MSI / Multiple Message Enable: 8  of vectors allocated\n"); break;
        case 4: xil_printf("[ I ] PCIe: MSI / Multiple Message Enable: 16 of vectors allocated\n"); break;
        case 5: xil_printf("[ I ] PCIe: MSI / Multiple Message Enable: 32 of vectors allocated\n"); break;
    }

    if (pcie_info.MSI_Control & 0x0080){
		xil_printf("[ I ] PCIe: MSI / MSI 64 bit address capable\n");
    }
	else {
		xil_printf("[ I ] PCIe: MSI / MSI 64 bit address is not capable\n");
	}

    if (pcie_info.MSI_Control & 0x0100){
		xil_printf("[ I ] PCIe: MSI / MSI Per-vector masking capable\n");
    }
	else {
		xil_printf("[ I ] PCIe: MSI / MSI Per-vector masking is not capable\n");
	}

	pcie_info.MSG_Data = (u16) (pcie_ctl.msi_data & 0xFFFF);

	xil_printf("[ I ] PCIe: MSI / Next Pointer         - 0x%02X\n", pcie_info.MSI_NxtCap);
	xil_printf("[ I ] PCIe: MSI / Next Message Address - 0x%08X%08X\n", pcie_info.MSG_AddrU, pcie_info.MSG_AddrL);
	xil_printf("[ I ] PCIe: MSI / Message Data         - 0x%04X\n", pcie_info.MSG_Data);

	XAxiPcie_GetEnabledInterrupts(XlnxEndPointPtr, &HeaderData);
	xil_printf("[ I ] PCIe: MSI / Enabled Interrupts   - 0x%04X\n", HeaderData);
	XAxiPcie_GetPendingInterrupts(XlnxEndPointPtr, &HeaderData);
	xil_printf("[ I ] PCIe: MSI / Pending Interrupts   - 0x%04X\n", HeaderData);

	return XST_SUCCESS;
}	

void pci_update_reg(XAxiPcie *XlnxEndPointPtr) {

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_MSI_CTL, &pcie_ctl.msi_ctl_stat);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_MSI_ADDRL, &pcie_info.MSG_AddrL);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_MSI_ADDRU, &pcie_info.MSG_AddrU);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_MSI_DATA, &pcie_ctl.msi_data);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_PHY_STATCTL_REG, &pcie_ctl.phy_ctl_stat);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_BRIDGE_INFO_REG, &pcie_ctl.bridge_info);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_BRIDGE_STATCTL_REG, &pcie_ctl.bridge_ctl_stat);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_INTR_DEC_REG, &pcie_ctl.intr_dec);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_INTR_MASK_REG, &pcie_ctl.intr_dec);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_AXIBAR2PCIEBAR_0U_REG, &pcie_ctl.axibar_u);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_AXIBAR2PCIEBAR_0L_REG, &pcie_ctl.axibar_l);

}

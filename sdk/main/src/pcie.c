#include "pcie.h"

PCIe_Info_t pcie_info;

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

	XAxiPcie_DisableInterrupts(XlnxEndPointPtr, XAXIPCIE_IM_ENABLE_ALL_MASK);		/* Disable all interrupts */
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

	return XST_SUCCESS;
}

void PCIe_PrintInfo(XAxiPcie *XlnxEndPointPtr){
	u32 HeaderData;

	xil_printf("%cc\r\n", 27); // Reset terminal buffer if complete
	xil_printf("[ I ] PCIe: Root Complex has configured this end point\n");
	xil_printf("[ I ] PCIe: BDF number          - %02X:%02X.%02X\n", pcie_info.BusNum, pcie_info.DeviceNum, pcie_info.FunctionNum);
	xil_printf("[ I ] PCIe: Vendor ID           - 0x%04X\n", pcie_info.VID);
	xil_printf("[ I ] PCIe: Device ID           - 0x%04X\n", pcie_info.PID);
	// xil_printf("[ I ] PCIe: Command Register    - 0x%04X\n", pcie_info.Command);
	xil_printf("[ I ] PCIe: Status Register     - 0x%04X\n", pcie_info.Status);
	xil_printf("[ I ] PCIe: Revision ID         - 0x%02X\n", pcie_info.RevisonID);
	xil_printf("[ I ] PCIe: Class Code          - 0x%06X\n", pcie_info.ClassCode);
	xil_printf("[ I ] PCIe: Cash Line           - 0x%02X\n", pcie_info.CashLine);
	xil_printf("[ I ] PCIe: Latency Timer       - 0x%02X\n", pcie_info.LatencyTimer);
	xil_printf("[ I ] PCIe: Header Type         - 0x%02X\n", pcie_info.HeaderType);
	xil_printf("[ I ] PCIe: BIST                - 0x%02X\n", pcie_info.BIST);
	xil_printf("[ I ] PCIe: BAR0                - 0x%08X\n", pcie_info.Bar0);
	xil_printf("[ I ] PCIe: BAR1                - 0x%08X\n", pcie_info.Bar1);
	xil_printf("[ I ] PCIe: BAR2                - 0x%08X\n", pcie_info.Bar2);
	xil_printf("[ I ] PCIe: BAR3                - 0x%08X\n", pcie_info.Bar3);
	xil_printf("[ I ] PCIe: BAR4                - 0x%08X\n", pcie_info.Bar4);
	xil_printf("[ I ] PCIe: BAR5                - 0x%08X\n", pcie_info.Bar5);
	xil_printf("[ I ] PCIe: IRQ Line            - 0x%02X\n", pcie_info.IRQLine);
	xil_printf("[ I ] PCIe: IRQ Pin             - 0x%02X\n", pcie_info.IRQPin);
	xil_printf("[ I ] PCIe: Min Gnt             - 0x%02X\n", pcie_info.Min_Gnt);
	xil_printf("[ I ] PCIe: Max Latency         - 0x%02X\n", pcie_info.Max_Lat);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0051, &HeaderData);
	xil_printf("[ I ] PCIe: PHY Status/Control        : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x004C, &HeaderData);
	xil_printf("[ I ] PCIe: Bridge Info               : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x004D, &HeaderData);
	xil_printf("[ I ] PCIe: Bridge Status and Control : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x004E, &HeaderData);
	xil_printf("[ I ] PCIe: Interrupt Decode          : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x004F, &HeaderData);
	xil_printf("[ I ] PCIe: Interrupt Mask            : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0050, &HeaderData);
	xil_printf("[ I ] PCIe: Bus Location              : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0082, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_0U         : 0x%08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0083, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_0L         : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0084, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_1U         : 0x%08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0085, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_1L         : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0086, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_2U         : 0x%08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0087, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_2L         : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0088, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_3U         : 0x%08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x0089, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_3L         : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x008A, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_4U         : 0x%08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x008B, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_4L         : 0x%08X\n", HeaderData);

	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x008C, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_5U         : 0x%08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, 0x008D, &HeaderData);
	xil_printf("[ I ] PCIe: AXIBAR2PCIEBAR_5L         : 0x%08X\n", HeaderData);
}

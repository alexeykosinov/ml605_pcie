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

#define SADDR0 		XPAR_BRAM_0_BASEADDR
#define DADDR0 		XPAR_V6DDR_0_S_AXI_BASEADDR + 0x00000100

#define BUF_LENGTH 	64

#define CDMA_CDMACR 0x00 /* CDMA Control register offset */
#define CDMA_CDMASR 0x04 /* CDMA Status register offset */
#define CDMA_SA 	0x18 /* CDMA Source Address offset */
#define CDMA_DA 	0x20 /* CDMA Destination Address offset */
#define CDMA_BTT 	0x28 /* CDMA TX Byte Size */


PCIe_Info pcie_info;

static XAxiPcie pcieInst;
static XAxiCdma cdmaInst;
static XTmrCtr xtmrInst;


int PCIe_Init(XAxiPcie *XlnxEndPointPtr);
void PCIe_Print_Reg(XAxiPcie *XlnxEndPointPtr);
int CDMA_Init(XAxiCdma *CdmaInstance);

int CDMA_Transfer(XAxiCdma *CdmaInstance, XTmrCtr *XTmrInstance, u32 TX, u32 RX, u16 Length);


volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;

u32 *BramPtr = (u32*)XPAR_BRAM_0_BASEADDR;
u32 *CdmaPtr = (u32*)XPAR_AXICDMA_0_BASEADDR;
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

				CDMA_Transfer(&cdmaInst, &xtmrInst, SADDR0, DADDR0, 256);
			}
			else if (strncmp(Rx_data, "rpci", Rx_indx) == 0) {
				xil_printf (">> READ PCI Regs\n");
				PCIe_Print_Reg(&pcieInst);
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
	 * */
	status = PCIe_Init(&pcieInst);
	if (status != XST_SUCCESS) {
		xil_printf("%c[1;31m[ E ] PCIe: Device is not working properly %c[0m\n", 27, 27); // in red color
		return XST_FAILURE;
	}
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
	/* Инициализация CDMA */
	status = CDMA_Init(&cdmaInst);
	if (status != XST_SUCCESS){
		xil_printf("%c[1;31m[ E ] CDMA: Peripheral is not working properly %c[0m\n", 27, 27);
		return XST_FAILURE;
	}


    /* Конфигурация прерываний для UART */
	XIntc_RegisterHandler(INTC_BASEADDR, UART_INTERRUPT_INTR, (XInterruptHandler)uart_handler, (void *)UART_BASEADDR);
	XIntc_MasterEnable(INTC_BASEADDR);
	XIntc_EnableIntr(INTC_BASEADDR, UART_INTERRUPT_MASK);
	XUartLite_EnableIntr(UART_BASEADDR);

	return XST_SUCCESS;
}


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

	//XAxiPcie_DisableInterrupts(XlnxEndPointPtr, XAXIPCIE_IM_ENABLE_ALL_MASK);		/* Disable all interrupts */
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

	xil_printf("dummy\n"); /* Delay coz host need more time to get address */
	xil_printf("dummy\n"); /* Delay coz host need more time to get address */
	xil_printf("dummy\n"); /* Delay coz host need more time to get address */
	xil_printf("dummy\n"); /* Delay coz host need more time to get address */

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

int CDMA_Init(XAxiCdma *CdmaInstance) {
	int status;
	XAxiCdma_Config* ConfigPtr;

	ConfigPtr = XAxiCdma_LookupConfig(0);
	if (ConfigPtr == NULL) {
		xil_printf("[ E ] CDMA: Configuration couldn't be found\n");
		return XST_FAILURE;
	}

	status = XAxiCdma_CfgInitialize(CdmaInstance, ConfigPtr, ConfigPtr->BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("[ E ] CDMA: Failed to configure instance\n");
		return XST_FAILURE;
	}

	XAxiCdma_Reset(CdmaInstance);
	XAxiCdma_IntrDisable(CdmaInstance, XAXICDMA_XR_IRQ_ALL_MASK);

	xil_printf("[ I ] CDMA: Control             - 0x%08X\n", Xil_In32(XPAR_AXICDMA_0_BASEADDR + CDMA_CDMACR));
	xil_printf("[ I ] CDMA: Status              - 0x%08X\n", Xil_In32(XPAR_AXICDMA_0_BASEADDR + CDMA_CDMASR));
	xil_printf("[ I ] CDMA: Source Address      - 0x%08X\n", Xil_In32(XPAR_AXICDMA_0_BASEADDR + CDMA_SA));
	xil_printf("[ I ] CDMA: Destination Address - 0x%08X\n", Xil_In32(XPAR_AXICDMA_0_BASEADDR + CDMA_DA));
	xil_printf("[ I ] CDMA: Payload Size        - 0x%08X\n", Xil_In32(XPAR_AXICDMA_0_BASEADDR + CDMA_BTT));








	return status;
}

int CDMA_Transfer(XAxiCdma *CdmaInstance, XTmrCtr *XTmrInstance, u32 TX, u32 RX, u16 Length) {

	int status;

	u32 TimerCount1 = 0;
	u32 TimerCount2 = 0;

	// axi_timer
	XTmrCtr_Initialize(XTmrInstance, 0);
	Xil_AssertNonvoid(XTmrInstance != NULL);
	Xil_AssertNonvoid(0 < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(XTmrInstance->IsReady == XIL_COMPONENT_IS_READY);
	XTmrCtr_WriteReg(XTmrInstance->BaseAddress, 0, XTC_TLR_OFFSET, 0);
	XTmrCtr_WriteReg(XTmrInstance->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_INT_OCCURED_MASK | XTC_CSR_LOAD_MASK);
	XTmrCtr_WriteReg(XTmrInstance->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_ENABLE_TMR_MASK);

	//start transfer
	status = XAxiCdma_SimpleTransfer(CdmaInstance, TX, RX, Length, NULL, NULL);
	if (status != XST_SUCCESS) {
		status = XAxiCdma_GetError(CdmaInstance);
		if (status != 0) {
			xil_printf("CDMA: Error, %x\n", status);
			return XST_FAILURE;
		}
		xil_printf("CDMA: Simple Transfer Error\n");
		return XST_FAILURE;
	}

	TimerCount1 = XTmrCtr_ReadReg(XTmrInstance->BaseAddress, 0, XTC_TCR_OFFSET);
	while (XAxiCdma_IsBusy(CdmaInstance));
	TimerCount2 = XTmrCtr_ReadReg(XTmrInstance->BaseAddress, 0, XTC_TCR_OFFSET);

	xil_printf("CDMA: Size: %d B / Speed: %d MB/s\n", Length, Length*100/(TimerCount2-TimerCount1));
	return XST_SUCCESS;
}


void PCIe_Print_Reg(XAxiPcie *XlnxEndPointPtr){
	u32 HeaderData;

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

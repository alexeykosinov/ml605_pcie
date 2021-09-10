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

#define SADDR0 	XPAR_BRAM_0_BASEADDR
#define DADDR0 	XPAR_DDR3_SDRAM_S_AXI_BASEADDR + 0x00000100
#define Length0 0x00FF

#define BUF_LENGTH 64 // BYTES!

static XAxiPcie XlnxPCIeEndPoint;
static XAxiCdma CdmaInstance;
//static XTmrCtr p;


int PCIe_Init(XAxiPcie *XlnxEndPointPtr);
int CDMA_Init(XAxiCdma *CdmaInstance);
// int BRAM_Init(XBram *BramInstance);

// int CDMA_Test(XAxiCdma *CdmaInstance, XTmrCtr *InstancePtr);
// int CDMA_Transfer(XAxiCdma *CdmaInstance, XTmrCtr *InstancePtr, u32 TX, u32 RX, u16 Length);


volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;

u32 *BramPtr = (u32*)XPAR_BRAM_0_BASEADDR;

u32 RX_Buf[BUF_LENGTH];
u32 TX_Buf[BUF_LENGTH];



/* Функция обратного вызова UART */
void uart_handler(void *baseaddr_p) {

	u32 i;

	while (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {
		Rx_byte = XUartLite_RecvByte(UART_BASEADDR);
		if (Rx_byte == '\r'){
			if (strncmp(Rx_data, "rbram", Rx_indx) == 0) {
				xil_printf (">> BRAM READ\n");
				for (i = 0; i < BUF_LENGTH; i++) {
					xil_printf(">> BRAM [ADDRESS: %08x] [DATA: %08x]\n", &BramPtr[i], BramPtr[i]);
				}
			}
			else if (strncmp(Rx_data, "wbram", Rx_indx) == 0) {
				xil_printf (">> BRAM WRITE\n");
				for(i = 0; i < BUF_LENGTH; i++){
					TX_Buf[i] = i;
				}
				memcpy(BramPtr, TX_Buf, sizeof(TX_Buf)*BUF_LENGTH);
			}
			else if (strncmp(Rx_data, "rddr", Rx_indx) == 0) {
				xil_printf (">> READ DDR\n");

			}
			else if (strncmp(Rx_data, "wddr", Rx_indx) == 0) {
				xil_printf (">> WRITE DDR\n");

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
	status = PCIe_Init(&XlnxPCIeEndPoint);
	if (status != XST_SUCCESS) {
		xil_printf("%c[1;31m[ E ] PCIe device is not working properly %c[0m\n", 27, 27); // in red color
		return XST_FAILURE;
	}

	/* Инициализация CDMA */
	status = CDMA_Init(&CdmaInstance);
	if (status != XST_SUCCESS){
		xil_printf("%c[1;31m[ E ] CDMA peripheral is not working properly %c[0m\n", 27, 27);
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
	u8  BusNum;
	u8  DeviceNum;
	u8  FunctionNum;
	u8  PortNumber;
	XAxiPcie_Config *ConfigPtr;

	/* Initialization of the driver */
	ConfigPtr = XAxiPcie_LookupConfig(0);
	if (ConfigPtr == NULL) {
		xil_printf("[ E ] PCIe end point instance couldn't be found \n");
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
		XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG,&HeaderData);
	}

	xil_printf("PCIe: \t Root Complex has configured this end point\n");
	XAxiPcie_GetRequesterId(XlnxEndPointPtr, &BusNum, &DeviceNum, &FunctionNum, &PortNumber);
	xil_printf("PCIe: \t BDF number: %02X:%02X.%02X\n", BusNum, DeviceNum, FunctionNum);

	/* Read configuration space */
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_ID_REG, &HeaderData);
	xil_printf("PCIe: \t Vendor ID/Device ID Register is %08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG, &HeaderData);
	xil_printf("PCIe: \t Command/Status Register is %08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CAH_LAT_HD_REG, &HeaderData);
	xil_printf("PCIe: \t Header Type/Latency Timer Register is %08X\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR_ZERO_REG, &HeaderData);
	xil_printf("PCIe: \t BAR 0 is %08X\n", HeaderData);

	return XST_SUCCESS;
}

int CDMA_Init(XAxiCdma *CdmaInstance) {
	int status;
	XAxiCdma_Config* ConfigPtr;

	ConfigPtr = XAxiCdma_LookupConfig(0);
	if (ConfigPtr == NULL) {
		xil_printf("[ E ] Configuration for CDMA couldn't be found\n");
		return XST_FAILURE;
	}

	status = XAxiCdma_CfgInitialize(CdmaInstance, ConfigPtr, ConfigPtr->BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("[ E ] Failed to configure CDMA instance\n");
		return XST_FAILURE;
	}

	XAxiCdma_Reset(CdmaInstance);
	XAxiCdma_IntrDisable(CdmaInstance, XAXICDMA_XR_IRQ_ALL_MASK);

	return status;
}

// int CDMA_Transfer(XAxiCdma *CdmaInstance, XTmrCtr *InstancePtr, u32 TX, u32 RX, u16 Length) {

// 	int status;

// 	u32 TimerCount1 = 0;
// 	u32 TimerCount2 = 0;

// 	// axi_timer
// 	XTmrCtr_Initialize(InstancePtr, 0);
// 	Xil_AssertNonvoid(InstancePtr != NULL);
// 	Xil_AssertNonvoid(0 < XTC_DEVICE_TIMER_COUNT);
// 	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
// 	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TLR_OFFSET, 0);
// 	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_INT_OCCURED_MASK | XTC_CSR_LOAD_MASK);
// 	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_ENABLE_TMR_MASK);

// 	//start transfer
// 	status = XAxiCdma_SimpleTransfer(CdmaInstance, TX, RX, Length, NULL, NULL);
// 	if (status != XST_SUCCESS) {
// 		status = XAxiCdma_GetError(CdmaInstance);
// 		if (status != 0) {
// 			xil_printf("Error Code = %x\n", status);
// 			return XST_FAILURE;
// 		}
// 		xil_printf("AXI CDMA Simple Transfer Error\n");
// 		return XST_FAILURE;
// 	}

// 	TimerCount1 = XTmrCtr_ReadReg(InstancePtr->BaseAddress, 0, XTC_TCR_OFFSET);
// 	while (XAxiCdma_IsBusy(CdmaInstance));
// 	TimerCount2 = XTmrCtr_ReadReg(InstancePtr->BaseAddress, 0, XTC_TCR_OFFSET);

// 	xil_printf("Size: %d B / Speed: %d MB/s\n", Length, Length*100/(TimerCount2-TimerCount1));
// 	xil_printf("Transfer is completed\n");

// 	return XST_SUCCESS;
// }


// int CDMA_Test(XAxiCdma *CdmaInstance, XTmrCtr *InstancePtr){

// 	int Status;

// 	u32 *BramPtr = (u32*)XPAR_BRAM_0_BASEADDR;
// 	// u32 *DdrPtr = (u32*)DADDR0;

// 	// for (Addr = 0; Addr < Length0; Addr++) {
// 	// 	xil_printf("BRAM Readed: %08x\n", BramPtr[Addr]);
// 	// }
// 	xil_printf("BRAM : first value 0x%08x\n", BramPtr[0]);
// 	xil_printf("BRAM : last value 0x%08x\n", BramPtr[Length0-1]);


// 	xil_printf("BRAM Transfer is completed\n");

// 	Status = CDMA_Transfer(CdmaInstance, InstancePtr, SADDR0, DADDR0, Length0);
// 	if (Status != XST_SUCCESS) {
// 		xil_printf("%c[1;31mError when initializing CDMA, code: %d%c[0m\n", 27, Status, 27);
// 		return XST_FAILURE;
// 	}

// 	return XST_SUCCESS;
// }

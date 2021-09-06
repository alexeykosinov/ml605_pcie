#include "resources.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xaxicdma.h"
#include "xaxipcie.h"
#include "xtmrctr.h"
#include "xuartlite_l.h"
#include "xintc_l.h"

Xuint32 DataRead;

static XAxiPcie XlnxPCIeEndPoint;
static XAxiCdma CdmaInstance;
static XTmrCtr p;

volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;

int PCIe_Init(XAxiPcie *XlnxEndPointPtr);
int CDMA_Init(XAxiCdma *CdmaInstance);
int TestCDMA(XAxiCdma *CdmaInstance);
int DmaDataTransfer(XAxiCdma *CdmaInstance);


/* Функция обратного вызова UART */
void uart_handler(void *baseaddr_p) {
	while (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {

		Rx_byte = XUartLite_RecvByte(UART_BASEADDR);

		if (Rx_byte == '\r'){

			if (strncmp(Rx_data, "test", Rx_indx) == 0) {
				int status;
				xil_printf ("test cmd\n");
				status = TestCDMA(&CdmaInstance);

				if (status != XST_SUCCESS) {
					xil_printf("Successfully ran PCIe End-Point CDMA Example\r\n");
				}

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
		xil_printf("%c[1;31mError when initializing PCIe device, code: %d%c[0m\n", 27, status, 27); // in red color
		return XST_FAILURE;
	}

	/* Инициализация CDMA */
	status = CDMA_Init(&CdmaInstance);
	if (status != XST_SUCCESS){
		xil_printf( "DMA peripheral configuration failed %d\n", status);
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
		xil_printf("PCIe: \t Failed to initialize PCIe End Point Instance\n");
		return XST_FAILURE;
	}

	Status = XAxiPcie_CfgInitialize(XlnxEndPointPtr, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("PCIe: \t Failed to initialize PCIe End Point Instance\n");
		return Status;
	}

	XAxiPcie_DisableInterrupts(XlnxEndPointPtr, XAXIPCIE_IM_ENABLE_ALL_MASK);		/* Disable all interrupts */
	XAxiPcie_ClearPendingInterrupts(XlnxEndPointPtr, XAXIPCIE_ID_CLEAR_ALL_MASK);	/* Clear the pending interrupt */

	/* Make sure link is up. */
	Status = XAxiPcie_IsLinkUp(XlnxEndPointPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("PCIe: \t Link is not up\n");
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
		xil_printf("CDMA: \t Configuration for CDMA not found\n");
		return XST_FAILURE;
	}

	status = XAxiCdma_CfgInitialize(CdmaInstance, ConfigPtr, ConfigPtr->BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("CDMA: \t Failed to initialize CDMA Instance\n");
		return XST_FAILURE;
	}

	XAxiCdma_Reset(CdmaInstance);
	XAxiCdma_IntrDisable(CdmaInstance, XAXICDMA_XR_IRQ_ALL_MASK);

	return status;
}

void CDMA_Transfer() {

	int Status;

	u32 cnt_array;

	//u8* tx_buffer = (u8*)XPAR_DDR3_SDRAM_S_AXI_BASEADDR + 0x00000010;
	u8* rx_buffer = (u8*)XPAR_DDR3_SDRAM_S_AXI_BASEADDR + 0x00000100;
	//u8* rx_buffer = (u8*)XPAR_PCI_EXPRESS_AXIBAR_0;

	u8* tx_buffer = (u8*)XPAR_PCI_EXPRESS_BASEADDR;

//	tx_buffer[0]   = 0x1A;
//	tx_buffer[1]   = 0x2B;
//	tx_buffer[2]   = 0x3C;
//	tx_buffer[3]   = 0x4D;
//	tx_buffer[124] = 0x15;
//	tx_buffer[125] = 0x26;
//	tx_buffer[126] = 0x37;
//	tx_buffer[127] = 0x48;

	// clear rx_buffer
	for(cnt_array = 0; cnt_array < LENGTH; cnt_array++) rx_buffer[cnt_array] = 0;

	u32* txBufferAddr = (u32*)&tx_buffer[0];
	u32* rxBufferAddr = (u32*)&rx_buffer[0];


	xil_printf("Transfer prepared\r\n");
	xil_printf("TxData: \t %02x %02x %02x %02x %02x %02x %02x %02x\r\n", tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3], tx_buffer[124], tx_buffer[125], tx_buffer[126], tx_buffer[127]);
	// xil_printf("RxData: \t %02x %02x %02x %02x %02x %02x %02x %02x\r\n", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[124], rx_buffer[125], rx_buffer[126], rx_buffer[127]);

	Xil_DCacheFlushRange((u32)rxBufferAddr, LENGTH);

	Status = XAxiCdma_SimpleTransfer(&CdmaInstance, (u32)txBufferAddr, (u32)rxBufferAddr, LENGTH, NULL, NULL);

	if (XAxiCdma_IsBusy(&CdmaInstance)) {
		xil_printf("CDMA is busy...\r\n");
		while(XAxiCdma_IsBusy(&CdmaInstance)) {};
	}

	if (Status != XST_SUCCESS)	xil_printf("DMA Transfer Error occurred: Status = %d\r\n", Status);

	Xil_DCacheInvalidateRange((u32)txBufferAddr, LENGTH);

	xil_printf("Transfer finished\r\n");
	xil_printf("TxData: \t %02x %02x %02x %02x %02x %02x %02x %02x\r\n", tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3], tx_buffer[124], tx_buffer[125], tx_buffer[126], tx_buffer[127]);
	xil_printf("RxData: \t %02x %02x %02x %02x %02x %02x %02x %02x\r\n", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[124], rx_buffer[125], rx_buffer[126], rx_buffer[127]);
}


int DmaDataTransfer(XAxiCdma *CdmaInstance) {

	int status;


	XTmrCtr *InstancePtr = &p;

	u32 TimerCount1 = 0;
	u32 TimerCount2 = 0;

	if (SADDR0 == 0) {
		xil_printf("SADDR0 == 0\n");
		return XST_FAILURE;
	}
	if (DADDR0 == 0) {
		xil_printf("SADDR0 == 0\n");
		return XST_FAILURE;
	}

	//u32 cnt_array;

//	u8* ddr_buffer = (u8*)DADDR0;
//	u8* pcie_buffer = (u8*)SADDR0;

//	ddr_buffer[0]   		= 0x1A;
//	ddr_buffer[1]   		= 0x2B;
//	ddr_buffer[2]   		= 0x3C;
//	ddr_buffer[3]   		= 0x4D;
//	ddr_buffer[Length0-3] 	= 0x99;
//	ddr_buffer[Length0-2] 	= 0x55;
//	ddr_buffer[Length0-1] 	= 0xAA;
//	ddr_buffer[Length0] 	= 0x3F;

	//u32* ddrBufferAddr = (u32*)&ddr_buffer[0];
	//u32* pcieBufferAddr = (u32*)&pcie_buffer[0];
	
	// clear rx_buffer
	//for(cnt_array = 0; cnt_array < Length0; cnt_array++) pcie_buffer[cnt_array] = 0;

//	xil_printf("Transfer prepared\r\n");
//	xil_printf("DDR Data: \t %02x %02x %02x %02x %02x %02x %02x %02x\r\n", ddr_buffer[0], ddr_buffer[1], ddr_buffer[2], ddr_buffer[3], ddr_buffer[Length0-3], ddr_buffer[Length0-2], ddr_buffer[Length0-1], ddr_buffer[Length0]);
	// xil_printf("PCIe Data: \t %02x %02x %02x %02x %02x %02x %02x %02x\r\n", pcie_buffer[0], pcie_buffer[1], pcie_buffer[2], pcie_buffer[3], pcie_buffer[Length0-3], pcie_buffer[Length0-2], pcie_buffer[Length0-1], pcie_buffer[Length0]);


	//Xil_DCacheFlushRange((u32)ddrBufferAddr, Length0);


	// axi_timer
	XTmrCtr_Initialize(InstancePtr, 0);
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(0 < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TLR_OFFSET, 0);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_INT_OCCURED_MASK | XTC_CSR_LOAD_MASK);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_ENABLE_TMR_MASK);

	//start transfer
	status = XAxiCdma_SimpleTransfer(CdmaInstance, DADDR0, SADDR0, Length0, NULL, NULL);
	if (status != XST_SUCCESS) {
		status = XAxiCdma_GetError(CdmaInstance);
		if (status != 0) {
			xil_printf("Error Code = %x\n", status);
			return XST_FAILURE;
		}
		xil_printf("AXI CDMA Simple Transfer Error\n");
		return XST_FAILURE;
	}

	TimerCount1 = XTmrCtr_ReadReg(InstancePtr->BaseAddress, 0, XTC_TCR_OFFSET);
	while (XAxiCdma_IsBusy(CdmaInstance));
	TimerCount2 = XTmrCtr_ReadReg(InstancePtr->BaseAddress, 0, XTC_TCR_OFFSET);

	xil_printf("AXI CDMA Transfer is complete without errors\r\n");
	xil_printf("-----------------------------------------------------\r\n");
	xil_printf(" - Counter	= %d\r\n", TimerCount2-TimerCount1);
	xil_printf(" - Length 	= %d Bytes\r\n", Length0);
	xil_printf(" - Speed 	= %d MByte/s\r\n", Length0*100/(TimerCount2-TimerCount1));
	xil_printf("-----------------------------------------------------\r\n");


	return XST_SUCCESS;
}


int TestCDMA(XAxiCdma *CdmaInstance){

	int Status;
	Status = DmaDataTransfer(CdmaInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("%c[1;31mError when initializing CDMA, code: %d%c[0m\n", 27, Status, 27);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

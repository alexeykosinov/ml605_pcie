#include "resources.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xaxicdma.h"
#include "xtmrctr.h"
#include <xuartlite_l.h>
#include <xintc_l.h>

#define UART_BASEADDR 		XPAR_RS232_UART_1_BASEADDR
#define UART_INTERRUPT_MASK XPAR_RS232_UART_1_INTERRUPT_MASK
#define UART_INTERRUPT_INTR	XPAR_MICROBLAZE_0_INTC_RS232_UART_1_INTERRUPT_INTR
#define INTC_BASEADDR 		XPAR_INTC_0_BASEADDR


#define UART_MAX_LENGTH_BUFFER 8
#define LENGTH 128

typedef volatile Xuint32 *U32Ptr;
#define WR_WORD(ADDR, DATA) (*(U32Ptr)(ADDR) = (DATA))
#define RD_WORD(ADDR, DATA) ((DATA) = *(U32Ptr)(ADDR))

Xuint32 DataRead;

#define SADDR0 	XPAR_PCI_EXPRESS_AXIBAR_HIGHADDR_0
//#define SADDR0 	XPAR_PCI_EXPRESS_BASEADDR

#define DADDR0 	XPAR_DDR3_SDRAM_S_AXI_BASEADDR + 0x00000100
#define Length0 0xFFFF

#define PCIE_CFG_ID_REG			0x0000		/* Vendor ID/Device ID offset */
#define PCIE_CFG_CMD_STATUS_REG	0x0001		/* Command/Status Register offset */
#define PCIE_CFG_CAH_LAT_HD_REG	0x0003		/* Cache Line / Latency Timer / Header Type / BIST Register Offset */
#define PCIE_CFG_BAR_ZERO_REG	0x0004		/* Bar 0 offset */
#define PCIE_CFG_CMD_BUSM_EN	0x00000004 	/* Bus master enable */

static XAxiCdma CdmaInstance;
static XTmrCtr p;


volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;




void uart_handler(void *baseaddr_p) {
	while (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {

		Rx_byte = XUartLite_RecvByte(UART_BASEADDR);

		if (Rx_byte == '\r'){

			if (strncmp(Rx_data, "test", Rx_indx) == 0) {
				xil_printf ("test cmd\n");
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

void init_platform(){
    enable_caches();
	XIntc_RegisterHandler(INTC_BASEADDR, UART_INTERRUPT_INTR, (XInterruptHandler)uart_handler, (void *)UART_BASEADDR);
	XIntc_MasterEnable(INTC_BASEADDR);
	XIntc_EnableIntr(INTC_BASEADDR, UART_INTERRUPT_MASK);
	XUartLite_EnableIntr(UART_BASEADDR);
}

void cleanup_platform() {
    disable_caches();
}

int PCIeEndPointInitialize(XAxiPcie *XlnxEndPointPtr, u16 DeviceId) {
	int Status;
	u32 HeaderData;
	u8  BusNum;
	u8  DeviceNum;
	u8  FunctionNum;
	u8  PortNumber;
	u32 InterruptMask;
	XAxiPcie_Config *ConfigPtr;

	/* Initialization of the driver */
	ConfigPtr = XAxiPcie_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		xil_printf("PCIe: \t Failed to initialize PCIe End Point Instance\r\n");
		return XST_FAILURE;
	}

	Status = XAxiPcie_CfgInitialize(XlnxEndPointPtr, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("PCIe: \t Failed to initialize PCIe End Point Instance\r\n");
		return Status;
	}

	XAxiPcie_DisableInterrupts(XlnxEndPointPtr, XAXIPCIE_IM_ENABLE_ALL_MASK);		/* Disable all interrupts */
	XAxiPcie_ClearPendingInterrupts(XlnxEndPointPtr, XAXIPCIE_ID_CLEAR_ALL_MASK);	/* Clear the pending interrupt */


	/* Make sure link is up. */
	Status = XAxiPcie_IsLinkUp(XlnxEndPointPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("PCIe: \t Link is not up\r\n");
		return XST_FAILURE;
	}

	xil_printf("PCIe: \t Link is up\r\n");

	/* See if root complex has already configured this end point. */
	while(!(HeaderData & PCIE_CFG_CMD_BUSM_EN)){
		XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG,&HeaderData);
	}

	xil_printf("PCIe: \t Root Complex has configured this end point\r\n");

	XAxiPcie_GetRequesterId(XlnxEndPointPtr, &BusNum, &DeviceNum, &FunctionNum, &PortNumber);

	xil_printf("PCIe: \t BDF number: %02X:%02X.%02X\r\n", BusNum, DeviceNum, FunctionNum);
	//xil_printf("Port Number is %02X\r\n", PortNumber);

	/* Read my configuration space */
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_ID_REG, &HeaderData);
	xil_printf("PCIe: \t Vendor ID/Device ID Register is %08X\r\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CMD_STATUS_REG, &HeaderData);
	xil_printf("PCIe: \t Command/Status Register is %08X\r\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_CAH_LAT_HD_REG, &HeaderData);
	xil_printf("PCIe: \t Header Type/Latency Timer Register is %08X\r\n", HeaderData);
	XAxiPcie_ReadLocalConfigSpace(XlnxEndPointPtr, PCIE_CFG_BAR_ZERO_REG, &HeaderData);
	xil_printf("PCIe: \t BAR 0 is %08X\r\n", HeaderData);


	XAxiPcie_GetEnabledInterrupts(XlnxEndPointPtr, &InterruptMask);
	xil_printf("Interrupts currently enabled are %8lX\r\n", InterruptMask);

	XAxiPcie_GetPendingInterrupts(XlnxEndPointPtr, &InterruptMask);
	xil_printf("Interrupts currently pending are %8lX\r\n", InterruptMask);

	return XST_SUCCESS;
}


int CDMA_Init(void) {
	int Status;
	XAxiCdma_Config* Cdma_Config = XAxiCdma_LookupConfig(XPAR_AXICDMA_0_DEVICE_ID);
	Status = XAxiCdma_CfgInitialize( &CdmaInstance, Cdma_Config, XPAR_AXICDMA_0_BASEADDR);
	if (Status != XST_SUCCESS){
		xil_printf( "DMA peripheral configuration failed %d\n", Status);
	}
	return Status;
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
	xil_printf("RxData: \t %02x %02x %02x %02x %02x %02x %02x %02x\r\n", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[124], rx_buffer[125], rx_buffer[126], rx_buffer[127]);

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


int DmaDataTransfer(u16 DeviceID) {

	int Status;
	volatile int Error;
	XAxiCdma_Config *ConfigPtr;
	Error = 0;

	XTmrCtr *InstancePtr = &p;

	u32 TimerCount1 = 0;
	u32 TimerCount2 = 0;

	if (SADDR0 == 0) return XST_FAILURE;
	if (DADDR0 == 0) return XST_FAILURE;

	ConfigPtr = XAxiCdma_LookupConfig(DeviceID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XAxiCdma_CfgInitialize(&CdmaInstance, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XAxiCdma_Reset(&CdmaInstance);

	XAxiCdma_IntrDisable(&CdmaInstance, XAXICDMA_XR_IRQ_ALL_MASK);

	// axi_timer
	XTmrCtr_Initialize(InstancePtr, 0);
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(0 < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TLR_OFFSET, 0);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_INT_OCCURED_MASK | XTC_CSR_LOAD_MASK);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_ENABLE_TMR_MASK);

	//start transfer
	XAxiCdma_SimpleTransfer(&CdmaInstance, SADDR0, DADDR0, Length0, 0, 0);

	TimerCount1 = XTmrCtr_ReadReg(InstancePtr->BaseAddress, 0, XTC_TCR_OFFSET);
	while (XAxiCdma_IsBusy(&CdmaInstance));
	TimerCount2 = XTmrCtr_ReadReg(InstancePtr->BaseAddress, 0, XTC_TCR_OFFSET);

	Error = XAxiCdma_GetError(&CdmaInstance);
	if (Error != 0x0) {
		xil_printf("AXI CDMA Transfer Error =  %8.8x\r\n");
		return XST_FAILURE;
	}

	xil_printf("AXI CDMA Transfer is complete without errors\r\n");
	xil_printf("-----------------------------------------------------\r\n");
	xil_printf(" - Counter	= %d\r\n", TimerCount2-TimerCount1);
	xil_printf(" - Length 	= %d Bytes\r\n", Length0);
	xil_printf(" - Speed 	= %d MByte/s\r\n", Length0*100/(TimerCount2-TimerCount1));
	xil_printf("-----------------------------------------------------\r\n");

	int i;
	for(i = 0; i < 256; i+=4){

		RD_WORD(DADDR0 + i, DataRead);
		xil_printf("Data from DDR:  %8x\r\n", DataRead);
	}

	return XST_SUCCESS;
}



#include "resources.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xaxicdma.h"
#include "xaxipcie.h"
#include "xtmrctr.h"
#include "xuartlite_l.h"
#include "xintc_l.h"
#include "xbram.h"

#define SADDR0 	XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR
#define DADDR0 	XPAR_DDR3_SDRAM_S_AXI_BASEADDR + 0x00000000
#define Length0 0xFFFF

static XAxiPcie XlnxPCIeEndPoint;
static XAxiCdma CdmaInstance;
static XTmrCtr p;
static XBram BramInstance;

volatile char Rx_byte;
char Rx_data[UART_MAX_LENGTH_BUFFER];
u8 Rx_indx = 0;

u32 buff[512];



int PCIe_Init(XAxiPcie *XlnxEndPointPtr);
int CDMA_Init(XAxiCdma *CdmaInstance);
int BRAM_Init(XBram *BramInstance);

int CDMA_Test(XAxiCdma *CdmaInstance, XTmrCtr *InstancePtr);
int CDMA_Transfer(XAxiCdma *CdmaInstance, XTmrCtr *InstancePtr, u32 TX, u32 RX, u16 Length);


/* Функция обратного вызова UART */
void uart_handler(void *baseaddr_p) {
	while (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {

		Rx_byte = XUartLite_RecvByte(UART_BASEADDR);

		if (Rx_byte == '\r'){

			if (strncmp(Rx_data, "test", Rx_indx) == 0) {
				xil_printf ("test cmd\n");
				CDMA_Test(&CdmaInstance, &p);
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

	status = BRAM_Init(&BramInstance);
	if (status != XST_SUCCESS){
		xil_printf("%c[1;31m[ E ] BRAM is not working properly %c[0m\n", 27, 27);
		return XST_FAILURE;
	}




    /* Конфигурация прерываний для UART */
	XIntc_RegisterHandler(INTC_BASEADDR, UART_INTERRUPT_INTR, (XInterruptHandler)uart_handler, (void *)UART_BASEADDR);
	XIntc_MasterEnable(INTC_BASEADDR);
	XIntc_EnableIntr(INTC_BASEADDR, UART_INTERRUPT_MASK);
	XUartLite_EnableIntr(UART_BASEADDR);












	return XST_SUCCESS;
}


int BRAM_Init(XBram *BramInstance) {
	// int status;
	// XBram_Config *ConfigPtr;

	// ConfigPtr = XBram_LookupConfig(0);
	// if (ConfigPtr == (XBram_Config *) NULL) {
	// 	return XST_FAILURE;
	// }

	// status = XBram_CfgInitialize(BramInstance, ConfigPtr, XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR);
	// if (status != XST_SUCCESS) {
	// 	return XST_FAILURE;
	// }

	// status = XBram_SelfTest(BramInstance, 0);
	// if (status != XST_SUCCESS) {
	// 	return XST_FAILURE;
	// }



	xil_printf("\n");
//	u32 Addr;
//	volatile u8 Data;


	u8 *BufferPtr = (u8*)XPAR_BRAM_0_BASEADDR;



	u32 cnt = 0;
	u8 src[64];
	 for (cnt = 0; cnt < sizeof cnt; cnt++) {
		src[cnt]=0x55;
	 }

	cnt = 0;
//    unsigned char dst[64];

	memcpy(BufferPtr, &src , sizeof src);


	 for (cnt = 0; cnt < sizeof src; cnt++) {
		xil_printf("%d SRC Data: %02x\n", cnt, src[cnt]);
		xil_printf("%d BRAM Data: %02x\n", cnt, BufferPtr[cnt]);
	 }

		// XBram_Out8(Addr, cnt);
		// xil_printf("%08x BRAM RD Data: %02x\n", Addr, cnt);
		// cnt++;
	// }






//	xil_printf("BRAM MEM ADDR BASE: %08x\n", ConfigPtr->MemBaseAddress);
//	xil_printf("BRAM MEM ADDR HIGH: %08x\n", ConfigPtr->MemHighAddress - 0x7F00);
//
//	for (Addr = ConfigPtr->MemBaseAddress; Addr < (ConfigPtr->MemHighAddress - 0x7F00); Addr++) {
//		XBram_Out8(Addr, cnt);
//		xil_printf("%08x BRAM RD Data: %02x\n", Addr, cnt);
//		cnt++;
//	}
//
//
//	for (Addr = ConfigPtr->MemBaseAddress; Addr < (ConfigPtr->MemHighAddress - 0x7F00); Addr++) {
//		Data = XBram_In8(Addr);
//		xil_printf("%08x BRAM RD Data: %02x\n", Addr, Data);
//	}


	xil_printf("BRAM Transfer is completed\n");
	// cnt = XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR;
	// u32 b_ct = 0;

	// for (cnt = XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR; cnt < 256; cnt ++) {

	// 	buff[b_ct] = XBram_In8(cnt);
	// 	b_ct++;
	// 	xil_printf("%d BRAM RD Data: %%04lx\n", b_ct, buff);
	// }










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

/*
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
*/

int CDMA_Transfer(XAxiCdma *CdmaInstance, XTmrCtr *InstancePtr, u32 TX, u32 RX, u16 Length) {

	int status;

	u32 TimerCount1 = 0;
	u32 TimerCount2 = 0;

	// axi_timer
	XTmrCtr_Initialize(InstancePtr, 0);
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(0 < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TLR_OFFSET, 0);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_INT_OCCURED_MASK | XTC_CSR_LOAD_MASK);
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 0, XTC_TCSR_OFFSET, XTC_CSR_ENABLE_TMR_MASK);

	//start transfer
	status = XAxiCdma_SimpleTransfer(CdmaInstance, TX, RX, Length, NULL, NULL);
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

	xil_printf("Size: %d B / Speed: %d MB/s\n", Length, Length*100/(TimerCount2-TimerCount1));
	xil_printf("Transfer is completed\n");

	return XST_SUCCESS;
}


int CDMA_Test(XAxiCdma *CdmaInstance, XTmrCtr *InstancePtr){

	int Status;



	Status = CDMA_Transfer(CdmaInstance, InstancePtr, (u32)buff, DADDR0, Length0);
	if (Status != XST_SUCCESS) {
		xil_printf("%c[1;31mError when initializing CDMA, code: %d%c[0m\n", 27, Status, 27);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

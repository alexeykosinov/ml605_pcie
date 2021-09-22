#include "dma.h"

int DMA_Init(XAxiDma *DmaInstance) {
	int status;
	XAxiDma_Config* ConfigPtr;

	ConfigPtr = XAxiDma_LookupConfig(0);
	if (ConfigPtr == NULL) {
		xil_printf("[ E ] DMA: Configuration couldn't be found\n");
		return XST_FAILURE;
	}

	status = XAxiDma_CfgInitialize(DmaInstance, ConfigPtr);
	if (status != XST_SUCCESS) {
		xil_printf("[ E ] DMA: Failed to configure instance\n");
		return XST_FAILURE;
	}

	/* Scatter-Gather is enable? */
	if (!XAxicDma_HasSg(DmaInstance)) {
		xil_printf("[ E ] DMA: Device configured as Simple Mode\n");
		return XST_FAILURE;
	}

	return status;
}

void DMA_PrintInfo(XAxiDma *DmaInstance) {
	xil_printf("[ I ] MM2S DMA Control Register        : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_MM2S_DMACR));
	xil_printf("[ I ] MM2S DMA Status Register         : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_MM2S_DMASR));
	xil_printf("[ I ] MM2S Current Descriptor Pointer  : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_MM2S_CURDESC));
	xil_printf("[ I ] MM2S Tail Descriptor Pointer     : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_MM2S_TAILDESC));
	xil_printf("[ I ] Scatter/Gather User and Cache    : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_SG_CTL));
	xil_printf("[ I ] S2MM DMA Control Register        : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_S2MM_DMACR));
	xil_printf("[ I ] S2MM DMA Status Register         : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_S2MM_DMASR));
	xil_printf("[ I ] S2MM Current Descriptor Pointer  : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_S2MM_CURDESC));
	xil_printf("[ I ] S2MM Tail Descriptor Pointer     : 0x%08X\n", Xil_In32(DmaInstance + AXI_DMA_S2MM_TAILDESC));
}

void DMA_SG_Test(u32 *TxBuffer, u32 *RxBuffer, u16 BufferSize, u16 DescCount){

	SGDesc RxDesc[DescCount];
	SGDesc TxDesc[DescCount];

	u32 DescAddr;
	u32 status;
	u32 countWait = 0x0;
	u16 desc;
	u32 i;

	Xil_DCacheDisable();

	for (desc = 0; desc < DescCount; desc++) {
		for (i = 0; i < BufferSize; i++) {
			TxBuffer[desc][i] = desc + i;
		}
	}

	for (i = 0; i < DescCount; i++) {
		TxDesc[i].NXTDESC = (u32)&TxDesc[i];
		TxDesc[i].NXTDESC_MSB = 0x0;
		TxDesc[i].BUFFER_ADDRESS = (u32)&TxBuffer[i][0];
		TxDesc[i].BUFFER_ADDRESS_MSB = 0x0;
		TxDesc[i].RESERVED0 = 0x0;
		TxDesc[i].RESERVED1 = 0x0;
		TxDesc[i].CONTROL = 0xC000000 + sizeof(TxBuffer[i]);
		TxDesc[i].STATUS = 0x0;
		TxDesc[i].APP0 = 0x0;
		TxDesc[i].APP1 = 0x0;
		TxDesc[i].APP2 = 0x0;
		TxDesc[i].APP3 = 0x0;
		TxDesc[i].APP4 = 0x0;
	}

	/* Скопируем дескрипторы передачи в память дескрипторов, которая расположена в программируемой логике */
	DescAddr = 0x40C08000;
	for (i = 0; i < DescCount; i++) {
		memcpy((u32 *)DescAddr, TxBuffer, sizeof(TxBuffer)*BUF_LENGTH);
		DescAddr += 0x40;
	}

	/* Запишем указатель на следующий элемент в списке дескрипторов */
	DescAddr = 0x40C08000;
	for (i = 0; i < DescCount - 1; i++) {
		Xil_Out32((u32 *)DescAddr, DescAddr + 0x40);
		DescAddr += 0x40;
	}

	Xil_Out32(DescAddr, DescAddr);

	/** Fill descriptor to receive */
	for (i = 0; i < DescCount; i++) {
		RxDesc[i].NXTDESC = (u32)&RxDesc[i];
		RxDesc[i].NXTDESC_MSB = 0x0;
		RxDesc[i].BUFFER_ADDRESS = (u32)&RxBuffer[i][0];
		RxDesc[i].BUFFER_ADDRESS_MSB = 0x0;
		RxDesc[i].RESERVED0 = 0x0;
		RxDesc[i].RESERVED1 = 0x0;
		RxDesc[i].CONTROL = sizeof(RxBuffer[i]);
		RxDesc[i].STATUS = 0x0;
		RxDesc[i].APP0 = 0x0;
		RxDesc[i].APP1 = 0x0;
		RxDesc[i].APP2 = 0x0;
		RxDesc[i].APP3 = 0x0;
		RxDesc[i].APP4 = 0x0;
	}

	/** Copy receive descriptor for memory of descriptors */
	DescAddr = 0x40C08000 + 0x4000;
	for (i = 0; i < DescCount; i++) {
		memcpy((u32 *)DescAddr, &RxDesc[i], sizeof(RxDesc[i]));
		DescAddr += 0x40;
	}

	/** Write pointer to next pointer */
	DescAddr = 0x40C08000 + 0x4000;
	for (i = 0; i < DescCount - 1; i++) {
		Xil_Out32(DescAddr, DescAddr + 0x40);
		DescAddr += 0x40;
	}

	/** Write pointer for last descriptor */
	Xil_Out32(DescAddr, DescAddr);

    Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_MM2S_DMACR, 0x0001DFE6); // Toggle reset 
    Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_MM2S_DMACR, 0x0001DFE2); // Toggle reset 
    Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_S2MM_DMACR, 0x0001DFE6); // Toggle reset 
    Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_S2MM_DMACR, 0x0001DFE2); // Toggle reset 

	Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_S2MM_CURDESC,  0x40004000); 	// S2MM_CURDESC
	Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_S2MM_DMACR,    0x0001DFE3); 	// S2MM_DMACR
	Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_S2MM_TAILDESC, 0x40007FC0); 	// S2MM_TAILDESC

	Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_MM2S_CURDESC,  0x40000000); 	// MM2S_CURDESC
	Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_MM2S_DMACR,    0x0001DFE3); 	// MM2S_DMACR
	Xil_Out32(XPAR_AXIDMA_0_BASEADDR + AXI_DMA_MM2S_TAILDESC, 0x40003FC0); 	// MM2S_TAILDESC

    /* Wait ready in last descriptor */
    while(1) {
    	status = Xil_In32(0x40003FDC);
    	if ((status & 0x80000000) == 0x80000000) {
    		break;
    	}
    	else {
    		countWait++;
    		for(i = 0; i < 0x000F000; i++) { }
    	}
    }
    xil_printf("Time %d\n", countWait);

	Xil_DCacheEnable();
}

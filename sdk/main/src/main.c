#include "xil_printf.h"
#include "xgpio.h"
#include "resources.h"

#define AXIPCIE_DEVICE_ID 		XPAR_AXIPCIE_0_DEVICE_ID
#define AXIDMA_DEVICE_ID		XPAR_AXICDMA_0_DEVICE_ID

XGpio GpioOutput;
XAxiPcie XlnxEndPoint_0;


void GPIO_Blinker(u8 GpioWidth);






int main() {

    //unsigned int cnt = 0;
	int Status;

	init_platform();

	//xil_printf("%c[2J", 27); // Clear the screen
    xil_printf("%c[1;32mHello, Asshole!%c[0m\n", 27, 27); // in green color
    xil_printf("Let's go!\n\r");

	Status = PCIeEndPointInitialize(&XlnxEndPoint_0, AXIPCIE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("%c[1;31mError when initializing PCIe device, code: %d%c[0m\n", 27, Status, 27); // in red color
		return XST_FAILURE;
	}

	CDMA_Init();
	//CDMA_Transfer();

	Status = DmaDataTransfer(AXIDMA_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("%c[1;31mError when initializing CDMA, code: %d%c[0m\n", 27, Status, 27);
		return XST_FAILURE;
	}

	xil_printf("Successfully ran PCIe End-Point CDMA Example\r\n");

    for (;;){
    	GPIO_Blinker(8);
//      xil_printf("Bang! @ %c[1;33m%d%c[0m\n", 27, cnt, 27); // Yellow
//    	xil_printf("%c[2K", 27); // Clear current line
//    	xil_printf("%c[A", 27);  // Return position to the current line
//      cnt++;
    }

    cleanup_platform();
    return 0;
}






void GPIO_Blinker(u8 GpioWidth) {

  u8 LedBit;
  u8 LedLoop;
  unsigned int i;
  int numTimes = 1;

  XGpio_Initialize(&GpioOutput, 0);				// Initialization
  XGpio_SetDataDirection(&GpioOutput, 1, 0x0); 	// Set the direction for all signals to be outputs
  XGpio_DiscreteWrite(&GpioOutput, 1, 0x0); 	// Set the GPIO outputs to low

  while (numTimes > 0) {
      for (LedBit = 0x0; LedBit < GpioWidth; LedBit++) {
          for (LedLoop = 0; LedLoop < 1; LedLoop++) {
			XGpio_DiscreteWrite(&GpioOutput, 1, 1 << LedBit); // Set the GPIO Output to High
			for(i = 0; i < 0x00F0000; i++){}

            XGpio_DiscreteClear(&GpioOutput, 1, 1 << LedBit); //  Clear the GPIO Output
            for(i = 0; i < 0x00F0000; i++){}
          }
      }
      numTimes--;
  }
}

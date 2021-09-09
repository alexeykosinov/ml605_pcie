#include "xil_printf.h"
#include "xgpio.h"
#include "resources.h"

XGpio GpioOutput;

void GPIO_Blinker(u8 GpioWidth);

int main() {

	int Status;

	Status = init_platform();
	if (Status != XST_SUCCESS) {
		xil_printf("%c[1;31mFailed when initializing platform, code: %d%c[0m\n", 27, Status, 27);
		return XST_FAILURE;
	}
    
   xil_printf("\n");
   xil_printf("%c[1;32m= Configuration completed ===================================%c[0m\n", 27, 27);
   xil_printf("\n");

   xil_printf("To run CDMA test type 'test'\n");



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

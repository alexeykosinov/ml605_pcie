#include "xil_printf.h"
#include "xgpio.h"
#include "resources.h"



int main() {

	int Status;

	Status = init_platform();
	if (Status != XST_SUCCESS) {
		xil_printf("%c[1;31m[ E ] Failed when initializing platform %c[0m\n", 27, 27);
		return XST_FAILURE;
	}

    
    xil_printf("%c[1;32m[ I ] Configuration completed %c[0m\n", 27, 27);
    xil_printf("\n");

    xil_printf("[ I ] Command palette:\n");
    xil_printf("[ I ] 'rbram' read BRAM data\n");
    xil_printf("[ I ] 'wbram' write BRAM data\n");
    xil_printf("[ I ] 'rddr'  read DDR data\n");
    xil_printf("[ I ] 'wddr'  write DDR data\n");
    xil_printf("\n");

    for (;;){
    	led_blinker(4);
//      xil_printf("Bang! @ %c[1;33m%d%c[0m\n", 27, cnt, 27); // Yellow
//    	xil_printf("%c[2K", 27); // Clear current line
//    	xil_printf("%c[A", 27);  // Return position to the current line
//      cnt++;
    }

    cleanup_platform();
    return 0;
}







#ifndef __RESOURCES_H_
#define __RESOURCES_H_

#include "xaxipcie.h"		/* XAxiPcie interface */
#include "xaxicdma.h"		/* AXICDMA interface */

void init_platform();
void cleanup_platform();

int PCIeEndPointInitialize(XAxiPcie *XlnxEndPointPtr, u16 DeviceId);
int DmaDataTransfer(u16 CdmaID);

int CDMA_Init();
void CDMA_Transfer();

int DmaDataTransfer(u16 DeviceID);

#endif

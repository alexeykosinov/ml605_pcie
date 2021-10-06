#ifndef __DMA_H_
#define __DMA_H_

#include "xbasic_types.h"
#include "xaxicdma.h"

#define AXI_DMA_MM2S_DMACR      0x00        /* MM2S DMA Control Register        */
#define AXI_DMA_MM2S_DMASR      0x04        /* MM2S DMA Status Register         */
#define AXI_DMA_MM2S_CURDESC    0x08        /* MM2S Current Descriptor Pointer  */
#define AXI_DMA_MM2S_TAILDESC   0x10        /* MM2S Tail Descriptor Pointer     */
#define AXI_DMA_SG_CTL          0x2C        /* Scatter/Gather User and Cache    */
#define AXI_DMA_S2MM_DMACR      0x30        /* S2MM DMA Control Register        */
#define AXI_DMA_S2MM_DMASR      0x34        /* S2MM DMA Status Register         */
#define AXI_DMA_S2MM_CURDESC    0x38        /* S2MM Current Descriptor Pointer  */
#define AXI_DMA_S2MM_TAILDESC   0x40        /* S2MM Tail Descriptor Pointer     */

#pragma pack (1)

typedef struct SGDesc {
	u32 NXTDESC;            // адрес следующего дескриптора;
	u32 NXTDESC_MSB;        // старшие 32 бита адреса следующего дескриптора;
	u32 BUFFER_ADDRESS;     // адрес буфера;
	u32 BUFFER_ADDRESS_MSB; // старшие 32 бита адреса буфера;
	u32 RESERVED[2];		// не используется;
	u32 CONTROL;            // задает размер буфера, признаки начала и конца пакета;
	u32 STATUS;             // показывает, сколько байт принято/передано, обработан/не обработан;
	u32 APP[5];             // используется для работы с каналом «Control/Status Stream»;
	u32 PADDING[3];			// выравнивание до 64 Б;
} SGDesc;

void DMA_PrintInfo();
int DMA_Init(XAxiCdma *DmaInstance);
void DMA_SG_Test(u32 *TxBuffer[], u32 *RxBuffer[], u16 BufferSize, u16 DescCount);

#endif




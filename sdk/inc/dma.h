/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xdebug.h"
#include "xaxidma.h"
#include "xil_exception.h"
#include "xscugic.h"

#include "Fixed.h"
#include "Layer.h"

/******************** Constant Definitions **********************************/
/*
 * Device hardware build related constants.
 */

#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#define RX_INTR_ID		XPAR_FABRIC_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_FABRIC_AXIDMA_0_MM2S_INTROUT_VEC_ID

/* Timeout loop counter for reset
 */
#define RESET_TIMEOUT_COUNTER	10000

#define DELAY_TIMER_COUNT		100

#define INTC			XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
int DMA_Init(void);
int DMA_IntrSetting(XScuGic *IntrCtrler, XAxiDma *AxiDmaPtr, u16 TxIntrId, u16 RxIntrId);
int TinyYolo_SendPacket(fixed *Addr, uint32_t FloatNum);
int TinyYolo_RxSetUp(fixed *Addr, Layer_t *LayerNow);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
extern XAxiDma AxiDma;

extern Layer_t *RxLayer;
extern unsigned int RxChNow;

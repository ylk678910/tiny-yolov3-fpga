/*
 * TinyYoloCtrl.h
 *
 *  Created on: 2021年8月21日
 *      Author: ylk67
 */

#ifndef SRC_TINYYOLOCTRL_H_
#define SRC_TINYYOLOCTRL_H_

#include "Fixed.h"
#include "Layer.h"

/************************** Constant Definitions *****************************/
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
//#define TINYYOLO_INTR_ID	0x0E
#define TINYYOLO_INTR_ID	XPAR_FABRIC_TINYYOLOV3_V1_1_0_IRQ_INTR
#define TINYYOLOV3_BASEADDR XPAR_TINYYOLOV3_V1_1_0_BASEADDR
#define TINYYOLOV3_HIGHADDR XPAR_TINYYOLOV3_V1_1_0_HIGHADDR

//TinyYolo IP Memory Offset
#define INTR_GLOBAL_ENABLE_REG	0x00	//(w/r)
#define INTR_ENABLE_REG 		0x04	//(w/r)
#define INTR_ACK_REG	 		0x08	//(w/r)
#define INTR_PENDING_REG		0x0C	//(r)
#define INTR_DATA_PARAM 		0x10	//(r)
#define INTR_RECV_PARAM 		0x14	//(r)
#define INTR_SEND_PARAM 		0x18	//(r)
#define CTRL_REG 				0x1C	//(w/r)

//TinyYolo IP Interrupt Mask
#define INTR_IP_RECEIVER_MASK 			0x00000001
#define INTR_IP_SENDER_MASK 			0x00000002
#define INTR_IP_DONE_MASK 				0x00000004

//TinyYolo IP INTR_DATA_PARAM Mask
//TinyYolo IP INTR_DATA_PARAM Offset

//TinyYolo IP INTR_RECV_PARAM Mask
#define INTR_IP_RECV_DATA_TYPE_MASK  	0x00018000
#define INTR_IP_RECV_START_MASK			0x00007FE0
#define INTR_IP_RECV_LAYER_MASK       	0x0000001F
//TinyYolo IP INTR_RECV_PARAM Offset
#define INTR_IP_RECV_DATA_TYPE_OFFSET 	15
#define INTR_IP_RECV_START_OFFSET     	5
#define INTR_IP_RECV_LAYER_OFFSET     	0


//TinyYolo IP INTR_RECV_PARAM Mask
#define INTR_IP_SEND_LAYER_MASK      	0x0000001F
//TinyYolo IP INTR_RECV_PARAM Offset
#define INTR_IP_SEND_LAYER_OFFSET		0

//TinyYolo IP Control (CTRL_REG) Mask
#define TINY_YOLO_START_MASK			0x00000001

//TinyYolo IP Receiver DataType Define
#define TINY_YOLO_NEW_IMG 		0
#define TINY_YOLO_STORED_DATA 	1
#define TINY_YOLO_WEIGHT 		2

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void TinyYolo_WriteReg(UINTPTR Offset, u32 Value);
u32 TinyYolo_ReadReg(UINTPTR Offset);
int TinyYolo_Mask(uint32_t Data, uint32_t Mask, uint32_t Offset);

int TinyYolo_StartDetect(void);

int TinyYolo_Init(void);
int TinyYolo_InitConfig(void);
int TinyYolo_IntrSetup(void);
int TinyYolo_Reset(void);

int TinyYolo_ConvCMP(void);

int isChMemNumAligned(Layer_t *Layer);
unsigned int AlignedMemNum_ch(Layer_t *Layer);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
extern XScuGic IntrController; 	    	 /* Instance of the Interrupt Controller */
extern unsigned int StoredDataRequestError_flg;
#endif /* SRC_TINYYOLOCTRL_H_ */

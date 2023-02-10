/******************************************************************************
 *
 * Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xscugic_example.c
 *
 * This file contains a design example using the Interrupt Controller driver
 * (XScuGic) and hardware device. Please reference other device driver examples
 * to see more examples of how the intc and interrupts can be used by a software
 * application.
 *
 * @note
 *
 * None
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------
 * 1.00a drg  01/18/10 First release
 * </pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include "xil_io.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xscugic.h"
#include "xaxidma.h"

#include "MemoryAddress.h"
#include "Video.h"
#include "TinyYoloCtrl.h"
#include "dma.h"
#include "Layer.h"

/************************** Function Prototypes ******************************/
int IntrSetup(XScuGic *IntrCtrler);
int SetUpInterruptSystem(XScuGic *IntrCtrler);

int DMA_IntrSetting(XScuGic *IntrCtrler, XAxiDma * AxiDmaPtr, u16 TxIntrId,
		u16 RxIntrId);
int TinyYolo_IntrSetting(XScuGic *IntrCtrler);

void TinyYolo_IntrHandler(void *CallbackRef);
int DoneIRQ_CallBack(void);
int RecvIRQ_CallBack(void);
int SendIRQ_CallBack(void);

/************************** Variable Definitions *****************************/
XScuGic IntrController; /* Instance of the Interrupt Controller */
static XScuGic_Config *GicConfig; /* The configuration parameters of the controller */

unsigned int LayerMemDataBusy = 0;
unsigned int StoredDataRequestError_flg = 0;
/***************************** Function Start ********************************/
static void AssertPrint(const char8 *FilenamePtr, s32 LineNumber) {
	printf("ASSERT: File Name: %s ", FilenamePtr);
	printf("Line Number: %d\r\n", (int) LineNumber);
}

void TinyYolo_WriteReg(UINTPTR Offset, u32 Value) {
	Xil_Out32(TINYYOLOV3_BASEADDR + Offset, Value);
}

u32 TinyYolo_ReadReg(UINTPTR Offset) {
	return Xil_In32(TINYYOLOV3_BASEADDR + Offset);
}

int TinyYolo_Mask(uint32_t Data, uint32_t Mask, uint32_t Offset) {
	return (Data & Mask) >> Offset;
}

int TinyYolo_StartDetect(void) {
	int Status = 0;

	//尚未驗證正確性
	//Set all mod_biases to layer memory.
	Status = TinyYolo_Reset();
	if (Status != XST_SUCCESS) {
		printf("IP Initialize failed.\r\n");
		return XST_FAILURE;
	}

	Status = TinyYolo_ReadReg(CTRL_REG);
	if (Status != 0) {
		printf("Start a new detection failed. IP is busy.\r\n");
		return XST_FAILURE;
	}
	TinyYolo_WriteReg(CTRL_REG, 0x1);
	return XST_SUCCESS;
}

int TinyYolo_Init(void) {
	int Status;
	Status = TinyYolo_InitConfig();
	if (Status != XST_SUCCESS) {
		printf("IP Configuration setup failed.\r\n");
		return XST_FAILURE;
	}

	Status = TinyYolo_IntrSetup();
	if (Status != XST_SUCCESS) {
		printf("IP Interrupt setup failed.\r\n");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

int TinyYolo_InitConfig(void) {
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This is the main function for the Interrupt Controller example.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 ****************************************************************************/
int TinyYolo_IntrSetup(void) {
	int Status;

	/*
	 * Setup an assert call back to get some info if we assert.
	 */
	Xil_AssertSetCallback(AssertPrint);

	/*
	 *  Run the Gic example , specify the Device ID generated in xparameters.h
	 */
	Status = IntrSetup(&IntrController);
	if (Status != XST_SUCCESS) {
		printf("GIC Example Test Failed\r\n");
		return XST_FAILURE;
	}

	//Enable IP global interrupt.
	TinyYolo_WriteReg(INTR_GLOBAL_ENABLE_REG, 0x1);
	//Enable IP receiver, sender and done interrupt.
	TinyYolo_WriteReg(INTR_ENABLE_REG,
	INTR_IP_RECEIVER_MASK | INTR_IP_SENDER_MASK | INTR_IP_DONE_MASK);

	return XST_SUCCESS;
}

int TinyYolo_IntrSetting(XScuGic *IntrCtrler) {
	int Status;
	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */

	//TinyYolo
	XScuGic_SetPriorityTriggerType(IntrCtrler, TINYYOLO_INTR_ID, 0xA8, 0x3);
	Status = XScuGic_Connect(IntrCtrler, TINYYOLO_INTR_ID,
			(Xil_ExceptionHandler) TinyYolo_IntrHandler, (void *) IntrCtrler);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_Enable(IntrCtrler, TINYYOLO_INTR_ID);

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 *
 * This function is an example of how to use the interrupt controller driver
 * (XScuGic) and the hardware device.  This function is designed to
 * work without any hardware devices to cause interrupts. It may not return
 * if the interrupt controller is not properly connected to the processor in
 * either software or hardware.
 *
 * This function relies on the fact that the interrupt controller hardware
 * has come out of the reset state such that it will allow interrupts to be
 * simulated by the software.
 *
 * @param	DeviceId is Device ID of the Interrupt Controller Device,
 *		typically XPAR_<INTC_instance>_DEVICE_ID value from
 *		xparameters.h
 *
 * @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE
 *
 * @note		None.
 *
 ******************************************************************************/
int IntrSetup(XScuGic *IntrCtrler) {
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	GicConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntrCtrler, GicConfig,
			GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	Status = XScuGic_SelfTest(IntrCtrler);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */
	//TinyYolo
	Status = TinyYolo_IntrSetting(IntrCtrler);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//DMA
	Status = DMA_IntrSetting(IntrCtrler, &AxiDma, TX_INTR_ID, RX_INTR_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 *  Simulate the Interrupt
	 */
	/*
	 Status = XScuGic_SoftwareIntr(&IntrCtrler,
	 TINYYOLO_INTR_ID,
	 XSCUGIC_SPI_CPU0_MASK);
	 if (Status != XST_SUCCESS) {
	 return XST_FAILURE;
	 }
	 */

	/*
	 * Setup the Interrupt System
	 */
	Status = SetUpInterruptSystem(IntrCtrler);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 *
 * This function connects the interrupt handler of the interrupt controller to
 * the processor.  This function is seperate to allow it to be customized for
 * each application.  Each processor or RTOS may require unique processing to
 * connect the interrupt handler.
 *
 * @param	IntrCtrler is the instance of the interrupt controller
 *		that needs to be worked on.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
int SetUpInterruptSystem(XScuGic *IntrCtrler) {

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler, IntrCtrler);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 *
 * This function is designed to look like an interrupt handler in a device
 * driver. This is typically a 2nd level handler that is called from the
 * interrupt controller interrupt handler.  This handler would typically
 * perform device specific processing such as reading and writing the registers
 * of the device to clear the interrupt condition and pass any data to an
 * application using the device driver.  Many drivers already provide this
 * handler and the user is not required to create it.
 *
 * @param	CallbackRef is passed back to the device driver's interrupt
 *		handler by the XScuGic driver.  It was given to the XScuGic
 *		driver in the XScuGic_Connect() function call.  It is typically
 *		a pointer to the device driver instance variable.
 *		In this example, we do not care about the callback
 *		reference, so we passed it a 0 when connecting the handler to
 *		the XScuGic driver and we make no use of it here.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
uint32_t Data, Ack = 0;
uint32_t Receiver_irq, Sender_irq, Done_irq;
uint32_t Recv_Layer, Recv_NeedDataType, Recv_StartCh;
uint32_t Send_Layer, Send_NeedDataType;

void TinyYolo_IntrHandler(void *CallbackRef) {
	int Status;
	printf("----------------------Got Interrupt----------------------\r\n");

	Data = TinyYolo_ReadReg(INTR_PENDING_REG);
	Receiver_irq = TinyYolo_Mask(Data, INTR_IP_RECEIVER_MASK, 0);
	Sender_irq = TinyYolo_Mask(Data, INTR_IP_SENDER_MASK, 0);
	Done_irq = TinyYolo_Mask(Data, INTR_IP_DONE_MASK, 0);

	if (Done_irq) {
		printf("Get done interrupt!!\r\n");
		Ack = Ack | INTR_IP_DONE_MASK;
	}

	if (Receiver_irq) {
		printf("Read receiver state.\r\n");

		Data = TinyYolo_ReadReg(INTR_RECV_PARAM);
		Recv_Layer = TinyYolo_Mask(Data, INTR_IP_RECV_LAYER_MASK,
		INTR_IP_RECV_LAYER_OFFSET);
		Recv_NeedDataType = TinyYolo_Mask(Data, INTR_IP_RECV_DATA_TYPE_MASK,
		INTR_IP_RECV_DATA_TYPE_OFFSET);
		Recv_StartCh = TinyYolo_Mask(Data, INTR_IP_RECV_START_MASK,
		INTR_IP_RECV_START_OFFSET);

		Ack = Ack | INTR_IP_RECEIVER_MASK;
	}

	if (Sender_irq) {
		printf("Read sender state.\r\n");

		Data = TinyYolo_ReadReg(INTR_SEND_PARAM);
		Send_Layer = TinyYolo_Mask(Data, INTR_IP_SEND_LAYER_MASK,
		INTR_IP_SEND_LAYER_OFFSET);
		Ack = Ack | INTR_IP_SENDER_MASK;

		//Callback
		Status = SendIRQ_CallBack();
		if (Status != XST_SUCCESS) {
			printf("Error at SendIRQ_CallBack. Status:%d\r\n", Status);
		}
	}

	//Send acknowledge
	TinyYolo_WriteReg(INTR_ACK_REG, Ack); //Send ACK

	//Callback
	if (Done_irq) {
		Status = DoneIRQ_CallBack();
		if (Status != XST_SUCCESS) {
			printf("Error at DoneIRQ_CallBack. Status:%d\r\n", Status);
		}
	}
	if (Receiver_irq) {
		Status = RecvIRQ_CallBack();
		if (Status != XST_SUCCESS) {
			printf("Error at RecvIRQ_CallBack. Status:%d\r\n", Status);
		}
	}
}

int DoneIRQ_CallBack(void) {
	printf("Done Callback\r\n");

	int CtrlState = 0;
	CtrlState = TinyYolo_ReadReg(CTRL_REG);
	if (CtrlState != 0) {
		printf("Start a new detection failed. IP is busy.\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int RecvIRQ_CallBack(void) {
	int Status;
	fixed *Addr;
	uint32_t FixedNum;
	Layer_t *LayerNow = GetLayer(Recv_Layer);

	printf("Receiver Callback\r\n");

	switch (Recv_NeedDataType) {
	case TINY_YOLO_NEW_IMG:
		printf("    Need Data Type: New Image\r\n");
		printf("    StartCh: %4u\r\n", (unsigned int) Recv_StartCh);
		Addr = AddFixedPtr(Img_NowImg(),
				(Recv_StartCh * LayerData[0].size_in_per_ch) * FIXED_SIZE);
		FixedNum = LayerData[0].size_in_per_ch;
		printf(
				"    Start sending data to PL. Addr(Char): %08x, DataNum(fixed): %u, DataNum(char): %u\r\n",
				(unsigned int) Addr, (unsigned int) FixedNum,
				(unsigned int) (FixedNum * FIXED_SIZE));
		printf("    First Data: 0x%04x\r\n", Addr[0]);
		break;
	case TINY_YOLO_STORED_DATA:
		printf("    Need Data Type: Stored Data\r\n");
		printf("    Layer: %2u, Recv_StartCh: %4u\r\n",
				(unsigned int) Recv_Layer, (unsigned int) Recv_StartCh);

		if (LayerMemDataBusy == 1) {
			printf(
					"Error at RecvIRQ_CallBack. LayerMemDataBusy = 1 but wants to get memory data.\r\n");
			printf(
					"Please wait until conv computing(TinyYolo_ConvCMP) finished.\r\n");
			StoredDataRequestError_flg = 1;
			return XST_FAILURE;
		}

		Addr = AddFixedPtr(LayerNow->mem,
				Recv_StartCh * AlignedMemNum_ch(LayerNow));
		FixedNum = LayerNow->size_in_per_ch;
		printf(
				"    Start sending data to PL. Addr(Char): %08x, DataNum(fixed): %u, DataNum(char): %u\r\n",
				(unsigned int) Addr, (unsigned int) FixedNum,
				(unsigned int) (FixedNum * FIXED_SIZE));
		printf("    First Data: 0x%04x\r\n", Addr[0]);
		break;
	case TINY_YOLO_WEIGHT:
		printf("    Need Data Type: Weight");
		printf("    Layer: %2u, Recv_StartCh: %4u\r\n",
				(unsigned int) Recv_Layer, (unsigned int) Recv_StartCh);

		Addr = AddFixedPtr(LayerNow->weight,
				(Recv_StartCh * AlignedWeightNum_ch(LayerNow)) * FIXED_SIZE);
		FixedNum = LayerNow->weight_per_ch;

		printf(
				"    Start sending data to PL. Addr(Char): %08x, DataNum(fixed): %u, DataNum(char): %u\r\n",
				(unsigned int) Addr, (unsigned int) FixedNum,
				(unsigned int) (FixedNum * FIXED_SIZE));
		printf("    First Data: 0x%04x\r\n", Addr[0]);
		break;
	default:
		printf("    Error at receiver interrupt.\n");
		return XST_FAILURE;
	}

	Status = TinyYolo_SendPacket(Addr, FixedNum);
	if (Status != XST_SUCCESS) {
		printf("Error at TinyYolo_SendPacket. Status:%d\r\n", Status);
		return XST_FAILURE;
	}
	return XST_SUCCESS;

}

int SendIRQ_CallBack(void) {
	int Status;
	fixed *Addr;
	uint32_t FixedNum;
	Layer_t *LayerNow = GetLayer(Send_Layer);
//改成接收到後就相加 全部相加完後才能給IP下一層資料
	printf("Sender Callback\r\n");

	printf("    Data Type: Stored Data\r\n");
	printf("    Layer: %2u\r\n", (unsigned int) Send_Layer);

	if (HasWeight(LayerNow)) {
		Addr = (fixed *) CPU_TMP_ADDR;
	} else {
		Addr = (fixed *) LayerNow->mem;
	}
	FixedNum = LayerNow->size_out;
	printf(
			"    Start receive data from PL. Addr(Char): %08x, DataNum(fixed): %u, DataNum(char): %u\r\n",
			(unsigned int) Addr, (unsigned int) FixedNum,
			(unsigned int) (FixedNum * 2));

	Status = TinyYolo_RxSetUp(Addr, LayerNow);
	if (Status != XST_SUCCESS) {
		printf("Error at TinyYolo_SendPacket. Status:%d\r\n", Status);
		return XST_FAILURE;
	}

	if (LayerMemDataBusy == 1) {
		printf(
				"Error at SendIRQ_CallBack. LayerMemDataBusy is busy but wants to receive data again.\r\n");
		return XST_FAILURE;
	} else {
		if (HasWeight(LayerNow)) {
			LayerMemDataBusy = 1;
		}
	}
	return XST_SUCCESS;
}

int TinyYolo_Reset(void) {
	Layer_t *layer;
	fixed *mem_ptr;

	for (unsigned int layer_ptr = 0; layer_ptr < LAYER_NUM; layer_ptr++) {
		layer = GetLayer(layer_ptr);
		mem_ptr = layer->mem;

		if (HasBN(layer)) {
			for (unsigned int ch_ptr = 0; ch_ptr < layer->c_in; ch_ptr++) {
				memcpy(mem_ptr, layer->mod_biases,
						layer->c_out * sizeof(fixed));
				if (isChMemNumAligned(layer)) {
					mem_ptr = AddFixedPtr(mem_ptr,
							layer->c_out * sizeof(fixed));
				} else {
					mem_ptr = AddFixedPtr(mem_ptr,
							(layer->c_out + 1) * sizeof(fixed));
				}
			}
		} else {
			if (mem_ptr != NULL) {
				memset(mem_ptr, 0, layer->mem_size);
			}
		}
	}
	return XST_SUCCESS;
}

static inline float leaky_activate(fixed x) {
	return (x > 0) ? x : .1 * x;
}
;

int TinyYolo_ConvCMP(void) {
	if (!HasWeight(RxLayer)) {
		return XST_SUCCESS;
	}

	fixed *org_mem, *dst_mem;

	//Sum a channel output to memory.
	unsigned int offset = RxChNow * AlignedMemNum_ch(RxLayer);
	org_mem = (fixed *) ((unsigned int) CPU_TMP_ADDR + offset);
	dst_mem = AddFixedPtr(RxLayer->mem, offset);
	for (unsigned int ptr = 0; ptr < RxLayer->size_out_per_ch; ptr++) {
		dst_mem[ptr] += org_mem[ptr];
	}

	RxChNow++;

	//Leaky
	if (RxChNow == RxLayer->c_out) { //last ch
		if (HasBN(RxLayer)) {
			dst_mem = RxLayer->mem;
			for (unsigned int ptr = 0; ptr < RxLayer->size_out_per_ch; ptr++) {
				dst_mem[ptr] = leaky_activate(dst_mem[ptr]);
				dst_mem = AddFixedPtr(RxLayer->mem, AlignedMemNum_ch(RxLayer));
			}
			LayerMemDataBusy = 0;
			if (StoredDataRequestError_flg == 1) {
				RecvIRQ_CallBack();
				StoredDataRequestError_flg = 0;
			}
		}
	}

	return XST_SUCCESS;
}

int isChMemNumAligned(Layer_t *Layer) {
	return !(Layer->l_out & 0x1);
}

unsigned int AlignedMemNum_ch(Layer_t *Layer) {
	return Layer->mem_per_ch + (isChWeightNumAligned(Layer) ? 0 : 2);
}

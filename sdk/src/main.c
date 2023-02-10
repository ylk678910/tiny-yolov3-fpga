/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************** Include files **********************************/
#include "xparameters.h"
#include "xil_exception.h"

#include "Video.h"
#include "WeightFileConverter.h"
#include "Video.h"
#include "dma.h"
#include "TinyYoloCtrl.h"
#include "Layer.h"
#include "Weight.h"

int main() {
	int Status;

	/*
	 char IMA_FILE[32] = "tree.bmp";

	 Status = Video_Test_SDImgtoScreen(IMA_FILE);
	 if (Status != XST_SUCCESS) {
	 xil_printf("Transfer of frames failed with error = %d\r\n", Status);
	 return XST_FAILURE;
	 }
	 else {
	 xil_printf("Transfer of frames started \r\n");
	 }
	 */
	Status = DMA_Init();
	if (Status != XST_SUCCESS) {
		xil_printf("DMA Initialize failed.\r\n");
		return XST_FAILURE;
	}
	Status = Weight_Init();
	if (Status != XST_SUCCESS) {
		xil_printf("Weight Initialize failed.\r\n");
		return XST_FAILURE;
	}
	Status = Layer_Init();
	if (Status != XST_SUCCESS) {
		xil_printf("Layer Initialize failed.\r\n");
		return XST_FAILURE;
	}

#ifdef DEBUG_PRINT_OUT
	//DEBUG
	Layer_PrintfAllConfig();
#endif

	Status = WeightFileConvert(0); //0:If IP.weight existed, ignore this line, 1:Overwrite IP.weight file
	if (Status != XST_SUCCESS) {
		xil_printf("Weight File Convert failed.\r\n");
		return XST_FAILURE;
	}

	Status = Weight_Load();
	if (Status != XST_SUCCESS) {
		xil_printf("IP Weight Initialize failed.\r\n");
		return XST_FAILURE;
	}

	Status = Img_Init();
	if (Status != XST_SUCCESS) {
		xil_printf("Image Initialize failed.\r\n");
		return XST_FAILURE;
	}

	Status = TinyYolo_Init();
	if (Status != XST_SUCCESS) {
		xil_printf("IP Initialize failed.\r\n");
		return XST_FAILURE;
	}

#ifndef USE_TEST_IMG
	//Set "dog.bmp" Image(in SD card) to "NewImg[0]" Memory
    char input[] = "dog.bmp";
    Img_LoadFromSD(0, input);
#else
	//Set Test Image to "NewImg[0]" Memory
	Img_TestData(0);
#endif

	//Set IP Start
	Status = TinyYolo_StartDetect();
	if (Status != XST_SUCCESS) {
		xil_printf("Failed at StartDetect.\r\n");
		return XST_FAILURE;
	}

	return 0;


//TinyYolo_test();

	while (1)
		;

	return 0;
}


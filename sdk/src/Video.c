/*
 * Video.c
 *
 *  Created on: 2021年9月5日
 *      Author: ylk67
 */
#include "stdlib.h"
#include "xparameters.h"
#include "xil_exception.h"

/******************** video timing control ****************************/
#include "xvtc.h"
#include "xvtc_hw.h"
/******************** read image from sd card  ****************************/
#include "xsdps.h"
#include "ff.h"
#include "bmp.h"

#include "MemoryAddress.h"
#include "lps_vdma.h"
#include "Video.h"
#include "Fixed.h"
#include "image.h"

XAxiVdma InstancePtr;
fixed *NewImg[3];

static inline uint32_t Img_ptr(uint32_t row, uint32_t col, uint32_t ch);

int Img_Init(void)
{
	if (NEW_IMG0_START_ADDR & (4 - 1)) {
		xil_printf("NEW_IMG0_START_ADDR: Address is not aligned.\r\n");
		return XST_FAILURE;
	}
	if (NEW_IMG1_START_ADDR & (4 - 1)) {
		xil_printf("NEW_IMG1_START_ADDR: Address is not aligned.\r\n");
		return XST_FAILURE;
	}
	if (NEW_IMG2_START_ADDR & (4 - 1)) {
		xil_printf("NEW_IMG2_START_ADDR: Address is not aligned.\r\n");
		return XST_FAILURE;
	}
	NewImg[0] = (fixed *) NEW_IMG0_START_ADDR;
	NewImg[1] = (fixed *) NEW_IMG1_START_ADDR;
	NewImg[2] = (fixed *) NEW_IMG2_START_ADDR;

	return XST_SUCCESS;
}

fixed *Img_NowImg(void)
{
	return NewImg[0];
}

int Video_Test_SDImgtoScreen(char ImgFileName[]) {
	int Status;
	/*--------------------------------------------------------------------------------*/
	/*																				  */
	/* Mount SD card to memory 														  */
	/*																				  */
	/*--------------------------------------------------------------------------------*/

	FATFS FS_instance;
	FRESULT f_result;
	FIL file;
	char *Path = "0:/";
	f_result = f_mount(&FS_instance, Path, 0);
	if (f_result != FR_OK)
		xil_printf("f_mount fail\r\n");
	else
		xil_printf("Mount SD card success.\r\n");

	/* Open the BMP file */
	f_result = f_open(&file, ImgFileName, FA_READ);
	if (f_result != FR_OK)
		xil_printf("Open %s file fail.\r\n", ImgFileName);
	else
		xil_printf("Open %s file success.\r\n", ImgFileName);

	xil_printf("file size = %d\r\n", f_size(&file));

	/*--------------------------------------------------------------------------------*/
	/*																				  */
	/* Read BMP from SD  	 														  */
	/*																				  */
	/*--------------------------------------------------------------------------------*/

	BMP_FILEHEADER* bmp_fileheader;
	BMP_INFOHEADER* bmp_infoheader;
	bmp_fileheader = malloc(sizeof(bmp_fileheader));
	bmp_infoheader = malloc(sizeof(bmp_infoheader));
	unsigned int br;

	/* Read the file header. */
	f_result = f_read(&file, bmp_fileheader, sizeof(BMP_FILEHEADER), &br);
	if (f_result != FR_OK)
		xil_printf("fileheader read error\r\n");

	xil_printf("offset to image data = %d\r\n", bmp_fileheader->offset);

	/* Read the info header. */
	f_result = f_read(&file, bmp_infoheader, sizeof(BMP_INFOHEADER), &br);
	if (f_result != FR_OK)
		xil_printf("infoheader read error\r\n");

	bmp_infoheader->imagesize = (bmp_infoheader->bits / 8)
			* bmp_infoheader->width * bmp_infoheader->height;
	xil_printf("image h_size = %d\r\n", bmp_infoheader->width);
	xil_printf("image y_size = %d\r\n", bmp_infoheader->height);
	xil_printf("image size = %d bytes\r\n", bmp_infoheader->imagesize);
	xil_printf("image bits per pixel = %d bits\r\n", bmp_infoheader->bits);

	BMP_BGR data_bgr[bmp_infoheader->width * bmp_infoheader->height];

	/* Read the Bitmap Data. */
	f_result = f_read(&file, &data_bgr[0],
			sizeof(u8) * bmp_infoheader->imagesize, &br);
	if (f_result != 0)
		xil_printf("read data color fail\r\n");
	/* Close the BMP file */
	f_result = f_close(&file);
	if (f_result != FR_OK)
		xil_printf("close file error\r\n");
	xil_printf("close file success\r\n");

	/*--------------------------------------------------------------------------------*/
	/*																				  */
	/* Adjust the Bitmap data														  */
	/* The screen scanning is top to bottom, but the bitmap data is bottom to top.	  */
	/*																				  */
	/*--------------------------------------------------------------------------------*/

	// original bmp formt scan image for bottom_left to top_right //
	// 24bits data follow the order of B, G, R
	u8* src = malloc(sizeof(FRAME_WIDTH * FRAME_HEIGHT) * 3 + 1);

	xil_printf("data_bgr address = %x\t", &data_bgr[0]);
	xil_printf("src address = %x\r\n", src);
	for (int i = 0; i < FRAME_WIDTH * FRAME_HEIGHT; i++) {
		BMP_BGR* ref_ptr = &data_bgr[bmp_infoheader->height
				* bmp_infoheader->width
				- ((i / bmp_infoheader->width) + 1) * bmp_infoheader->width
				+ (i % bmp_infoheader->width)];
		src[i * 3] = ref_ptr->Blue | 0x1;
		src[i * 3 + 1] = ref_ptr->Green;
		src[i * 3 + 2] = ref_ptr->Red;
	}

	Xil_DCacheFlush();

	/*--------------------------------------------------------------------------------*/
	/*																				  */
	/* Setup the VTC config 														  */
	/*																				  */
	/*--------------------------------------------------------------------------------*/

	u32 CtrlRegValue;
	/* Read Control register value back */
	CtrlRegValue = XVtc_ReadReg(VTC_BASEADDR, (XVTC_CTL_OFFSET));
	/* Change the value according to the enabling type and write it back */
	XVtc_WriteReg(VTC_BASEADDR, (XVTC_CTL_OFFSET),
			(CtrlRegValue | XVTC_CTL_GE_MASK));

	xil_printf("Setting finish.\n\r");
	/*--------------------------------------------------------------------------------*/
	/*																				  */
	/* Starting the first VDMA 														  */
	/*																				  */
	/*--------------------------------------------------------------------------------*/

	/* Calling the API to configure and start VDMA without frame counter interrupt */
	Status = run_triple_frame_buffer(&InstancePtr, 0, FRAME_WIDTH * 3,
			FRAME_HEIGHT, (int) src, 0, 0);
	if (Status != XST_SUCCESS) {
		xil_printf("Transfer of frames failed with error = %d\r\n", Status);
		return XST_FAILURE;
	} else {
		xil_printf("Transfer of frames started \r\n");
	}
	return XST_SUCCESS;
}

void Img_TestData(uint32_t which) {
	xil_printf("Start setting New Image file.\n\r");

	fixed TestData = 0;
	for (uint32_t row = 0; row < 416; ++row) {
		for (uint32_t col = 0; col < 416; ++col) {
			for (uint32_t bgr = 0; bgr < 3; ++bgr) {
				NewImg[which][Img_ptr(row, col, bgr)] = TestData++;
			}
		}
	}
}

void Img_LoadFromSD(uint32_t which, char *filename) {
	xil_printf("Start setting New Image file.\n\r");

    image im = load_image_color(filename, 0, 0);
    image sized = letterbox_image(im, 416, 416);	//! image是動態陣列 可能導致空間不足 記得刪除(可能會和輸出到VGA的陣列搶空間)
    free_image(im);
    //原本的sized.data理應是pixel0ch0, pixel1ch0, pixel2ch0...排序

	for (uint32_t row = 0; row < 416; ++row) {
		for (uint32_t col = 0; col < 416; ++col) {
			for (uint32_t bgr = 0; bgr < 3; ++bgr) {
				NewImg[which][Img_ptr(row, col, bgr)] = float_to_fixed(sized.data[Img_ptr(row, col, bgr)]);
			}
		}
	}
    free_image(sized);
}


static inline uint32_t Img_ptr(uint32_t row, uint32_t col, uint32_t ch)
{
	return (ch*416*416) + (row*416) + col;
}

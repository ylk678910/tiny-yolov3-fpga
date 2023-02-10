/*
 * Video.h
 *
 *  Created on: 2021年9月5日
 *      Author: ylk67
 */

#ifndef INC_VIDEO_H_
#define INC_VIDEO_H_

#include "Fixed.h"

#define VTC_BASEADDR XPAR_V_TC_0_BASEADDR

#include "lps_vdma.h"

#define FRAME_WIDTH 1024
#define FRAME_HEIGHT 768

extern XAxiVdma InstancePtr;
extern fixed *NewImg[3];

int Img_Init(void);
fixed *Img_NowImg(void);
void Img_TestData(uint32_t which);
void Img_LoadFromSD(uint32_t which, char *filename);

int Video_Test_SDImgtoScreen(char ImgFileName[]);

#endif /* INC_VIDEO_H_ */

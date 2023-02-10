/*
 * bmp.h
 *
 *  Created on: 2018年2月28日
 *      Author: Da
 */

#ifndef SRC_BMP_H_
#define SRC_BMP_H_

typedef struct {
	/* type : Magic identifier,一般為BM(0x42,0x4d) */
	unsigned short int type;
	unsigned int size;/* File size in bytes,全部的檔案大小 */
	unsigned short int reserved1, reserved2; /* 保留欄位 */
	unsigned int offset;/* Offset to image data, bytes */
}__attribute__((packed)) BMP_FILEHEADER;

typedef struct {
	unsigned int size;/* Info Header size in bytes */
	int width,height;/* Width and height of image */
	unsigned short int planes;/* Number of colour planes */
	unsigned short int bits; /* Bits per pixel */
	unsigned int compression; /* Compression type */
	unsigned int imagesize; /* Image size in bytes */
	int xresolution,yresolution; /* Pixels per meter */
	unsigned int ncolours; /* Number of colours */
	unsigned int importantcolours; /* Important colours */
}__attribute__((packed)) BMP_INFOHEADER;

typedef struct{
	u8 Blue; /* blue value(0~255) */
	u8 Green; /*green value(0~255) */
	u8 Red;  /*red value(0~255) */
}__attribute__((packed)) BMP_BGR;

typedef struct{
	u8 Blue; /* blue value(0~255) */
	u8 Green; /*green value(0~255) */
	u8 Red;  /*red value(0~255) */
	u8 A;
}__attribute__((packed)) BMP_BGRA;





#endif /* SRC_BMP_H_ */

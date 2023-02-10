/*
 * MemoryAddress.h
 *
 *  Created on: 2021年8月29日
 *      Author: ylk67
 */

#ifndef SRC_MEMORYADDRESS_H_
#define SRC_MEMORYADDRESS_H_

#include "Fixed.h"
#include "Weight.h"

#define FLOAT_FIZE 4

#define DMA_START_ADDR		(unsigned int)0x10000000
#define DMA_HIGHEST_ADDR	(unsigned int)0x1FFFFFFF

#define RX_BD_SPACE_BASE	(DMA_START_ADDR)
#define RX_BD_SPACE_HIGH	(DMA_START_ADDR + 0x0000FFFF)
#define TX_BD_SPACE_BASE	(DMA_START_ADDR + 0x00010000)
#define TX_BD_SPACE_HIGH	(DMA_START_ADDR + 0x0001FFFF)

/* IP WEIGHT */
#define WEIGHT_SIZE				(unsigned int)17690976				//conv weight
#define WEIGHT_START_ADDR		(DMA_START_ADDR + 0x00020000)
#define MOD_BIASES_SIZE			(unsigned int)7388					//mod_biases
#define MOD_BIASES_START_ADDR	(WEIGHT_START_ADDR + WEIGHT_SIZE)

#define STORED_SIZE			(unsigned int)15363940
#define STORED_START_ADDR	(MOD_BIASES_START_ADDR + MOD_BIASES_SIZE)

#define NEW_IMG_SIZE		(unsigned int)(FIXED_SIZE * 416*416*3)
#define NEW_IMG0_START_ADDR	(STORED_START_ADDR + STORED_SIZE)	//STORED_START_ADDR + STORED_SIZE
#define NEW_IMG1_START_ADDR	(NEW_IMG0_START_ADDR + NEW_IMG_SIZE)
#define NEW_IMG2_START_ADDR	(NEW_IMG1_START_ADDR + NEW_IMG_SIZE)

#define CPU_TMP_ADDR		(NEW_IMG2_START_ADDR + NEW_IMG_SIZE)

#endif /* SRC_MEMORYADDRESS_H_ */
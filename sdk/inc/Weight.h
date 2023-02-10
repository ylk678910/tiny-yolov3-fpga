#ifndef INC_WEIGHT_H_
#define INC_WEIGHT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Fixed.h"
#include "Layer.h"

#define WEIGHT_NUM 8848672	 	//modified weight without modified biases and BN_weight

typedef struct
{
    fixed rolling_mean;
    fixed rolling_variznce;
    fixed scales;
    fixed biases;
    fixed *conv_weight;
} IPWeightDBL_t;

typedef struct
{
	fixed biases;
	fixed *conv_weight;
} IPWeightConv_t;

typedef fixed ConvWeight_t;


extern fixed *Weight, *mod_biases;

int Weight_Init(void);
int Weight_Load(void);
int isChWeightNumAligned(Layer_t *Layer);
int HasWeight(Layer_t *Layer);
int HasBN(Layer_t *Layer);
unsigned int AlignedWeightNum_ch(Layer_t *Layer);

#endif /* INC_WEIGHT_H_ */

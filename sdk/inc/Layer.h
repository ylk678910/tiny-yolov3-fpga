/*
 * Layer.h
 *
 *  Created on: 2021年9月4日
 *      Author: ylk67
 */

#ifndef INC_LAYER_H_
#define INC_LAYER_H_

#include "Fixed.h"

typedef enum
{
    CONV_NORMAL = 0,
    CONV_DBL,
    MAXPOOL,
    UPSAMMPLING,
    ROUTE,
    YOLO
} LayerType_t;

typedef struct
{
    unsigned int l_in;				//input length
    unsigned int l_out;				//output length
    unsigned int c_in;				//input number of channel
    unsigned int c_out;				//output number of channel
    unsigned int filter_size;		//filter length
    unsigned int size_in_per_ch;	//number of fixed of the input per channel of this layer
    unsigned int size_in;			//number of fixed of the input of this layer
    unsigned int size_out_per_ch;	//number of fixed of the output per channel of this layer
    unsigned int size_out;			//number of fixed of the output of this layer
    LayerType_t layer_type;			//layer type
    fixed *weight;					//weight start location of this layer(without mod_biases)
    unsigned int weight_per_ch;		//number of weight per a channel of this layer
    unsigned int weight_size;		//weight size of this layer
    fixed *mod_biases;				//mod_biases start location of this layer
    fixed *mem;						//memory start location of this layer
    unsigned int mem_per_ch;		//number of memory size per a channel
    unsigned int mem_size;			//memory size of this layer
} Layer_t;

#include "Weight.h"

#define LAYER_NUM 24

#define LAYER0_LAYER_TYPE CONV_DBL
#define LAYER1_LAYER_TYPE MAXPOOL
#define LAYER2_LAYER_TYPE CONV_DBL
#define LAYER3_LAYER_TYPE MAXPOOL
#define LAYER4_LAYER_TYPE CONV_DBL
#define LAYER5_LAYER_TYPE MAXPOOL
#define LAYER6_LAYER_TYPE CONV_DBL
#define LAYER7_LAYER_TYPE MAXPOOL
#define LAYER8_LAYER_TYPE CONV_DBL
#define LAYER9_LAYER_TYPE MAXPOOL
#define LAYER10_LAYER_TYPE CONV_DBL
#define LAYER11_LAYER_TYPE MAXPOOL
#define LAYER12_LAYER_TYPE CONV_DBL
#define LAYER13_LAYER_TYPE CONV_DBL
#define LAYER14_LAYER_TYPE CONV_DBL
#define LAYER15_LAYER_TYPE CONV_NORMAL
#define LAYER16_LAYER_TYPE YOLO
#define LAYER17_LAYER_TYPE ROUTE
#define LAYER18_LAYER_TYPE CONV_DBL
#define LAYER19_LAYER_TYPE UPSAMMPLING
#define LAYER20_LAYER_TYPE ROUTE
#define LAYER21_LAYER_TYPE CONV_DBL
#define LAYER22_LAYER_TYPE CONV_NORMAL
#define LAYER23_LAYER_TYPE YOLO

#define LAYER0_LEN_IN  416
#define LAYER1_LEN_IN  416
#define LAYER2_LEN_IN  208
#define LAYER3_LEN_IN  208
#define LAYER4_LEN_IN  104
#define LAYER5_LEN_IN  104
#define LAYER6_LEN_IN  52
#define LAYER7_LEN_IN  52
#define LAYER8_LEN_IN  26
#define LAYER9_LEN_IN  26
#define LAYER10_LEN_IN 13
#define LAYER11_LEN_IN 13
#define LAYER12_LEN_IN 13
#define LAYER13_LEN_IN 13
#define LAYER14_LEN_IN 13
#define LAYER15_LEN_IN 13
#define LAYER18_LEN_IN 13
#define LAYER19_LEN_IN 13
#define LAYER21_LEN_IN 26
#define LAYER22_LEN_IN 26

#define LAYER0_LEN_OUT  416
#define LAYER1_LEN_OUT  208
#define LAYER2_LEN_OUT  208
#define LAYER3_LEN_OUT  104
#define LAYER4_LEN_OUT  104
#define LAYER5_LEN_OUT  52
#define LAYER6_LEN_OUT  52
#define LAYER7_LEN_OUT  26
#define LAYER8_LEN_OUT  26
#define LAYER9_LEN_OUT  13
#define LAYER10_LEN_OUT 13
#define LAYER11_LEN_OUT 13
#define LAYER12_LEN_OUT 13
#define LAYER13_LEN_OUT 13
#define LAYER14_LEN_OUT 13
#define LAYER15_LEN_OUT 13
#define LAYER18_LEN_OUT 13
#define LAYER19_LEN_OUT 26
#define LAYER21_LEN_OUT 26
#define LAYER22_LEN_OUT 26

#define LAYER0_CH_IN  3
#define LAYER1_CH_IN  16
#define LAYER2_CH_IN  16
#define LAYER3_CH_IN  32
#define LAYER4_CH_IN  32
#define LAYER5_CH_IN  64
#define LAYER6_CH_IN  64
#define LAYER7_CH_IN  128
#define LAYER8_CH_IN  128
#define LAYER9_CH_IN  256
#define LAYER10_CH_IN 256
#define LAYER11_CH_IN 512
#define LAYER12_CH_IN 512
#define LAYER13_CH_IN 1024
#define LAYER14_CH_IN 256
#define LAYER15_CH_IN 512
#define LAYER18_CH_IN 256
#define LAYER19_CH_IN 128
#define LAYER21_CH_IN 384
#define LAYER22_CH_IN 256

#define LAYER0_CH_OUT  16
#define LAYER1_CH_OUT  16
#define LAYER2_CH_OUT  32
#define LAYER3_CH_OUT  32
#define LAYER4_CH_OUT  64
#define LAYER5_CH_OUT  64
#define LAYER6_CH_OUT  128
#define LAYER7_CH_OUT  128
#define LAYER8_CH_OUT  256
#define LAYER9_CH_OUT  256
#define LAYER10_CH_OUT 512
#define LAYER11_CH_OUT 512
#define LAYER12_CH_OUT 1024
#define LAYER13_CH_OUT 256
#define LAYER14_CH_OUT 512
#define LAYER15_CH_OUT 255
#define LAYER18_CH_OUT 128
#define LAYER19_CH_OUT 128
#define LAYER21_CH_OUT 256
#define LAYER22_CH_OUT 255

#define LAYER0_FILTER_SIZE 3
#define LAYER1_FILTER_SIZE 2
#define LAYER2_FILTER_SIZE 3
#define LAYER3_FILTER_SIZE 2
#define LAYER4_FILTER_SIZE 3
#define LAYER5_FILTER_SIZE 2
#define LAYER6_FILTER_SIZE 3
#define LAYER7_FILTER_SIZE 2
#define LAYER8_FILTER_SIZE 3
#define LAYER9_FILTER_SIZE 2
#define LAYER10_FILTER_SIZE 3
#define LAYER11_FILTER_SIZE 2
#define LAYER12_FILTER_SIZE 3
#define LAYER13_FILTER_SIZE 1
#define LAYER14_FILTER_SIZE 3
#define LAYER15_FILTER_SIZE 1
#define LAYER18_FILTER_SIZE 1
#define LAYER21_FILTER_SIZE 3
#define LAYER22_FILTER_SIZE 1

extern Layer_t LayerData[LAYER_NUM];

int Layer_Init(void);
Layer_t *GetLayer(uint32_t Layer);
void Layer_PrintfAllConfig(void);

#endif /* INC_LAYER_H_ */

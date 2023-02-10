/*
 * Layer.c
 *
 *  Created on: 2021年9月5日
 *      Author: ylk67
 */

#include "xstatus.h"
#include "xil_printf.h"

#include "stdlib.h"

#include "Layer.h"
#include "Weight.h"
#include "MemoryAddress.h"

Layer_t LayerData[LAYER_NUM];

Layer_t *GetLayer(uint32_t Layer) {
	return &LayerData[Layer];
}

int Layer_Init(void) {
	if (STORED_START_ADDR & (4 - 1)) {
		xil_printf("STORED_START_ADDR: Address is not aligned.\r\n");
		return XST_FAILURE;
	}
	//Layer0
	{
		LayerData[0].l_in = LAYER0_LEN_IN;
		LayerData[0].l_out = LAYER0_LEN_OUT;
		LayerData[0].c_in = LAYER0_CH_IN;
		LayerData[0].c_out = LAYER0_CH_OUT;
		LayerData[0].filter_size = LAYER0_FILTER_SIZE;
		LayerData[0].layer_type = LAYER0_LAYER_TYPE;
		LayerData[0].size_in_per_ch = LayerData[0].l_in * LayerData[0].l_in;
		LayerData[0].size_in = LayerData[0].size_in_per_ch * LayerData[0].c_in;
		LayerData[0].size_out = LayerData[0].l_out * LayerData[0].l_out;
		LayerData[0].size_out = LayerData[0].size_out_per_ch
				* LayerData[0].c_out;
		LayerData[0].weight_per_ch = LayerData[0].c_out
				* LayerData[0].filter_size * LayerData[0].filter_size;
		LayerData[0].weight_size = (((LayerData[0].weight_per_ch
				+ (LayerData[0].weight_per_ch & 0x1)) * LAYER0_CH_IN))
				* FIXED_SIZE;
		LayerData[0].weight = (fixed *) Weight;
		LayerData[0].mod_biases = (fixed *) mod_biases;
		LayerData[0].mem_per_ch = LayerData[0].l_out * LayerData[0].l_out
				* FIXED_SIZE;
		LayerData[0].mem_size =
				(LayerData[0].mem_per_ch
						+ 2 * ((LayerData[0].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER0_CH_OUT;
		LayerData[0].mem = (fixed *) STORED_START_ADDR;
	}

	//Layer1
	{
		LayerData[1].l_in = LAYER1_LEN_IN;
		LayerData[1].l_out = LAYER1_LEN_OUT;
		LayerData[1].c_in = LAYER1_CH_IN;
		LayerData[1].c_out = LAYER1_CH_OUT;
		LayerData[1].filter_size = LAYER1_FILTER_SIZE;
		LayerData[1].layer_type = LAYER1_LAYER_TYPE;
		LayerData[1].size_in_per_ch = LayerData[1].l_in * LayerData[1].l_in;
		LayerData[1].size_in = LayerData[1].size_in_per_ch * LayerData[1].c_in;
		LayerData[1].size_out = LayerData[1].l_out * LayerData[1].l_out;
		LayerData[1].size_out = LayerData[1].size_out_per_ch
				* LayerData[1].c_out;
		LayerData[1].weight_per_ch = 0;
		LayerData[1].weight_size = 0;
		LayerData[1].weight = NULL;
		LayerData[1].mod_biases = NULL;
		LayerData[1].mem_per_ch = LayerData[1].l_out * LayerData[1].l_out
				* FIXED_SIZE;
		LayerData[1].mem_size =
				(LayerData[1].mem_per_ch
						+ 2 * ((LayerData[1].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER1_CH_OUT;
		LayerData[1].mem = (fixed *) ((uint32_t) LayerData[0].mem
				+ LayerData[0].mem_size);
	}

	//Layer2
	{
		LayerData[2].l_in = LAYER2_LEN_IN;
		LayerData[2].l_out = LAYER2_LEN_OUT;
		LayerData[2].c_in = LAYER2_CH_IN;
		LayerData[2].c_out = LAYER2_CH_OUT;
		LayerData[2].filter_size = LAYER2_FILTER_SIZE;
		LayerData[2].layer_type = LAYER2_LAYER_TYPE;
		LayerData[2].size_in_per_ch = LayerData[2].l_in * LayerData[2].l_in;
		LayerData[2].size_in = LayerData[2].size_in_per_ch * LayerData[2].c_in;
		LayerData[2].size_out = LayerData[2].l_out * LayerData[2].l_out;
		LayerData[2].size_out = LayerData[2].size_out_per_ch
				* LayerData[2].c_out;
		LayerData[2].weight_per_ch = LayerData[2].c_out
				* LayerData[2].filter_size * LayerData[2].filter_size;
		LayerData[2].weight_size = (((LayerData[2].weight_per_ch
				+ (LayerData[2].weight_per_ch & 0x1)) * LAYER2_CH_IN))
				* FIXED_SIZE;
		LayerData[2].weight = (fixed *) ((uint32_t) LayerData[0].weight
				+ LayerData[0].weight_size);
		LayerData[2].mod_biases = (fixed *) ((uint32_t) LayerData[0].mod_biases
				+ LayerData[0].c_out * FIXED_SIZE);
		LayerData[2].mem_per_ch = LayerData[2].l_out * LayerData[2].l_out
				* FIXED_SIZE;
		LayerData[2].mem_size =
				(LayerData[2].mem_per_ch
						+ 2 * ((LayerData[2].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER2_CH_OUT;
		LayerData[2].mem = (fixed *) ((uint32_t) LayerData[1].mem
				+ LayerData[1].mem_size);
	}

	//Layer3
	{
		LayerData[3].l_in = LAYER3_LEN_IN;
		LayerData[3].l_out = LAYER3_LEN_OUT;
		LayerData[3].c_in = LAYER3_CH_IN;
		LayerData[3].c_out = LAYER3_CH_OUT;
		LayerData[3].filter_size = LAYER3_FILTER_SIZE;
		LayerData[3].layer_type = LAYER3_LAYER_TYPE;
		LayerData[3].size_in_per_ch = LayerData[3].l_in * LayerData[3].l_in;
		LayerData[3].size_in = LayerData[3].size_in_per_ch * LayerData[3].c_in;
		LayerData[3].size_out = LayerData[3].l_out * LayerData[3].l_out;
		LayerData[3].size_out = LayerData[3].size_out_per_ch
				* LayerData[3].c_out;
		LayerData[3].weight_per_ch = 0;
		LayerData[3].weight_size = 0;
		LayerData[3].weight = NULL;
		LayerData[3].mod_biases = NULL;
		LayerData[3].mem_per_ch = LayerData[3].l_out * LayerData[3].l_out
				* FIXED_SIZE;
		LayerData[3].mem_size =
				(LayerData[3].mem_per_ch
						+ 2 * ((LayerData[3].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER3_CH_OUT;
		LayerData[3].mem = (fixed *) ((uint32_t) LayerData[2].mem
				+ LayerData[2].mem_size);
	}

	//Layer4
	{
		LayerData[4].l_in = LAYER4_LEN_IN;
		LayerData[4].l_out = LAYER4_LEN_OUT;
		LayerData[4].c_in = LAYER4_CH_IN;
		LayerData[4].c_out = LAYER4_CH_OUT;
		LayerData[4].filter_size = LAYER4_FILTER_SIZE;
		LayerData[4].layer_type = LAYER4_LAYER_TYPE;
		LayerData[4].size_in_per_ch = LayerData[4].l_in * LayerData[4].l_in;
		LayerData[4].size_in = LayerData[4].size_in_per_ch * LayerData[4].c_in;
		LayerData[4].size_out = LayerData[4].l_out * LayerData[4].l_out;
		LayerData[4].size_out = LayerData[4].size_out_per_ch
				* LayerData[4].c_out;
		LayerData[4].weight_per_ch = LayerData[4].c_out
				* LayerData[4].filter_size * LayerData[4].filter_size;
		LayerData[4].weight_size = (((LayerData[4].weight_per_ch
				+ (LayerData[4].weight_per_ch & 0x1)) * LAYER4_CH_IN))
				* FIXED_SIZE;
		LayerData[4].weight = (fixed *) ((uint32_t) LayerData[2].weight
				+ LayerData[2].weight_size);
		LayerData[4].mod_biases = (fixed *) ((uint32_t) LayerData[2].mod_biases
				+ LayerData[2].c_out * FIXED_SIZE);
		LayerData[4].mem_per_ch = LayerData[4].l_out * LayerData[4].l_out
				* FIXED_SIZE;
		LayerData[4].mem_size =
				(LayerData[4].mem_per_ch
						+ 2 * ((LayerData[4].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER4_CH_OUT;
		LayerData[4].mem = (fixed *) ((uint32_t) LayerData[3].mem
				+ LayerData[3].mem_size);
	}

	//Layer5
	{
		LayerData[5].l_in = LAYER5_LEN_IN;
		LayerData[5].l_out = LAYER5_LEN_OUT;
		LayerData[5].c_in = LAYER5_CH_IN;
		LayerData[5].c_out = LAYER5_CH_OUT;
		LayerData[5].filter_size = LAYER5_FILTER_SIZE;
		LayerData[5].layer_type = LAYER5_LAYER_TYPE;
		LayerData[5].size_in_per_ch = LayerData[5].l_in * LayerData[5].l_in;
		LayerData[5].size_in = LayerData[5].size_in_per_ch * LayerData[5].c_in;
		LayerData[5].size_out = LayerData[5].l_out * LayerData[5].l_out;
		LayerData[5].size_out = LayerData[5].size_out_per_ch
				* LayerData[5].c_out;
		LayerData[5].weight_per_ch = 0;
		LayerData[5].weight_size = 0;
		LayerData[5].weight = NULL;
		LayerData[5].mod_biases = NULL;
		LayerData[5].mem_per_ch = LayerData[5].l_out * LayerData[5].l_out
				* FIXED_SIZE;
		LayerData[5].mem_size =
				(LayerData[5].mem_per_ch
						+ 2 * ((LayerData[5].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER5_CH_OUT;
		LayerData[5].mem = (fixed *) ((uint32_t) LayerData[4].mem
				+ LayerData[4].mem_size);
	}

	//Layer6
	{
		LayerData[6].l_in = LAYER6_LEN_IN;
		LayerData[6].l_out = LAYER6_LEN_OUT;
		LayerData[6].c_in = LAYER6_CH_IN;
		LayerData[6].c_out = LAYER6_CH_OUT;
		LayerData[6].filter_size = LAYER6_FILTER_SIZE;
		LayerData[6].layer_type = LAYER6_LAYER_TYPE;
		LayerData[6].size_in_per_ch = LayerData[6].l_in * LayerData[6].l_in;
		LayerData[6].size_in = LayerData[6].size_in_per_ch * LayerData[6].c_in;
		LayerData[6].size_out = LayerData[6].l_out * LayerData[6].l_out;
		LayerData[6].size_out = LayerData[6].size_out_per_ch
				* LayerData[6].c_out;
		LayerData[6].weight_per_ch = LayerData[6].c_out
				* LayerData[6].filter_size * LayerData[6].filter_size;
		LayerData[6].weight_size = (((LayerData[6].weight_per_ch
				+ (LayerData[6].weight_per_ch & 0x1)) * LAYER6_CH_IN))
				* FIXED_SIZE;
		LayerData[6].weight = (fixed *) ((uint32_t) LayerData[4].weight
				+ LayerData[4].weight_size);
		LayerData[6].mod_biases = (fixed *) ((uint32_t) LayerData[4].mod_biases
				+ LayerData[4].c_out * FIXED_SIZE);
		LayerData[6].mem_per_ch = LayerData[6].l_out * LayerData[6].l_out
				* FIXED_SIZE;
		LayerData[6].mem_size =
				(LayerData[6].mem_per_ch
						+ 2 * ((LayerData[6].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER6_CH_OUT;
		LayerData[6].mem = (fixed *) ((uint32_t) LayerData[5].mem
				+ LayerData[5].mem_size);
	}

	//Layer7
	{
		LayerData[7].l_in = LAYER7_LEN_IN;
		LayerData[7].l_out = LAYER7_LEN_OUT;
		LayerData[7].c_in = LAYER7_CH_IN;
		LayerData[7].c_out = LAYER7_CH_OUT;
		LayerData[7].filter_size = LAYER7_FILTER_SIZE;
		LayerData[7].layer_type = LAYER7_LAYER_TYPE;
		LayerData[7].size_in_per_ch = LayerData[7].l_in * LayerData[7].l_in;
		LayerData[7].size_in = LayerData[7].size_in_per_ch * LayerData[7].c_in;
		LayerData[7].size_out = LayerData[7].l_out * LayerData[7].l_out;
		LayerData[7].size_out = LayerData[7].size_out_per_ch
				* LayerData[7].c_out;
		LayerData[7].weight_per_ch = 0;
		LayerData[7].weight_size = 0;
		LayerData[7].weight = NULL;
		LayerData[7].mod_biases = NULL;
		LayerData[7].mem_per_ch = LayerData[7].l_out * LayerData[7].l_out
				* FIXED_SIZE;
		LayerData[7].mem_size =
				(LayerData[7].mem_per_ch
						+ 2 * ((LayerData[7].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER7_CH_OUT;
		LayerData[7].mem = (fixed *) ((uint32_t) LayerData[6].mem
				+ LayerData[6].mem_size);
	}

	//Layer8
	{
		LayerData[8].l_in = LAYER8_LEN_IN;
		LayerData[8].l_out = LAYER8_LEN_OUT;
		LayerData[8].c_in = LAYER8_CH_IN;
		LayerData[8].c_out = LAYER8_CH_OUT;
		LayerData[8].filter_size = LAYER8_FILTER_SIZE;
		LayerData[8].layer_type = LAYER8_LAYER_TYPE;
		LayerData[8].size_in_per_ch = LayerData[8].l_in * LayerData[8].l_in;
		LayerData[8].size_in = LayerData[8].size_in_per_ch * LayerData[8].c_in;
		LayerData[8].size_out = LayerData[8].l_out * LayerData[8].l_out;
		LayerData[8].size_out = LayerData[8].size_out_per_ch
				* LayerData[8].c_out;
		LayerData[8].weight_per_ch = LayerData[8].c_out
				* LayerData[8].filter_size * LayerData[8].filter_size;
		LayerData[8].weight_size = (((LayerData[8].weight_per_ch
				+ (LayerData[8].weight_per_ch & 0x1)) * LAYER8_CH_IN))
				* FIXED_SIZE;
		LayerData[8].weight = (fixed *) ((uint32_t) LayerData[6].weight
				+ LayerData[6].weight_size);
		LayerData[8].mod_biases = (fixed *) ((uint32_t) LayerData[6].mod_biases
				+ LayerData[6].c_out * FIXED_SIZE);
		LayerData[8].mem_per_ch = LayerData[8].l_out * LayerData[8].l_out
				* FIXED_SIZE;
		LayerData[8].mem_size =
				(LayerData[8].mem_per_ch
						+ 2 * ((LayerData[8].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER8_CH_OUT;
		LayerData[8].mem = (fixed *) ((uint32_t) LayerData[7].mem
				+ LayerData[7].mem_size);
	}

	//Layer9
	{
		LayerData[9].l_in = LAYER9_LEN_IN;
		LayerData[9].l_out = LAYER9_LEN_OUT;
		LayerData[9].c_in = LAYER9_CH_IN;
		LayerData[9].c_out = LAYER9_CH_OUT;
		LayerData[9].filter_size = LAYER9_FILTER_SIZE;
		LayerData[9].layer_type = LAYER9_LAYER_TYPE;
		LayerData[9].size_in_per_ch = LayerData[9].l_in * LayerData[9].l_in;
		LayerData[9].size_in = LayerData[9].size_in_per_ch * LayerData[9].c_in;
		LayerData[9].size_out = LayerData[9].l_out * LayerData[9].l_out;
		LayerData[9].size_out = LayerData[9].size_out_per_ch
				* LayerData[9].c_out;
		LayerData[9].weight_per_ch = 0;
		LayerData[9].weight_size = 0;
		LayerData[9].weight = NULL;
		LayerData[9].mod_biases = NULL;
		LayerData[9].mem_per_ch = LayerData[9].l_out * LayerData[9].l_out
				* FIXED_SIZE;
		LayerData[9].mem_size =
				(LayerData[9].mem_per_ch
						+ 2 * ((LayerData[9].mem_per_ch & 0x3) ? 1 : 0))
						* LAYER9_CH_OUT;
		LayerData[9].mem = (fixed *) ((uint32_t) LayerData[8].mem
				+ LayerData[8].mem_size);
	}

	//Layer10
	{
		LayerData[10].l_in = LAYER10_LEN_IN;
		LayerData[10].l_out = LAYER10_LEN_OUT;
		LayerData[10].c_in = LAYER10_CH_IN;
		LayerData[10].c_out = LAYER10_CH_OUT;
		LayerData[10].filter_size = LAYER10_FILTER_SIZE;
		LayerData[10].layer_type = LAYER10_LAYER_TYPE;
		LayerData[10].size_in_per_ch = LayerData[10].l_in * LayerData[10].l_in;
		LayerData[10].size_in = LayerData[10].size_in_per_ch
				* LayerData[10].c_in;
		LayerData[10].size_out = LayerData[10].l_out * LayerData[10].l_out;
		LayerData[10].size_out = LayerData[10].size_out_per_ch
				* LayerData[10].c_out;
		LayerData[10].weight_per_ch = LayerData[10].c_out
				* LayerData[10].filter_size * LayerData[10].filter_size;
		LayerData[10].weight_size = (((LayerData[10].weight_per_ch
				+ (LayerData[10].weight_per_ch & 0x1)) * LAYER10_CH_IN))
				* FIXED_SIZE;
		LayerData[10].weight = (fixed *) ((uint32_t) LayerData[8].weight
				+ LayerData[8].weight_size);
		LayerData[10].mod_biases = (fixed *) ((uint32_t) LayerData[8].mod_biases
				+ LayerData[8].c_out * FIXED_SIZE);
		LayerData[10].mem_per_ch = LayerData[10].l_out * LayerData[10].l_out
				* FIXED_SIZE;
		LayerData[10].mem_size = (LayerData[10].mem_per_ch
				+ 2 * ((LayerData[10].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER10_CH_OUT;
		LayerData[10].mem = (fixed *) ((uint32_t) LayerData[9].mem
				+ LayerData[9].mem_size);
	}

	//Layer11
	{
		LayerData[11].l_in = LAYER11_LEN_IN;
		LayerData[11].l_out = LAYER11_LEN_OUT;
		LayerData[11].c_in = LAYER11_CH_IN;
		LayerData[11].c_out = LAYER11_CH_OUT;
		LayerData[11].filter_size = LAYER11_FILTER_SIZE;
		LayerData[11].layer_type = LAYER11_LAYER_TYPE;
		LayerData[11].size_in_per_ch = LayerData[11].l_in * LayerData[11].l_in;
		LayerData[11].size_in = LayerData[11].size_in_per_ch
				* LayerData[11].c_in;
		LayerData[11].size_out = LayerData[11].l_out * LayerData[11].l_out;
		LayerData[11].size_out = LayerData[11].size_out_per_ch
				* LayerData[11].c_out;
		LayerData[11].weight_per_ch = 0;
		LayerData[11].weight_size = 0;
		LayerData[11].weight = NULL;
		LayerData[11].mod_biases = NULL;
		LayerData[11].mem_per_ch = LayerData[11].l_out * LayerData[11].l_out
				* FIXED_SIZE;
		LayerData[11].mem_size = (LayerData[11].mem_per_ch
				+ 2 * ((LayerData[11].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER11_CH_OUT;
		LayerData[11].mem = (fixed *) ((uint32_t) LayerData[10].mem
				+ LayerData[10].mem_size);
	}

	//Layer12
	{
		LayerData[12].l_in = LAYER12_LEN_IN;
		LayerData[12].l_out = LAYER12_LEN_OUT;
		LayerData[12].c_in = LAYER12_CH_IN;
		LayerData[12].c_out = LAYER12_CH_OUT;
		LayerData[12].filter_size = LAYER12_FILTER_SIZE;
		LayerData[12].layer_type = LAYER12_LAYER_TYPE;
		LayerData[12].size_in_per_ch = LayerData[12].l_in * LayerData[12].l_in;
		LayerData[12].size_in = LayerData[12].size_in_per_ch
				* LayerData[12].c_in;
		LayerData[12].size_out = LayerData[12].l_out * LayerData[12].l_out;
		LayerData[12].size_out = LayerData[12].size_out_per_ch
				* LayerData[12].c_out;
		LayerData[12].weight_per_ch = LayerData[12].c_out
				* LayerData[12].filter_size * LayerData[12].filter_size;
		LayerData[12].weight_size = (((LayerData[12].weight_per_ch
				+ (LayerData[12].weight_per_ch & 0x1)) * LAYER12_CH_IN))
				* FIXED_SIZE;
		LayerData[12].weight = (fixed *) ((uint32_t) LayerData[10].weight
				+ LayerData[10].weight_size);
		LayerData[12].mod_biases =
				(fixed *) ((uint32_t) LayerData[10].mod_biases
						+ LayerData[10].c_out * FIXED_SIZE);
		LayerData[12].mem_per_ch = LayerData[12].l_out * LayerData[12].l_out
				* FIXED_SIZE;
		LayerData[12].mem_size = (LayerData[12].mem_per_ch
				+ 2 * ((LayerData[12].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER12_CH_OUT;
		LayerData[12].mem = (fixed *) ((uint32_t) LayerData[11].mem
				+ LayerData[11].mem_size);
	}

	//Layer13
	{
		LayerData[13].l_in = LAYER13_LEN_IN;
		LayerData[13].l_out = LAYER13_LEN_OUT;
		LayerData[13].c_in = LAYER13_CH_IN;
		LayerData[13].c_out = LAYER13_CH_OUT;
		LayerData[13].filter_size = LAYER13_FILTER_SIZE;
		LayerData[13].layer_type = LAYER13_LAYER_TYPE;
		LayerData[13].size_in_per_ch = LayerData[13].l_in * LayerData[13].l_in;
		LayerData[13].size_in = LayerData[13].size_in_per_ch
				* LayerData[13].c_in;
		LayerData[13].size_out = LayerData[13].l_out * LayerData[13].l_out;
		LayerData[13].size_out = LayerData[13].size_out_per_ch
				* LayerData[13].c_out;
		LayerData[13].weight_per_ch = LayerData[13].c_out
				* LayerData[13].filter_size * LayerData[13].filter_size;
		LayerData[13].weight_size = (((LayerData[13].weight_per_ch
				+ (LayerData[13].weight_per_ch & 0x1)) * LAYER13_CH_IN))
				* FIXED_SIZE;
		LayerData[13].weight = (fixed *) ((uint32_t) LayerData[12].weight
				+ LayerData[12].weight_size);
		LayerData[13].mod_biases =
				(fixed *) ((uint32_t) LayerData[12].mod_biases
						+ LayerData[12].c_out * FIXED_SIZE);
		LayerData[13].mem_per_ch = LayerData[13].l_out * LayerData[13].l_out
				* FIXED_SIZE;
		LayerData[13].mem_size = (LayerData[13].mem_per_ch
				+ 2 * ((LayerData[13].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER13_CH_OUT;
		LayerData[13].mem = (fixed *) ((uint32_t) LayerData[12].mem
				+ LayerData[12].mem_size);
	}

	//Layer14
	{
		LayerData[14].l_in = LAYER14_LEN_IN;
		LayerData[14].l_out = LAYER14_LEN_OUT;
		LayerData[14].c_in = LAYER14_CH_IN;
		LayerData[14].c_out = LAYER14_CH_OUT;
		LayerData[14].filter_size = LAYER14_FILTER_SIZE;
		LayerData[14].layer_type = LAYER14_LAYER_TYPE;
		LayerData[14].size_in_per_ch = LayerData[14].l_in * LayerData[14].l_in;
		LayerData[14].size_in = LayerData[14].size_in_per_ch
				* LayerData[14].c_in;
		LayerData[14].size_out = LayerData[14].l_out * LayerData[14].l_out;
		LayerData[14].size_out = LayerData[14].size_out_per_ch
				* LayerData[14].c_out;
		LayerData[14].weight_per_ch = LayerData[14].c_out
				* LayerData[14].filter_size * LayerData[14].filter_size;
		LayerData[14].weight_size = (((LayerData[14].weight_per_ch
				+ (LayerData[14].weight_per_ch & 0x1)) * LAYER14_CH_IN))
				* FIXED_SIZE;
		LayerData[14].weight = (fixed *) ((uint32_t) LayerData[13].weight
				+ LayerData[13].weight_size);
		LayerData[14].mod_biases =
				(fixed *) ((uint32_t) LayerData[13].mod_biases
						+ LayerData[13].c_out * FIXED_SIZE);
		LayerData[14].mem_per_ch = LayerData[14].l_out * LayerData[14].l_out
				* FIXED_SIZE;
		LayerData[14].mem_size = (LayerData[14].mem_per_ch
				+ 2 * ((LayerData[14].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER14_CH_OUT;
		LayerData[14].mem = (fixed *) ((uint32_t) LayerData[13].mem
				+ LayerData[13].mem_size);
	}

	//Layer15
	{
		LayerData[15].l_in = LAYER15_LEN_IN;
		LayerData[15].l_out = LAYER15_LEN_OUT;
		LayerData[15].c_in = LAYER15_CH_IN;
		LayerData[15].c_out = LAYER15_CH_OUT;
		LayerData[15].filter_size = LAYER15_FILTER_SIZE;
		LayerData[15].layer_type = LAYER15_LAYER_TYPE;
		LayerData[15].size_in_per_ch = LayerData[15].l_in * LayerData[15].l_in;
		LayerData[15].size_in = LayerData[15].size_in_per_ch
				* LayerData[15].c_in;
		LayerData[15].size_out = LayerData[15].l_out * LayerData[15].l_out;
		LayerData[15].size_out = LayerData[15].size_out_per_ch
				* LayerData[15].c_out;
		LayerData[15].weight_per_ch = LayerData[15].c_out
				* LayerData[15].filter_size * LayerData[15].filter_size;
		LayerData[15].weight_size = (((LayerData[15].weight_per_ch
				+ (LayerData[15].weight_per_ch & 0x1)) * LAYER15_CH_IN))
				* FIXED_SIZE;
		LayerData[15].weight = (fixed *) ((uint32_t) LayerData[14].weight
				+ LayerData[14].weight_size);
		LayerData[15].mod_biases =
				(fixed *) ((uint32_t) LayerData[14].mod_biases
						+ LayerData[14].c_out * FIXED_SIZE);
		LayerData[15].mem_per_ch = LayerData[15].l_out * LayerData[15].l_out
				* FIXED_SIZE;
		LayerData[15].mem_size = (LayerData[15].mem_per_ch
				+ 2 * ((LayerData[15].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER15_CH_OUT;
		LayerData[15].mem = (fixed *) ((uint32_t) LayerData[14].mem
				+ LayerData[14].mem_size);
	}

	//Layer16
	{
		LayerData[16].l_in = 0;
		LayerData[16].l_out = 0;
		LayerData[16].c_in = 0;
		LayerData[16].c_out = 0;
		LayerData[16].filter_size = 0;
		LayerData[16].layer_type = LAYER16_LAYER_TYPE;
		LayerData[16].size_out = 0;
		LayerData[16].weight_per_ch = 0;
		LayerData[16].weight_size = 0;
		LayerData[16].weight = NULL;
		LayerData[16].mod_biases = NULL;
		LayerData[16].mem_size = 0;
		LayerData[16].mem = NULL;
	}

	//Layer17
	{
		LayerData[17].l_in = 0;
		LayerData[17].l_out = 0;
		LayerData[17].c_in = 0;
		LayerData[17].c_out = 0;
		LayerData[17].filter_size = 0;
		LayerData[17].layer_type = LAYER17_LAYER_TYPE;
		LayerData[17].size_out = 0;
		LayerData[17].weight_per_ch = 0;
		LayerData[17].weight_size = 0;
		LayerData[17].weight = NULL;
		LayerData[17].mod_biases = NULL;
		LayerData[17].mem_size = 0;
		LayerData[17].mem = NULL;
	}

	//Layer18
	{
		LayerData[18].l_in = LAYER18_LEN_IN;
		LayerData[18].l_out = LAYER18_LEN_OUT;
		LayerData[18].c_in = LAYER18_CH_IN;
		LayerData[18].c_out = LAYER18_CH_OUT;
		LayerData[18].filter_size = LAYER18_FILTER_SIZE;
		LayerData[18].layer_type = LAYER18_LAYER_TYPE;
		LayerData[18].size_in_per_ch = LayerData[18].l_in * LayerData[18].l_in;
		LayerData[18].size_in = LayerData[18].size_in_per_ch
				* LayerData[18].c_in;
		LayerData[18].size_out = LayerData[18].l_out * LayerData[18].l_out;
		LayerData[18].size_out = LayerData[18].size_out_per_ch
				* LayerData[18].c_out;
		LayerData[18].weight_per_ch = LayerData[18].c_out
				* LayerData[18].filter_size * LayerData[18].filter_size;
		LayerData[18].weight_size = (((LayerData[18].weight_per_ch
				+ (LayerData[18].weight_per_ch & 0x1)) * LAYER18_CH_IN))
				* FIXED_SIZE;
		LayerData[18].weight = (fixed *) ((uint32_t) LayerData[15].weight
				+ LayerData[15].weight_size);
		LayerData[18].mod_biases =
				(fixed *) ((uint32_t) LayerData[15].mod_biases
						+ LayerData[15].c_out * FIXED_SIZE);
		LayerData[18].mem_per_ch = LayerData[18].l_out * LayerData[18].l_out
				* FIXED_SIZE;
		LayerData[18].mem_size = (LayerData[18].mem_per_ch
				+ 2 * ((LayerData[18].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER18_CH_OUT;
		LayerData[18].mem = (fixed *) ((uint32_t) LayerData[15].mem
				+ LayerData[15].mem_size);
	}

	//Layer19
	{
		LayerData[19].l_in = LAYER19_LEN_IN;
		LayerData[19].l_out = LAYER19_LEN_OUT;
		LayerData[19].c_in = LAYER19_CH_IN;
		LayerData[19].c_out = LAYER19_CH_OUT;
		LayerData[19].filter_size = 0;
		LayerData[19].layer_type = LAYER19_LAYER_TYPE;
		LayerData[19].size_in_per_ch = LayerData[19].l_in * LayerData[19].l_in;
		LayerData[19].size_in = LayerData[19].size_in_per_ch
				* LayerData[19].c_in;
		LayerData[19].size_out = LayerData[19].l_out * LayerData[19].l_out;
		LayerData[19].size_out = LayerData[19].size_out_per_ch
				* LayerData[19].c_out;
		LayerData[19].weight_per_ch = 0;
		LayerData[19].weight_size = 0;
		LayerData[19].weight = NULL;
		LayerData[19].mod_biases = NULL;
		LayerData[19].mem_per_ch = LayerData[19].l_out * LayerData[19].l_out
				* FIXED_SIZE;
		LayerData[19].mem_size = (LayerData[19].mem_per_ch
				+ 2 * ((LayerData[19].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER19_CH_OUT;
		LayerData[19].mem = (fixed *) ((uint32_t) LayerData[18].mem
				+ LayerData[18].mem_size);
	}

	//Layer20
	{
		LayerData[20].l_in = 0;
		LayerData[20].l_out = 0;
		LayerData[20].c_in = 0;
		LayerData[20].c_out = 0;
		LayerData[20].filter_size = 0;
		LayerData[20].layer_type = LAYER20_LAYER_TYPE;
		LayerData[20].size_out = 0;
		LayerData[20].weight_per_ch = 0;
		LayerData[20].weight_size = 0;
		LayerData[20].weight = NULL;
		LayerData[20].mod_biases = NULL;
		LayerData[20].mem_size = 0;
		LayerData[20].mem = NULL;
	}

	//Layer21
	{
		LayerData[21].l_in = LAYER21_LEN_IN;
		LayerData[21].l_out = LAYER21_LEN_OUT;
		LayerData[21].c_in = LAYER21_CH_IN;
		LayerData[21].c_out = LAYER21_CH_OUT;
		LayerData[21].filter_size = LAYER21_FILTER_SIZE;
		LayerData[21].layer_type = LAYER21_LAYER_TYPE;
		LayerData[21].size_in_per_ch = LayerData[21].l_in * LayerData[21].l_in;
		LayerData[21].size_in = LayerData[21].size_in_per_ch
				* LayerData[21].c_in;
		LayerData[21].size_out = LayerData[21].l_out * LayerData[21].l_out;
		LayerData[21].size_out = LayerData[21].size_out_per_ch
				* LayerData[21].c_out;
		LayerData[21].weight_per_ch = LayerData[21].c_out
				* LayerData[21].filter_size * LayerData[21].filter_size;
		LayerData[21].weight_size = (((LayerData[21].weight_per_ch
				+ (LayerData[21].weight_per_ch & 0x1)) * LAYER21_CH_IN))
				* FIXED_SIZE;
		LayerData[21].weight = (fixed *) ((uint32_t) LayerData[18].weight
				+ LayerData[18].weight_size);
		LayerData[21].mod_biases =
				(fixed *) ((uint32_t) LayerData[18].mod_biases
						+ LayerData[18].c_out * FIXED_SIZE);
		LayerData[21].mem_per_ch = LayerData[21].l_out * LayerData[21].l_out
				* FIXED_SIZE;
		LayerData[21].mem_size = (LayerData[21].mem_per_ch
				+ 2 * ((LayerData[21].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER21_CH_OUT;
		LayerData[21].mem = (fixed *) ((uint32_t) LayerData[19].mem
				+ LayerData[19].mem_size);
	}

	//Layer22
	{
		LayerData[22].l_in = LAYER22_LEN_IN;
		LayerData[22].l_out = LAYER22_LEN_OUT;
		LayerData[22].c_in = LAYER22_CH_IN;
		LayerData[22].c_out = LAYER22_CH_OUT;
		LayerData[22].filter_size = LAYER22_FILTER_SIZE;
		LayerData[22].layer_type = LAYER22_LAYER_TYPE;
		LayerData[22].size_in_per_ch = LayerData[22].l_in * LayerData[22].l_in;
		LayerData[22].size_in = LayerData[22].size_in_per_ch
				* LayerData[22].c_in;
		LayerData[22].size_out = LayerData[22].l_out * LayerData[22].l_out;
		LayerData[22].size_out = LayerData[22].size_out_per_ch
				* LayerData[22].c_out;
		LayerData[22].weight_per_ch = LayerData[22].c_out
				* LayerData[22].filter_size * LayerData[22].filter_size;
		LayerData[22].weight_size = (((LayerData[22].weight_per_ch
				+ (LayerData[22].weight_per_ch & 0x1)) * LAYER22_CH_IN))
				* FIXED_SIZE;
		LayerData[22].weight = (fixed *) ((uint32_t) LayerData[21].weight
				+ LayerData[21].weight_size);
		LayerData[22].mod_biases =
				(fixed *) ((uint32_t) LayerData[21].mod_biases
						+ LayerData[21].c_out * FIXED_SIZE);
		LayerData[22].mem_per_ch = LayerData[22].l_out * LayerData[22].l_out
				* FIXED_SIZE;
		LayerData[22].mem_size = (LayerData[22].mem_per_ch
				+ 2 * ((LayerData[22].mem_per_ch & 0x3) ? 1 : 0))
				* LAYER22_CH_OUT;
		LayerData[22].mem = (fixed *) ((uint32_t) LayerData[21].mem
				+ LayerData[21].mem_size);
	}

	//Layer23
	{
		LayerData[23].l_in = 0;
		LayerData[23].l_out = 0;
		LayerData[23].c_in = 0;
		LayerData[23].c_out = 0;
		LayerData[23].filter_size = 0;
		LayerData[23].layer_type = LAYER23_LAYER_TYPE;
		LayerData[23].size_out = 0;
		LayerData[23].weight_per_ch = 0;
		LayerData[23].weight_size = 0;
		LayerData[23].weight = NULL;
		LayerData[23].mod_biases = NULL;
		LayerData[23].mem_size = 0;
		LayerData[23].mem = NULL;
	}
	return XST_SUCCESS;
}

void Layer_PrintfAllConfig(void) {
	xil_printf("----- Print All Layer Configuration -----\n\r");
	for (int layer = 0; layer < 24; ++layer) {
		xil_printf("Layer: %d\n\r", layer);
		xil_printf("\tlayer_type: %d\n\r", LayerData[layer].layer_type);

		xil_printf("\tl_in: %d\n\r", LayerData[layer].l_in);
		xil_printf("\tl_out: %d\n\r", LayerData[layer].l_out);
		xil_printf("\tc_in: %d\n\r", LayerData[layer].c_in);
		xil_printf("\tc_out: %d\n\r", LayerData[layer].c_out);
		xil_printf("\tfilter_size: %d\n\r", LayerData[layer].filter_size);
		xil_printf("\tsize_out: %d\n\r", LayerData[layer].size_out);
		xil_printf("\tmem_size: %d\n\r", LayerData[layer].mem_size);
		xil_printf("\tmem: 0x%08x\n\r", LayerData[layer].mem);
		xil_printf("\tweight_size: %d\n\r", LayerData[layer].weight_size);
		xil_printf("\tweight_per_ch: %d\n\r", LayerData[layer].weight_per_ch);
		xil_printf("\tweight: 0x%08x\n\r", LayerData[layer].weight);
		xil_printf("\tmod_biases: 0x%08x\n\r", LayerData[layer].mod_biases);
	}
	xil_printf("----- End Print All Layer Configuration -----\n\r");
}

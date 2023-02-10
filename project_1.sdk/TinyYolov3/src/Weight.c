#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"
#include "xsdps.h"

/******************** Include files **********************************/
#include "xil_exception.h"
#include "xparameters.h"

#include "Fixed.h"
#include "Layer.h"
#include "MemoryAddress.h"
#include "Weight.h"
#include "WeightFileConverter.h"

fixed *Weight, *mod_biases;

int Weight_Init(void)
{
	if (WEIGHT_START_ADDR & (4 - 1))
	{
		printf("WEIGHT_START_ADDR: Address is not aligned.\r\n");
		return XST_FAILURE;
	}
	Weight = (fixed *)WEIGHT_START_ADDR;
	mod_biases = (fixed *)MOD_BIASES_START_ADDR;

	return XST_SUCCESS;
}

int Weight_Load(void)
{
	char FOR_IP_WEIGHT_FILE[] = "IP.weights";

	FATFS FS_instance;
	FRESULT f_result;
	FIL fp;
	unsigned int br;
	char *Path = "0:/";

	f_result = f_mount(&FS_instance, Path, 0);
	if (f_result != FR_OK)
		printf("f_mount fail\r\n");
	else
		printf("Mount SD card success.\r\n");

	/*--------------------------------------------------------------------------------*/
	/*																				  */
	/* 								  Open the file									  */
	/*																				  */
	/*--------------------------------------------------------------------------------*/
	f_result = f_open(&fp, FOR_IP_WEIGHT_FILE, FA_READ);
	if (f_result != FR_OK)
	{
		printf("Open %s file fail. f_result=%d\r\n", FOR_IP_WEIGHT_FILE,
			   f_result);
		return XST_FAILURE;
	}
	else
		printf("Open %s file success.\r\n", FOR_IP_WEIGHT_FILE);
	/*--------------------------------------------------------------------------------*/
	/*																				  */
	/* 									Read from SD  		 						  */
	/*																				  */
	/*--------------------------------------------------------------------------------*/
	Layer_t *layer;
	fixed *weight_ptr;

	for (unsigned int layer_ptr = 0; layer_ptr < LAYER_NUM; layer_ptr++)
	{
		layer = GetLayer(layer_ptr);
		if (HasWeight(layer))
		{
			weight_ptr = layer->weight;
			/*
			if (HasBN(layer))
			{
				//BN_weight
				f_result = f_read(&fp, weight_ptr, layer->c_out * sizeof(fixed),
								  &br);
				if (f_result != FR_OK)
				{
					printf("BN_weight read error\r\n");
					return XST_FAILURE;
				}
				weight_ptr = AddFixedPtr(weight_ptr,
										 layer->c_out * sizeof(fixed));
			}
			*/
			//conv_weight
			f_result = f_read(&fp, weight_ptr,
							  AlignedWeightNum_ch(layer) * layer->c_in * sizeof(fixed),
							  &br);
			if (f_result != FR_OK)
			{
				printf("conv_weight read error\r\n");
				return XST_FAILURE;
			}

			//mod_biases
			f_result = f_read(&fp, layer->mod_biases, layer->c_out * sizeof(fixed),
							  &br);
			if (f_result != FR_OK)
			{
				printf("mod_biases read error\r\n");
				return XST_FAILURE;
			}
		}
	}
	/*
	for (unsigned int layer_ptr = 0; layer_ptr < LAYER_NUM; layer_ptr++)
	{
		layer = GetLayer(layer_ptr);
		if (HasWeight(layer))
		{
			printf("DEBUG: Layer %d\r\n", layer_ptr);
			weight_ptr = layer->weight;
			//if (HasBN(layer))
			//{
			//	printf("\t\t\tBN_weight[0]: %f, BN_weight[last]: %f\r\n",
			//		   fixed_to_float(weight_ptr[0]),
			//		   fixed_to_float(weight_ptr[layer->c_out - 1]));
			//	weight_ptr = AddFixedPtr(weight_ptr, layer->c_out * sizeof(fixed));
			//}
			printf("\t\t\tconv_weight[0]: %f, conv_weight[%d]: %f\r\n",
				   fixed_to_float(weight_ptr[0]),
				   AlignedWeightNum_ch(layer) * layer->c_in - (isChWeightNumAligned(layer) ? 1 : 2),
				   fixed_to_float(
					   weight_ptr[AlignedWeightNum_ch(layer) * layer->c_in - (isChWeightNumAligned(layer) ? 1 : 2)]));
			weight_ptr = layer->mod_biases;
			printf("\t\t\tmod_biases[0]: %f, mod_biases[%d]: %f\r\n",
				   fixed_to_float(weight_ptr[0]), layer->c_out - 1,
				   fixed_to_float(weight_ptr[layer->c_out - 1]));
		}
	}
	*/
	/*--------------------------------------------------------------------------------*/
	/*																				  */
	/* 								 	   Clear			  						  */
	/*																				  */
	/*--------------------------------------------------------------------------------*/
	f_result = f_close(&fp);
	if (f_result != FR_OK)
	{
		printf("f_close %s error\r\n", FOR_IP_WEIGHT_FILE);
	}

	f_result = f_mount(0, "", 0);
	if (f_result != FR_OK)
	{
		printf("f_mount clear error\r\n");
	}

	return XST_SUCCESS;
}

int HasWeight(Layer_t *Layer)
{
	return Layer->layer_type == CONV_NORMAL || Layer->layer_type == CONV_DBL;
}

int HasBN(Layer_t *Layer)
{
	return Layer->layer_type == CONV_DBL;
}

int isChWeightNumAligned(Layer_t *Layer)
{
	return !(Layer->weight_per_ch & 0x1);
}

unsigned int AlignedWeightNum_ch(Layer_t *Layer)
{
	return Layer->weight_per_ch + (isChWeightNumAligned(Layer) ? 0 : 1);
}

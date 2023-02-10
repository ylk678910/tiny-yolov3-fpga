#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"
#include "xsdps.h"

#include "Fixed.h"
#include "Layer.h"
#include "MemoryAddress.h"
#include "Weight.h"
#include "WeightFileConverter.h"

int WeightFileConvert(int config)
{
	char ORIGIN_WEIGHT_FILE[] = "yolov3-tiny.weights";
	char FOR_IP_WEIGHT_FILE[] = "IP.weights";

    FATFS FS_instance;
    FRESULT f_result;
    FIL org_fp, mod_fp;
    unsigned int org_br, mod_br;
    char *Path = "0:/";

    f_result = f_mount(&FS_instance, Path, 0);
    if (f_result != FR_OK)
        xil_printf("f_mount fail\r\n");
    else
        xil_printf("Mount SD card success.\r\n");

    /* Open the file */
    f_result = f_open(&mod_fp, FOR_IP_WEIGHT_FILE, ((config == 0) ? (FA_WRITE | FA_CREATE_NEW) : FA_WRITE));
    if (f_result != FR_OK)
    {
        if (f_result == FR_EXIST)
        {
            xil_printf("%s file EXIST. Do nothing\r\n", FOR_IP_WEIGHT_FILE);
            f_result = f_mount(0, "", 0);
            if (f_result != FR_OK)
                xil_printf("f_mount clear error\r\n");
            return XST_SUCCESS;
        }
        else
        {
            xil_printf("Open %s file fail. f_result=%d\r\n", FOR_IP_WEIGHT_FILE,
                       f_result);
            return XST_FAILURE;
        }
    }
    else
    {
        xil_printf("Open %s file success.\r\n", FOR_IP_WEIGHT_FILE);
    }

    f_result = f_open(&org_fp, ORIGIN_WEIGHT_FILE, FA_READ);
    if (f_result != FR_OK)
    {
        xil_printf("Open %s file fail. f_result=%d\r\n", ORIGIN_WEIGHT_FILE,
                   f_result);
        return XST_FAILURE;
    }
    else
        xil_printf("Open %s file success.\r\n", ORIGIN_WEIGHT_FILE);

    //xil_printf("file size = %d\r\n", f_size(&file));
    /*--------------------------------------------------------------------------------*/
    /*																				  */
    /* Read from SD  		 														  */
    /*																				  */
    /*--------------------------------------------------------------------------------*/

    f_lseek(&org_fp, 5 * sizeof(float));
    if (f_result != FR_OK)
    {
        xil_printf("file seek error\r\n");
        return XST_FAILURE;
    }

    /* Read data. */
    float *biases, *scales, *rolling_mean, *rolling_variance, *conv_weight,
        *BN_weight, *mod_biases;
    fixed *fixed_data, fixed_tmp = 0;
    Layer_t *layer;
    int ptr;

    biases = (float *)CPU_TMP_ADDR;

    for (uint32_t layer_ptr = 0; layer_ptr < LAYER_NUM; layer_ptr++)
    {
        layer = GetLayer(layer_ptr);
        if (HasWeight(layer))
        {
            /*	Read from original file.	*/
            //biases
            f_result = f_read(&org_fp, biases, layer->c_out * sizeof(float),
                              &org_br);
            if (f_result != FR_OK)
            {
                xil_printf("file data read error\r\n");
                return XST_FAILURE;
            }

            scales = biases + layer->c_out * sizeof(float);
            rolling_mean = scales + layer->c_out * sizeof(float);
            rolling_variance = rolling_mean + layer->c_out * sizeof(float);
            conv_weight = rolling_variance + layer->c_out * sizeof(float);
            if (HasBN(layer))
            {
                //scales
                f_result = f_read(&org_fp, scales, layer->c_out * sizeof(float),
                                  &org_br);
                if (f_result != FR_OK)
                {
                    xil_printf("file data read error\r\n");
                    return XST_FAILURE;
                }

                //rolling_mean
                f_result = f_read(&org_fp, rolling_mean,
                                  layer->c_out * sizeof(float), &org_br);
                if (f_result != FR_OK)
                {
                    xil_printf("file data read error\r\n");
                    return XST_FAILURE;
                }

                //rolling_variance
                f_result = f_read(&org_fp, rolling_variance,
                                  layer->c_out * sizeof(float), &org_br);
                if (f_result != FR_OK)
                {
                    xil_printf("file data read error\r\n");
                    return XST_FAILURE;
                }
            }
            //conv_weight
            f_result = f_read(&org_fp, conv_weight,
                              layer->c_in * layer->c_out * layer->filter_size * layer->filter_size * sizeof(float), &org_br);
            if (f_result != FR_OK)
            {
                xil_printf("file data read error\r\n");
                return XST_FAILURE;
            }

            printf("\tLayer %u\r\n", (unsigned int)layer_ptr);
            printf("\t\t\tbiases[0] = %f, biases[%d] = %f\n", biases[0],
                   layer->c_out - 1, biases[layer->c_out - 1]);
            if (HasBN(layer))
            {
                printf("\t\t\tscales[0] = %f, scales[%d] = %f\n", scales[0],
                       layer->c_out - 1, scales[layer->c_out - 1]);
                printf("\t\t\trolling_mean[0] = %f, rolling_mean[%d] = %f\n",
                       rolling_mean[0], layer->c_out - 1,
                       rolling_mean[layer->c_out - 1]);
                printf(
                    "\t\t\trolling_variance[0] = %f, rolling_variance[%d] = %f\n",
                    rolling_variance[0], layer->c_out - 1,
                    rolling_variance[layer->c_out - 1]);
            }
            printf("\t\t\tconv_weight[0] = %f, conv_weight[%d] = %f\n",
                   conv_weight[0],
                   layer->c_in * layer->c_out * layer->filter_size * layer->filter_size - 1,
                   conv_weight[layer->c_in * layer->c_out * layer->filter_size * layer->filter_size - 1]);
            /*	Calculate BN_weight.	*/
            //BN_weight
            if (HasBN(layer))
            {
                BN_weight = scales;
                for (ptr = 0; ptr < layer->c_out; ptr++)
                {
                    BN_weight[ptr] /= (float)sqrt(
                        (double)rolling_variance[ptr]);
                }
            }
            /*	Calculate conv_weight.	*/
            int weight_size_filter = layer->filter_size * layer->filter_size * layer->c_in;
            for (int f = 0; f < layer->c_out; ++f)
            {
                for (int x = 0; x < weight_size_filter; ++x)
                {
                	conv_weight[f * weight_size_filter + x] *= BN_weight[f];
                }
            }
            f_result = f_read(&org_fp, conv_weight,
                              layer->c_in * layer->c_out * layer->filter_size * layer->filter_size * sizeof(float), &org_br);
            /*	Calculate mod_biases.	*/
            //mod_biases
            mod_biases = biases;
            if (HasBN(layer))
            {
                for (ptr = 0; ptr < layer->c_out; ptr++)
                {
                    mod_biases[ptr] -= BN_weight[ptr] * rolling_mean[ptr];
                }
            }
            /********************************/
            /*								*/
            /*		  Store BN_weight		*/
            /*								*/
            /********************************/
            /*
            if (HasBN(layer))
            {
                //	float to fixed
                fixed_data = (fixed *)BN_weight;
                for (ptr = 0; ptr < layer->c_out; ptr++)
                {
                    fixed_data[ptr] = float_to_fixed(BN_weight[ptr]);
                }
                //	Store
                f_result = f_write(&mod_fp, fixed_data,
                                   layer->c_out * sizeof(fixed), &mod_br);
                if (f_result != FR_OK)
                {
                    xil_printf("BN_weight writes error\r\n");
                    return XST_FAILURE;
                }
            }
            */
            /********************************/
            /*								*/
            /*	     Store conv_weight	 	*/
            /*								*/
            /********************************/
            /*	float to fixed 	*/
            fixed_data = (fixed *)conv_weight;
            for (ptr = 0;
                 ptr < layer->c_in * layer->c_out * layer->filter_size * layer->filter_size; ptr++)
            {
                fixed_data[ptr] = float_to_fixed(conv_weight[ptr]);
            }
            printf("\t\t\tconv_weight(fixed)[0] = %f, conv_weight(fixed)[%d] = %f\n",
            		fixed_to_float(fixed_data[0]),
                   layer->c_in * layer->c_out * layer->filter_size * layer->filter_size - 1,
				   fixed_to_float(fixed_data[layer->c_in * layer->c_out * layer->filter_size * layer->filter_size - 1]));
            /*	Store 	*/
            for (unsigned int ch = 0; ch < layer->c_in; ch++)
            {
				for(unsigned int filter=0; filter<layer->c_out; filter++)
				{
					f_result = f_write(&mod_fp, &(fixed_data[filter*(layer->filter_size*layer->filter_size*layer->c_in) + ch*(layer->filter_size * layer->filter_size)]),
									   layer->filter_size * layer->filter_size * sizeof(fixed), &mod_br);
					if (f_result != FR_OK)
					{
						xil_printf("conv_weight writes error\r\n");
						return XST_FAILURE;
					}
				}

                if (!isChWeightNumAligned(layer))
                { //Memory Alignment
                    f_result = f_write(&mod_fp, &fixed_tmp, sizeof(fixed),
                                       &mod_br);
                    if (f_result != FR_OK)
                    {
                        xil_printf("conv_weight writes error\r\n");
                        return XST_FAILURE;
                    }
                }
            }

            /********************************/
            /*								*/
            /*		  Store mod_biases		*/
            /*								*/
            /********************************/
            /*	float to fixed 	*/
            fixed_data = (fixed *)mod_biases;
            for (ptr = 0; ptr < layer->c_out; ptr++)
            {
                fixed_data[ptr] = float_to_fixed(mod_biases[ptr]);
            }
            /*	Store	*/
            f_result = f_write(&mod_fp, fixed_data,
                               layer->c_out * sizeof(fixed), &mod_br);
            if (f_result != FR_OK)
            {
                xil_printf("mod_biases writes error\r\n");
                return XST_FAILURE;
            }
            printf("mod_fptr = %u\r\n", (unsigned int)f_tell(&mod_fp));
        }
    }

    /*--------------------------------------------------------------------------------*/
    /*																				  */
    /* Clear			  	 														  */
    /*																				  */
    /*--------------------------------------------------------------------------------*/
    f_result = f_close(&org_fp);
    if (f_result != FR_OK)
        xil_printf("f_close org_fp error\r\n");

    f_result = f_close(&mod_fp);
    if (f_result != FR_OK)
        xil_printf("f_close mod_fp error\r\n");

    f_result = f_mount(0, "", 0);
    if (f_result != FR_OK)
        xil_printf("f_mount clear error\r\n");
    return XST_SUCCESS;
}

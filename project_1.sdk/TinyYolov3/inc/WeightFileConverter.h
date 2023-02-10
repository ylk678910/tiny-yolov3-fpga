#include <stdint.h>
#include <stdio.h>

typedef struct
{
    float *biases;
    float *scales;
    float *rolling_mean;
    float *rolling_variznce;
    float *weight;
}OriginWeight_DBL_t;

typedef struct
{
    float *biases;
    float *weight;
} OriginWeight_Conv_t;

int WeightFileConvert(int config);

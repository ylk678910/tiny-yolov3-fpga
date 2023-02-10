/*
 * image.h
 *
 *  Created on: 2021年11月1日
 *      Author: ylk67
 */

#ifndef INC_IMAGE_H_
#define INC_IMAGE_H_

typedef struct
{
    int w;
    int h;
    int c;
    float *data;
} image;

image load_image_color(char *filename, int w, int h);
image letterbox_image(image im, int w, int h);
void free_image(image m);

#endif /* INC_IMAGE_H_ */

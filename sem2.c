#include "seamcarving.h"
#include "c_img.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    // Creates gradient matrix
    create_img(grad, im -> height, im -> width);
    
    // nt, nb, nl, nr = neighbour top, bottom, left, right
    for(int h=0; h < (*grad) -> height; h++){
        int nb = h + 1;
        int nt = h - 1;

        if(nt < 0){ //If on top edge
            nt = (*grad) -> height - 1;
        } else if(nb > (*grad) -> height-1){ //If on bottom edge
            nb = 0;
        }

        for(int w=0; w < (*grad) -> width; w++){
            int nl = w - 1;
            int nr = w + 1;
            if(nl < 0){ //If on left edge
                nl = (*grad) -> width - 1;
            } else if(nr > (*grad) -> width-1){ //If on right edge
                nr = 0;
            }

            //Access pixels and calc gradient by colour
            int accum = 0;// Saves component gradients of each pixel
            for(int c=0; c<3; c++){
                uint8_t pnt = get_pixel(im, nt, w, c);
                uint8_t pnb = get_pixel(im, nb, w, c);
                uint8_t pnl = get_pixel(im, h, nl, c);
                uint8_t pnr = get_pixel(im, h, nr, c);

                int hor_d2 = pow(pnr - pnl, 2);
                int ver_d2 = pow(pnb - pnt, 2);
                accum += (hor_d2 + ver_d2);
            }
            uint8_t gradpt = (uint8_t)(sqrt(accum)/10);
            set_pixel(*grad, h, w, gradpt, 0, 0); //Stores in red channel of grad
        } 
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    //This will be indexed by rows first, then cols
    *best_arr = (double *)calloc(grad->height * grad->width, sizeof(double));
    //Initialize top row
    for(int w=0; w < grad -> width; w++){
        (*best_arr)[w] = (double)get_pixel(grad, 0, w, 0);
    }
    //Go down the rows and apply the recursive formula
    for(int h=1; h < grad -> height; h++){
        for(int w=0; w < grad -> width; w++){
            //Only nearest neighbours can be picked; so min(pixelw-1, pixel w, pixel w+1) of previous row
            double nleft = (*best_arr)[(h-1)*(grad->width) + (int)fmax(w-1, 0)]; //If at edge, will double access ntop, but it's okay
            double ntop = (*best_arr)[(h-1)*(grad->width) + (int)fmax(w, 0)];
            double nright = (*best_arr)[(h-1)*(grad->width) + (int)fmin(w+1, grad->width-1)];

            double cur_pix = (double)get_pixel(grad, h, w, 0);
            (*best_arr)[(h*grad->width) + w] = (double)(cur_pix + fmin(ntop, fmin(nleft, nright)));
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    *path = (int*)malloc(height*sizeof(int)); //Stores column position of pixel to be removed in each row
    //Initialize bottom row
    double smallest = best[(height-1)*width]; //Initialize smallest as leftmost bottom corner pixel
    int i_smallest = 0;
    for(int w=1; w < width; w++){
        if((best)[((height-1)*width) + w] < smallest){
            smallest = best[((height-1)*width) + w];
            i_smallest = w;
        }
    (*path)[height-1] = i_smallest; //Keeps starting position col index

    } 
    for(int h=height-2; h >= 0; h--){
        //Find smallest value in row, then append to path
        //Only nearest neighbours can be picked; so min(pixelw-1, pixel w, pixel w+1) of previous row
        int w = (*path)[h+1];
        double left = best[h*width + (w-1 < 0 ? 0 : w-1)];
        double middle = best[h*width + w];
        double right = best[h*width + (w+1 >= width ? width-1 : w+1)];
        if(left < middle && left < right){
            (*path)[h] = w-1 < 0 ? 0 : w-1;
        }
        else if(right < middle && right < left){
            (*path)[h] = w+1 >= width ? width-1 : w+1;
        }
        else{
            (*path)[h] = w;
        }
    }

    /* WRONG SOLUTION
        if(best[h*width + i_smallest - 1] < best[h*width + i_smallest] && best[h*width + i_smallest - 1] < best[h*width + i_smallest + 1]){
            i_smallest--;
        }
        else if(best[h*width + i_smallest] > best[h*width + i_smallest + 1]){
            i_smallest++;
        }
        (*path)[h] = i_smallest;
    }
    */
}


void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    create_img(dest, src -> height, src -> width-1);
    for(int h=0; h < src -> height; h++){
        int passed = 0; //Records whether the removed pixel has passed; 1 if passed
        for(int w=0; w < src -> width; w++){
            //If column of pixel not equal to path entry, write to dest; else skip
            if(w == path[h]){
                passed = 1; //Makes future iterations along this row skip one pixel
            }else{
                uint8_t r = get_pixel(src, h, w, 0);
                uint8_t g = get_pixel(src, h, w, 1);
                uint8_t b = get_pixel(src, h, w, 2);
                set_pixel(*dest, h, w-passed, r, g, b);
            }
        }
    }
}

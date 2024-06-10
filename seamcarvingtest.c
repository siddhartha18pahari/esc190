#include "seamcarving.h"
#include "c_img.h"
#include <stdio.h>

int main(){
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "Boeing747.bin");
    
    for(int i = 0; i <= 1800; i++){
        printf("Iteration %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        if(i%200 == 0){
            char filename[200];
            sprintf(filename, "747Sx%dP.bin", i);
            write_img(cur_im, filename);
        }

        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);
}

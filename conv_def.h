#ifndef _H_CONV_DEF
#define _H_CONV_DEF

struct feature_map{
	int h;
	int w;
	double *data;
	int *prev_px;
};
struct kernel{
	int w;
	double *data;
};

struct feature_map *init_fmaps(int n_fmaps, int w, int h);
void free_fm(struct feature_map *fm);

struct kernel *init_kernels(int n_conv_layers, int n_kernels, int kernel_width);
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "net_errno.h"
#include "net_structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iplimage.h"
#include "conv_def.h"
#include "conv.h"
#include "pool.h" 
#include "relu.h"
#include "net_structs.h"
#include "netpass.h"

double *netfpass(struct neuronet *net, double *inp)
{
	int i, j, k;
	double *out;
	double *w;
	
	if ((out = (double *)malloc(sizeof(double) * net->total_nn)) == NULL) {
		net_errno = NET_ENOMEM;
		out = NULL;
		goto exit_failure;
	}
		
	w = net->w;

	for (i = 0; i < net->nl; i++) {
		for (j = 0; j < net->nn[i]; j++) {
			double sum = 0.0;
			int nw = net->nw[i];
			//printf("w = %lf\n", *w);
			for (k = 0; k < nw; k++) 
				sum += (*w++) * inp[k];
			//printf("sum = %lf\n", sum);
			*out++ = 1.0 / (1.0 + exp(-sum));
			//printf("out = %lf\n", *(out - 1));
		}
 
		inp = out - net->nn[i];
	}
	
	out -= net->total_nn;

	return out;

exit_failure:
	return NULL;
}

static double *connect_fm(struct feature_map *fm, int n)
{
	double *res;
	int i, j, g;
	g = 0;
	if ((res = (double *)malloc(sizeof(double) * n * fm->w * fm->h)) == NULL) {
		fprintf(stderr, "error in malloc res\n");
		goto exit_failure;	
	}

	for (i = 0; i < n; i++)
		for (j = 0; j < fm[i].w * fm[i].h; j++)
			res[g++] = fm[i].data[j];

	return res;

exit_failure:
	return NULL;
}

static void norm_val(double *data, int k, double max_val)
{
	int i;
	for (i = 0; i < k; i++)
		data[i] = (double)data[i] / (double)max_val;	
}


int netbpass(struct neuronet *net, double *inp, double *out, double *target, struct convnet *cnet, struct data *src, double eta)
{
	double *errors, *w;
	int i, j, k, m, ne, nl, nn, nw;
	int x, y, g, u, v;
	double sum;
		
	int newne;
	double *newerrors;
	
	nl = net->nl;
	ne = net->nn[nl - 1];
	target += ne - 1;
	out += net->total_nn - 1;
	w = net->w + net->total_nw - 1;
	inp += net->nw[0] - 1;
	if ((errors = (double *)malloc(sizeof(double) * ne)) == NULL) {
		net_errno = NET_ENOMEM;
		goto exit_failure;
	}
	errors += ne - 1;

	for (i = 0; i < ne; i++, out--, target--) 
		*(errors - i) = (*out) * (1.0 - (*out)) * ((*target) - (*out));
	
	
	for (i = nl - 1; i >= 0; i--) {
		nn = net->nn[i];
		nw = net->nw[i]; 
		if (i > 0) {
			for (j = 0, m = 0; j < nn; j++)  
				for (k = 0; k < nw; k++, m++)
					*(w - m) += eta * *(errors - j) * *(out - k);
		} else {
			for (j = 0, m = 0; j < nn; j++)
				for (k = 0; k < nw; k++, m++) 
					*(w - m) += eta * *(errors - j) * *(inp - k);
		}
		if (i > 0) {
			newne = net->nn[i - 1];
			if ((newerrors = (double *)calloc(newne, sizeof(double))) == NULL) {
				net_errno = NET_ENOMEM;
				goto free_errors;
			}
			newerrors += newne - 1; 
			for (j = 0; j < newne; j++, out--) {
				for (k = 0; k < ne; k++) 
					*(newerrors - j) += *w-- * (*(errors - k));
				*(newerrors - j) *= *out * (1.0 - *out);
			}
			free(errors - ne + 1);
			errors = newerrors;
			ne = newne;
		} else {
			newne = net->nw[i];
			if ((newerrors = (double *)calloc(newne, sizeof(double))) == NULL) {
				net_errno = NET_ENOMEM;
				goto free_errors;
			}
			newerrors += newne - 1; 
			for (j = 0; j < newne; j++, out--) {
				for (k = 0; k < ne; k++) 
					*(newerrors - j) += *w-- * (*(errors - k));
			}
			free(errors - ne + 1);
			errors = newerrors - newne + 1;
			ne = newne;	
		}
	}
	newne = cnet->n_kernels * cnet->fmaps->w * cnet->fmaps->h;
	newerrors = (double *)malloc(sizeof(double) * newne);
	for (i = 0, g = 0, j = 0; i < cnet->n_kernels; i++) {
		for (y = 0; y < cnet->fmaps->h - 1; y += 2)
			for (x = 0; x < cnet->fmaps->w - 1; x += 2, j++)
				for (v = y; v < y + 2; v++)
					for (u = x; u < x + 2; u++, g++) {
						newerrors[g] = cnet->fmaps[i].data[v * cnet->fmaps->w + u] * (1.0 - cnet->fmaps[i].data[v * cnet->fmaps->w + u]) * errors[j];
						printf("%lf | %lf /// ", newerrors[g], errors[j]);
					}
		printf("\n");
	}
	sum = 0;
	for (i = 0; i < cnet->n_kernels; i++)
		for (y = 0; y < cnet->knls->w; y++)
			for (x = 0; x < cnet->knls->w; x++) {
				sum = cnet->knls[i].data[y * cnet->k_width + x];
				k = 1;
				for (u = y, g = i * (newne / cnet->n_kernels); u < y + (src->h - ((cnet->k_width / 2) * 2)); u++)
					for (v = x; v < x + (src->w - ((cnet->k_width / 2) * 2)); v++, g++) {
						sum += cnet->knls[i].data[y * cnet->k_width + x] + src->data[u * src->w + v] * newerrors[g] * eta;
						k++;
					}
				cnet->knls[i].data[y * cnet->k_width + x] = sum / k;
			}

	free(newerrors);
	free(errors);
	return 0;

free_errors:
	free(errors);
exit_failure:
	return -1;
}


double *convfpass(struct data *frame, struct convnet *cnet, struct neuronet *net)
{
	int i, x, y;
	double *data, *out;
	
	for (i = 0; i < cnet->n_kernels; i++) {
		for (y = 0; y < cnet->k_width; y++) {
			for (x = 0; x < cnet->k_width; x++)
				printf("%3.1lf ", cnet->knls[i].data[y * cnet->k_width + x]);
			printf("\n");
		}
		printf("\n");
	}
	if((cnet->fmaps = init_fmaps(cnet->n_kernels, frame->w - (cnet->k_width / 2 * 2), frame->h - (cnet->k_width / 2 * 2))) == NULL) {
		fprintf(stderr, "error in init fmaps\n");
		goto exit_failure;
	}
	if((cnet->pmaps = init_fmaps(cnet->n_kernels, (frame->w - (cnet->k_width / 2 * 2)) / 2, (frame->h - (cnet->k_width / 2 * 2)) / 2)) == NULL) {
		fprintf(stderr, "error in init pooled_maps");
		goto exit_failure;
	}

	//ipl_scaleimg(&frame, 25, 10);
	
	for (i = 0; i < cnet->n_kernels; i++) {
		if(conv(frame, &(cnet->fmaps[i]), &(cnet->knls[i])) == -1) {
			fprintf(stderr, "error in convolution\n");
			goto exit_failure;
		}
		for (y = 0; y < cnet->fmaps->h; y++) {
			for (x = 0; x < cnet->fmaps->w; x++)
				printf("%3.1lf ", cnet->fmaps[i].data[y * cnet->fmaps->w + x]);
			printf("\n");
		}
		if(pool(&(cnet->fmaps[i]), &(cnet->pmaps[i])) == -1) {
			fprintf(stderr, "error in pooling\n");
			goto exit_failure;
		}
		relu(&(cnet->pmaps[i]));
	}
	if ((data = connect_fm(cnet->pmaps, cnet->n_kernels)) == NULL) {
		fprintf(stderr, "error in connect feature maps\n");
		goto exit_failure;
	}
	//norm_val(data, cnet->pmaps->w * cnet->pmaps->h * cnet->n_kernels, (double)(cnet->k_width * cnet->k_width));
	out = netfpass(net, data);
	
	printf("\n");
	for (y = 0; y < cnet->pmaps->h * cnet->n_kernels; y++) {
		for (x = 0; x < cnet->pmaps->w; x++)
			printf("%3.1lf  ", data[y * cnet->pmaps->w + x]);
		printf("\n");
	}
		
	//printf("out = %lf\n", *(out));	

	return out;

exit_failure:
	return NULL;

}

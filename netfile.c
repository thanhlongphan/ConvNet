#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net_structs.h"
#include "net_errno.h"
#include "conv_def.h"

void nettofile(struct neuronet *net, struct convnet *cnet, char *path)
{
	int i, j;
	FILE *f;
	double *w = net->w;		 //for moving pointer
	
	if ((f = fopen(path, "w+")) == NULL) {
		fprintf(stderr, "Can not open file %s: %s\n", path, strerror(errno));
		return;
	}
	//f = stdout;
	fprintf(f, "%d\n", net->nl);
	for (i = 0; i < net->nl; i++)	
		fprintf(f, "%d\n", *(net->nn + i));
	for (i = 0; i < net->nl; i++)	
		fprintf(f, "%d\n", *(net->nw + i));

	fprintf(f, "%d\n", net->total_nn);
	fprintf(f, "%d\n", net->total_nw);

	for (i = 0; i < net->nl; i++)
		for (j = 0; j < *(net->nn + i) * *(net->nw + i); j++)
			fprintf(f, "%lf\n", *w++);

	fprintf(f, "%d\n", cnet->n_kernels);
	fprintf(f, "%d\n", cnet->k_width);
	for (i = 0; i < cnet->n_kernels; i++)
		for (j = 0; j < cnet->k_width * cnet->k_width; j++)
			fprintf(f, "%lf\n", cnet->knls[i].data[j]);	
	
	fclose(f);
}

int netfromfile(struct neuronet *net, struct convnet *cnet, char *path)
{
	int i, j;
	FILE *f;

	if ((f = fopen(path, "r")) == NULL) {
		net_errno = NET_EINVAL;
		goto exit_failure;
	}
	/*fseek(f, 0, SEEK_END);
	if(!ftell(f)) {
		rewind(f);
		printf("Neurodata is empty. Creating new net\n");
		return 1;
	}
	rewind(f);*/

	fscanf(f, "%d\n", &(net->nl));

	net->nn = (int *)malloc(sizeof(int) * net->nl);
	net->nw = (int *)malloc(sizeof(int) * net->nl);
	
	for (i = 0; i < net->nl; i++)
		fscanf(f, "%d\n", net->nn + i);

	for (i = 0; i < net->nl; i++)
		fscanf(f, "%d\n", net->nw + i);
	
	fscanf(f, "%d\n", &(net->total_nn));
	fscanf(f, "%d\n", &(net->total_nw));
	
	net->w = (double *)malloc(sizeof(double) * net->total_nw);

	for (i = 0; i < net->total_nw; i++)
		fscanf(f, "%lf\n", net->w++);
	
	net->w -= net->total_nw;

	fscanf(f, "%d\n", &(cnet->n_kernels));
	fscanf(f, "%d\n", &(cnet->k_width));
	for (i = 0; i < cnet->n_kernels; i++)
		for (j = 0; j < cnet->k_width * cnet->k_width; j++)
			fscanf(f, "%lf\n", &(cnet->knls[i].data[j]));

	fclose(f);

	return 0;

exit_failure:
	return -1;
}

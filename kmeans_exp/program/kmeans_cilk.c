#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

#define MICRO_IN_SEC 1000000.00

#define sqr(x)				((x)*(x))
#define MAX_ITERATIONS		100
#define DOUBLE_INFINITY		(INFINITY)
#define CLUSTER_NUM			1000

void kmeans(
			int		dim,						// dimension of data 
			double	*ipmatrix,					// pointer to data
			int		numele,						// number of elements
			int		cluster_num,				// number of clusters
			double	*centmatrix_final,			// output cluster centroids
			int		*cluster_assignment_final	// output
		   );
//__declspec(target(mic)) double begin_time, end_time;
double begin_time, end_time;
//__declspec(target(mic))
double microtime(){
	int tv_sec,tv_usec;
	double time;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);

	return tv.tv_sec+tv.tv_usec/MICRO_IN_SEC;
}

//calculate the distance between points p1 and p2
//__declspec(target(mic))
inline double calc_distance(int dim, double *p1, double *p2)
{
	double distance_sq_sum = 0;
	int ii=0; 
	for (ii = 0; ii < dim; ii++)
	  distance_sq_sum += sqr(p1[ii] - p2[ii]);

	return distance_sq_sum;

}

inline void fail(char * const str) {
	printf("%s\n",str);
	exit(-1);
}

//print the final result
void cluster_diag(int dim, int n, int k, double *X, int *cluster_assignment_index, double *cluster_centroid)
{
	int i;
	int cluster_member_count[CLUSTER_NUM];
	for(i=0; i < k; i++) {
		cluster_member_count[i] = 0;
	}

	for(i=0; i < n; i++) {
		cluster_member_count[ cluster_assignment_index[i] ]++;
	}

	printf("  Final clusters \n");

	for ( i = 0; i < k; i++) 
	  printf("    cluster %d:     members: %8d, centroid (%.1f %.1f) \n", i, cluster_member_count[i], cluster_centroid[i*dim + 0], cluster_centroid[i*dim + 1]);
}

void copy_result(int in, int * const isrc, int * const itgt, int dn, double * const dsrc, double * const dtgt)
{
	int i;
	for (i = 0; i < in; i++)
	  itgt[i] = isrc[i];
	for(i = 0; i < dn; i++)
		dtgt[i] = dsrc[i];
}

int main(int argc, char** argv)
{
	int numele=0;	//number of rows
	int dim = 0;	//number of columns
	int i,j,k;
	FILE *ipfile;	//input file

	begin_time = microtime();

	if ((ipfile = fopen(argv[1],"r"))==NULL)
	{
		printf("Error: can't open input file %s\n",argv[1]);
		exit(1);
	} 
	if(fscanf(ipfile,"# num rows=%d num columns=%d",&numele,&dim)!=2)
	{
		printf("Format error in first line\n");
		exit(1);
	} else {
		printf("num rows=%d num columns=%d\n",numele,dim);
	}

	double * ipmatrix=(double *)malloc(numele*dim*sizeof(double));	//ipmatrix is the pointer of the array of input matrix
	if (ipmatrix==NULL) {
		printf(" malloc failed\n");
		exit(1);
	}

	for (i=0; i<numele; i++)
	{
		for (j=0; j<dim; j++) 
		{
			fscanf(ipfile,"%lf",&(ipmatrix[i*dim+j]));
		}
	}

	int * cluster_assignment_final = (int *) malloc(sizeof(int)*numele);	//the cluster assignment of every row data of ipmatrix

	double * centmatrix_final = (double *) malloc (CLUSTER_NUM*dim*sizeof(double));	//centmatrix is the pointer of the array of initial centroid


	kmeans(dim, ipmatrix, numele, CLUSTER_NUM, centmatrix_final, cluster_assignment_final);	//call kmeans

	printf("successful\n");
	cluster_diag(dim, numele, CLUSTER_NUM, ipmatrix, cluster_assignment_final, centmatrix_final);
	free(ipmatrix);
	free(cluster_assignment_final);
	free(centmatrix_final);
	return 0;
}

int kmeans_itera_operator(
			int		dim,
			double	*ipmatrix,
			int		numele,
			int		cluster_num,
			double	*centmatrix_cur,
			double	*centmatrix_pre,
			int		*cluster_assignment_cur,
			int		*cluster_assignment_pre,
			double	*tot_D
			) {
	int		*cluster_count	= (int *)malloc(sizeof(int) * cluster_num);
//	int		change_count	= 0;
	int i,j,cluster_assignment;
	
	cluster_assignment=0;

	double min_D,dist;
	// init
//	double tot_D_tmp = 0;
	for(i=0; i < cluster_num; i++) {
		cluster_count[i] = 0;
		for(j=0; j < dim; j++) {
			centmatrix_cur[i*dim+j] = 0;
		}
	}

	end_time = microtime();
	printf("serial part cost time: %fs\n",end_time-begin_time);
	begin_time = end_time;
	//calculate all distance and choose the cluster, at the same time calculate total distance and new centroids.
/*
#pragma offload target(mic) \
	in(ipmatrix:length(numele*dim) alloc_if(1) free_if(1)) \
	in(centmatrix_pre:length(cluster_num*dim) alloc_if(1) free_if(1)) \
	in(cluster_assignment_pre:length(numele) alloc_if(1) free_if(1)) \
	out(cluster_assignment_cur:length(numele) alloc_if(1) free_if(1))
*/
	cilk::reducer_opadd<double> tot_D_tmp(0);
	cilk::reducer_opadd<int> change_count(0);
	//double tot_D_tmp=0;
	//int change_count=0;
//#pragma omp parallel for private(min_D, cluster_assignment, dist) reduction(+:tot_D_tmp, change_count)
	cilk_for(int i=0; i < numele; i++) {
		min_D = DOUBLE_INFINITY;
		cluster_assignment = -1;
		//calculate the distance and choose the cluster
		for(int j=0; j < cluster_num; j++) {
			dist = calc_distance(dim, ipmatrix+i*dim, centmatrix_pre+j*dim);

			if(dist < min_D) {
				min_D = dist;
				cluster_assignment = j;
			}

		}

		if(cluster_assignment != cluster_assignment_pre[i]) {
			change_count++ ;
		}

		//calculate the total distance and new centroids
		cluster_assignment_cur[i] = cluster_assignment;
		tot_D_tmp += min_D;
	}

	end_time = microtime();
	printf("parallel part cost time: %fs\n",end_time-begin_time);
	begin_time = end_time;

	for(i=0; i< numele; i++) {
		cluster_count[ cluster_assignment_cur[i] ]++;
		for(j=0; j < dim; j++) {
			centmatrix_cur[ cluster_assignment_cur[i] * dim + j] += ipmatrix[i*dim + j];
		}
	}
//	for(i=0;i<20; i++)
//	  printf("cluster%d count:%d\n",i,cluster_count[i]);

	//*tot_D = tot_D_tmp;
	*tot_D = tot_D_tmp.get_value();
	for(i=0; i < cluster_num; i++) {
		if( cluster_count[i] < 1 )
		  continue;
		for(j=0; j < dim; j++) {
			centmatrix_cur[i*dim + j] = centmatrix_cur[i*dim + j] / cluster_count[i];
		}
	}

	end_time = microtime();
	printf("serail part cost time: %fs\n",end_time-begin_time);
//	if (cluster_count !=NULL)
//	free(cluster_count);
	//return change_count;
	return change_count.get_value();
}

void kmeans(
			int		dim,						// dimension of data 
			double	*ipmatrix,					// pointer to data
			int		numele,						// number of elements
			int		cluster_num,				// number of clusters
			double	*centmatrix_final,			// output cluster centroids
			int		*cluster_assignment_final	// output
		   )
{
	int		*cluster_assignment_cur		= (int *)malloc(sizeof(int) * numele);
	int		*cluster_assignment_pre		= (int *)malloc(sizeof(int) * numele);
	double	*centmatrix_cur				= (double *)malloc(sizeof(double) * cluster_num * dim);
	double	*centmatrix_pre				= (double *)malloc(sizeof(double) * cluster_num * dim);
	int		i, j, *cluster_assignment_tmp;
	double totD, prev_totD, *centmatrix_tmp;

	if (!cluster_assignment_cur || !cluster_assignment_pre || !centmatrix_pre || !centmatrix_cur)
	  fail("Error allocating memory!");

	// initial setup
	//choose the first CLUSTER_NUM of ipmatrix as the centmatrix
	for( i=0; i< cluster_num * dim ; i++)
	{
		centmatrix_pre[i] = ipmatrix[i];
	}
	kmeans_itera_operator(dim, ipmatrix, numele, cluster_num, centmatrix_cur, centmatrix_pre, cluster_assignment_cur, cluster_assignment_pre, &totD);
	prev_totD = DOUBLE_INFINITY;

	exit(0);

	// Iteration calculate
	int batch_iteration = MAX_ITERATIONS;
	while ( batch_iteration-- )
	{
		printf(" batch_iteration = %d\n", batch_iteration);
		//	cluster_diag(dim, numele, CLUSTER_NUM, ipmatrix, cluster_assignment_cur, centmatrix_cur);
		prev_totD = totD;

		cluster_assignment_tmp = cluster_assignment_cur;
		cluster_assignment_cur = cluster_assignment_pre;
		cluster_assignment_pre = cluster_assignment_tmp;

		centmatrix_tmp = centmatrix_cur;
		centmatrix_cur = centmatrix_pre;
		centmatrix_pre = centmatrix_tmp;

		if(kmeans_itera_operator(dim, ipmatrix, numele, cluster_num, centmatrix_cur, centmatrix_pre, cluster_assignment_cur, cluster_assignment_pre, &totD) == 0) {
			break;
		}
		if( prev_totD < totD ) {
			printf("%lf %lf\n",prev_totD,totD);
			cluster_assignment_tmp = cluster_assignment_cur;
			cluster_assignment_cur = cluster_assignment_pre;
			cluster_assignment_pre = cluster_assignment_tmp;

			centmatrix_tmp = centmatrix_cur;
			centmatrix_cur = centmatrix_pre;
			centmatrix_pre = centmatrix_tmp;
			break;
		}
	}
	//	cluster_diag(dim, numele, CLUSTER_NUM, ipmatrix, cluster_assignment_cur, centmatrix_cur);
	copy_result(numele, cluster_assignment_cur, cluster_assignment_final, cluster_num * dim, centmatrix_cur, centmatrix_final);

	free(cluster_assignment_cur);
	free(cluster_assignment_pre);
	free(centmatrix_cur);
	free(centmatrix_pre);
}

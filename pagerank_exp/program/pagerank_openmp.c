#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <memory.h>
#include <sys/time.h>

#ifndef REAL
#define REAL float
#endif

#define ALPHA 0.85
#define EPSILON 0.01
#define ARRAY_LENGTH 600000000
#define MAX_TIMES 1000

#define MICRO_IN_SEC 1000000.00
double begin_time, end_time, serial_time=0, parallel_time=0;
double microtime(){
	int tv_sec,tv_usec;
	double time;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	
	return tv.tv_sec+tv.tv_usec/MICRO_IN_SEC;
}

typedef struct{
	int nodei;
	int nodej;
	REAL p;
} EDGE;


int caculate(char *ifn);
int check_result(REAL *r, REAL *rtmp, int noden);


int main(int argc,char * argv[])
{
	char *ifn=NULL;
	if(argc<2)
	{
		printf("wrong command format! usage:parallel_pagerank INPUTFILENAME\n");
		return 0;
	}
	else
	{
		ifn=argv[1];
		caculate(ifn);
		return 0;
	}
}

int caculate(char *ifn) {
	begin_time = microtime();
	FILE *ifp=NULL,*ofp=NULL;
	EDGE edge,*array_edge=NULL;
	char *ofn="CaculateResult.txt";
	int noden,edgen,foffset,linen,i,j,begin,topi,counter,index_i,edge_i;
	REAL *r=NULL,*rtmp=NULL,*tmp,*array=NULL;
	if((ifp=fopen(ifn,"r"))==NULL)
	{
		printf("%s file open error!\n",ifn);
		exit(0);
	}
	else
	{
		printf("%s file opened success!\n",ifn);
	}
	if((ofp=fopen(ofn,"w"))==NULL)
	{
		printf("%s file open error!\n",ofn);
		fclose(ifp);
		exit(0);
	}
	else
	{
		printf("%s file opened success!\n",ofn);
	}
	fscanf(ifp,"%d%d",&noden,&edgen);
	foffset=ftell(ifp);
	printf("Allocing Memory!\n");
	if((array_edge=(EDGE *)malloc(edgen*sizeof(EDGE)))==NULL) {
		printf("Memory alloc ERROR !\n");
		fclose(ifp);
		fclose(ofp);
		exit(0);
	}
	linen=ARRAY_LENGTH/noden;
	if(linen<=0) {
		printf("ArrayLength is too short for this caculate!\nPlase change the value of ARRAYLENGTH\n");
		free(array_edge);
		fclose(ifp);
		fclose(ofp);
		exit(0);
	}
	if((array=(REAL *)malloc(linen*noden*sizeof(REAL)))==NULL) {
		printf("Memory alloc ERROR !\n");
		fclose(ifp);
		fclose(ofp);
		free(array_edge);
		exit(0);
	}
	if((r=(REAL *)malloc(noden*sizeof(REAL)))==NULL) {
		printf("Memory alloc ERROR !\n");
		fclose(ifp);
		fclose(ofp);
		free(array);
		exit(0);
	}
	if((rtmp=(REAL *)malloc(noden*sizeof(REAL)))==NULL) {
		fclose(ifp);
		fclose(ofp);
		free(array);
		free(r);
		exit(0);
	}
	for(i=0;i<noden;i++) {
		*(r+i)=1.0;
	}
	printf("Memory Alloc done!\n");
	printf("Caculating pagerank!\n");
	printf("Loding Data!\n");
	for(i=0;i<edgen;i++) {
		fscanf(ifp,"%d%d%f",&((array_edge+i)->nodei),&((array_edge+i)->nodej),&((array_edge+i)->p));
	}
	printf("Data loaded!\n");
        end_time = microtime();
        printf("read file and alloc memory time consuming:%fs\n",end_time-begin_time);
        begin_time = end_time;

	printf("Begin Caculate!\n");

	counter=MAX_TIMES;
	REAL pr_tmp=0.0;
	REAL matrix_probablity = 0;


	begin_time = microtime();

	if(noden<=linen) {
/*
		end_time = microtime();
		printf("read file and alloc memory time consuming:%fs\n",end_time-begin_time);
		begin_time = end_time;
*/
#pragma omp parallel for
		for(i=0;i<noden*noden;i++) {
			array[i]=0;
		}
/*
		end_time = microtime();
		parallel_time += end_time-begin_time;
		begin_time = end_time;
*/
		for(i=0;i<edgen;i++) {
			*(array+(((array_edge+i)->nodei)*noden+(array_edge+i)->nodej))=(array_edge+i)->p;
		}
/*
		end_time = microtime();
		serial_time += end_time-begin_time;
		begin_time= end_time;
*/
		do {
			tmp=rtmp;
			rtmp=r;
			r=tmp;

			//caculate PageRank
//#pragma omp parallel for
			for(i=0;i<noden;i++) {
				pr_tmp = 0.0;
/*
				end_time = microtime();
				serial_time += end_time - begin_time;
				begin_time = end_time;
*/

#pragma omp parallel for reduction(+:pr_tmp)
				for(j=0;j<noden;j++) {
					matrix_probablity = array[i*noden+j];
					pr_tmp += ( ( ALPHA * matrix_probablity ) + ( 1.0 - ALPHA ) / ( REAL ) noden ) * rtmp[j];
				}
/*				
				end_time = microtime();
				parallel_time += end_time - begin_time;
				begin_time = end_time;
*/
				*(r+i) = pr_tmp;
			}
/*
			end_time = microtime();
			serial_time += end_time-begin_time;
			begin_time = end_time;
*/
			printf("parallel part time consuming:%fs\n",microtime()-begin_time);


			counter--;

			printf("counter = %d     ", counter);
			printf(" first pagerank = %f, noden= %d linen= %d \n",r[0], noden, linen);

		} while((!check_result(r,rtmp,noden)) && counter);
	} else {
		int block_counter=0;
		int ii=0;

		do {
			tmp=rtmp;
			rtmp=r;
			r=tmp;

			begin=0;
			edge_i=0;
			block_counter=0;
/*
			end_time = microtime();
			serial_time += end_time-begin_time;
			begin_time = end_time;
*/
			for(ii=0;ii<noden/linen;ii++) {
/*
				end_time = microtime();
				serial_time += end_time-begin_time;
				begin_time = end_time;
*/
#pragma omp parallel for
				for(i=0;i<linen*noden;i++) {
					array[i]=0;
				}
/*
				end_time = microtime();
				parallel_time += end_time - begin_time;
				begin_time = end_time;
*/
				do{
					if((array_edge+edge_i)->nodei>=begin+linen) {
						break;
					} else {
						*(array+(((array_edge+edge_i)->nodei%linen)*noden+(array_edge+edge_i)->nodej))=(array_edge+edge_i)->p;
						edge_i++;
					}
				} while(edge_i<edgen);


//#pragma omp parallel for
				for(index_i=begin;index_i<begin+linen;index_i++) {
					pr_tmp = 0.0;

#pragma omp parallel for reduction(+:pr_tmp)
					for(j=0;j<noden;j++) {
						matrix_probablity = array[(index_i%linen)*noden+j];
						pr_tmp += ( ( ALPHA * matrix_probablity ) + ( 1.0 - ALPHA ) / ( REAL ) noden ) * rtmp[j];
					}
					*(r+index_i) = pr_tmp;

//					r[index_i] = __sec_reduce_add ( (array+(index_i%linen) *noden)[0:noden] * rtmp[0:noden]);
				}

				begin+=linen;

				block_counter++;
				if(block_counter%1000 == 0) printf("block_counter:%d\n",block_counter);
				if(block_counter == 6000 || block_counter == (noden/linen - 1)){
/*
					end_time = microtime();
					serial_time += end_time - begin_time;
					begin_time = end_time;
*/
					printf("block_counter:%d\n parallel part time consuming:%fs\n",block_counter,microtime()-begin_time);
					exit(0);
				}
			}

			if(noden%linen != 0) {

/*
				end_time = microtime();
				serial_time += end_time - begin_time;
				begin_time = end_time;
*/

#pragma omp parallel for
				for(i=0;i<(noden%linen)*noden;i++) {
					array[i]=0;
				}
/*
				end_time = microtime();
				parallel_time += end_time - begin_time;
				begin_time = end_time;
*/

				do{
					if((array_edge+edge_i)->nodei>=begin+linen) {
						break;
					} else {
						*(array+(((array_edge+edge_i)->nodei%linen)*noden+(array_edge+edge_i)->nodej))=(array_edge+edge_i)->p;
						edge_i++;
					}
				} while(edge_i<edgen);

//#pragma omp parallel for
				for(index_i=begin;index_i<noden;index_i++) {
					pr_tmp = 0.0;
/*
					end_time = microtime();
					serial_time += end_time - begin_time;
					begin_time = end_time;
*/
#pragma omp parallel for reduction(+:pr_tmp)
					for(j=0;j<noden;j++) {
						matrix_probablity = array[(index_i%linen)*noden+j];
						pr_tmp += ( ( ALPHA * matrix_probablity ) + ( 1.0 - ALPHA ) / ( REAL ) noden ) * rtmp[j];
					}
/*
					end_time = microtime();
					parallel_time += end_time - begin_time;
					begin_time = end_time;
*/
					*(r+index_i) = pr_tmp;
				}

				block_counter++;
				if(block_counter%100 == 0) printf("block_counter=%d  begin=%d\n",block_counter,begin);
			}

			counter--;

			printf("counter = %d     ", counter);
			printf(" first pagerank = %f, noden= %d linen= %d \n",r[0], noden, linen);


		} while((!check_result(r,rtmp,noden)) && counter);
	}

	printf("caculate done !\n");
	printf("outputing result to %s\n",ofn);
	for(i=0;i<noden;i++) {
		fprintf(ofp,"%d\t%f\n",i,*(r+i));
	}
	printf("output done!,counter times:%d\n",counter);
	fclose(ifp);
	fclose(ofp);
	free(array);
	free(array_edge);
	free(rtmp);
	free(r);
	return 0;
}

int check_result(REAL *r, REAL *rtmp, int noden) {
	int i;

	for(i=0;i<noden;i++) {
		if(!(*(r+i)-*(rtmp+i)<EPSILON && *(rtmp+i)-*(r+i)<EPSILON)) {
			return 0;
		}
	}
	return 1;
}

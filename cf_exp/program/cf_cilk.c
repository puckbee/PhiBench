#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include <omp.h>

#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

#include <time.h>
#include <sys/time.h>
#define MICRO_IN_SEC 1000000.00

// the item number and user number should be start from 1, if it start from 0, our program in get_*_data should be modified
#define USER_COUNT 384546
//#define USER_COUNT 3500

#define ITEM_COUNT 1019318
#define TEST_COUNT 19315653

#define RECORD_COUNT 29057933

#define ITEM_BLOCK 3500
//#define ITEM_BLOCK 384546


#define ITEM_ROUND (ITEM_COUNT/ITEM_BLOCK)
#define ITEM_LAST  (ITEM_COUNT%ITEM_BLOCK)

#define USER_BLOCK 6000
#define USER_ROUND (USER_COUNT/USER_BLOCK)
#define USER_LAST  (USER_COUNT%USER_BLOCK)

#define K_SORT 100

#define BLOCK_MINI 100

typedef struct record_item_struct_define
{
	int item_id;
	int user_id;
	float rating;
}record_item_struct;


typedef struct k_similarity_struct_define
{
	int k_index;
	double k_similarity;
}k_similarity_struct;

double microtime(){
        int tv_sec,tv_usec;
        double time;
        struct timeval tv;
        struct timezone tz;
        gettimeofday(&tv,&tz);

        return tv.tv_sec+tv.tv_usec/MICRO_IN_SEC;
}


float compute_multiply(float* rating_m_block, float* rating_n_block)
{
	float sum_multiply;
	return sum_multiply = __sec_reduce_add(rating_m_block[0:BLOCK_MINI] * rating_n_block[0:BLOCK_MINI]);

}


// Here, we assume that the item in {user,item,predict_rating} of the test dataset will cover all the items.
void get_pearson_similarity(float* rating_m, float* rating_n, double* average_matrix, k_similarity_struct * k_similarity_matrix, int start_index_m, int start_index_n, int flag){
       

	printf(" in pearson_similarity start_index_m=%d, start_index_n=%d\n", start_index_m, start_index_n);
	
	int nPadded = ( USER_COUNT%8 == 0 ? USER_COUNT : USER_COUNT + (8-USER_COUNT%8) );
 
	double similarity;
	
  //      int i,j,m,n,s,k;

	int ua,ub,uc;
/* 
	double sum_numerator=0;
	double sum_denominator_square_m=0;
	double sum_denominator_square_n=0;
*/
	double sum_denominator=0;


	double* sum_numerator_matrix =(double*) _mm_malloc(sizeof(double)*USER_COUNT,64);
	double* sum_denominator_matrix_m =(double*) _mm_malloc(sizeof(double)*USER_COUNT,64);
	double* sum_denominator_matrix_n =(double*) _mm_malloc(sizeof(double)*USER_COUNT,64);

	memset(sum_numerator_matrix,0,sizeof(double)*USER_COUNT);	
	memset(sum_denominator_matrix_m,0,sizeof(double)*USER_COUNT);	
	memset(sum_denominator_matrix_n,0,sizeof(double)*USER_COUNT);	

	
//        float * simi_temp = (float*)_mm_malloc(sizeof(float)*ITEM_BLOCK*USER_COUNT,64);
	
	int numthreads;	
        int item_index;

//	int block_m,block_n;
	
	int end_m=(ITEM_COUNT<(start_index_m+ITEM_BLOCK) ? ITEM_COUNT:(start_index_m+ITEM_BLOCK));
	int end_n=(ITEM_COUNT<(start_index_n+ITEM_BLOCK) ? ITEM_COUNT:(start_index_n+ITEM_BLOCK));

	//printf("the number of threads is %d\n", omp_get_num_threads());
	int block_mini_int = BLOCK_MINI;
	printf(" end_m = %d , end_n = %d,...BLOCK_MINI = %d\n",end_m,end_n,block_mini_int);
/*
	cilk::reducer_opadd<unsigned long> kk(0);
	cilk_for (int k=0; k<1000; k++)
	{
		kk+=k;
	}
*/
	double a = microtime();
	
        //compute the pearson similarity
	//#pragma omp parallel for collapse(2) private(i,j,k,m,n) reduction(+:sum_numerator,sum_denominator_square_m,sum_denominator_square_n)
	//#pragma omp parallel for collapse(2) private(i,j,k,m,n) 
	//#pragma omp parallel  
	
//	#pragma cilk grainsize = ((end_m-start_index_m)*(end_n-start_index_n))/244
       // cilk_for (unsigned int mn = 0; mn < (((end_m-start_index_m)*(end_n-start_index_n))/16)*16; mn++) 

        cilk_for (unsigned int mn = 0; mn < (end_m-start_index_m)*(end_n-start_index_n); mn++) 
        {      
//		if (m%100==0) printf ("m = %d, percent= %f/%\n",m,(float)m/ITEM_COUNT*100);
 //               for(int n=start_index_n; n< end_n; n++)
 //               {

			int m = start_index_m + mn / ( end_n - start_index_n );
			int n = start_index_n + mn % ( end_n - start_index_n ); 
			
			//float* block_m = rating_m + (m - start_index_m)*USER_COUNT;
			//float* block_n = rating_n + (n - start_index_n)*USER_COUNT;	
		
			int block_m = m - start_index_m;
			int block_n = n - start_index_n;
			

			//similarity = cilk_spawn compute_kernel(rating_m,rating_n,block_m,block_n);
			


//			int block_m=1,block_n=1;

			float sum_numerator=0;
			float sum_denominator_square_m=0;
			float sum_denominator_square_n=0;			
				
//			cilk::reducer_opadd<float> sum_numerator(0);
//			cilk::reducer_opadd<float> sum_denominator_square_m(0);
//			cilk::reducer_opadd<float> sum_denominator_square_n(0);
//			cilk::reducer_opadd<double> sum_denominator(0);

			float * rating_m_block = rating_m+block_m*USER_COUNT;
                        float * rating_n_block = rating_n+block_n*USER_COUNT;


			//#pragma omp for schedule(static) nowait

	//		#pragma omp parallel for private(i) 
	//		#pragma simd reduction(+:sum_numerator,sum_denominator_square_m,sum_denominator_square_n)
	//		#pragma vector aligned 

		//	#pragma simd vecremainder vectorlength(16) private(sum_numerator,sum_denominator_square_m,sum_denominator_square_n)
//			#pragma simd vecremainder
//			#pragma vector aligned
//			#pragma ivdep
//			#pragma vector always
//			#pragma cilk grainsize=(USER_COUNT/1)
             //           for (int i=0;i < USER_COUNT/BLOCK_MINI; i++)
                        for (int i=0;i < USER_COUNT; i++)
                        {  
			       	//compute numerator
/*                           //   	simi_temp[block_m*USER_COUNT+i] = rating_m[block_m*USER_COUNT+i];


				float sum_numerator_tmp = cilk_spawn compute_multiply(rating_m_block + i*BLOCK_MINI, rating_n_block + i*BLOCK_MINI);
				sum_numerator += sum_numerator_tmp;
				float sum_denominator_square_m_tmp = cilk_spawn compute_multiply(rating_m_block + i*BLOCK_MINI, rating_m_block + i*BLOCK_MINI);
				sum_denominator_square_m += sum_denominator_square_m_tmp;
				float sum_denominator_square_n_tmp = cilk_spawn compute_multiply(rating_n_block + i*BLOCK_MINI, rating_n_block + i*BLOCK_MINI);
				sum_denominator_square_n += sum_denominator_square_n_tmp;
*/


                              	sum_numerator += (rating_m[block_m*USER_COUNT+i])*(rating_n[block_n*USER_COUNT+i]);
                                //compute the squre in denominator
       	  //                      sum_denominator_square_m += powf(rating_m[block_m*USER_COUNT+i],2.0);
       	                        sum_denominator_square_m += rating_m[block_m*USER_COUNT+i]*rating_m[block_m*USER_COUNT+i];
            // 	                sum_denominator_square_n += powf(rating_n[block_n*USER_COUNT+i],2.0);
        	       	        sum_denominator_square_n += rating_n[block_n*USER_COUNT+i]*rating_n[block_n*USER_COUNT+i];
	//			if( m==100 && n==100)	
	//			printf("m=%d,n=%d,i=%d, running on thread %d\n",m,n,i, omp_get_thread_num());

                        }			

/*			
			float * rating_m_block = rating_m+block_m*USER_COUNT;
			float * rating_n_block = rating_n+block_n*USER_COUNT;

//			#pragma vector aligned (rating_m_block,rating_n_block)
			float sum_numerator = __sec_reduce_add(rating_m_block[0:USER_COUNT] * rating_n_block[0:USER_COUNT]);
//			float sum_numerator = cilk_spawn compute_numerator(rating_m_block, rating_n_block);

//			#pragma vector aligned (rating_m_block)
			float sum_denominator_square_m = __sec_reduce_add(rating_m_block[0:USER_COUNT]*rating_m_block[0:USER_COUNT]);

  //                      #pragma vector aligned (rating_n_block)
			float sum_denominator_square_n = __sec_reduce_add(rating_n_block[0:USER_COUNT]*rating_n_block[0:USER_COUNT]);
*/			

                        //compute the denominator
                        sum_denominator = sqrt (sum_denominator_square_m*sum_denominator_square_n);
                        if(sum_denominator!=0)
				similarity = sum_numerator/sum_denominator;
			else
				similarity = 0;
/*

			for (j=0;j<K_SORT;j++)
			{	
				item_index = k_similarity_matrix[m*K_SORT+j].k_index;
				if(item_index==-1 || similarity > k_similarity_matrix[m*K_SORT+j].k_similarity)
				{
					for (s=K_SORT-1;s>j;s--)
					{
						k_similarity_matrix[m*K_SORT+s].k_index = k_similarity_matrix[m*K_SORT+s-1].k_index;
						k_similarity_matrix[m*K_SORT+s].k_similarity = k_similarity_matrix[m*K_SORT+s-1].k_similarity;
					}
					k_similarity_matrix[m*K_SORT+j].k_index = n;
					k_similarity_matrix[m*K_SORT+j].k_similarity = similarity;
					break;
				}
				else if( similarity == k_similarity_matrix[m*K_SORT+j].k_similarity && item_index == n)
				{
					break;
				}
			}

			if(flag==0) continue;

			for (k=0;k<K_SORT;k++)
			{	
				item_index = k_similarity_matrix[n*K_SORT+k].k_index;
				if(item_index==-1 || similarity > k_similarity_matrix[n*K_SORT+k].k_similarity)
				{
					for (s=K_SORT-1;s>j;s--)
					{
						k_similarity_matrix[n*K_SORT+s].k_index = k_similarity_matrix[n*K_SORT+s-1].k_index;
						k_similarity_matrix[n*K_SORT+s].k_similarity = k_similarity_matrix[n*K_SORT+s-1].k_similarity;
					}
					k_similarity_matrix[n*K_SORT+k].k_index = m;
					k_similarity_matrix[n*K_SORT+k].k_similarity = similarity;
					break;
				}
				else if( similarity == k_similarity_matrix[n*K_SORT+k].k_similarity && item_index == m)
				{
					break;
				}
			}
*/

   //             }
        }

	double b = microtime();
	double duration = b-a;

	printf(" time consumed: %fs\n", duration);
exit(0);
}

int get_predict_rating(double* predict_rating, float* rating, k_similarity_struct* index_matrix, int* test, double* average_index, int user_start_index,int test_start_index)
{
	// Firstly, we should find the union set between rating&index_matrix for the users in test[];
/*
	printf(" in predict_rating ...........user_start_index=%d, test_start_index=%d\n", user_start_index, test_start_index);
	
	int m,n,i,j;

	int user_id,item_id,k_id;
	double sum_rating = 0;
	int sum_rating_index = 0;
	int sum_no_rating_index=0;

	double numerator = 0;
	double denominator = 0;

	int block_user_id;
	
//	#pragma omp parallel for private(m,i) reduction(+:numerator,denominator)
	for (m = test_start_index; m < TEST_COUNT; m++)
	{
		user_id = test[m*2+0];
		item_id = test[m*2+1];
		numerator =0;
		denominator = 0;

		if( user_id < user_start_index) printf(" error__________________+++++++++++++++\n");

		if( user_id > ((user_start_index+USER_BLOCK) > USER_COUNT ? USER_COUNT:(user_start_index+USER_BLOCK)))      break;

		block_user_id = user_id - user_start_index;

		for (i = 0; i < K_SORT; i++)
		{
			k_id = index_matrix[item_id*K_SORT+i].k_index;
			if ( rating[block_user_id*ITEM_COUNT+k_id] !=0 )
			{
				numerator += index_matrix[item_id*K_SORT+i].k_similarity*(rating[block_user_id*ITEM_COUNT+k_id]-average_index[k_id]);
				denominator += index_matrix[item_id*K_SORT+i].k_similarity;
			}
		}
		
		if(denominator !=0) 
			predict_rating[m] = average_index[item_id] +  numerator/ denominator;
		else
			predict_rating[m] = average_index[item_id];

	}

//	return predict_rating;
	return m;
*/
}

void print_matrix_double(double * matrix,int row,int column)
{
/*
	int i,j;
	int sum_0=0;
	for (i=0;i<row;i++)
	{
		for(j=0;j<column;j++)
		if (matrix[i*column+j]==0) sum_0++;
		//	printf("%lf ",matrix[i*column+j]);
	//	printf("\n");
	}
	
	printf("sum_0 is %d in a whole %d\n",sum_0,row);
*/
}

void get_item_data(record_item_struct* item_data, char* filename)
{
	FILE *fp;
	
        if ((fp=fopen(filename,"r")) == NULL)
        {
                printf("cannot open this file");
                exit(0);
        }

        int user_id, item_id, timestamp;
	float rating=0;
        int i=0;

        while (fscanf(fp,"%d %d %f %d", &item_id, &user_id, &rating, &timestamp) != EOF)
        {
                item_data[i].item_id = item_id;
		item_data[i].user_id = user_id;
		item_data[i].rating  = rating;
		i++;
        }

        fclose(fp);
}

int get_item_block_data(int round, float* data, int file_start_index, record_item_struct* item_data)
{
//        int i=0;
	int p=0;
//	#pragma omp parallel for
	cilk_for (int i = 0; i<ITEM_BLOCK*USER_COUNT;i++)
	{
		data[i]=0;
	}

       // memset(data, 0, sizeof(float)*ITEM_BLOCK*USER_COUNT);

        int user_id, item_id;
	float rating=0;

	for(int i=file_start_index; ;i++)
	{
		item_id = item_data[i].item_id;
		user_id = item_data[i].user_id;
		rating = item_data[i].rating;

		if ((item_id-1) >= (round+1)*ITEM_BLOCK) break;
                data[(item_id-1-(round*ITEM_BLOCK))*USER_COUNT + user_id-1] = rating;
		p++;
	}
	return p;
}


long get_user_block_data(int round, float* data, long file_start_index)
{

	FILE *fp;
        int i=0;

	//int large_user_id=0;
	//int large_item_id=0;
/*
        float * data = (float*)malloc(sizeof(float)*USER_BLOCK*ITEM_COUNT);
	if (data==NULL)
	{
		printf(" malloc of base_data failed\n");
		exit(1);
	}
*/
        memset(data, 0, sizeof(float)*USER_BLOCK*ITEM_COUNT);

        if ((fp=fopen("r1.train.raw","r")) == NULL)
        {
                printf("cannot open this file");
                exit(0);
        }

        int user_id, item_id, timestamp;
	float rating=0;

	long file_offset = 0;

	fseek(fp, file_start_index, 0);

        while (fscanf(fp,"%d %d %f %d", &user_id, &item_id, &rating, &timestamp) != EOF)
        {
		if ((user_id-1) < round*USER_BLOCK) continue;
		
		if ((user_id-1) >= (round+1)*USER_BLOCK) break;
	
                data[(user_id-1-(round*USER_BLOCK))*ITEM_COUNT + item_id-1] = rating;

		file_offset =  ftell(fp);		

        //        data[(user_id-1)*ITEM_COUNT + item_id-1] = rating;
	//	if (user_id > large_user_id) large_user_id = user_id;
	//	if (item_id > large_item_id) large_item_id = item_id;
	//	printf("getting base_data on line i=%d,user_id=%d,item_id=%d,rating=%f\n",i++,user_id,item_id,rating);
        }
		
//	printf("the largest user_id is %d\n the largest item_id is %d\n",large_user_id,large_item_id);

        fclose(fp);

	return file_offset;
//        return data;
}

void get_test_data(int* data, float* rating)
{
	FILE *fp;
        int i=0;

        memset(data, 0, sizeof(int));

        if ((fp=fopen("r1.test","r")) == NULL)
        {
                printf("cannot open this file");
                exit(0);
        }

        int user_id, item_id, timestamp;
	float rating_temp;

        while (fscanf(fp,"%d %d %f %d", &user_id, &item_id, &rating_temp, &timestamp) != EOF)
        {

		data[i*2+0] = user_id-1;
		data[i*2+1] = item_id-1;
		rating[i] = rating_temp;
		i++;
		//printf("getting test_data on line i=%d,user_id=%d,item_id=%d,rating_temp=%f\n",i,user_id,item_id,rating_temp);
        }

        fclose(fp);
}

double get_rmse(float* test_rating, double* predict_data)
{
/*
	int m,n,i,j;

	double numerator = 0;
	double denominator = TEST_COUNT;
	
	#pragma omp parallel for private(i) reduction(+:numerator)
	for(i=0;i<TEST_COUNT;i++)
	{
		numerator += pow ((test_rating[i] - predict_data[i]),2.0);
	}
		
	return (numerator/denominator);
*/
	return 0;
}

void get_item_average_matrix(float* rating,double* average_matrix, int start_index)
{
//	int m,n,i,j;
//        int average_index =0;
//        int average_sum = 0;

//	int block_m = 0;

	double average_item=0;

//	#pragma omp parallel for private(m,n) reduction(+:average_sum,average_index)
        cilk_for (int m = start_index; m<(ITEM_COUNT < (start_index+ITEM_BLOCK) ? ITEM_COUNT:(start_index+ITEM_BLOCK)); m++ )
        {
		cilk::reducer_opadd<int> average_sum(0);
		cilk::reducer_opadd<int> average_index(0);
		int block_m = m-start_index;
		#pragma simd
		#pragma vector aligned
		#pragma vector always
		#pragma ivdep
		for(int n=0;n<USER_COUNT;n++)
		{
			if(rating[block_m*USER_COUNT+n] !=0)
			{
				average_sum += rating[block_m*USER_COUNT+n];
				average_index += 1;
			}
		}
		if(average_index.get_value()!=0)
		{
			average_matrix[m]=(double)average_sum.get_value()/(double)average_index.get_value();
		}
		else
		{
			average_matrix[m]=0;
		}
        }

//	#pragma omp parallel for private(m,n) 
//	#pragma vector aligned
//	#pragma ivdep
        cilk_for (int m = start_index; m<(ITEM_COUNT < (start_index+ITEM_BLOCK) ? ITEM_COUNT:(start_index+ITEM_BLOCK)); m++ )
        {
		int block_m = m-start_index;
		average_item = average_matrix[m];
		#pragma simd
		for(int n=0;n<USER_COUNT;n++)
		{
			rating[block_m*USER_COUNT+n] -= (float)average_item;
		}
        }

}


int main(int argc, char ** argv){


	//first, read the data in files into an array in order to process it more efficiently.
	
	record_item_struct * item_data = (record_item_struct *) _mm_malloc(sizeof(record_item_struct)*RECORD_COUNT,64);

        memset(item_data, 0, sizeof(record_item_struct)*RECORD_COUNT);

	get_item_data(item_data, argv[1]);

        float * item_block_data_i = (float*)_mm_malloc(sizeof(float)*ITEM_BLOCK*USER_COUNT,64);
        float * item_block_data_j = (float*)_mm_malloc(sizeof(float)*ITEM_BLOCK*USER_COUNT,64);
	if (item_block_data_i==NULL || item_block_data_j == NULL)
	{
		printf(" malloc of base_data failed\n");
		exit(1);
	}
	double * item_average_matrix = (double*)_mm_malloc(sizeof(double)*ITEM_COUNT,64);
	memset(item_average_matrix,0,sizeof(double)*ITEM_COUNT);
	
	k_similarity_struct * item_index_matrix = (k_similarity_struct *) _mm_malloc (sizeof(k_similarity_struct)*K_SORT*ITEM_COUNT,64);
	memset (item_index_matrix,-1,sizeof(k_similarity_struct));

	int item_start_index_i=0;
	int item_start_index_j=0;
	

//	int i,j;
	for (int i=0; i <= ITEM_ROUND; i++)
	{
		printf("round %d ================== with ITEM_BLOCK %d\n",i,ITEM_BLOCK);

		//block_data
		printf("get item_block_data begins\n");	
		item_start_index_i = get_item_block_data(i, item_block_data_i,item_start_index_i, item_data);
		printf("get_item_block_data ends\n");

		//average matrix
		printf("get_item_average_matrix begins\n");
		get_item_average_matrix(item_block_data_i, item_average_matrix,i*ITEM_BLOCK);
		printf("get_item_average_matrix ends\n");

		item_start_index_j = 0;	
	
		for(int j=0; j<= i;j++)
		{
			if( i==j) 
			{
				//the index of item after sorting the similarity matrix
				printf("get k_similarity_begins\n");
				get_pearson_similarity(item_block_data_i,item_block_data_i,item_average_matrix,item_index_matrix,i*ITEM_BLOCK,i*ITEM_BLOCK,0);
				printf("get k_similarity_ends\n");
				continue;
			}
			//block_data
			printf("get item_block_data begins\n");	
			item_start_index_j = get_item_block_data(j, item_block_data_j, item_start_index_j, item_data);
			printf("get_item_block_data ends\n");

			//the index of item after sorting the similarity matrix
			printf("get k_similarity_begins\n");
			get_pearson_similarity(item_block_data_i,item_block_data_j,item_average_matrix,item_index_matrix,i*ITEM_BLOCK,j*ITEM_BLOCK,1);
			printf("get k_similarity_ends\n");
			
		}
	}

	_mm_free(item_block_data_i);
	_mm_free(item_block_data_j);

	int *test_data;
	float *test_rating;
	test_data = (int*)_mm_malloc (sizeof(int)*2*TEST_COUNT,64);
	test_rating= (float*)_mm_malloc(sizeof(float)*TEST_COUNT,64); 

	printf("get_test_data begins\n");
	get_test_data(test_data,test_rating);
	printf("get_test_data ends\n");

	long user_file_start_index = 0;
        float * user_block_data = (float*)_mm_malloc(sizeof(float)*USER_BLOCK*ITEM_COUNT,64);
	if (user_block_data==NULL)
	{
		printf(" malloc of base_data failed\n");
		exit(1);
	}
	int test_start_index = 0;
	double * item_predict_rating = (double*)_mm_malloc (sizeof(double)*TEST_COUNT,64);
	for(int i=0;i<=USER_ROUND;i++)
	{
		user_file_start_index = get_user_block_data(i,user_block_data, user_file_start_index);

		printf("get_predict_rating begins\n");
		test_start_index=get_predict_rating(item_predict_rating, user_block_data, item_index_matrix, test_data,item_average_matrix,i*USER_BLOCK, test_start_index);
		printf("get_predict_rating ends\n");

		if ( test_start_index == TEST_COUNT)
			break;
	}
	
	
	_mm_free (user_block_data);

	double rmse;
	printf("get_rmse begins\n");
	rmse = get_rmse(test_rating,item_predict_rating);
	printf("ge_rmse ends\n");
	printf("rmse= %f\n", rmse);

	return 0;

}

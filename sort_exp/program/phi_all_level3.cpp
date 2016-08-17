/*
 * bitonic sort implementation
 * rather than sort [A B] B in other direction
 * sort A B in the same direction but reverse B in next comparsion first
 * a another say to express:
 * the input array A,B are sorted in same direction assending or decending order
 * so when merge A,B using bitonic way, B should be reversed at first
 * But we can consider that A[0] compare with B[n-1] A[1] compare with B[n-2], the same as reversed
 *
 * */

#include <iostream>
#include <fstream>
#include <algorithm>
//#include <string>
//#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <unistd.h>
#include <sys/time.h>
#include <immintrin.h>
#include <memory.h>
#include <assert.h>
//#include <rdtsc.h>

#include <omp.h>
//#include <mpi.h>

#define MICRO_IN_SEC 1000000.00

#define PI acos(-1)
#define eps 1e-9

#define out(x) (cout<<#x<<":"<<x<<" ")
#define outln(x) (cout<<#x<<":"<<x<<endl)
#define outs(x) (cout<<x)
#define outline (cout<<endl)
#define mssleep(time) usleep((time)*(10*1000))

#define FOR_I(begin,end) for (int i=begin;i<end;i++)
#define FOR_J(begin,end) for (int j=begin;j<end;j++)
#define FOR_K(begin,end) for (int k=begin;k<end;k++)
#define FOR_I_J(B1,E1,B2,E2) FOR_I(B1,E1) FOR_J(B2,E2)
#define FOR_I_J_K(B1,E1,B2,E2,B3,E3) FOR_I_J(B1,E1,B2,E2) FOR_K(B3,E3)
#define FOR(begin,end) FOR_I(begin,end)
#define FORN(end) FOR_I(0,end)

#define HERE cout<<"HERE"<<endl
#define THERE cout<<"THERE"<<endl

#define NORM_C (printf("\x1B[0m"))
#define BOLD(x) {printf("\x1B[1m");x;NORM_C;}
#define RED(x) {printf("\x1B[31m");x;NORM_C;}
#define BRED(x) {printf("\x1B[41m");x;NORM_C;}
#define GREEN(x) {printf("\x1B[32m");x;NORM_C;}
#define BGREEN(x) {printf("\x1B[42m");x;NORM_C;}
#define BLUE(x) {printf("\x1B[34m");x;NORM_C;}
#define BBLUE(x) {printf("\x1B[44m");x;NORM_C;}
#define CYAN(x) {printf("\x1B[36m");x;NORM_C;}
#define BCYAN(x) {printf("\x1B[46m");x;NORM_C;}
#define SHINE(x) {printf("\x1B[93m");x;NORM_C;}

using namespace std;

//for mpi
#define MCW MPI_COMM_WORLD

void get_data(FILE* fp, int* data, int nn)
{
        int i=0;
	int data_item;

        while (fscanf(fp,"%d", &data_item) != EOF)
        {
		if (i>=nn) break;
		data[i]=data_item;
                i++;
        }

	printf(" i= %u \n",i);

        fclose(fp);
}


double microtime(){
        int tv_sec,tv_usec;
        double time;
        struct timeval tv;
        struct timezone tz;
        gettimeofday(&tv,&tz);

        return tv.tv_sec+tv.tv_usec/MICRO_IN_SEC;
}







	template <typename T>
	void debug_a(T * data,int begin,int end){
		for (int i=begin;i<end;i++) cout<<"["<<i<<"]: "<<data[i]<<"\t";cout<<endl;
	}
	template <typename T>
	void debug_a(T * data,int end){
		debug_a(data,0,end);
	}
	template <typename T>
	void debug_a2(T ** data,int end1,int end2){
		for (int i=0;i<end1;i++){cout<<"row "<<i<<endl; for (int j=0;j<end2;j++) cout<<"["<<i<<","<<j<<"] "<<data[i][j]<<"\t";cout<<endl;} 
	}
	template <typename T>
	void debug_a2(T * data,int n,int m){
		cout<<"mn mode"<<endl;
		for (int i=0;i<n;i++){cout<<"row "<<i<<endl; for (int j=0;j<m;j++) cout<<"["<<i<<","<<j<<"] "<<data[i*m+j]<<"\t";cout<<endl;}
	}
	double get_sec(const struct timeval & tval){
		return ((double)(tval.tv_sec*1000*1000 + tval.tv_usec))/1000000.0;
	}

	template <typename T>
	T checkmin(T & data,T value){
		data = min(data,value);
		return data;
	}
struct Watch{
	timeval begin,end;
	void start(){gettimeofday(&begin,NULL);}
	double time(){return get_sec(end)-get_sec(begin);}
	double stop(){gettimeofday(&end,NULL);return time();}
};
	template<typename T>
	void show_trends(T *data,int n){
		cout<<"["<<0<<"]: "<<data[0]<<"\t";
		FOR_I(1,n){
			T compare = data[i] - data[i-1];
			if (0 == compare)
				cout<<"["<<i<<"]: "<<data[i]<<"\t";
			else if ( 0 < compare){
				GREEN(cout<<"["<<i<<"]: "<<data[i]<<"\t");
			}
			else if ( compare < 0)
				BLUE(cout<<"["<<i<<"]: "<<data[i]<<"\t");
		}
		cout<<endl<<endl;
	}

	inline __m512i load(const int* addr){
		return _mm512_load_epi32(addr);
	}
	inline void store(int* addr, const __m512i value){
		_mm512_store_epi32(addr,value);
	}
	inline void show(const __m512i a){
		int *buffer=0;
		posix_memalign((void **)&buffer,64,16*sizeof(int));
		store(buffer,a);
		FOR_I(0,16) printf("%d\t",buffer[i]);printf("\n");
	}

int key_interval_size;
int interval_size;
int num_keys;

int *data;
int *data2;
int *correct;

int test_arr[16*64] __attribute__ ((aligned(0x100)));

int buffers[8][8][20];
int permute_arr[20];

const _MM_CMPINT_ENUM global_op = _MM_CMPINT_GE;

int nn;
	
__m512i vconst_sort1,vconst_sort2,vconst_sort3,vconst_sort4;
__m512i vconst_merge1,vconst_merge2,vconst_merge3;

const int SEG_NUM = 512;

	void generate_data(int *arr,int n,int MOD){
		FOR_I(0,n){
			arr[i] = rand() % MOD;
		}
	}
	void check_correctness(int *data1,int *data2,int n){
		FOR_I(0,n)
		if ( data1[i] != data2[i]){
			BCYAN(cout<<"Wrong at "<<i<<" "<<data1[i]<<" <> "<<data2[i]<<endl);
			exit(0);
		}
	}

	inline int bsearch(int *data1,int *data2,int n){
		int left(0),right(n);
		while (left < right){
			int mid = (left+right)/2;
			if (data1[mid] < data2[mid])
				left = mid + 1;
			else right = mid;
		}
		return left;
	}

	template<bool less_than=false>
	inline void vector_swap(__m512i & a, __m512i & b){
		__m512i t;
		t = a;
		if (!less_than){
			a = _mm512_min_epi32(a,b);
			b = _mm512_max_epi32(t,b);
		}
		else {
			a = _mm512_max_epi32(a,b);
			b = _mm512_min_epi32(t,b);
		}
	}

	template<bool reversed=false>
	inline void register_sort(__m512i & a){
		const int mask1 = reversed? 0xFFFF:0;
		const int mask2 = reversed? 0:0xFFFF;
		__m512i b;
		b = _mm512_shuffle_epi32(a,_MM_PERM_CDAB);
		a = _mm512_mask_min_epi32(a,0b0110100110010110^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0110100110010110^mask2,a,b);

		b = _mm512_shuffle_epi32(a,_MM_PERM_BADC);
		a = _mm512_mask_min_epi32(a,0b0011110011000011^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0011110011000011^mask2,a,b);

		b = _mm512_shuffle_epi32(a,_MM_PERM_CDAB);
		a = _mm512_mask_min_epi32(a,0b0101101010100101^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0101101010100101^mask2,a,b);

		b = _mm512_permute4f128_epi32(a,_MM_PERM_CDAB);
		a = _mm512_mask_min_epi32(a,0b0000111111110000^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0000111111110000^mask2,a,b);

		b = _mm512_shuffle_epi32(a,_MM_PERM_BADC);
		a = _mm512_mask_min_epi32(a,0b0011001111001100^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0011001111001100^mask2,a,b);

		b = _mm512_shuffle_epi32(a,_MM_PERM_CDAB);
		a = _mm512_mask_min_epi32(a,0b0101010110101010^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0101010110101010^mask2,a,b);

		b = _mm512_permute4f128_epi32(a,_MM_PERM_BADC);
		a = _mm512_mask_min_epi32(a,0b0000000011111111^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0000000011111111^mask2,a,b);

		b = _mm512_permute4f128_epi32(a,_MM_PERM_CDAB);
		a = _mm512_mask_min_epi32(a,0b0000111100001111^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0000111100001111^mask2,a,b);

		b = _mm512_shuffle_epi32(a,_MM_PERM_BADC);
		a = _mm512_mask_min_epi32(a,0b0011001100110011^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0011001100110011^mask2,a,b);

		b = _mm512_shuffle_epi32(a,_MM_PERM_CDAB);
		a = _mm512_mask_min_epi32(a,0b0101010101010101^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0101010101010101^mask2,a,b);
	}

	template<bool reversed = false>
	inline void register_merge16(__m512i &a){
		const int mask1 = reversed? 0xFFFF:0;
		const int mask2 = reversed? 0:0xFFFF;
		__m512i b;
		b = _mm512_permute4f128_epi32(a,_MM_PERM_BADC);
		a = _mm512_mask_min_epi32(a,0b0000000011111111^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0000000011111111^mask2,a,b);

		b = _mm512_permute4f128_epi32(a,_MM_PERM_CDAB);
		a = _mm512_mask_min_epi32(a,0b0000111100001111^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0000111100001111^mask2,a,b);

		b = _mm512_shuffle_epi32(a,_MM_PERM_BADC);
		a = _mm512_mask_min_epi32(a,0b0011001100110011^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0011001100110011^mask2,a,b);

		b = _mm512_shuffle_epi32(a,_MM_PERM_CDAB);
		a = _mm512_mask_min_epi32(a,0b0101010101010101^mask1,a,b);
		a = _mm512_mask_max_epi32(a,0b0101010101010101^mask2,a,b);
	}
	void register_sort(__m512i & a,__m512i & b,bool direction){
		/*# of instruction: 77*/
		__m512i a2;
		a2 = _mm512_mask_min_epi32(a,0b0000000000000000^0xFFFF,a,b);
		b = _mm512_mask_max_epi32(b,0b0000000000000000^0xFFFF,a,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_CDAB);
		
		a = _mm512_mask_min_epi32(a2,0b0110011001100110^0xFFFF,a2,b);
		a = _mm512_mask_max_epi32(a,0b0110011001100110,a2,b);
		b = _mm512_mask_max_epi32(b,0b0110011001100110^0xFFFF,a2,b);
		b = _mm512_mask_min_epi32(b,0b0110011001100110,a2,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_CDAB);
		
		a2 = _mm512_mask_min_epi32(a,0b1010101010101010^0xFFFF,a,b);
		a2 = _mm512_mask_max_epi32(a2,0b1010101010101010,a,b);
		b = _mm512_mask_max_epi32(b,0b1010101010101010^0xFFFF,a,b);
		b = _mm512_mask_min_epi32(b,0b1010101010101010,a,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_BADC);
		
		a = _mm512_mask_min_epi32(a2,0b0011110000111100^0xFFFF,a2,b);
		a = _mm512_mask_max_epi32(a,0b0011110000111100,a2,b);
		b = _mm512_mask_max_epi32(b,0b0011110000111100^0xFFFF,a2,b);
		b = _mm512_mask_min_epi32(b,0b0011110000111100,a2,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_ABCD);
		
		a2 = _mm512_mask_min_epi32(a,0b0101101001011010^0xFFFF,a,b);
		a2 = _mm512_mask_max_epi32(a2,0b0101101001011010,a,b);
		b = _mm512_mask_max_epi32(b,0b0101101001011010^0xFFFF,a,b);
		b = _mm512_mask_min_epi32(b,0b0101101001011010,a,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_CDAB);
		
		a = _mm512_mask_min_epi32(a2,0b0110011001100110^0xFFFF,a2,b);
		a = _mm512_mask_max_epi32(a,0b0110011001100110,a2,b);
		b = _mm512_mask_max_epi32(b,0b0110011001100110^0xFFFF,a2,b);
		b = _mm512_mask_min_epi32(b,0b0110011001100110,a2,b);
		b = _mm512_permute4f128_epi32(b,_MM_PERM_CDAB);
		
		a2 = _mm512_mask_min_epi32(a,0b0000111111110000^0xFFFF,a,b);
		a2 = _mm512_mask_max_epi32(a2,0b0000111111110000,a,b);
		b = _mm512_mask_max_epi32(b,0b0000111111110000^0xFFFF,a,b);
		b = _mm512_mask_min_epi32(b,0b0000111111110000,a,b);
		b = _mm512_permutevar_epi32(vconst_sort1,b);
		
		a = _mm512_mask_min_epi32(a2,0b0011001111001100^0xFFFF,a2,b);
		a = _mm512_mask_max_epi32(a,0b0011001111001100,a2,b);
		b = _mm512_mask_max_epi32(b,0b0011001111001100^0xFFFF,a2,b);
		b = _mm512_mask_min_epi32(b,0b0011001111001100,a2,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_ABCD);
		
		a2 = _mm512_mask_min_epi32(a,0b0101010110101010^0xFFFF,a,b);
		a2 = _mm512_mask_max_epi32(a2,0b0101010110101010,a,b);
		b = _mm512_mask_max_epi32(b,0b0101010110101010^0xFFFF,a,b);
		b = _mm512_mask_min_epi32(b,0b0101010110101010,a,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_CDAB);
		
		a = _mm512_mask_min_epi32(a2,0b1001011010010110^0xFFFF,a2,b);
		a = _mm512_mask_max_epi32(a,0b1001011010010110,a2,b);
		b = _mm512_mask_max_epi32(b,0b1001011010010110^0xFFFF,a2,b);
		b = _mm512_mask_min_epi32(b,0b1001011010010110,a2,b);
		b = _mm512_permute4f128_epi32(b,_MM_PERM_BADC);
		
		a2 = _mm512_mask_min_epi32(a,0b1111111100000000^0xFFFF,a,b);
		a2 = _mm512_mask_max_epi32(a2,0b1111111100000000,a,b);
		b = _mm512_mask_max_epi32(b,0b1111111100000000^0xFFFF,a,b);
		b = _mm512_mask_min_epi32(b,0b1111111100000000,a,b);
		b = _mm512_permute4f128_epi32(b,_MM_PERM_ABCD);
		
		a = _mm512_mask_min_epi32(a2,0b1111000011110000^0xFFFF,a2,b);
		a = _mm512_mask_max_epi32(a,0b1111000011110000,a2,b);
		b = _mm512_mask_max_epi32(b,0b1111000011110000^0xFFFF,a2,b);
		b = _mm512_mask_min_epi32(b,0b1111000011110000,a2,b);
		b = _mm512_permutevar_epi32(vconst_sort2,b);
		
		a2 = _mm512_mask_min_epi32(a,0b1100110011001100^0xFFFF,a,b);
		a2 = _mm512_mask_max_epi32(a2,0b1100110011001100,a,b);
		b = _mm512_mask_max_epi32(b,0b1100110011001100^0xFFFF,a,b);
		b = _mm512_mask_min_epi32(b,0b1100110011001100,a,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_ABCD);
		
		a = _mm512_mask_min_epi32(a2,0b1010101010101010^0xFFFF,a2,b);
		a = _mm512_mask_max_epi32(a,0b1010101010101010,a2,b);
		b = _mm512_mask_max_epi32(b,0b1010101010101010^0xFFFF,a2,b);
		b = _mm512_mask_min_epi32(b,0b1010101010101010,a2,b);
		b = _mm512_shuffle_epi32(b,_MM_PERM_CDAB);
		
		a2 = _mm512_mask_min_epi32(a,0b0110100110010110^0xFFFF,a,b);
		a2 = _mm512_mask_max_epi32(a2,0b0110100110010110,a,b);
		b = _mm512_mask_max_epi32(b,0b0110100110010110^0xFFFF,a,b);
		b = _mm512_mask_min_epi32(b,0b0110100110010110,a,b);
		b = _mm512_permute4f128_epi32(b,_MM_PERM_BADC);
		
		
		if (!direction){
		a = _mm512_min_epi32(a2,b);
		b = _mm512_max_epi32(b,a2);
		a = _mm512_permutevar_epi32(vconst_sort3,a);
		b = _mm512_permutevar_epi32(vconst_sort3,b);
		}
		else {
		a = _mm512_max_epi32(a2,b);
		b = _mm512_min_epi32(b,a2);
		a = _mm512_permutevar_epi32(vconst_sort4,a);
		b = _mm512_permutevar_epi32(vconst_sort4,b);
		}
	}

	template<bool dir1,bool dir2>
	inline void register_merge(__m512i &a, __m512i &b){
		/* # of instructions: 2 + 12*2 = 26*/
		vector_swap<false>(a,b);
		register_merge16<dir1>(a);
		register_merge16<dir2>(b);
	}
/*
	template<bool dir1,bool dir2>
	inline void register_merge2(__m512i &a, __m512i &b){
		__m512i a2,b2;
a2 = _mm512_mask_min_epi32(a,0b1111111100000000^0xFFFF,a,b);
a2 = _mm512_mask_max_epi32(a2,0b1111111100000000,a,b);
b2 = _mm512_mask_max_epi32(b,0b1111111100000000^0xFFFF,a,b);
b2 = _mm512_mask_min_epi32(b2,0b1111111100000000,a,b);
b2 = _mm512_permute4f128_epi32(b2,_MM_PERM_BADC);

a = _mm512_mask_min_epi32(a2,0b0000111111110000^0xFFFF,a2,b2);
a = _mm512_mask_max_epi32(a,0b0000111111110000,a2,b2);
b = _mm512_mask_max_epi32(b2,0b0000111111110000^0xFFFF,a2,b2);
b = _mm512_mask_min_epi32(b,0b0000111111110000,a2,b2);
b = _mm512_permute4f128_epi32(b,_MM_PERM_CDAB);

a2 = _mm512_mask_min_epi32(a,0b0011110000111100^0xFFFF,a,b);
a2 = _mm512_mask_max_epi32(a2,0b0011110000111100,a,b);
b2 = _mm512_mask_max_epi32(b,0b0011110000111100^0xFFFF,a,b);
b2 = _mm512_mask_min_epi32(b2,0b0011110000111100,a,b);
b2 = _mm512_shuffle_epi32(b2,_MM_PERM_BADC);

a = _mm512_mask_min_epi32(a2,0b0110011001100110^0xFFFF,a2,b2);
a = _mm512_mask_max_epi32(a,0b0110011001100110,a2,b2);
b = _mm512_mask_max_epi32(b2,0b0110011001100110^0xFFFF,a2,b2);
b = _mm512_mask_min_epi32(b,0b0110011001100110,a2,b2);
b = _mm512_shuffle_epi32(b,_MM_PERM_CDAB);

a2 = _mm512_mask_min_epi32(a,0b1010101010101010^0xFFFF,a,b);
a2 = _mm512_mask_max_epi32(a2,0b1010101010101010,a,b);
b2 = _mm512_mask_max_epi32(b,0b1010101010101010^0xFFFF,a,b);
b2 = _mm512_mask_min_epi32(b2,0b1010101010101010,a,b);
		b = _mm512_permutevar_epi32(vconst_merge1,b2);
		
		a = _mm512_min_epi32(a2,b);
		b = _mm512_max_epi32(b,a2);

		if (!dir1)
		a = _mm512_permutevar_epi32(vconst_merge2,a);
		else a = _mm512_permutevar_epi32(vconst_merge3,a);
		if (!dir2)
		b = _mm512_permutevar_epi32(vconst_merge2,b);
		else b = _mm512_permutevar_epi32(vconst_merge3,b);
	}
*/


	template<bool direction>
	void merge(int *data,int *data_out,int n){
		const int kx = direction? -16:16;
		int i(0),j(n-16);
		int k;
		int endk;
		if (direction)
			k = n-16,endk = 0;
		else k=0, endk = n;
		
		__m512i small,large;
		int a16,b16;
		int compare =0;
		small = load(data+i);a16 = data[i+15];
		large = load(data+j);b16 = data[j];
		while (true){
			compare = a16 - b16 ;
			if (compare < 0){
				register_merge<direction,true>(small,large);
				i+=16;
			}
			else {
				register_merge<direction,false>(small,large);
				j-=16;
			}
			store(data_out+k,small);k+=kx;
			if (i>=j) break;
			if (compare <0){
				small = load(data+i);
				a16 = data[i+15];
			}
			else {
				small = load(data+j);
				b16 = data[j];
			}
		}//end while
		register_merge16<direction>(large);
		store(data_out+k,large);
	}

	template<bool dir>
	void basic_sort(int *data){
		__m512i a,b;
		a = load(data);
		register_sort< dir >(a);
		b = load(data+16);
		register_sort< !dir >(b);
		vector_swap<dir>(a,b);
		register_merge16<dir>(a);
		store(data,a);
		register_merge16<dir>(b);
		store(data+16,b);
	}
	void basic_sort(int *data,bool dir){
		__m512i a,b,a2,b2;
		a = load(data);
		b = load(data+16);
		register_sort(a,b,dir);
		store(data,a);
		store(data+16,b);
	}
	int * one_core_merge_sort(int *data,int *data_out,int n,bool direction){
		int len = n>>1;
		if ( n<=32 ){
			if (direction) basic_sort<true>(data);
			else basic_sort<false>(data);
			//basic_sort(data,direction);
			return data;
		}
		int *addr1,*addr2;


		addr1 = one_core_merge_sort(data,data_out,len,false);
		one_core_merge_sort(data+len,data_out+len,len,true);


		if (addr1 == data)
			addr2 = data_out;
		else addr2 = data;

		if (direction)
			merge<true>(addr1,addr2,n);
		else merge<false>(addr1,addr2,n);
		return addr2;
	}


	void first_level(int *data,int *data2,int n){
		int seg_len = n / SEG_NUM;
		#pragma omp parallel for
		FOR_I(0,SEG_NUM){
			int a = seg_len*i;
			one_core_merge_sort(data+a,data2+a,seg_len,i%2);
		}
	}

	inline __m512i load_unaligned(const int* addr){
#pragma warning( disable : 592 )
		__m512i x;
		return _mm512_loadunpackhi_epi32(_mm512_loadunpacklo_epi32(x, addr),addr+16);
	}
int cut[2500],cut2[2500];

	template<bool direction>
	void merge_nonealign(int *data,int n1, int * data2,int n2,int *data_out,int n){
		const int kx = direction? -16:16;
		int i(0),j(n2-16);
		int k;
		int endk;
		if (direction)
			k = n-16,endk = 0;
		else k=0, endk = n;
		
		__m512i small,large,hill;
		__m512i i_second_half,j_first_half;
		int part1 = n1 % 16;
		hill = load_unaligned(data+n1-part1);
		small = load_unaligned(data2-part1);
		int mask_hill = ((1<<part1)-1)^0xFFFF ;
		hill = _mm512_mask_mov_epi32(hill,mask_hill,small);
		int a16,b16;
		int compare =0;
		small = load_unaligned(data+i);a16 = data[i+15];
		large = load_unaligned(data2+j);b16 = data2[j];
		if (n1 >=16 && n2 >=16)
		while (true){
			compare = a16 - b16 ;
			if (compare < 0){
				register_merge<direction,true>(small,large);
				i+=16;
			}
			else {
				register_merge<direction,false>(small,large);
				j-=16;
			}
			store(data_out+k,small);k+=kx;
			if ( i >= n1-part1 || j < 0 ) break;

			if (compare <0){
				small = load_unaligned(data+i);
				a16 = data[i+15];
			}
			else {
				small = load_unaligned(data2+j);
				b16 = data2[j];
			}
		}//end while
		else {
			if ( n1 >= 16 ) large = small;
		}

		if (0 == part1){
			if (i >= n1){
				j -= 16;
				while (j>=0){
					small = load_unaligned(data2+j);
					register_merge<direction,false>(small,large);
					j -= 16;
					store(data_out+k,small);
					k+=kx;
				}
			}
			else {
				i+=16;
				while (i<n1){
					small = load_unaligned(data+i);
					register_merge<direction,true>(small,large);
					i += 16;
					store(data_out+k,small);
					k+=kx;
				}
			}
		}
		else {
			if (i >= n1 - part1){
				register_merge<direction,false>(hill,large);
				store(data_out+k,hill);
				k += kx;
				j -= 16;
				while (j>=0){
					small = load_unaligned(data2+j);
					register_merge<direction,false>(small,large);
					j -= 16;
					store(data_out+k,small);
					k += kx;
				}
			}
			else {
				register_merge<direction,true>(hill,large);
				store(data_out+k,hill);
				k += kx;
				i += 16;
				while (i < n1-part1 ){
					small = load_unaligned(data+i);
					register_merge<direction,true>(small,large);
					i += 16;
					store(data_out+k,small);
					k += kx;
				}
			}
		}
		register_merge16<direction>(large);
		store(data_out+k,large);
	}

		int * parallel_merge_sort(int *data,int *data2,int n,bool direction){
		
		int n2;
		int seg_len = n / SEG_NUM;
		first_level(data,data2,n);
		//swap(data,data2);
		//show_trends(data,n);
		// if SEG_NUM is odd no need to swap

		for (int level = 1,n2 = seg_len*2;n2 <= n;n2 <<= 1,level += 1){
			int len = n2 >> 1;
			int nump = 1 << level;
		#pragma omp parallel for
			for (int j=0;j<SEG_NUM;j++){
				int *addr1,*addr2;
				int id = j % nump;
				int part_id = j / nump;
				/*- parallel merge -*/
				addr1 = data + part_id*n2;
				addr2 = addr1 + len;
				int pos,pos2;
				int bsearch_len = (id+1)*seg_len;
				if ( id+1 != nump ){
					if (id < nump/2){
						addr2 += len - bsearch_len;
						pos = bsearch(addr1 , addr2 , bsearch_len);
						pos2 = bsearch_len - pos;
						pos2 = len - pos2;
					}
					else {
						pos = bsearch(addr1 , addr2 , bsearch_len);
						bsearch_len = n2-(id+1)*seg_len;
						addr1 += len - bsearch_len;
						pos = bsearch(addr1 , addr2 , bsearch_len);
						pos2 = bsearch_len - pos;
						pos += len - bsearch_len;
						pos2 = bsearch_len - pos2;
					}
				cut[j] = pos;
				cut2[j] = pos2;
				}

			}

		#pragma omp parallel for
			FOR_J(0,SEG_NUM){
				//j = 2;
				int part_id = j / nump;
				int begin1,begin2,end1,end2;
				int id = j % nump;
				if (0 == id){
					begin1 = 0;
					end2 = len;
				}
				else {
					begin1 = cut[j-1];
					end2 = cut2[j-1];
				}
				if ( nump == id+1 ){//last one
					end1 = len;
					begin2 = 0;
				}
				else {
					end1 = cut[j];
					begin2 = cut2[j];
				}
				int size1 = end1 - begin1;
				int size2 = end2 - begin2;


				int * addr1 = data + part_id*n2 + begin1;
				int * addr2 = data + part_id*n2 + len+begin2;
				int *addr3;
				//cout<<"To merge"<<endl;
				//show_trends(addr1,size1);
				//show_trends(addr2,size2);

				if (part_id % 2 != 0){
					addr3 = data2 + (part_id+1)*n2 - (id+1)*seg_len;
					merge_nonealign<true>(addr1,size1,addr2,size2,addr3,seg_len);
				}
				else {
					addr3 = data2 + j*seg_len;
					merge_nonealign<false>(addr1,size1,addr2,size2,addr3,seg_len);
				}
				//cout<<"after merge"<<endl;
				//show_trends(addr3,seg_len);
			}
			

			swap(data2,data);
			//cout<<"--------------------------------"<<endl;
			//show_trends(data,n);
			
	
		}
		return data;
	}

	void work(int* data_temp){

		
		int seed =0;
		Watch watch;
//		srand(seed);
		//nn = 4*1024*1024;
		//nn = 32*2*16*2;
		//nn = 32*4*4*16;
		//int mod = 1231328761;
		//mod = 123;
		//generate_data(data,nn,mod);
		//fun0(data);

		memcpy(data,data_temp,sizeof(int)*nn);
//		posix_memalign((void **)&correct,64,sizeof(int)*nn);
//		memcpy(correct,data,sizeof(int)*nn);
		memcpy(data2,data,sizeof(int)*nn);
		watch.start();
//		sort(correct,correct + nn);
		double time2 = watch.stop();
		double time1;
		watch.start();
		//show_trends(data,nn);//debug
		int * result_pointer = parallel_merge_sort(data,data2,nn,false);
		time1 = watch.stop();
//		outln(time1);outln(time2);
		//show_trends(result_pointer,nn);
		//BBLUE(debug_a(correct,nn));//debug
//		check_correctness(result_pointer,correct,nn);

		//free(correct);
	}
	void inits(){
		vconst_sort1 = _mm512_set_epi32( 9,8,11,10,13,12,15,14,1,0,3,2,5,4,7,6 );
		vconst_sort2 = _mm512_set_epi32( 9,8,11,10,13,12,15,14,1,0,3,2,5,4,7,6 );
		vconst_sort3 = _mm512_set_epi32( 7,15,14,6,13,5,4,12,11,3,2,10,1,9,8,0 );
		vconst_sort4 = _mm512_set_epi32( 0,8,9,1,10,2,3,11,12,4,5,13,6,14,15,7 );
		vconst_merge1 = _mm512_set_epi32( 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 );
		vconst_merge2 = _mm512_set_epi32( 5,10,11,4,9,6,7,8,13,2,3,12,1,14,15,0 );
		vconst_merge3 = _mm512_set_epi32( 0,15,14,1,12,3,2,13,8,7,6,9,4,11,10,5 );
	}

int main(int argc, char** argv){
//      BRED(cout<<"PROGRAM BEGIN"<<endl);
        printf(" Begin \n");

        char* ifn = argv[1];
float file_begin=microtime();
        inits();
       FILE* ifp;

         if((ifp=fopen(ifn,"r"))==NULL)
         {
                printf("%s file open error!\n",ifn);
                    exit(0);
         }
         fscanf(ifp,"%d",&nn);
        //work(148);return 0;
int * data_temp;

        posix_memalign((void **)&data_temp,64,sizeof(int)*nn);
        posix_memalign((void **)&data,64,sizeof(int)*nn);
        posix_memalign((void **)&data2,64,sizeof(int)*nn);
       get_data(ifp,data_temp,nn);

printf(" file reading time is %fs \n", microtime()-file_begin);
double parallel_begin = microtime();

        FOR_I(1,100)
        work(data_temp);
printf(" parallel time is %fs \n", microtime()-parallel_begin);
                free(data_temp);
                free(data);
                free(data2);
        return 0;
}







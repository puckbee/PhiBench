#include <sys/time.h>   
#include <stdio.h>   
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <string>

//#pragma offload_attribute(push,target(mic))
#include <cilk/cilk.h>

#define DataType float
#define INF HUGE_VAL
#define PHIDEV 1
#define VectorLength 16
#define MY_THREADS 12
#define ScheduleState static
#define ALIGNLENGTH 128
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
#define CachingSize 10

struct Kernel_params{
	DataType gamma;
	DataType coef0;
	int degree;
	DataType b;
	std::string kernel_type;
};

DataType parameterA = -0.125;
DataType parameterB = 1.0;
DataType parameterC = 3.0;
DataType cost = 1.0;
DataType tolerance = 1e-3;
DataType epsilon = 1e-5; 
int* data_index; 
int* row_index;
DataType* data; 
DataType* data_square;
DataType* labels;
DataType* alpha;
DataType* tempF1;
DataType* tempF2;
DataType* devF;
DataType* devKernelDiag;
DataType* highKernel;
DataType* lowKernel;
DataType* highRow;
DataType* lowRow;
DataType cEpsilon, alphaHighDiff, alphaLowDiff, bLow, bHigh, alphaHighOld, alphaLowOld, eta;
Kernel_params kp;
int nPoints, iLow, iHigh, iteration, max_index, elements, caching_size, caching_index;

enum SelectionHeuristic {FIRSTORDER, SECONDORDER, RANDOM, ADAPTIVE};
enum KernelType {LINEAR, POLYNOMIAL, GAUSSIAN, SIGMOID};

  static DataType selfKernel() {
    return 1.0;
  }
  static DataType kernel(int a, int b) {
    int i,j;
    DataType accumulant=0;
    for(int i=row_index[a];i<row_index[a+1];i++)
    	accumulant+=(data[i]*data[i]);
    for(int j=row_index[b];j<row_index[b+1];j++)
    	accumulant+=(data[j]*data[j]);
    i=row_index[a];
    j=row_index[b];
    while(i<row_index[a+1]&&j<row_index[b+1]){
	    if(data_index[i]==data_index[j]){
		    accumulant-=(2*data[i]*data[j]);
			i++;
			j++;
	    }else if(data_index[i]>data_index[j]){
		    j++;
	    }else{
		    i++;
	    }
    }    
    return exp(parameterA * accumulant);
  }  
//#pragma offload_attribute(pop)
	/*
  static DataType kernel(int a, int b) {
    int i,j;
    DataType accumulant=0;
    i=row_index[a];
    j=row_index[b];
    while(i<row_index[a+1]&&j<row_index[b+1]){
	    if(data_index[i]==data_index[j]){
		    accumulant+=((data[i]-data[j])*(data[i]-data[j]));
			i++;
			j++;
	    }else if(data_index[i]>data_index[j]){
		    accumulant+=(data[j]*data[j]);
		    j++;
	    }else{
		    accumulant+=(data[i]*data[i]);
		    i++;
	    }
    }    
    while(i<row_index[a+1]){
		    accumulant+=(data[i]*data[i]);
		    i++;
	}
	while(j<row_index[b+1]){
	    accumulant+=(data[j]*data[j]);
	    j++;
	}
    return exp(parameterA * accumulant);
  }  
	
  static DataType kernel(DataType* pointerA, DataType* pointerB) {
    //DataType accumulant = __sec_reduce_add((pointerA[0:nDimension]-pointerB[0:nDimension])*(pointerA[0:nDimension]-pointerB[0:nDimension]));
    int i;
    DataType accumulant=0;
    for(i=0;i<nDimension;i++)
    	accumulant+=((pointerA[i]-pointerB[i])*(pointerA[i]-pointerB[i]));
    return exp(parameterA * accumulant);
  }  
  
  void initial_data_square(){
	  int i,length;
	  #pragma omp parallel for schedule(ScheduleState) private(i,length) //#pragma vector always aligned
	  for(i=0;i<nPoints;i++){
		  length = (row_index[i+1]-row_index[i]);
		  data_square[i] = __sec_reduce_add(data[row_index[i]:length]*data[row_index[i]:length]);
		  //for(j=row_index[i];j<row_index[i+1];j++)		  	
	  }
  }
  */
static char *line = NULL;
static int max_line_len;
//template<class Kernel>
void initializeArrays() { 	
	for (int index = 0;index < nPoints;index++) {
		devKernelDiag[index] = selfKernel();
		devF[index] = -labels[index];
	}
}

void launchInitialization() {  
    initializeArrays();   
}

//template<class Kernel>
void takeFirstStep() {                                
	eta = devKernelDiag[iHigh] + devKernelDiag[iLow];
	DataType phiAB = kernel(iHigh, iLow);	
	//printf("phiAB: %f\n",phiAB);
	eta = eta - 2*phiAB;
	alphaLowOld = alpha[iLow];
	alphaHighOld = alpha[iHigh]; 
	//And we know eta > 0
	DataType alphaLowNew = 2/eta; //Just boil down the algebra
	if (alphaLowNew > cost) {
		alphaLowNew = cost;
	}
	//alphaHighNew == alphaLowNew for the first step
	alpha[iLow] = alphaLowNew;
	alpha[iHigh] = alphaLowNew;	
}
void launchTakeFirstStep(int kType) {
     takeFirstStep();
  }

void performTraining(){	
	int kType = GAUSSIAN;
	int row_index_size=nPoints+1;
  	struct timeval start,finish;
  	DataType trainingTime;
  	caching_size = CachingSize*1024*1024/nPoints/sizeof(DataType);
	if (kp.kernel_type.compare(0,3,"rbf") == 0) {
		parameterA = -kp.gamma;
		kType = GAUSSIAN;
		printf("Gaussian kernel: gamma = %f\n", -parameterA);
	} 
	printf("--Cost: %f, Tolerance: %f, Epsilon: %f\n", cost, tolerance, epsilon); 
	cEpsilon = cost - epsilon; 
	devKernelDiag = (DataType*)malloc(sizeof(DataType) * nPoints);
	devF = (DataType*)malloc(sizeof(DataType) * nPoints);
	tempF1 = (DataType*)malloc(sizeof(DataType) * nPoints);
	tempF2 = (DataType*)malloc(sizeof(DataType) * nPoints);
	data_square = (DataType*)calloc(nPoints,sizeof(DataType));
	highKernel = (DataType*)malloc(sizeof(DataType) * nPoints);
	lowKernel = (DataType*)malloc(sizeof(DataType) * nPoints);
	highRow = (DataType*)malloc(sizeof(DataType) * max_index);
	lowRow = (DataType*)malloc(sizeof(DataType) * max_index);
	
	launchInitialization(); 
	printf("Initialization complete\n");

  	bLow = 1;
  	bHigh = -1;
  	iLow = -1;
  	iHigh = -1;
  	for (int i = 0; i < nPoints; i++) {
    	if (labels[i] < 0) {
      		if (iLow == -1) {
        		iLow = i;
        		if (iHigh > -1) {
          		i = nPoints; //Terminate
        		}
      		}
    	} else {
      		if (iHigh == -1) {
        		iHigh = i;
        		if (iLow > -1) {
          			i = nPoints; //Terminate
        		}
      		}
    	}
  } 
  launchTakeFirstStep(kType);
  alphaLowDiff = alpha[iLow] - alphaLowOld;
  alphaHighDiff = -labels[iHigh] * labels[iLow] * alphaLowDiff;
  printf("Starting iterations\n");
  iteration = 1;  
  gettimeofday(&start, 0);

/*  #pragma offload target(mic:PHIDEV)\    
  in(parameterA, cost, tolerance, epsilon, alphaLowDiff, alphaHighDiff)\ 
  in(data:length(elements) alloc_if(1))\
  in(data_index:length(elements) alloc_if(1))\
  in(row_index:length(row_index_size) alloc_if(1))\
  in(labels:length(nPoints) alloc_if(1))\
  in(devF:length(nPoints) alloc_if(1))\ 
  in(devKernelDiag:length(nPoints) alloc_if(1))\ 
  inout(alpha:length(nPoints) alloc_if(1))\
  nocopy(tempF1:length(nPoints) alloc_if(1))\ 
  nocopy(tempF2:length(nPoints) alloc_if(1))\
  nocopy(data_square:length(nPoints) alloc_if(1))\ 
  nocopy(highKernel:length(nPoints) alloc_if(1))\
  nocopy(lowKernel:length(nPoints) alloc_if(1))\ 
  nocopy(highRow:length(max_index) alloc_if(1))\ 
  nocopy(lowRow:length(max_index) alloc_if(1))
  {
*/
	int i,j;
	
//	#pragma omp parallel for schedule(ScheduleState) private(i,j) 
	  cilk_for(int i=0;i<nPoints;i++){
//xbw		  #pragma vector always aligned
		  for(int j=row_index[i];j<row_index[i+1];j++)
		  	data_square[i]+=data[j]*data[j];
		  //j = (row_index[i+1]-row_index[i]);
		  //data_square[i] = __sec_reduce_add(data[row_index[i]:j]*data[row_index[i]:j]);		  
	  }
	
	
	while(bLow > bHigh + 2*tolerance)
	{	
	  
  	  //#pragma vector always aligned
  	  //for(i=0;i<max_index;i++){
	  //	  highRow[i]=0;
	  //	  lowRow[i]=0;
  	  //}
  	  highRow[0:max_index]=lowRow[0:max_index]=0;
  	  
//xbw  	  #pragma vector always aligned
  	  for(int j=row_index[iHigh];j<row_index[iHigh+1];j++)
  	  	highRow[data_index[j]]=data[j];  	  
  //xbw	  #pragma vector always aligned
  	  for(int j=row_index[iLow];j<row_index[iLow+1];j++)
  	  	lowRow[data_index[j]]=data[j];
  	
//  	  #pragma omp parallel for schedule(ScheduleState)
//xbw  	  #pragma vector always aligned
  	  cilk_for (int i = 0; i < nPoints; i++ ) { 
  		highKernel[i] = data_square[i]+data_square[iHigh]; 
  		lowKernel[i] = data_square[i]+data_square[iLow];
  		
  		for (int j = row_index[i]; j < row_index[i+1]; j++) { 
    		highKernel[i] -= 2*highRow[data_index[j]]*data[j]; 
    		lowKernel[i] -= 2*lowRow[data_index[j]]*data[j];
  		}
  		
  		devF[i] = devF[i] + alphaHighDiff * labels[iHigh] * exp(parameterA*highKernel[i]) + alphaLowDiff * labels[iLow] * exp(parameterA*lowKernel[i]);
  		  		
  		if(((labels[i] > 0) && (alpha[i] < cEpsilon)) || ((labels[i] < 0) && (alpha[i] > epsilon)))
			  	tempF1[i]=devF[i];
			  else
			  	tempF1[i]=INF;
			  	
		if(((labels[i] > 0) && (alpha[i] > epsilon)) || ((labels[i] < 0) && (alpha[i] < cEpsilon)))
			  	tempF2[i]=devF[i];
			  else
			  	tempF2[i]=-INF;
	  } 
	  iHigh = __sec_reduce_min_ind(tempF1[0:nPoints]);
	  iLow = __sec_reduce_max_ind(tempF2[0:nPoints]);
		  
	  bHigh = devF[iHigh];
	  bLow = devF[iLow];
	
	  highRow[0:max_index]=highRow[0:max_index]-lowRow[0:max_index];
	  eta = devKernelDiag[iHigh] + devKernelDiag[iLow] - 2 * exp(parameterA * __sec_reduce_add(highRow[0:max_index]*highRow[0:max_index]));
  	    
	  alphaHighOld = alpha[iHigh];
	  alphaLowOld = alpha[iLow];
	  DataType alphaDiff = alphaLowOld - alphaHighOld;
	  DataType lowLabel = labels[iLow];
	  DataType sign = labels[iHigh] * lowLabel;
	  DataType alphaLowUpperBound;
	  DataType alphaLowLowerBound;
	  if (sign < 0) {
          if (alphaDiff < 0) {
              alphaLowLowerBound = 0;
              alphaLowUpperBound = cost + alphaDiff;
          } else {
			  alphaLowLowerBound = alphaDiff;
              alphaLowUpperBound = cost;
		  }
      } else {
		DataType alphaSum = alphaLowOld + alphaHighOld;
		if (alphaSum < cost) {
			alphaLowUpperBound = alphaSum;
			alphaLowLowerBound = 0;
		} else {
			alphaLowLowerBound = alphaSum - cost;
			alphaLowUpperBound = cost;
        }
	  }

      DataType alphaLowNew;
      if (eta > 0) {
		alphaLowNew = alphaLowOld + lowLabel*(bHigh - bLow)/eta;
		if (alphaLowNew < alphaLowLowerBound) alphaLowNew = alphaLowLowerBound;
        else if (alphaLowNew > alphaLowUpperBound) alphaLowNew = alphaLowUpperBound;
	  } else {
		DataType slope = lowLabel * (bHigh - bLow);
		DataType delta = slope * (alphaLowUpperBound - alphaLowLowerBound);
		if (delta > 0) {
			if (slope > 0)	alphaLowNew = alphaLowUpperBound;
			else	alphaLowNew = alphaLowLowerBound;
		} else	alphaLowNew = alphaLowOld;
	  }
    alphaLowDiff = alphaLowNew - alphaLowOld;
    alphaHighDiff = -sign*(alphaLowDiff);
    alpha[iLow] = alphaLowNew;
    alpha[iHigh] = (alphaHighOld + alphaHighDiff);
	  	iteration++;
	  	//printf("iLow: %d, bLow: %lf; iHigh: %d, bHigh: %lf\n",iLow,bLow,iHigh,bHigh);
	  	//if(iteration==30)
	  	//	break;
	  }
//  }
  gettimeofday(&finish, 0);
  trainingTime = (DataType)(finish.tv_sec - start.tv_sec) + ((DataType)(finish.tv_usec - start.tv_usec)) * 1e-6;	
  printf("Training time : %f seconds\n", trainingTime);
  printf("--- %d iterations --- ", iteration);
  printf("bLow: %f, bHigh: %f\t", bLow, bHigh);
  kp.b = (bLow + bHigh) / 2;
  printf("b: %f\n", kp.b);
  FILE * sparse_time = fopen("sparse2_time","a+");
  fprintf(sparse_time,"%f\n", trainingTime);
}

void exit_input_error(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

static char* readline(FILE *input)
{
	int len;
	
	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

void read_problem(const char *filename)
{
	int inst_max_index, i, j;
	FILE *fp = fopen(filename,"r");
	char *endptr;
	char *idx, *val, *label;

	if(fp == NULL)
	{
		fprintf(stderr,"can't open input file %s\n",filename);
		exit(1);
	}

	nPoints = 0;
	elements = 0;

	max_line_len = 1024;
	line = Malloc(char,max_line_len);
	while(readline(fp)!=NULL)
	{
		char *p = strtok(line," \t"); // label
		// features
		while(1)
		{
			p = strtok(NULL," \t");
			if(p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
				break;
			++elements;
		}
		//++elements;
		++nPoints;
	}
	rewind(fp);
	//prob.y = Malloc(double,nPoints);
	labels = (DataType *)malloc(nPoints*sizeof(DataType));
	row_index = (int *)malloc((nPoints+1)*sizeof(int));
	//prob.x = Malloc(struct svm_node *,nPoints);
	data = (DataType *)malloc(elements*sizeof(DataType));
	data_index = (int *)malloc(elements*sizeof(int));
	//x_space = Malloc(struct svm_node,elements);

	max_index = 0;
	j=0;
	for(int i=0;i<nPoints;i++)
	{
		inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
		readline(fp);
		//prob.x[i] = &x_space[j];
		row_index[i]=j;
		label = strtok(line," \t\n");
		if(label == NULL) // empty line
			exit_input_error(i+1);
		//prob.y[i] = strtod(label,&endptr);
		labels[i] = strtod(label,&endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_error(i+1);

		while(1)
		{
			idx = strtok(NULL,":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;

			errno = 0;
			//x_space[j].index = (int) strtol(idx,&endptr,10);
			data_index[j] = (int) strtol(idx,&endptr,10);
			//if(endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
			if(endptr == idx || errno != 0 || *endptr != '\0' || data_index[j] <= inst_max_index)
				exit_input_error(i+1);
			else
				//inst_max_index = x_space[j].index;
				inst_max_index = data_index[j];

			errno = 0;
			//x_space[j].value = strtod(val,&endptr);
			data[j] = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(i+1);

			++j;
		}

		if(inst_max_index > max_index)
			max_index = inst_max_index;
		//x_space[j++].index = -1;
	}	
	row_index[i]=elements;
	//if(param.gamma == 0 && max_index > 0)
	//	param.gamma = 1.0/max_index;		
	fclose(fp);
	/*
	FILE * verify_input =fopen("verify_input","w");
	for(i=0;i<nPoints;i++){
		fprintf(verify_input,"%d ",(int)labels[i]);
		for(j=row_index[i];j<row_index[i+1];j++)
			fprintf(verify_input,"%d:%f ",data_index[j],data[j]);
		fprintf(verify_input,"\n");
	
	}*/
}

void printModel(const char* outputFile) { 
	printf("Output File: %s\n", outputFile);
	FILE* outputFilePointer = fopen(outputFile, "w");
	if (outputFilePointer == NULL) {
		printf("Can't write %s\n", outputFile);
		exit(1);
	}
	int nSV = 0;
	int pSV = 0;
	for(int i = 0; i < nPoints; i++) {
		if (alpha[i] > epsilon) {
			if (labels[i] > 0) {
				pSV++;
			} else {
				nSV++;
			}
		}
	}
  bool printGamma = false;
  bool printCoef0 = false;
  bool printDegree = false;
  const char* kernelType = kp.kernel_type.c_str();
  if (strncmp(kernelType, "polynomial", 10) == 0) {
    printGamma = true;
    printCoef0 = true;
    printDegree = true;
  } else if (strncmp(kernelType, "rbf", 3) == 0) {
    printGamma = true;
  } else if (strncmp(kernelType, "sigmoid", 7) == 0) {
    printGamma = true;
    printCoef0 = true;
  }
	
	fprintf(outputFilePointer, "svm_type c_svc\n");
	fprintf(outputFilePointer, "kernel_type %s\n", kp.kernel_type.c_str());
  if (printDegree) {
    fprintf(outputFilePointer, "degree %i\n", kp.degree);
  }
  if (printGamma) {
    fprintf(outputFilePointer, "gamma %f\n", kp.gamma);
  }
  if (printCoef0) {
    fprintf(outputFilePointer, "coef0 %f\n", kp.coef0);
  }
	fprintf(outputFilePointer, "nr_class 2\n");
	fprintf(outputFilePointer, "total_sv %d\n", nSV + pSV);
	fprintf(outputFilePointer, "rho %.10f\n", kp.b);
	fprintf(outputFilePointer, "label 1 -1\n");
	fprintf(outputFilePointer, "nr_sv %d %d\n", pSV, nSV);
	fprintf(outputFilePointer, "SV\n");
	for (int i = 0; i < nPoints; i++) {
		if (alpha[i] > epsilon) {
			fprintf(outputFilePointer, "%.10f ", labels[i]*alpha[i]);
			for (int j = row_index[i]; j < row_index[i+1]; j++)
				fprintf(outputFilePointer, "%d:%.10f ", data_index[j], data[j]);
			//for (int j = 0; j < nDimension; j++)
			//	fprintf(outputFilePointer, "%d:%.10f ", j+1, data[i*nDimension + j]);
			fprintf(outputFilePointer, "\n");
		}
	}
	fclose(outputFilePointer);
}

void printHelp() {
  printf("Usage: svmTrain [options] trainingData.svm\n");
  printf("Options:\n");
  printf("\t-o outputFilename\t Location of output file\n");
  printf("Kernel types:\n");
  printf("\t--gaussian\tGaussian or RBF kernel (default): Phi(x, y; gamma) = exp{-gamma*||x-y||^2}\n");
  printf("\t--linear\tLinear kernel: Phi(x, y) = x . y\n");
  printf("\t--polynomial\tPolynomial kernel: Phi(x, y; a, r, d) = (ax . y + r)^d\n");
  printf("\t--sigmoid\tSigmoid kernel: Phi(x, y; a, r) = tanh(ax . y + r)\n");
  printf("Parameters:\n");
  printf("\t-c, --cost\tSVM training cost C (default = 1)\n");
  printf("\t-g\tGamma for Gaussian kernel (default = 1/nDimension)\n");
  printf("\t-a\tParameter a for Polynomial and Sigmoid kernels (default = 1/l)\n");
  printf("\t-r\tParameter r for Polynomial and Sigmoid kernels (default = 1)\n");
  printf("\t-d\tParameter d for Polynomial kernel (default = 3)\n");
  printf("Convergence parameters:\n");
  printf("\t--tolerance, -t\tTermination criterion tolerance (default = 0.001)\n");
  printf("\t--epsilon, -e\tSupport vector threshold (default = 1e-5)\n");
  printf("Internal options:\n");
  printf("\t--heuristic, -h\tWorking selection heuristic:\n");
  printf("\t\t0: First order\n");
  printf("\t\t1: Second order\n");
  printf("\t\t2: Random (either first or second order)\n");
  printf("\t\t3: Adaptive (default)\n");
}

static int kType = GAUSSIAN;

int main( const int argc, const char** argv)  {   	  	
	int currentOption;
  	bool parameterASet = false;
  	bool parameterBSet = false;
  	bool parameterCSet = false;  
  	SelectionHeuristic heuristicMethod = ADAPTIVE; 
  	char* outputFilename = NULL; 
  	struct timeval init_start,init_finish;
	DataType initTime;
  gettimeofday(&init_start, 0);

  while (1) {
    static struct option longOptions[] = {
      {"gaussian", no_argument, &kType, GAUSSIAN},
      {"polynomial", no_argument, &kType, POLYNOMIAL},
      {"sigmoid", no_argument, &kType, SIGMOID},
      {"linear", no_argument, &kType, LINEAR},
      {"cost", required_argument, 0, 'c'},
      {"heuristic", required_argument, 0, 'h'},
      {"tolerance", required_argument, 0, 't'},
      {"epsilon", required_argument, 0, 'e'},
      {"output", required_argument, 0, 'o'},
      {"version", no_argument, 0, 'v'},
      {"help", no_argument, 0, 'f'}
    };
    int optionIndex = 0;
    currentOption = getopt_long(argc, (char *const*)argv, "c:h:t:e:o:a:r:d:g:v:f", longOptions, &optionIndex);
    if (currentOption == -1) {
      break;
    }
    int method = 3;
    switch (currentOption) {
    case 0:
      break;
    case 'v':
      printf("MICSVM version: 1.0\n");
      return(0);
    case 'f':
      printHelp();
      return(0);
    case 'c':
      sscanf(optarg, "%f", &cost);
      break;
    case 'h':
      sscanf(optarg, "%i", &method);
      switch (method) {
      case 0:
        heuristicMethod = FIRSTORDER;
        break;
      case 1:
        heuristicMethod = SECONDORDER;
        break;
      case 2:
        heuristicMethod = RANDOM;
        break;
      case 3:
        heuristicMethod = ADAPTIVE;
        break;
      }
      break;
    case 't':
      sscanf(optarg, "%f", &tolerance);
      break;
    case 'e':
      sscanf(optarg, "%f", &epsilon);
      break;
    case 'o':
      outputFilename = (char*)malloc(strlen(optarg));
      strcpy(outputFilename, optarg);
      break;
    case 'a':
      sscanf(optarg, "%f", &parameterA);
      parameterASet = true;
      break;
    case 'r':
      sscanf(optarg, "%f", &parameterB);
      parameterBSet = true;
      break;
    case 'd':
      sscanf(optarg, "%f", &parameterC);
      parameterCSet = true;
      break;
    case 'g':
      sscanf(optarg, "%f", &parameterA);
      parameterA = -parameterA;
      parameterASet = true;
      break;
    case '?':
      break;
    default:
      abort();
      break;
    }
  }

  if (optind != argc - 1) {
    printHelp();
    return(0);
	}

	const char* trainingFilename = argv[optind];  
  
  if (outputFilename == NULL) {
    int inputNameLength = strlen(trainingFilename);
    outputFilename = (char*)malloc(sizeof(char)*(inputNameLength + 5));
    strncpy(outputFilename, trainingFilename, inputNameLength + 4);
    char* period = strrchr(outputFilename, '.');
    if (period == NULL) {
      period = outputFilename + inputNameLength;
    }
    strncpy(period, ".mdl\0", 5);
  }  	
	//readSvm(trainingFilename, &nPoints, &nDimension);
	read_problem(trainingFilename);
	//return 0;
	printf("Input data found: %d points, %d is the maximum dimension\n", nPoints, max_index); 
	
	alpha = (DataType*)calloc(nPoints, sizeof(DataType));
  
  if (kType == LINEAR) {
    printf("Linear kernel\n");
    //kp.kernel_type = "linear";
  	} else if (kType == POLYNOMIAL) {
    if (!(parameterCSet)) {
      parameterC = 3.0f;
    }
    if (!(parameterASet)) {
      //parameterA = 1.0/nPoints;
	  //parameterA = 1.0/nDimension;
	  parameterA = 1.0/max_index;
    }
    if (!(parameterBSet)) {
      parameterB = 0.0f;
    }
    //printf("Polynomial kernel: a = %f, r = %f, d = %f\n", parameterA, parameterB, parameterC);
    if ((parameterA <= 0) || (parameterB < 0) || (parameterC < 1.0)) {
      printf("Invalid parameters\n");
      exit(1);
    }
    kp.kernel_type = "polynomial";
    kp.gamma = parameterA;
    kp.coef0 = parameterB;
    kp.degree = (int)parameterC;
  	} else if (kType == GAUSSIAN) {
    if (!(parameterASet)) {
      //parameterA = 1.0/nPoints;
	  parameterA = 1.0/max_index;
    } else {
      parameterA = -parameterA;
    }
    //printf("Gaussian kernel: gamma = %f\n", parameterA);
    if (parameterA < 0) {
      printf("Invalid parameters\n");
      exit(1);
    }
    kp.kernel_type = "rbf";
    kp.gamma = parameterA;
  } else if (kType == SIGMOID) {
    if (!(parameterASet)) {
      //parameterA = 1.0/nPoints;
	  parameterA = 1.0/max_index;
    }
    if (!(parameterBSet)) {
      parameterB = 0.0f;
    }
    //printf("Sigmoid kernel: a = %f, r = %f\n", parameterA, parameterB);
    if ((parameterA <= 0) || (parameterB < 0)) {
      printf("Invalid Parameters\n");
      exit(1);
    }
    kp.kernel_type = "sigmoid";
    kp.gamma = parameterA;
    kp.coef0 = parameterB;
  }
	
	  gettimeofday(&init_finish, 0);
  initTime = (DataType)(init_finish.tv_sec - init_start.tv_sec) + ((DataType)(init_finish.tv_usec - init_start.tv_usec)) * 1e-6;
  printf("Init time : %f seconds\n", initTime);

	//struct timeval start,finish;
	//gettimeofday(&start, 0);
	performTraining();
	//gettimeofday(&finish, 0);
	//DataType trainingTime = (DataType)(finish.tv_sec - start.tv_sec) + ((DataType)(finish.tv_usec - start.tv_usec)) * 1e-6;	
	//printf("all time : %f seconds\n", trainingTime);
	printModel(outputFilename);
	return 0;
}

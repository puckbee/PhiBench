#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <memory.h>
#include <sys/time.h>

#define REAL double
#define EPSILON 0.000001

#define MICRO_IN_SEC 1000000.00
__declspec(target(mic)) double begin, end;
__declspec(target(mic))
double microtime(){
	int tv_sec,tv_usec;
	double time;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);

	return tv.tv_sec+tv.tv_usec/MICRO_IN_SEC;
}

int caculateNB_train(char *ifn) ;
int main(int argc,char* argv[]) {

	char *ifn=NULL,*argument=NULL;
	if(argc<3)
	{
		printf("Wrong command format! usage:COMMAND ARGUMENT INPUTFILENAME\nARGUMENT:\n\ttrain:\ttrain the classifier\n\tclassify:\tclassify the dataset\n");
		return 0;
	}
	else
	{
		ifn=argv[2];
		argument=argv[1];
		if(!strcmp(argument,"train")) {
			caculateNB_train(ifn);
		} else if(!strcmp(argument,"classify")) {
			caculateNB_classify(ifn);
		} else {
			printf("Error command!\n");
		}
		return 0;
	}
}

int caculateNB_train(char *ifn) {

	char *ofn="nb_train_result.txt";
	FILE *ifp,*ofp;
	int i,j,k,class,a,linen,propertyn,classn,*array_valuen,*array_class,*array_property_class,*array_counts,*array_index,array_length;
	REAL *array_class_probability,*array_probability;

	begin = microtime();

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
	
	printf("Get base info\n");
	fscanf(ifp,"%d%d%d",&linen,&propertyn,&classn);  //linen is the number of dataset lines, \
	propertyn is the number of property of every dataset, classn is the number of classes;
	
	printf("Read data\n");
	//array_valuen is an array of the max value of every property 
	if((array_valuen=(int *)malloc(propertyn*sizeof(int)))==NULL) {
		printf("Memory alloc ERROR!\n");
		fclose(ifp);
		fclose(ofp);
		exit(0);
	}

	printf("Get property number\n");
	for(i=0;i<propertyn;i++) {
		fscanf(ifp,"%d",array_valuen+i);
	}

	//array_index is an array of the index of every property in array_probability
	if((array_index=(int *)malloc(propertyn*sizeof(int)))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		exit(0);
	}


	for(i=0;i<propertyn;i++) {
		array_index[i] = 0;
	}

	array_length = array_valuen[0] * classn;
	array_index[0] = 0;

	if(propertyn>1) {
		for(i=1;i<propertyn;i++) {
			array_length += array_valuen[i] * classn;
			array_index[i] = array_valuen[i-1] * classn + array_index[i-1];
		}
	}


	//the array_class is the array of count of class
	if((array_class=(int *)malloc(classn*(sizeof(int))))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_index);
		exit(0);
	}
	memset(array_class,0,classn*sizeof(int));

	//the array_property_class is the array of the count of every property of every class
	if((array_property_class=(int *)malloc(propertyn*classn*(sizeof(int))))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_class);
		free(array_index);
		exit(0);
	}
	memset(array_property_class,0,propertyn*classn*sizeof(int));

	//array_counts is an array of the pointer of counter of every property of every class
	if((array_counts=(int *)malloc(array_length*(sizeof(int))))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_index);
		free(array_class);
		free(array_property_class);
		exit(0);
	}
	memset(array_counts,0,array_length*(sizeof(int)));
		
	printf("Get every needed info\n");
	for(i=0;i<linen;i++) {
		fscanf(ifp,"%d",&class);
		array_class[class]++;
		for(j=0;j<propertyn;j++) {
			fscanf(ifp,"%d",&a);
			array_counts[ array_index[j] + a*classn + class ] ++;
			array_property_class[j*classn+class]++;
		}
	}

	//array_class_probability is an array of the classes
	if((array_class_probability=(REAL *)malloc(classn*(sizeof(REAL))))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_index);
		free(array_counts);
		free(array_class);
		free(array_property_class);
		exit(0);
	}
	
	//array_probability is an array of the pointer of probability of every property of every class
	if((array_probability=(REAL *)malloc(array_length*(sizeof(REAL))))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_index);
		free(array_counts);
		free(array_class);
		free(array_class_probability);
		free(array_property_class);
		exit(0);
	}

	end = microtime();
	printf("\nalloc memory and reading data consuming time: %fs\n\n",end-begin);
	begin = end;

	printf("Get P(C)\n");
	//caculate the p(c)
#pragma offload target(mic) \
	inout(array_class_probability:length(classn)) \
	in(array_class:length(classn))
#pragma omp parallel for
	for(i=0;i<classn;i++) {
		array_class_probability[i]=(REAL)(array_class[i]+1)/(REAL)(linen+classn);
	}

	printf("Get P(A|C)\n");
	//caculate the p(a|c)
#pragma offload target(mic) \
	inout(array_probability:length(array_length)) \
	in(array_counts:length(array_length)) \
	in(array_index:length(propertyn)) \
	in(array_property_class:length(propertyn*classn)) \
	in(array_valuen:length(propertyn))
#pragma omp parallel for
	for(i=0;i<propertyn;i++) {
		for(j=0;j<array_valuen[i];j++) {
			for(k=0;k<classn;k++) {
				array_probability[ array_index[i] + j*classn+k ]=(REAL)( array_counts[ array_index[i] + j*classn+k ] + 1 )/(REAL)(array_property_class[i*classn+k]+array_valuen[i]);
			}
		}
	}

	end = microtime();
	printf("\ntrain the classifier consuming time: %fs\n\n",end - begin);
	begin = end;

	//p(c) and p(a|c) is the training result
	
	printf("Outputing the training result to %s\n",ofn);

	fprintf(ofp,"%d %d\n",propertyn,classn);
	for(i=0;i<propertyn;i++) {
		fprintf(ofp,"%d ",array_valuen[i]);
	}
	fprintf(ofp,"\n");

	for(i=0;i<classn;i++) {
		fprintf(ofp,"%f ",array_class_probability[i]);
	}
	fprintf(ofp,"\n");

	for(i=0;i<propertyn;i++) {
		for(j=0;j<array_valuen[i];j++) {
			for(k=0;k<classn;k++) {
				fprintf(ofp,"%f ",array_probability[ array_index[i] + j*classn+k]);
			}
			fprintf(ofp,"\n");
		}
	}

	printf("Recycle all resources\n");
	//recycle all resources
	fclose(ifp);
	fclose(ofp);
	free(array_valuen);
	free(array_index);
	free(array_property_class);

	free(array_probability);

	free(array_counts);

	free(array_class);
	free(array_class_probability);

	printf("\nPlease DON'T change %s either its name and content!!\n",ofn);
	return 0;
}

int caculateNB_classify(char *ifn) {
	char *ofn="nb_classify_result.txt",*ifn_classifier="nb_train_result.txt";
	FILE *ifp,*ofp,*ifp_classifier;
	int i,j,k,max,linen,propertyn,classn,*array_valuen,*array_test_class,*array_test,*array_probability_index,array_probability_length;
	REAL *array_class_probability,*array_probability,*array_test_class_probability;

	begin = microtime();

	if((ifp_classifier=fopen(ifn_classifier,"r"))==NULL)
	{
		printf("%s file open error!\n",ifn_classifier);
		exit(0);
	}
	else
	{
		printf("%s file opened success!\n",ifn_classifier);
	}
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
	
	printf("Get base info from %s and configure the classifier\n",ifn_classifier);
	fscanf(ifp_classifier,"%d%d",&propertyn,&classn);  //propertyn is the number of property of every dataset, classn is the number of classes;

	//array_valuen is an array of the max value of every property 
	if((array_valuen=(int *)malloc(propertyn*sizeof(int)))==NULL) {
		printf("Memory alloc ERROR!\n");
		fclose(ifp);
		fclose(ofp);
		exit(0);
	}
	for(i=0;i<propertyn;i++) {
		fscanf(ifp_classifier,"%d",array_valuen+i);
	}

	//array_class_probability is an array of the classes
	if((array_class_probability=(REAL *)malloc(classn*(sizeof(REAL))))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		exit(0);
	}
	for(i=0;i<classn;i++) {
		fscanf(ifp_classifier,"%f",array_class_probability+i);
	}
	
	//array_probability_index is an array of the index of every property in array_probability
	if((array_probability_index=(int *)malloc(propertyn*sizeof(int)))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_class_probability);
		exit(0);
	}
	for(i=0;i<propertyn;i++) {
		array_probability_index[i] = 0;
	}

	array_probability_length = array_valuen[0] * classn;
	array_probability_index[0] = 0;

	if(propertyn>1) {
		for(i=1;i<propertyn;i++) {
			array_probability_length += array_valuen[i] * classn;
			array_probability_index[i] = array_valuen[i-1] * classn + array_probability_index[i-1];
		}
	}

	//array_probability is an array of probability of every property of every class
	if((array_probability=(REAL *)malloc(array_probability_length*(sizeof(REAL))))==NULL) {
		printf("memory alloc error!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_class_probability);
		exit(0);
	}
	for(i=0;i<propertyn;i++) {
		for(j=0;j<array_valuen[i];j++) {
			for(k=0;k<classn;k++) {
				fscanf(ifp_classifier,"%f",&array_probability[ array_probability_index[i] + j*classn + k ]);
			}
		}
	}

	printf("Classifier initialize done!\n");

	printf("Begin classify the dataset\n");

	fscanf(ifp,"%d",&linen);
	//array_test is an array of the input data
	if((array_test=(int *)malloc(linen*propertyn*sizeof(int)))==NULL) {
		printf("Memory alloc ERROR!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_probability);
		free(array_class_probability);
		exit(0);
	}

	for(i=0;i<linen*propertyn;i++) {
			fscanf(ifp,"%d",array_test+i);
	}

	//array_test_class is an array of the classify result of every test data record
	if((array_test_class=(int *)malloc(linen*sizeof(int)))==NULL) {
		printf("Memory alloc ERROR!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_probability);
		free(array_class_probability);
		free(array_test);
		exit(0);
	}
	//array_test_class_probability is an array of the probability of every test data record of every class
	if((array_test_class_probability=(REAL *)malloc(linen*classn*sizeof(REAL)))==NULL) {
		printf("Memory alloc ERROR!\n");
		fclose(ifp);
		fclose(ofp);
		free(array_valuen);
		free(array_probability);
		free(array_class_probability);
		free(array_test_class);
		free(array_test);
		exit(0);
	}

	end = microtime();
	printf("\nalloc memory and reading data consuming time: %fs\n\n",end-begin);
	begin = end;

#pragma offload target(mic) \
	nocopy(array_test_class_probability:length(linen*classn) alloc_if(1) free_if(1)) \
	in(array_class_probability:length(classn)) \
	in(array_probability:length(array_probability_length)) \
	in(array_probability_index:length(propertyn)) \
	in(array_test:length(linen*propertyn)) \
	out(array_test_class:length(linen))
#pragma omp parallel for
	for(i=0;i<linen;i++) {
		for(j=0;j<classn;j++) {
			array_test_class_probability[i*classn+j] = log( array_class_probability[j] );
		}
		for(j=0;j<propertyn;j++) {
			for(k=0;k<classn;k++) {
				array_test_class_probability[i*classn+k] += log( array_probability[ array_probability_index[j] + array_test[i*propertyn+j]*classn + k] );
				//				printf("j=%d k=%d p=%f\n",j,k,array_test_class_probability[i*classn+k]);
			}
		}
		//		exit(0);
		max=0;
		for(j=0;j<classn;j++) {
			if(array_test_class_probability[i*classn+j]-array_test_class_probability[i*classn+max]>EPSILON) {
				max=j;
			}
		}
		array_test_class[i]=max;
	}

	end = microtime();
	printf("\nclassify the data consuming time: %fs\n\n",end - begin);
	begin = end;

	printf("Classify done\n");
	for(i=0;i<linen;i++) {
		fprintf(ofp,"%d %d\n",i,array_test_class[i]);
	}
	printf("Result outputed to %s\n",ofn);

	fclose(ifp);
	fclose(ofp);
	free(array_valuen);
	free(array_probability);
	free(array_class_probability);
	free(array_test_class);
	free(array_test_class_probability);
	free(array_test);

	return 0;
}

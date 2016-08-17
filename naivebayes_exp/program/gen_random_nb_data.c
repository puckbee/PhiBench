#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#define TRAIN_LINEN 1500000
#define TEST_LINEN 11000
#define PROPERTYN 500
#define CLASSN 20
#define WIDTH 500

int main(int argc,char* argv[]) {

	char *ifn=NULL;
//	if(argc<2)
//	{
//		printf("wrong command format! usage:COMMAND CLASSNUMBER\n");
//		return 0;
//	}
//	else
//	{
//		ifn=argv[1];
		gen_data();
		return 0;
//	}
}

int gen_data() {
	char *ofn_train="nb_data_train.txt";
	char *ofn_test="nb_data_test.txt";
	int i,j,array_valuen[PROPERTYN];
	FILE *ofp_train,*ofp_test;

	if((ofp_train=fopen(ofn_train,"w"))==NULL) {
		printf("File %s open error!\n",ofn_train);
		exit(0);
	}
	if((ofp_test=fopen(ofn_test,"w"))==NULL) {
		fclose(ofp_train);
		printf("File %s open error!\n",ofn_test);
		exit(0);
	}

	srand((unsigned)time(NULL));
	printf("Genereting dataset!\n");
	//linen propertyn classn
	fprintf(ofp_train,"%d %d %d\n",TRAIN_LINEN,PROPERTYN,CLASSN);
	//array_valuen
	for(i=0;i<PROPERTYN;i++) {
		fprintf(ofp_train,"%d ",(array_valuen[i] = (rand()%WIDTH)+2));
	}
	fprintf(ofp_train,"\n");

	for(i=0;i<TRAIN_LINEN;i++) {
		fprintf(ofp_train,"%d ",rand()%CLASSN);
		for(j=0;j<PROPERTYN;j++) {
			fprintf(ofp_train,"%d ",rand()%array_valuen[j]);
		}
		fprintf(ofp_train,"\n");
	}

	//linen, because of the training set has already told the classifier propertyn,classn and array_valuen
	fprintf(ofp_test,"%d\n",TEST_LINEN);
	for(i=0;i<TEST_LINEN;i++) {
		for(j=0;j<PROPERTYN;j++) {
			fprintf(ofp_test,"%d ",rand()%array_valuen[j]);
		}
		fprintf(ofp_test,"\n");
	}

	printf("Dataset genereted! Saved in file %s %s\n",ofn_train,ofn_test);

	fclose(ofp_train);
	fclose(ofp_test);

	return 0;
}

/**********************************************************************
		        Joint Sentiment-Topic (JST) Model
***********************************************************************

(C) Copyright 2013, Chenghua Lin and Yulan He

Written by: Chenghua Lin, University of Aberdeen, chenghua.lin@abdn.ac.uk.
Part of code is from http://gibbslda.sourceforge.net/.

This file is part of JST implementation.

JST is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

JST is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA

***********************************************************************/
   

#include "model.h"
#include "inference.h"
#include "utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <iostream>
#include <map>
using namespace std; 

void show_help();


int main(int argc, char ** argv) {

	int model_status = MODEL_STATUS_UNKNOWN;
	utils *putils = new utils();
	model_status = putils->parse_args(argc, argv, model_status);
	
	if (putils)
		delete putils;

	if (model_status == MODEL_STATUS_UNKNOWN) {
		printf("Please specify the task you would like to perform, training (-est) or inference (-inf)!\n");
		show_help();
		return 1;
	}
	else if (model_status == MODEL_STATUS_EST){
		model jst;
	
		if (jst.init(argc, argv)) {
			show_help();
			return 1;
		}
		
		if(jst.excute_model()) return 1;
	}
	else if (model_status == MODEL_STATUS_INF) {
		Inference jst;
		
		if (jst.init(argc, argv)) {
			show_help();
			return 1;
		}
	}

	return 0;
}


void show_help() {

	printf("Command line usage:\n");
	printf("jst -est|-inf [options]\n");
	printf("-est \t Estimate the DJST model from scratch.\n");
	printf("-inf \t Perform inference on unseen (new) data using a trained model.\n");
	
	printf("\n-----------------------------------------------------------\n");
	printf("Command line opitions:\n\n");

	printf("-nsentiLabs \t The number of sentiment labels. The default is 3.\n");
	printf("-ntopics \t The number of topics. The default is 50.\n");
	printf("-niters \t The number of Gibbs sampling iterations. The default is 1000.\n");
	printf("-savestep \t The step (counted by the number of Gibbs sampling iterations) at which the model is saved to hard disk. The default is 200.\n");
	printf("-updateParaStep The step (counted by the number of Gibbs sampling iterations) at which the hyperparameters are updated. The default is 40.\n");
	printf("-twords \t The number of most likely words to be printed for each topic. The default is 20.\n");
	printf("-data_dir \t The directory where the input training data is stored.\n");
	printf("-result_dir \t The directory where the output models and parameters will be stored.\n");
	printf("-datasetFile \t The input training data file.\n");
	printf("-sentiFile \t The sentiment lexicon file.\n");
	printf("-vocab \t\t The vocabulary file.\n");
	printf("-alpha \t\t The hyperparameter of the per-document sentiment specific topic proportion. The default is avgDocLength*0.05/(numSentiLabs*numTopics).\n");
	printf("-beta \t\t The hyperparameter of the per-corpus sentiment specific topic-word distribution. The default is 0.01.\n");
	printf("-gamma \t\t The hyperparameter of the per-document sentiment proportion. The default is avgDocLength*0.05/numSentiLabs.\n");
	printf("-model_dir \t\t The directory of the previously trained model. (for inference only).\n");
	printf("-model \t\t The name of the previously trained model. (for inference only).\n");
}


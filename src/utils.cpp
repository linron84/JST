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

#include <stdio.h>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include "strtokenizer.h"
#include "utils.h"
#include "model.h"
#include "inference.h"
#include "dataset.h"
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

#undef WINDOWS
#ifdef _WIN32
    #define WINDOWS
#endif
#ifdef __WIN32__
    #define WINDOWS
#endif

#ifdef WINDOWS
	#include <direct.h>  // For _mkdir().
	#include <io.h>      // For access().
#else 
	#include <unistd.h>  // For access().
#endif


utils::utils() {
	model_status = MODEL_STATUS_UNKNOWN;
	model_dir = "";
	data_dir = "";
	result_dir = "";
    model_name = "";
	configfile = "";

	wordmapfile = "";
	sentiLexFile = "";
    datasetFile = "";
    configfile = "";
    numSentiLabs = 0;
	numTopics = 0;
    niters = 0;
    savestep = 0;
    twords = 0;
	updateParaStep = -1; 

	alpha = -1.0;
	beta = -1.0;
    gamma = -1.0;
}


int utils::parse_args(int argc, char ** argv, int&  model_status) {
    int i = 1;
    while (i < argc) {
		string arg = argv[i];
		if (arg == "-est") {
			model_status = MODEL_STATUS_EST;
			break;
		}
		else if (arg == "-estc") {
			model_status = MODEL_STATUS_ESTC;
			break;
		}
		else if (arg == "-inf") {
			model_status = MODEL_STATUS_INF;
			break;
		}
		i++;
	}

    this->model_status = model_status;
    cout << "model_status = " << this->model_status<< endl;
	return (model_status);
}



int utils::parse_args_est(int argc, char ** argv, model * pmodel) {

    int i = 1;
    while (i < argc) {
	    string arg = argv[i];
		if (arg == "-config") {
			configfile = argv[++i];
			break;
		}
		i++;
	}

	if (configfile != "") {
		if (read_config_file(configfile)) {
			return 1;
		}
	}
	
	if (wordmapfile != "")   
		pmodel->wordmapfile = wordmapfile;
			
	if (sentiLexFile != "")
		pmodel->sentiLexFile = sentiLexFile;
	
	if (datasetFile != "") {
		pmodel->datasetFile = datasetFile;
	}

	if (numSentiLabs > 0) pmodel->numSentiLabs = numSentiLabs;
	if (numTopics > 0) pmodel->numTopics = numTopics;
	if (niters > 0)  pmodel->niters = niters;
	if (savestep > 0) pmodel->savestep = savestep;
	if (twords > 0)   pmodel->twords = twords;
	pmodel->updateParaStep = updateParaStep; // -1: no parameter optimization

	if (alpha > 0.0) pmodel->_alpha = alpha;
	if (beta > 0.0) pmodel->_beta = beta;
	if (gamma > 0.0) pmodel->_gamma = gamma;

	if (data_dir != "")	{
		if (data_dir[data_dir.size() - 1] != '/') {
			data_dir += "/";
		}
		pmodel->data_dir = data_dir;
	}
	else {
		printf("Please specify input data dir!\n");
		return 1;
	}
	
	if (result_dir != "")	{
	    if (make_dir(result_dir)) return 1;
	    if (result_dir[result_dir.size() - 1] != '/') {
		    result_dir += "/";
	    }
		pmodel->result_dir = result_dir;
	}
	else {
		printf("Please specify output dir!\n");
		return 1;
	}

    return 0;
}
   

int utils::parse_args_inf(int argc, char ** argv, Inference * pmodel_inf) {

	int i = 1; 
	while (i < argc) {
		string arg = argv[i];
	    printf("arg=%s\n", arg.c_str());
		if (arg == "-config") {
			configfile = argv[++i];
			break;
		}
		i++;
	}
	if (configfile != "") {
		if (read_config_file(configfile)) return 1;
	}
    
	if (wordmapfile != "") 
		pmodel_inf->wordmapfile = wordmapfile;
		
	if (sentiLexFile != "") 
		pmodel_inf->sentiLexFile = sentiLexFile;
	
	if (datasetFile != "")
		pmodel_inf->datasetFile = datasetFile;
	else {
		printf("Please specify input dataset file!\n");
		return 1;
	}
	
	if (model_dir != "")	{
		if (model_dir[model_dir.size() - 1] != '/') model_dir += "/";
		pmodel_inf->model_dir = model_dir;
	}
	
	if (data_dir != "")	{
		if (data_dir[data_dir.size() - 1] != '/') data_dir += "/";
		pmodel_inf->data_dir = data_dir;
	}
	else {
		printf("Please specify input data dir!\n");
		return 1;
	}
	
	if (result_dir != "")	{
		if (make_dir(result_dir)) return 1;
		if (result_dir[result_dir.size() - 1] != '/') result_dir += "/";
		pmodel_inf->result_dir = result_dir;
	}
	else {
		printf("Please specify output dir!\n");
		return 1;
	}
	
	if (model_name != "")
		pmodel_inf->model_name = model_name;
	else {
		printf("Please specify the trained dJST model name!\n");
		return 1;
	}
    
	if (niters > 0) pmodel_inf->niters = niters;

	
	if (twords > 0) pmodel_inf->twords = twords;
	if (savestep > 0) pmodel_inf->savestep = savestep;
	if (updateParaStep > 0) pmodel_inf->updateParaStep = updateParaStep;
	if (alpha > 0.0) pmodel_inf->_alpha = alpha; 
	if (beta > 0.0) pmodel_inf->_beta = beta;
	if (gamma > 0.0) pmodel_inf->_gamma = gamma;

    return 0;
}
   

int utils::read_config_file(string filename) {

	char buff[BUFF_SIZE_SHORT];
    string line;

	FILE * fin = fopen(filename.c_str(), "r");
    if (!fin) {
		printf("Cannot read file %s\n", filename.c_str());
		return 1;
    }
    
    while (fgets(buff, BUFF_SIZE_SHORT - 1, fin)) {
			line = buff;
			strtokenizer strtok(line, "= \t\r\n");
			int count = strtok.count_tokens();
	
			// line invalid, ignore
			if (count != 2) {
			    continue;
			}
			
			string optstr = strtok.token(0);
			string optval = strtok.token(1);
			
			if(optstr == "nsentiLabs")
				numSentiLabs = atoi(optval.c_str());
			else if(optstr == "ntopics") 
				numTopics = atoi(optval.c_str());	
			else if(optstr == "niters") 
				niters = atoi(optval.c_str());	
			else if(optstr == "savestep") 
				savestep = atoi(optval.c_str());				
			else if (optstr == "updateParaStep") 
				updateParaStep = atoi(optval.c_str());
			else if(optstr == "twords") 
				twords = atoi(optval.c_str());	
			else if(optstr == "data_dir") 
				data_dir = optval;	
			else if (optstr == "model_dir") 
				model_dir = optval;	    
			else if(optstr == "result_dir") 
				result_dir = optval;	
			else if(optstr == "datasetFile") 
				datasetFile = optval;	
			else if(optstr == "sentiFile") 
				sentiLexFile = optval;	
			else if (optstr == "vocabFile") 
				wordmapfile = optval;					
			else if (optstr == "alpha")
				alpha = atof(optval.c_str());
			else if (optstr == "beta")    
				beta = atof(optval.c_str());
			else if (optstr == "gamma")    
				gamma = atof(optval.c_str());
			else if (optstr == "model")  
				model_name = optval;
		}
		
		fclose(fin);
		    
    return 0;
}


string utils::generate_model_name(int iter)  {

	string model_name;
	std::stringstream out;
	char buff[BUFF_SIZE_SHORT];
	
	sprintf(buff, "%05d", iter);

	if (iter >= 0)
		model_name = buff;
	else
		model_name = "final";
	
	return model_name;
}


#ifdef WINDOWS
int utils::make_dir(string strPath) {
	if(_access(strPath.c_str(), 0) == 0) 
		return 0;
	else if(_mkdir(strPath.c_str()) == 0) 
		return 0;
	else {
		printf("Throw exception in creating directory %s !\n",strPath.c_str());
		return 1;
	} 
}
#else
int utils::make_dir(string strPath) {
	if(access(strPath.c_str(), 0) == 0)  
		return 0;
	else if(mkdir(strPath.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
		return 0;
	else { 
		cout<<"Throw exception in creating directory "<<strPath.c_str()<<endl;
		return 1; 
	}
}
#endif

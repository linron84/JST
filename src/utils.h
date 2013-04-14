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

#ifndef _UTILS_H
#define _UTILS_H

#include "dataset.h"
#include <string>
#include <algorithm>
using namespace std;

// for sorting word probabilitys
struct sort_pred {
    bool operator()(const std::pair<int,double> &left, const std::pair<int,double> &right) {
	    return left.second > right.second;
    }
};

class model;
class Inference;

class utils {
private:
	int model_status;
    string model_dir;
	string data_dir;
	string result_dir;
    string model_name;
	string wordmapfile;
	string sentiLexFile;
    string datasetFile;
    string configfile;

    int numSentiLabs;
	int numTopics;
    int niters;
    int savestep;
    int twords;
	int updateParaStep;
	double alpha;
	double beta;
    double gamma;
	
	
public:
	utils();
		
    // parse command line arguments
    int parse_args(int argc, char ** argv, int&  model_status);
  	int parse_args_est(int argc, char ** argv, model * pmodel);
	int parse_args_inf(int argc, char ** argv, Inference * pmodel_inf);
    
    // read configuration file
	int read_config_file(string configfile);
  
    // generate the model name for the current iteration
    string generate_model_name(int iter);  

    // make directory
    int make_dir(string strPath);
    
    // sort    
    void sort(vector<double> & probs, vector<int> & words);
};

#endif


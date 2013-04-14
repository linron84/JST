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
using namespace std;


model::model(void) {

	wordmapfile = "wordmap.txt";
	tassign_suffix = ".tassign";
	pi_suffix = ".pi";
	theta_suffix = ".theta";
	phi_suffix = ".phi";
	others_suffix = ".others";
	twords_suffix = ".twords";
	
	numTopics = 50;
	numSentiLabs = 3;
	vocabSize = 0;
	numDocs = 0;
	corpusSize = 0;
	aveDocLength = 0;
	
	niters = 1000;
	liter = 0;
	savestep = 200; 
	twords = 20; 
	updateParaStep = 40;

	_alpha  = -1.0;
	_beta = -1.0;
	_gamma = -1.0;

	putils = new utils();
}


model::~model(void) {
	if (putils) delete putils;
}


int model::init(int argc, char ** argv) {

    if (putils->parse_args_est(argc, argv, this)) {
        return 1;
    }
    
	cout<<"data_dir = "<<data_dir<<endl;
	cout<<"datasetFile = "<<datasetFile<<endl;
	cout<<"result_dir = "<<result_dir<<endl;
	cout<<"sentiLexFile = "<<sentiLexFile<<endl;
	cout<<"wordmapfile = "<<wordmapfile<<endl;

	cout<<"numTopics = "<<numTopics<<endl;
	cout<<"numSentiLabs = "<<numSentiLabs<<endl;
	cout<<"niters = "<<niters<<endl;
	cout<<"savestep = "<<savestep<<endl;
	cout<<"twords = "<<twords<<endl;
	cout<<"updateParaStep = "<<updateParaStep<<endl;

	cout<<"_alpha = "<<_alpha<<endl;
	cout<<"_beta = "<<_beta<<endl;
	cout<<"_gamma = "<<_gamma<<endl;
	
	return 0;
}



int model::excute_model() {

	pdataset = new dataset(result_dir);

	if (sentiLexFile != "") {
	    if (pdataset->read_senti_lexicon((sentiLexFile).c_str())) {
		    printf("Error! Cannot read sentiFile %s!\n", (sentiLexFile).c_str());
			    delete pdataset;
				return 1;
		}
		this->sentiLex = pdataset->sentiLex;
	}
				
	// read training data
	fin.open((data_dir+datasetFile).c_str(), ifstream::in);
	if(!fin) {
	    printf("Error! Cannot read dataset %s!\n", (data_dir+datasetFile).c_str());
	    return 1;
	}
    if(pdataset->read_dataStream(fin)) {
		printf("Throw exception in function read_dataStream()! \n");
		delete pdataset;
		return 1;
	}

	word2atr = pdataset->word2atr;
	id2word =  pdataset->id2word;
	init_model_parameters();
	if (init_estimate()) return 1;
	if(estimate()) return 1;
	delete_model_parameters();
	fin.close();

	return 0;
}


int model::init_model_parameters()
{
	numDocs = pdataset->numDocs;
	vocabSize = pdataset->vocabSize;
	corpusSize = pdataset->corpusSize;
	aveDocLength = pdataset->aveDocLength;
	
	// model counts
	nd.resize(numDocs);
	for (int m = 0; m < numDocs; m++) {
		nd[m]  = 0;
	}

	ndl.resize(numDocs);
	for (int m = 0; m < numDocs; m++) {
		ndl[m].resize(numSentiLabs);
		for (int l = 0; l < numSentiLabs; l++)
		    ndl[m][l] = 0;
	}

	ndlz.resize(numDocs);
	for (int m = 0; m < numDocs; m++) {
		ndlz[m].resize(numSentiLabs);
		for (int l = 0; l < numSentiLabs; l++) {
			ndlz[m][l].resize(numTopics);
			for (int z = 0; z < numTopics; z++)
				ndlz[m][l][z] = 0; 
		}
	}

	nlzw.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		nlzw[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
			nlzw[l][z].resize(vocabSize);
			for (int r = 0; r < vocabSize; r++)
			    nlzw[l][z][r] = 0;
		}
	}

	nlz.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		nlz[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
		    nlz[l][z] = 0;
		}
	}

	// posterior P
	p.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		p[l].resize(numTopics);
	}

	// model parameters
	pi_dl.resize(numDocs);
	for (int m = 0; m < numDocs; m++) {
		pi_dl[m].resize(numSentiLabs);
	}

	theta_dlz.resize(numDocs);
	for (int m = 0; m < numDocs; m++) {
		theta_dlz[m].resize(numSentiLabs);
		for (int l = 0; l < numSentiLabs; l++) {
			theta_dlz[m][l].resize(numTopics);
		}
	}

	phi_lzw.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		phi_lzw[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
			phi_lzw[l][z].resize(vocabSize);
		}
	}

	// init hyperparameters
	alpha_lz.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		alpha_lz[l].resize(numTopics);
	}

	alphaSum_l.resize(numSentiLabs);
	
	if (_alpha <= 0) {
		_alpha =  (double)aveDocLength * 0.05 / (double)(numSentiLabs * numTopics);
	}

	for (int l = 0; l < numSentiLabs; l++) {
		alphaSum_l[l] = 0.0;
	    for (int z = 0; z < numTopics; z++) {
		    alpha_lz[l][z] = _alpha;
		    alphaSum_l[l] += alpha_lz[l][z];
	    }
	}

	opt_alpha_lz.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		opt_alpha_lz[l].resize(numTopics);
	}

	//beta
	if (_beta <= 0) _beta = 0.01;

	beta_lzw.resize(numSentiLabs);
	betaSum_lz.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		beta_lzw[l].resize(numTopics);
		betaSum_lz[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
			betaSum_lz[l][z] = 0.0;
			beta_lzw[l][z].resize(vocabSize);
			for (int r = 0; r < vocabSize; r++) {
				beta_lzw[l][z][r] = _beta;
			}
		} 		
	}

	// word prior transformation matrix
	lambda_lw.resize(numSentiLabs); 
	for (int l = 0; l < numSentiLabs; l++) {
	    lambda_lw[l].resize(vocabSize);
		for (int r = 0; r < vocabSize; r++) {
			lambda_lw[l][r] = 1; 	
		}
	}

	// incorporate prior information into beta
	this->prior2beta();
	this->set_gamma();

	return 0;
}


int model::set_gamma() {

	mapname2labs::iterator it;

	if (_gamma <= 0 ) {
		_gamma = (double)aveDocLength * 0.05 / (double)numSentiLabs;
	}

	gamma_dl.resize(numDocs);
	gammaSum_d.resize(numDocs);

	for (int d = 0; d < numDocs; d++) {
		gamma_dl[d].resize(numSentiLabs);
		gammaSum_d[d] = 0.0;
		for (int l = 0; l < numSentiLabs; l++) {
			gamma_dl[d][l] = _gamma;
			gammaSum_d[d] += gamma_dl[d][l];
		}
	}

	return 0;
}


int model::prior2beta() {

	mapword2atr::iterator wordIt;
	mapword2prior::iterator sentiIt;
	
	for (sentiIt = sentiLex.begin(); sentiIt != sentiLex.end(); sentiIt++) {
		wordIt = word2atr.find(sentiIt->first);
		if (wordIt != word2atr.end()) {
			for (int j = 0; j < numSentiLabs; j++)  {
				lambda_lw[j][wordIt->second.id] = sentiIt->second.labDist[j];
			}
		}
	}
	
	for (int l = 0; l < numSentiLabs; l++) {
		for (int z = 0; z < numTopics; z++) {
			betaSum_lz[l][z] = 0.0;
		    for (int r = 0; r < vocabSize; r++) {
			    beta_lzw[l][z][r] = beta_lzw[l][z][r] * lambda_lw[l][r];  
			    betaSum_lz[l][z] += beta_lzw[l][z][r];
		    }
		}
	}

	return 0;
}


void model::compute_phi_lzw() {

	for (int l = 0; l < numSentiLabs; l++)  {
	    for (int z = 0; z < numTopics; z++) {
			for(int r = 0; r < vocabSize; r++) {
				phi_lzw[l][z][r] = (nlzw[l][z][r] + beta_lzw[l][z][r]) / (nlz[l][z] + betaSum_lz[l][z]);
			}
		}
	}
}



void model::compute_pi_dl() {

	for (int m = 0; m < numDocs; m++) {
	    for (int l = 0; l < numSentiLabs; l++) {
		    pi_dl[m][l] = (ndl[m][l] + gamma_dl[m][l]) / (nd[m] + gammaSum_d[m]);
		}
	}
}

void model::compute_theta_dlz() {

	for (int m = 0; m < numDocs; m++) {
	    for (int l = 0; l < numSentiLabs; l++)  {
			for (int z = 0; z < numTopics; z++) {
			    theta_dlz[m][l][z] = (ndlz[m][l][z] + alpha_lz[l][z]) / (ndl[m][l] + alphaSum_l[l]);    
			}
		}
	}
}


int model::save_model(string model_name) {

	if (save_model_tassign(result_dir + model_name + tassign_suffix)) 
		return 1;
	
	if (save_model_twords(result_dir + model_name + twords_suffix)) 
		return 1;

	if (save_model_pi_dl(result_dir + model_name + pi_suffix)) 
		return 1;

	if (save_model_theta_dlz(result_dir + model_name + theta_suffix)) 
		return 1;

	if (save_model_phi_lzw(result_dir + model_name + phi_suffix))
		return 1;

	if (save_model_others(result_dir + model_name + others_suffix)) 
		return 1;

	return 0;
}


int model::save_model_tassign(string filename) {
    
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
	    printf("Cannot save file %s!\n", filename.c_str());
	    return 1;
    }

	for (int m = 0; m < pdataset->numDocs; m++) {    
		fprintf(fout, "%s \n", pdataset->pdocs[m]->docID.c_str());
		for (int n = 0; n < pdataset->pdocs[m]->length; n++) {
	        fprintf(fout, "%d:%d:%d ", pdataset->pdocs[m]->words[n], l[m][n], z[m][n]); //  wordID:sentiLab:topic
	    }
	    fprintf(fout, "\n");
    }

    fclose(fout);
	return 0;
}


int model::save_model_twords(string filename) 
{   
    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
	    printf("Cannot save file %s!\n", filename.c_str());
	    return 1;
    }
    
    if (twords > vocabSize) {
	    twords = vocabSize; // print out entire vocab list
    }
    
    mapid2word::iterator it;
   
    for (int l = 0; l < numSentiLabs; l++) { 
        for (int k = 0; k < numTopics; k++) { 
	        vector<pair<int, double> > words_probs;
	        pair<int, double> word_prob;
	        for (int w = 0; w < vocabSize; w++) { 
		        word_prob.first = w; // w: word id/index
	            word_prob.second = phi_lzw[l][k][w]; // topic-word probability
	            words_probs.push_back(word_prob);
	        }
    
		    std::sort(words_probs.begin(), words_probs.end(), sort_pred());

	        fprintf(fout, "Label%d_Topic%d\n", l, k);
	        for (int i = 0; i < twords; i++) { 
		        it = id2word.find(words_probs[i].first);
	            if (it != id2word.end()) 
			        fprintf(fout, "%s   %15f\n", (it->second).c_str(), words_probs[i].second);
	        }
	    }
    }
     
    fclose(fout);      
    return 0;    
}



int model::save_model_pi_dl(string filename) {

	FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
		printf("Cannot save file %s!\n", filename.c_str());
		return 1;
    }

	for (int m = 0; m < numDocs; m++) { 
		fprintf(fout, "d_%d %s ", m, pdataset->pdocs[m]->docID.c_str());
		for (int l = 0; l < numSentiLabs; l++) {
			fprintf(fout, "%f ", pi_dl[m][l]);
		}
		fprintf(fout, "\n");
    }
   
    fclose(fout);       
	return 0;
}


int model::save_model_theta_dlz(string filename) {

	FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
		printf("Cannot save file %s!\n", filename.c_str());
		return 1;
    }
    
    for(int m = 0; m < numDocs; m++) {
        fprintf(fout, "Document %d\n", m);
	    for (int l = 0; l < numSentiLabs; l++) {
	        for (int z = 0; z < numTopics; z++) {
		        fprintf(fout, "%f ", theta_dlz[m][l][z]);
	        }
		    fprintf(fout, "\n");
		 }
    }

    fclose(fout);
    return 0;
}


int model::save_model_phi_lzw(string filename) {

	FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
	    printf("Cannot save file %s!\n", filename.c_str());
	    return 1;
    }
    
	for (int l = 0; l < numSentiLabs; l++) {  
	    for (int z = 0; z < numTopics; z++) { 
		    fprintf(fout, "Label:%d  Topic:%d\n", l, z);
     	    for (int r = 0; r < vocabSize; r++) {
			    fprintf(fout, "%.15f ", phi_lzw[l][z][r]);
     	    }
            fprintf(fout, "\n");
	   }
    }
    
    fclose(fout);
	return 0;
}



int model::save_model_others(string filename) {

	FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
	    printf("Cannot save file %s!\n", filename.c_str());
	    return 1;
    }
    
	fprintf(fout, "data_dir=%s\n", this->data_dir.c_str());
	fprintf(fout, "datasetFile=%s\n", this->datasetFile.c_str());
	fprintf(fout, "result_dir=%s\n", this->result_dir.c_str());
	fprintf(fout, "sentiLexFile=%s\n", this->sentiLexFile.c_str());

	fprintf(fout, "\n-------------------- Corpus statistics -----------------------\n");
    fprintf(fout, "numDocs=%d\n", numDocs);
    fprintf(fout, "corpusSize=%d\n", corpusSize);
	fprintf(fout, "aveDocLength=%d\n", aveDocLength);
    fprintf(fout, "vocabSize=%d\n", vocabSize);

    fprintf(fout, "\n---------------------- Model settings -----------------------\n");
	fprintf(fout, "numSentiLabs=%d\n", numSentiLabs);
	fprintf(fout, "numTopics=%d\n", numTopics);
	fprintf(fout, "liter=%d\n", liter);
	fprintf(fout, "savestep=%d\n", savestep);
	fprintf(fout, "updateParaStep=%d\n", updateParaStep);

	fprintf(fout, "_alpha=%f\n", _alpha);
	fprintf(fout, "_beta=%f\n", _beta);
	fprintf(fout, "_gamma=%f\n", _gamma);

	fclose(fout);
    return 0;
}


int model::init_estimate() {

    int sentiLab, topic;
	srand(time(0)); // initialize for random number generation
	z.resize(numDocs);
	l.resize(numDocs);

	for (int m = 0; m < numDocs; m++) {
		int docLength = pdataset->pdocs[m]->length;
		z[m].resize(docLength);
		l[m].resize(docLength);

        for (int t = 0; t < docLength; t++) {
		    if (pdataset->pdocs[m]->words[t] < 0) {
			    printf("ERROE! word token %d has index smaller than 0 at doc[%d][%d]\n", pdataset->pdocs[m]->words[t], m, t);
				return 1;
			}

    	    if ((pdataset->pdocs[m]->priorSentiLabels[t] > -1) && (pdataset->pdocs[m]->priorSentiLabels[t] < numSentiLabs)) {
			    sentiLab = pdataset->pdocs[m]->priorSentiLabels[t]; // incorporate prior information into the model

			}
			else {
			    sentiLab = (int)(((double)rand() / RAND_MAX) * numSentiLabs);
			    if (sentiLab == numSentiLabs) sentiLab = numSentiLabs -1;  // to avoid over array boundary
			}
    	    l[m][t] = sentiLab;

			// random initialize the topic assginment
			topic = (int)(((double)rand() / RAND_MAX) * numTopics);
			if (topic == numTopics)  topic = numTopics - 1; // to avoid over array boundary
			z[m][t] = topic;

			// model count assignments
			nd[m]++;
			ndl[m][sentiLab]++;
			ndlz[m][sentiLab][topic]++;
			nlzw[sentiLab][topic][pdataset->pdocs[m]->words[t]]++;
			nlz[sentiLab][topic]++;
        }
    }

    return 0;
}



int model::estimate() {

	int sentiLab, topic;
	mapname2labs::iterator it;

	printf("Sampling %d iterations!\n", niters);
	for (liter = 1; liter <= niters; liter++) {
	    printf("Iteration %d ...\n", liter);
		for (int m = 0; m < numDocs; m++) {
		    for (int n = 0; n < pdataset->pdocs[m]->length; n++) {
				sampling(m, n, sentiLab, topic);
				l[m][n] = sentiLab;
				z[m][n] = topic;
			}
		}
		
		if (updateParaStep > 0 && liter % updateParaStep == 0) {
			this->update_Parameters();
		}
		
		if (savestep > 0 && liter % savestep == 0) {
			if (liter == niters) break;

			printf("Saving the model at iteration %d ...\n", liter);
			compute_pi_dl();
			compute_theta_dlz();
			compute_phi_lzw();
			save_model(putils->generate_model_name(liter));
		}
	}
	
	printf("Gibbs sampling completed!\n");
	printf("Saving the final model!\n");
	compute_pi_dl();
	compute_theta_dlz();
	compute_phi_lzw();
	save_model(putils->generate_model_name(-1));
	
	return 0;
}


int model::sampling(int m, int n, int& sentiLab, int& topic) {

	sentiLab = l[m][n];
	topic = z[m][n];
	int w = pdataset->pdocs[m]->words[n]; // the ID/index of the current word token in vocabulary 
	double u;
	
	nd[m]--;
	ndl[m][sentiLab]--;
	ndlz[m][sentiLab][topic]--;
	nlzw[sentiLab][topic][pdataset->pdocs[m]->words[n]]--;
	nlz[sentiLab][topic]--;

	// do multinomial sampling via cumulative method
	for (int l = 0; l < numSentiLabs; l++) {
		for (int k = 0; k < numTopics; k++) {
			p[l][k] = (nlzw[l][k][w] + beta_lzw[l][k][w]) / (nlz[l][k] + betaSum_lz[l][k]) *
		   		(ndlz[m][l][k] + alpha_lz[l][k]) / (ndl[m][l] + alphaSum_l[l]) *
				(ndl[m][l] + gamma_dl[m][l]) / (nd[m] + gammaSum_d[m]);
		}
	}
	
	// accumulate multinomial parameters
	for (int l = 0; l < numSentiLabs; l++)  {
		for (int k = 0; k < numTopics; k++) {
			if (k==0)  {
			    if (l==0) continue;
		        else p[l][k] += p[l-1][numTopics-1]; // accumulate the sum of the previous array
			}
			else p[l][k] += p[l][k-1];
		}
	}

	// probability normalization
	u = ((double)rand() / RAND_MAX) * p[numSentiLabs-1][numTopics-1];

	// sample sentiment label l, where l \in [0, S-1]
	bool loopBreak=false;
	for (sentiLab = 0; sentiLab < numSentiLabs; sentiLab++) {   
		for (topic = 0; topic < numTopics; topic++) { 
		    if (p[sentiLab][topic] > u) {
		        loopBreak = true;
		        break;
		    }
		}
		if (loopBreak == true) {
			break;
		}
	}
    
	if (sentiLab == numSentiLabs) sentiLab = numSentiLabs - 1; // to avoid over array boundary
	if (topic == numTopics) topic = numTopics - 1;

	// add estimated 'z' and 'l' to count variables
	nd[m]++;
	ndl[m][sentiLab]++;
	ndlz[m][sentiLab][topic]++;
	nlzw[sentiLab][topic][pdataset->pdocs[m]->words[n]]++;
	nlz[sentiLab][topic]++;

    return 0;
}


int model::update_Parameters() {

	int ** data; // temp valuable for exporting 3-dimentional array to 2-dimentional
	double * alpha_temp;
	data = new int*[numTopics];
	for (int k = 0; k < numTopics; k++) {
		data[k] = new int[numDocs];
		for (int m = 0; m < numDocs; m++) {
			data[k][m] = 0;
		}
	}

	alpha_temp = new double[numTopics];
	for (int k = 0; k < numTopics; k++){
		alpha_temp[k] = 0.0;
	}

	// update alpha
	for (int j = 0; j < numSentiLabs; j++) {
		for (int k = 0; k < numTopics; k++) {
			for (int m = 0; m < numDocs; m++) {
				data[k][m] = ndlz[m][j][k]; // ntldsum[j][k][m];
			}
		}

		for (int k = 0; k < numTopics; k++) {
			alpha_temp[k] =  alpha_lz[j][k]; //alpha[j][k];
		}

		polya_fit_simple(data, alpha_temp, numTopics, numDocs);

		// update alpha
		alphaSum_l[j] = 0.0;
		for (int k = 0; k < numTopics; k++) {
			alpha_lz[j][k] = alpha_temp[k];
			alphaSum_l[j] += alpha_lz[j][k];
		}
	}
	
	return 0;
}

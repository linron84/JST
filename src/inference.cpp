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
   
#include "inference.h"
using namespace std;

Inference::Inference(void) {

    numSentiLabs = 0; 
	numTopics = 0;
	numDocs = 0; 
	vocabSize = 0;
	newNumDocs = 0;
	newVocabSize = 0;
	_beta = -1.0;
	
	wordmapfile = "wordmap.txt";
    tassign_suffix = ".newtassign";
    pi_suffix = ".newpi";
    theta_suffix = ".newtheta";
    phi_suffix = ".newphi";
    others_suffix = ".newothers";
    twords_suffix = ".newtwords";
	model_name = "";
	data_dir = "";
	datasetFile = "";
	result_dir = "";
	sentiLexFile = "";

	updateParaStep = -1;
	savestep = 20;
	twords = 20;
	niters = 40;
	
	putils = new utils();
	pmodelData = NULL;
	pnewData = NULL;
}


Inference::~Inference(void) {

	if (putils)
		delete putils;
		
	if (pmodelData)
		delete pmodelData;
	
	if (pnewData)
		delete pnewData;
}


int Inference::init(int argc, char ** argv) {

    if (putils->parse_args_inf(argc, argv, this)) {
	    return 1;
    }

	if(init_inf()) {
	    printf("Throw expectation in init_inf()!  \n");
		return 1; 
	}

	if(inference()) {
	    printf("Throw expectation in inference()!  \n");
		return 1; 
	}

    return 0;
}


// read '.others' file
int Inference::read_model_setting(string filename) {

	char buff[BUFF_SIZE_LONG];
	string line;
	numSentiLabs = 0;
	numTopics = 0;
	numDocs = 0;
	vocabSize = 0;

	FILE * fin = fopen(filename.c_str(), "r");
	if (!fin) {
        printf("Cannot read file %s!\n", filename.c_str());
        return 1;
	}
    
	while (fgets(buff, BUFF_SIZE_LONG - 1, fin) != NULL) {
		line = buff; 
		strtokenizer values(line, ": \t\r\n={}[]"); // \t\r\n are separators

		if (values.token(0) == "numSentiLabs") {
			numSentiLabs = atoi(values.token(1).c_str());
		}
		else if (values.token(0) == "numTopics") {
			numTopics = atoi(values.token(1).c_str());
		}
		else if (values.token(0) == "numDocs") {
			numDocs = atoi(values.token(1).c_str());
		}
		else if (values.token(0) == "vocabSize") {
			vocabSize = atoi(values.token(1).c_str());
		}
		if (numSentiLabs > 0 && numTopics > 0 && numDocs > 0 && vocabSize > 0) {
			break;
		}
	}

	fclose(fin);
	
	if (numSentiLabs == 0 || numTopics == 0 || numDocs == 0 || vocabSize == 0) {
		cout << "Throw exception in reading model parameter settings!\n" << filename << endl;
		return 1;
	}
	else {
		cout<<"data_dir = "<<data_dir<<endl;
		cout<<"datasetFile = "<<datasetFile<<endl;
		cout<<"result_dir = "<<result_dir<<endl;
		cout<<"sentiLexFile = "<<sentiLexFile<<endl;
		cout<<"model_dir = "<<model_dir<<endl;
		cout<<"model_name = "<<model_name<<endl;
		cout<<"wordmapfile = "<<wordmapfile<<endl;
		cout<<"numTopics = "<<numTopics<<endl;
		cout<<"numSentiLabs = "<<numSentiLabs<<endl;
		cout<<"niters = "<<niters<<endl;
		cout<<"savestep = "<<savestep<<endl;
		cout<<"twords = "<<twords<<endl;
		cout<<"updateParaStep = "<<updateParaStep<<endl;
	}


	return 0;
}


// read '.tassign' file of previously trained model
int Inference::load_model(string filename) {

    char buff[BUFF_SIZE_LONG];
	string line;
    
    FILE * fin = fopen(filename.c_str(), "r");
    if (!fin) {
	    printf("Cannot read file %s!\n", filename.c_str());
	    return 1;
    }

	pmodelData->pdocs = new document*[numDocs];
	pmodelData->vocabSize= vocabSize;
	pmodelData->numDocs= numDocs;
	l.resize(pmodelData->numDocs);
	z.resize(pmodelData->numDocs);

    for (int m = 0; m < numDocs; m++) {
		fgets(buff, BUFF_SIZE_LONG - 1, fin);  // first line - ignore the document ID
		fgets(buff, BUFF_SIZE_LONG - 1, fin);  // second line - read the sentiment label / topic assignments
		line = buff; 
	    strtokenizer strtok(line, " \t\r\n");
	    int length = strtok.count_tokens();
	
	    vector<int> words;
		vector<int> sentiLabs;
	    vector<int> topics;

	    for (int j = 0; j < length; j++) {
	        string token = strtok.token(j);
	        strtokenizer tok(token, ":");
	        if (tok.count_tokens() != 3) {
		        printf("Invalid word-sentiment-topic assignment format!\n");
		        return 1;
	        }
	    
	        words.push_back(atoi(tok.token(0).c_str()));
			sentiLabs.push_back(atoi(tok.token(1).c_str()));
	        topics.push_back(atoi(tok.token(2).c_str()));
	    }
	
		// allocate and add training document to the corpus
		document * pdoc = new document(words);
		pmodelData->add_doc(pdoc, m);

		l[m].resize(sentiLabs.size());
		for (int j = 0; j < (int)sentiLabs.size(); j++) {
			l[m][j] = sentiLabs[j];
		}

		z[m].resize(topics.size());
		for (int j = 0; j < (int)topics.size(); j++) {
			z[m][j] = topics[j];
		}
	}
    fclose(fin);
    
	// init model counts
	nlzw.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		nlzw[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
			nlzw[l][z].resize(vocabSize);
			for (int r = 0; r < vocabSize; r++) {
			    nlzw[l][z][r] = 0;
			}
		}
	}

	nlz.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		nlz[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
            nlz[l][z] = 0;
		}
	}

	// recover count values from trained model
	for (int m = 0; m < pmodelData->numDocs; m++) {
		int docLength = pmodelData->pdocs[m]->length;
		for (int n = 0; n < docLength; n++) {
			int w = pmodelData->pdocs[m]->words[n];
			int sentiLab = this->l[m][n];
			int topic = this->z[m][n];

			nlzw[sentiLab][topic][w]++;
			nlz[sentiLab][topic]++;
		}
	}
	
    return 0;
}



int Inference::init_inf() {

	pmodelData = new dataset();
	pnewData = new dataset(result_dir);

	if(read_model_setting(model_dir + model_name + ".others")) {
	    printf("Throw exception in read_para_setting()!\n");
		return 1; 
	}

	// load model
	if(load_model(model_dir + model_name + ".tassign")) {
	    printf("Throw exception in load_model()!\n");
		return 1; 
	}

	// *** TODO move the function to dataset class
	if(read_newData(data_dir + datasetFile)) {
	    printf("Throw exception in read_newData()!\n");
		return 1; 
	}

	if(init_parameters()) {
	    printf("Throw exception in init_parameters!\n");
		return 1; 
	}

	printf("Testset statistics: \n");
	printf("numDocs = %d\n", pnewData->numDocs);
	printf("vocabSize = %d\n", pnewData->vocabSize);
	printf("numNew_word = %d\n", (int)(pnewData->newWords.size()));

	// init inf
	int sentiLab, topic; 
	new_z.resize(pnewData->numDocs);
	new_l.resize(pnewData->numDocs);

	for (int m = 0; m < pnewData->numDocs; m++) {
		int docLength = pnewData->_pdocs[m]->length;
		new_z[m].resize(docLength);
		new_l[m].resize(docLength);
		for (int t = 0; t < docLength; t++) {
		    if (pnewData->_pdocs[m]->words[t] < 0) {
			    printf("ERROE! word token %d has index smaller than 0 in doc[%d][%d]\n", pnewData->_pdocs[m]->words[t], m, t);
				return 1;
			}

			// sample sentiment label
		    if ((pnewData->pdocs[m]->priorSentiLabels[t] > -1) && (pnewData->pdocs[m]->priorSentiLabels[t] < numSentiLabs)) {
			    sentiLab = pnewData->pdocs[m]->priorSentiLabels[t]; // incorporate prior information into the model  
			}
			else {
			    sentiLab = (int)(((double)rand() / RAND_MAX) * numSentiLabs);
			    if (sentiLab == numSentiLabs) sentiLab = numSentiLabs -1;
			}
		    new_l[m][t] = sentiLab;

			// sample topic label 
			topic = (int)(((double)rand() / RAND_MAX) * numTopics);
			if (topic == numTopics)  topic = numTopics - 1;
			new_z[m][t] = topic;

			new_nd[m]++;
			new_ndl[m][sentiLab]++;
			new_ndlz[m][sentiLab][topic]++;
			new_nlzw[sentiLab][topic][pnewData->_pdocs[m]->words[t]]++;
			new_nlz[sentiLab][topic]++;
       } 
	}
	
	return 0;
}


int Inference::inference() {

	int sentiLab, topic;
	printf("Sampling %d iterations for inference!\n", niters);

	liter = 0; 
	for (liter = 1; liter <= niters; liter++) {
		printf("Iteration %d ...\n", liter);
		for (int m = 0; m < pnewData->numDocs; m++) {
			for (int n = 0; n < pnewData->pdocs[m]->length; n++) {
				inf_sampling(m, n, sentiLab, topic);
				new_l[m][n] = sentiLab; 
				new_z[m][n] = topic; 
			} 
		}
		
		if (savestep > 0 && liter % savestep == 0) {
			if (liter == niters) break;
				
			printf("Saving the model at iteration %d ...\n", liter);
			compute_newpi();
			compute_newtheta();
			compute_newphi();
			save_model(model_name + "_" + putils->generate_model_name(liter));
		}
	}
    
	printf("Gibbs sampling completed!\n");
	printf("Saving the final model!\n");
	compute_newpi();
	compute_newtheta();
	compute_newphi();
	save_model(model_name + "_" + putils->generate_model_name(-1));

	return 0;
}


int Inference::init_parameters() {

	// model counts
	new_p.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) 	{
		new_p[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
		    new_p[l][z] = 0.0;
		}
	}

	new_nd.resize(pnewData->numDocs); 
	for (int m = 0; m < pnewData->numDocs; m++) {
	    new_nd[m] = 0;
	}

	new_ndl.resize(pnewData->numDocs);
	for (int m = 0; m < pnewData->numDocs; m++) {
		new_ndl[m].resize(numSentiLabs);
		for (int l = 0; l < numSentiLabs; l++) {
		    new_ndl[m][l] = 0;
		}
	}

	new_ndlz.resize(pnewData->numDocs);
	for (int m = 0; m < pnewData->numDocs; m++) {
		new_ndlz[m].resize(numSentiLabs);
	    for (int l = 0; l < numSentiLabs; l++)	{
			new_ndlz[m][l].resize(numTopics);
			for (int z = 0; z < numTopics; z++) {
			    new_ndlz[m][l][z] = 0; 
			}
		}
	}

	new_nlzw.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		new_nlzw[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
			new_nlzw[l][z].resize(pnewData->vocabSize);
			for (int r = 0; r < pnewData->vocabSize; r++) {
			    new_nlzw[l][z][r] = 0;
			}
		}
	}

	new_nlz.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		new_nlz[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
		    new_nlz[l][z] = 0;
		}
	}

	// model parameters
	newpi_dl.resize(pnewData->numDocs);
	for (int m = 0; m < pnewData->numDocs; m++) {
		newpi_dl[m].resize(numSentiLabs);
	}

	newtheta_dlz.resize(pnewData->numDocs);
	for (int m = 0; m < pnewData->numDocs; m++) {
		newtheta_dlz[m].resize(numSentiLabs);
		for (int l = 0; l < numSentiLabs; l++) {
			newtheta_dlz[m][l].resize(numTopics);
		}
	}

	newphi_lzw.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		newphi_lzw[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
			newphi_lzw[l][z].resize(pnewData->vocabSize);
		}
	}

	// hyperparameters
	_alpha =  (double)pnewData->aveDocLength * 0.05 / (double)(numSentiLabs * numTopics);
	alpha_lz.resize(numSentiLabs);
	alphaSum_l.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		alphaSum_l[l] = 0.0;
		alpha_lz[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
			alpha_lz[l][z] = _alpha;
			alphaSum_l[l] += alpha_lz[l][z];
		}
	}

	// gamma
	gamma_l.resize(numSentiLabs);
	gammaSum = 0.0;
	for (int l = 0; l < numSentiLabs; l++) {
		gamma_l[l] = (double)pnewData->aveDocLength * 0.05 / (double)numSentiLabs;
		gammaSum += gamma_l[l];
	}

	//beta
	if (_beta <= 0) {
		_beta = 0.01;
	}
	beta_lzw.resize(numSentiLabs);
	betaSum_lz.resize(numSentiLabs);
	for (int l = 0; l < numSentiLabs; l++) {
		beta_lzw[l].resize(numTopics);
		betaSum_lz[l].resize(numTopics);
		for (int z = 0; z < numTopics; z++) {
			beta_lzw[l][z].resize(pnewData->vocabSize);
			for (int r = 0; r < pnewData->vocabSize; r++) {
				beta_lzw[l][z][r] = _beta; 
				betaSum_lz[l][z] += beta_lzw[l][z][r];
			}
		} 		
	}
	
	// incorporate prior knowledge into beta
	if (sentiLexFile != "") {
		// word prior transformation matrix
		lambda_lw.resize(numSentiLabs);
		for (int l = 0; l < numSentiLabs; l++) {
		  lambda_lw[l].resize(pnewData->vocabSize);
			for (int r = 0; r < pnewData->vocabSize; r++)
				lambda_lw[l][r] = 1;
		}
		// MUST init beta_lzw first before incorporating prior information into beta
		this->prior2beta();
	}

	return 0;
}



int Inference::inf_sampling(int m, int n, int& sentiLab, int& topic) {
	sentiLab = new_l[m][n];
	topic = new_z[m][n];
	int w = pnewData->pdocs[m]->words[n];   // word index of trained model
	int _w = pnewData->_pdocs[m]->words[n]; // word index of test data
	double u;
	
	new_nd[m]--;
	new_ndl[m][sentiLab]--;
	new_ndlz[m][sentiLab][topic]--;
	new_nlzw[sentiLab][topic][_w]--;
	new_nlz[sentiLab][topic]--;

    // do multinomial sampling via cumulative method
    for (int l = 0; l < numSentiLabs; l++) {
  	    for (int k = 0; k < numTopics; k++) {
		    new_p[l][k] = (nlzw[l][k][w] + new_nlzw[l][k][_w] + beta_lzw[l][k][_w]) / (nlz[l][k] + new_nlz[l][k] + betaSum_lz[l][k]) *
			    (new_ndlz[m][l][k] + alpha_lz[l][k]) / (new_ndl[m][l] + alphaSum_l[l]) *
			    (new_ndl[m][l] + gamma_l[l]) / (new_nd[m] + gammaSum);
		}
	}

	// accumulate multinomial parameters
	for (int l = 0; l < numSentiLabs; l++) {
		for (int k = 0; k < numTopics; k++) {
			if (k==0) {
			    if (l==0) continue;
		        else new_p[l][k] += new_p[l-1][numTopics-1];
			}
			else new_p[l][k] += new_p[l][k-1];
	    }
	}
	// probability normalization
	u = ((double)rand() / RAND_MAX) * new_p[numSentiLabs-1][numTopics-1];

	for (sentiLab = 0; sentiLab < numSentiLabs; sentiLab++) {
		for (topic = 0; topic < numTopics; topic++) {
		    if (new_p[sentiLab][topic] > u) {
		    	goto stop;
		    }
		}
	}
    
	stop:
	if (sentiLab == numSentiLabs) sentiLab = numSentiLabs - 1; // the max value of label is (S - 1)
	if (topic == numTopics) topic = numTopics - 1; 

	// add estimated 'z' and 'l' to counts
	new_nd[m]++;
	new_ndl[m][sentiLab]++;
	new_ndlz[m][sentiLab][topic]++;
	new_nlzw[sentiLab][topic][_w]++;
	new_nlz[sentiLab][topic]++;

    return 0;  
}


int Inference::read_newData(string filename) {

	mapword2id::iterator it;
    map<int, int>::iterator _it;
	mapword2atr::iterator itatr;
	mapword2prior::iterator sentiIt;
	string line;
	char buff[BUFF_SIZE_LONG];

	pmodelData->read_wordmap(model_dir + "wordmap.txt", word2id);  // map word2id
    pmodelData->read_wordmap(model_dir + "wordmap.txt", id2word);  // map id2word

	// read sentiment lexicon file
	if (sentiLexFile != "") {
		if (pnewData->read_senti_lexicon((sentiLexFile).c_str())) {
			printf("Error! Cannot read sentiFile %s!\n", sentiLexFile.c_str());
			delete pnewData;
			return 1;
		}
		else {
			this->sentiLex = pnewData->sentiLex;
		}
	}

    if (word2id.size() <= 0) {
	    printf("Invalid wordmap!\n");
	    return 1;
    }

    // read test data
	ifstream fin;
	fin.open(filename.c_str(), ifstream::in);
    if(!fin) {
	    printf("Cannot read file %s!\n", filename.c_str());
	    return 1;
  	}

	vector<string> docs;
	int numDocs = 0;

	while (fin.getline(buff, BUFF_SIZE_LONG)) {
		line = buff;
		if(!line.empty()) {
			docs.push_back(line);
			numDocs++;
	    }
	}
	fin.close();

	if (numDocs <= 0) {
		printf("Error! No documents found in test data %s.\n", filename.c_str());
		return 1;
	}

	pnewData->numDocs = numDocs;
	// allocate memory
    if (pnewData->pdocs) {
		pnewData->deallocate();
    }
	else {
		pnewData->pdocs = new document*[pnewData->numDocs];
	}
    pnewData->_pdocs = new document*[pnewData->numDocs];
	pnewData->vocabSize = 0;
	pnewData->corpusSize = 0;

	// process each document
	for (int i = 0; i < pnewData->numDocs; i++) {
		line = docs.at(i);
		strtokenizer strtok(line, " \t\r\n"); // \t\r\n are separators
		int docLength = strtok.count_tokens();
		if (docLength <= 0) {
			printf("Invalid (empty) document!\n");
			pnewData->deallocate();
			pnewData->numDocs = 0;
			pnewData->vocabSize = 0;
			return 1;
		}

	    pnewData->corpusSize += docLength - 1;
	    vector<int> doc;
	    vector<int> _doc;
	    vector<int> priorSentiLabels;

	    // process each token in the document
	    for (int k = 1; k < docLength; k++) {
			it = word2id.find(strtok.token(k));
			if (it == word2id.end()) {
				pnewData->newWords.push_back(strtok.token(k).c_str());
			  // word not found, i.e., word unseen in training data
			  // do anything? (future decision)
			}
			else {
				int _id;
				_it = id2_id.find(it->second);
				if (_it == id2_id.end()) {
				    _id = id2_id.size();
				    id2_id.insert(pair<int, int>(it->second, _id));
				    _id2id.insert(pair<int, int>(_id, it->second));
				}
				else {
				    _id = _it->second;
				}

				doc.push_back(it->second);
				_doc.push_back(_id);

				// 'word2atr' is specific to new/test dataset
				itatr = word2atr.find(strtok.token(k).c_str());
				int priorSenti = -1;
				if (itatr == word2atr.end()) {
					sentiIt = sentiLex.find(strtok.token(k).c_str()); // check whether the word token can be found in the sentiment lexicon
					if (sentiIt != sentiLex.end()) {
						priorSenti = sentiIt->second.id;
					}
					// encode sentiment info into word2atr
					Word_atr temp = {_id, priorSenti};  // vocabulary index; word polarity
					word2atr.insert(pair<string, Word_atr>(strtok.token(k), temp));
					priorSentiLabels.push_back(priorSenti);
				}
				else {
					priorSentiLabels.push_back(itatr->second.polarity);
				}

			}
		}

		// allocate memory for new doc
		document * pdoc = new document(doc, priorSentiLabels, "inference");
		document * _pdoc = new document(_doc, priorSentiLabels, "inference");

		pdoc->docID = strtok.token(0).c_str();
		_pdoc->docID = strtok.token(0).c_str();

		// add new doc
		pnewData->add_doc(pdoc, i);
		pnewData->_add_doc(_pdoc, i);
	}

    // update number of new words
	pnewData->vocabSize = id2_id.size();
	pnewData->aveDocLength = pnewData->corpusSize / pnewData->numDocs;
	this->newNumDocs = pnewData->numDocs;
	this->newVocabSize = pnewData->vocabSize;

    if (newVocabSize == 0) {
	    printf("ERROR! Vocabulary size of test set after removing unseen words is 0! \n");
		return 1;
	}

	return 0;
}


void Inference::compute_newpi() {

	for (int m = 0; m < pnewData->numDocs; m++) {
	    for (int l = 0; l < numSentiLabs; l++) {
		    newpi_dl[m][l] = (new_ndl[m][l] + gamma_l[l]) / (new_nd[m] + gammaSum);
	    }
	}
}


void Inference::compute_newtheta() {

	for (int m = 0; m < pnewData->numDocs; m++) {
	    for (int l = 0; l < numSentiLabs; l++)  {
			for (int z = 0; z < numTopics; z++) {
			    newtheta_dlz[m][l][z] = (new_ndlz[m][l][z] + alpha_lz[l][z]) / (new_ndl[m][l] + alphaSum_l[l]);
			}
		}
	}
}


int Inference::compute_newphi() {
	map<int, int>::iterator it;

	for (int l = 0; l < numSentiLabs; l++)  {
	    for (int z = 0; z < numTopics; z++) {
			for(int r = 0; r < pnewData->vocabSize; r++) {
			    it = _id2id.find(r);
				if (it != _id2id.end()) {
				    newphi_lzw[l][z][r] = (nlzw[l][z][it->second] + new_nlzw[l][z][r] + beta_lzw[l][z][r]) / (nlz[l][z] + new_nlz[l][z] + betaSum_lz[l][z]);
				}
				else {
				    printf("Error! Cannot find word [%d] !\n", r);
					return 1; 
				}
			}
		}
	}

	return 0;
}


int Inference::save_model(string model_name) {

	if (save_model_newtassign(result_dir + model_name + tassign_suffix))
		return 1;
	
	if (save_model_newtwords(result_dir + model_name + twords_suffix)) 
		return 1;

	if (save_model_newpi_dl(result_dir + model_name + pi_suffix)) 
		return 1;

	if (save_model_newtheta_dlz(result_dir + model_name + theta_suffix))
		return 1;

	if (save_model_newphi_lzw(result_dir + model_name + phi_suffix)) 
		return 1;

	if (save_model_newothers(result_dir + model_name + others_suffix)) 
		return 1;

	return 0;
}



int Inference::save_model_newpi_dl(string filename) {

    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
		printf("Cannot save file %s!\n", filename.c_str());
		return 1;
    }

	for (int m = 0; m < pnewData->numDocs; m++)	{
		fprintf(fout, "d_%d %s ", m, pnewData->pdocs[m]->docID.c_str());
		for (int l = 0; l < numSentiLabs; l++) {
			fprintf(fout, "%f ", newpi_dl[m][l]);
		}
		fprintf(fout, "\n");
    }
   
    fclose(fout);       
	return 0;
}



int Inference::save_model_newtheta_dlz(string filename) {

    FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
		printf("Cannot save file %s!\n", filename.c_str());
		return 1;
    }
    
    for(int m = 0; m < pnewData->numDocs; m++) {
        fprintf(fout, "Document %d\n", m);
	    for (int l = 0; l < numSentiLabs; l++) {
	        for (int z = 0; z < numTopics; z++) {
		        fprintf(fout, "%f ", newtheta_dlz[m][l][z]);
	        }
		    fprintf(fout, "\n");
		 }
    }

    fclose(fout);
	return 0;
}



int Inference::save_model_newphi_lzw(string filename) {

	FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
	    printf("Cannot save file %s!\n", filename.c_str());
	    return 1;
    }
    
	for (int l = 0; l < numSentiLabs; l++) {
	    for (int z = 0; z < numTopics; z++) {
		    fprintf(fout, "Label:%d  Topic:%d\n", l, z);
     	    for (int r = 0; r < pnewData->vocabSize; r++) {
     	    	fprintf(fout, "%.15f ", newphi_lzw[l][z][r]);
     	    }
            fprintf(fout, "\n");
	    }
    }
    
    fclose(fout);    
	return 0;
}


int Inference::save_model_newothers(string filename) {
	
	FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
	    printf("Cannot save file %s!\n", filename.c_str());
	    return 1;
    }
    
	fprintf(fout, "model_dir=%s\n", model_dir.c_str());
	fprintf(fout, "model_name=%s\n", model_name.c_str());
	fprintf(fout, "data_dir=%s\n", data_dir.c_str());
	fprintf(fout, "datasetFile=%s\n", datasetFile.c_str());
	fprintf(fout, "result_dir=%s\n", result_dir.c_str());
	fprintf(fout, "niters-inf=%d\n", niters);
	fprintf(fout, "savestep-inf=%d\n", savestep);

	fprintf(fout, "\n------------------ Testset ** %s ** statistics ----------------------\n", datasetFile.c_str());
    fprintf(fout, "newNumDocs=%d\n", pnewData->numDocs);
    fprintf(fout, "newCorpusSize=%d\n", pnewData->corpusSize);
    fprintf(fout, "newVocabSize=%d\n", pnewData->vocabSize);
	fprintf(fout, "numNewWords=%d\n", (int)(pnewData->newWords.size()));
	fprintf(fout, "aveDocLength=%d\n", pnewData->aveDocLength);
	fprintf(fout, "\n------------------ Loaded model settings ----------------------\n");
	fprintf(fout, "numSentiLabs=%d\n", numSentiLabs);
	fprintf(fout, "numTopics=%d\n", numTopics);
	fprintf(fout, "numDocs=%d\n", pmodelData->numDocs);
	fprintf(fout, "corpusSize=%d\n", pmodelData->corpusSize);
	fprintf(fout, "vocabSize=%d\n", pmodelData->vocabSize);

	fclose(fout);
	return 0;
}



int Inference::save_model_newtwords(string filename) {

	mapid2word::iterator it; // typedef map<int, string> mapid2word
	map<int, int>::iterator _it;

	FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
	    printf("Cannot save file %s!\n", filename.c_str());
	    return 1;
    }
    
    if (twords > pnewData->vocabSize) {
	    twords = pnewData->vocabSize;
    }
   
    for (int l = 0; l < numSentiLabs; l++) {
        fprintf(fout, "Label %dth\n", l);
        for (int k = 0; k < numTopics; k++) {
	        vector<pair<int, double> > words_probs;
	        pair<int, double> word_prob;
	        for (int w = 0; w < pnewData->vocabSize; w++) {
		        word_prob.first = w;
	            word_prob.second = newphi_lzw[l][k][w];
	            words_probs.push_back(word_prob);
	        }
    
		    std::sort(words_probs.begin(), words_probs.end(), sort_pred());

	        fprintf(fout, "Topic %dth:\n", k);
	        for (int i = 0; i < twords; i++) {
				_it = _id2id.find(words_probs[i].first);
				if (_it == _id2id.end()) {
		            continue;
	            }
				it = id2word.find(_it->second);
	            if (it != id2word.end()) {
			        fprintf(fout, "\t%s   %f\n", (it->second).c_str(), words_probs[i].second);
	            } 
	        }
	    } // for topic
    } // for label
     
    fclose(fout);      
	return 0;
}


int Inference::save_model_newtassign(string filename) {

	FILE * fout = fopen(filename.c_str(), "w");
    if (!fout) {
	    printf("Cannot save file %s!\n", filename.c_str());
	    return 1;
    }

	for (int m = 0; m < pnewData->numDocs; m++) {
		fprintf(fout, "%s \n", pnewData->pdocs[m]->docID.c_str());
		for (int n = 0; n < pnewData->pdocs[m]->length; n++) {
	        fprintf(fout, "%d:%d:%d ", pnewData->pdocs[m]->words[n], new_l[m][n], new_z[m][n]); //  wordID:sentiLab:topic
	    }
	    fprintf(fout, "\n");
    }

    fclose(fout);
	return 0;
}



int Inference::prior2beta() {
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

	// Note: the 'r' index of lambda[j][r] is corresponding to the vocabulary ID.
	// Therefore the correct prior info can be incorporated to corresponding word cound nlzw,
	// as 'w' is also corresponding to the vocabulary ID.
	for (int l = 0; l < numSentiLabs; l++) {
		for (int z = 0; z < numTopics; z++) {
			betaSum_lz[l][z] = 0.0;
		    for (int r = 0; r < pnewData->vocabSize; r++) {
			    beta_lzw[l][z][r] = beta_lzw[l][z][r] * lambda_lw[l][r];
			    betaSum_lz[l][z] += beta_lzw[l][z][r];
		    }
		}
	}

	return 0;
}

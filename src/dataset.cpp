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

#include "dataset.h"
#include "document.h"
#include "model.h"
#include "map_type.h"
#include "strtokenizer.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
using namespace std; 


dataset::dataset() {
	pdocs = NULL;
	_pdocs = NULL;
	word2atr.clear();
	result_dir = ".";
	wordmapfile = "wordmap.txt";

	numDocs = 0;
	aveDocLength = 0;
	vocabSize = 0;
	corpusSize = 0;
}

dataset::dataset(string result_dir) {
	pdocs = NULL;
	_pdocs = NULL;
	word2atr.clear();
	this->result_dir = result_dir;
	wordmapfile = "wordmap.txt";

	numDocs = 0; 
	aveDocLength = 0;
	vocabSize = 0; 
	corpusSize = 0;
}


dataset::~dataset(void) {
	deallocate();
}


int dataset::read_dataStream(ifstream& fin) {
	string line;
	char buff[BUFF_SIZE_LONG];
	docs.clear();
	numDocs = 0;
	
	while (fin.getline(buff, BUFF_SIZE_LONG)) {
		line = buff;
		if(!line.empty()) {
			docs.push_back(line);
			numDocs++;
		}
	}
	
	if (numDocs > 0) {
		this->analyzeCorpus(docs);
	}
	
	return 0;
}


int dataset::analyzeCorpus(vector<string>& docs) {

	mapword2atr::iterator it;
	mapword2id::iterator vocabIt;   
	mapword2prior::iterator sentiIt;
	map<int,int>::iterator idIt;
		
	string line;
	numDocs = docs.size();
	vocabSize = 0;
	corpusSize = 0;
	aveDocLength = 0; 

  // allocate memory for corpus
	if (pdocs) {
		deallocate();
		pdocs = new document*[numDocs];
    } 
	else {
		pdocs = new document*[numDocs];
	}
	
	for (int i = 0; i < (int)docs.size(); ++i) {			
		line = docs.at(i);
		strtokenizer strtok(line, " \t\r\n");  // \t\r\n are the separators
		int docLength = strtok.count_tokens();
	
		if (docLength <= 0) {
			printf("Invalid (empty) document!\n");
			deallocate();
			numDocs = vocabSize = 0;
			return 1;
		}
	
		corpusSize += docLength - 1; // the first word is document name/id
		
		// allocate memory for the new document_i 
		document * pdoc = new document(docLength-1);
		pdoc->docID = strtok.token(0).c_str();

		// generate ID for the tokens in the corpus, and assign each word token with the corresponding vocabulary ID.
		for (int k = 0; k < docLength-1; k++) {
			int priorSenti = -1;	
			it = word2atr.find(strtok.token(k+1).c_str());
		
			if (it == word2atr.end()) { //  i.e., new word
				pdoc->words[k] = word2atr.size();
				sentiIt = sentiLex.find(strtok.token(k+1).c_str()); // check whether the word token can be found in the sentiment lexicon
				// incorporate sentiment lexicon
				if (sentiIt != sentiLex.end()) {
				    priorSenti = sentiIt->second.id;
				}
					
				// insert sentiment info into word2atr
				Word_atr temp = {word2atr.size(), priorSenti};  // vocabulary index; word polarity
				word2atr.insert(pair<string, Word_atr>(strtok.token(k+1), temp));
				pdoc->priorSentiLabels[k] = priorSenti;
				
			} 
			else { // word seen before
				pdoc->words[k] = it->second.id;
				pdoc->priorSentiLabels[k] = it->second.polarity;
			}
		}
		
		add_doc(pdoc, i);
	} 
	    
	    
	// update number of words
	vocabSize = word2atr.size();
	aveDocLength = corpusSize/numDocs;

	if (write_wordmap(result_dir + wordmapfile, word2atr)) {
		printf("ERROE! Can not read wordmap file %s!\n", wordmapfile.c_str());
		return 1;
	}
	if (read_wordmap(result_dir + wordmapfile, id2word)) {
		printf("ERROE! Can not read wordmap file %s!\n", wordmapfile.c_str());
		return 1;
	}

	docs.clear();
	return 0;
}



void dataset::deallocate() 
{
	if (pdocs) {
		for (int i = 0; i < numDocs; i++) 
			delete pdocs[i];		
		delete [] pdocs;
		pdocs = NULL;
	}
	
	if (_pdocs) {
		for (int i = 0; i < numDocs; i++) 
			delete _pdocs[i];
		delete [] _pdocs;
		_pdocs = NULL;
	}
}
    

void dataset::add_doc(document * doc, int idx) {
    if (0 <= idx && idx < numDocs)
        pdocs[idx] = doc;
}   

void dataset::_add_doc(document * doc, int idx) {
    if (0 <= idx && idx < numDocs) {
	    _pdocs[idx] = doc;
    }
}


int dataset::read_senti_lexicon(string sentiLexiconFile) {
	sentiLex.clear();
	char buff[BUFF_SIZE_SHORT];
    string line;
    vector<double> wordPrior;
    int labID;
    double tmp, val;
    int numSentiLabs;
    
    FILE * fin = fopen(sentiLexiconFile.c_str(), "r");
    if (!fin) {
		printf("Cannot read file %s!\n", sentiLexiconFile.c_str());
		return 1;
    }    
     
    while (fgets(buff, BUFF_SIZE_SHORT - 1, fin) != NULL) {
		line = buff;
		strtokenizer strtok(line, " \t\r\n");
			
		if (strtok.count_tokens() < 1)  {
			printf("Warning! The strtok count in the lexicon line [%s] is smaller than 2!\n", line.c_str());
		}
		else {	
			tmp = 0.0;
			labID = 0;
			wordPrior.clear();
			numSentiLabs = strtok.count_tokens();
			for (int k = 1; k < strtok.count_tokens(); k++) {
				val = atof(strtok.token(k).c_str());
				if (tmp < val) {
					tmp = val;
					labID = k-1;
				}
				wordPrior.push_back(val);
			}
			Word_Prior_Attr temp = {labID, wordPrior};  // sentiment label ID, sentiment label distribution
			sentiLex.insert(pair<string, Word_Prior_Attr >(strtok.token(0), temp));
		}
    }
    
	if (sentiLex.size() <= 0) {
		printf("Can not find any sentiment lexicon in file %s!\n", sentiLexiconFile.c_str());
		return 1;
	}
	
    fclose(fin);
    return 0;
}


int dataset::write_wordmap(string wordmapfile, mapword2atr &pword2atr) {

    FILE * fout = fopen(wordmapfile.c_str(), "w");
    if (!fout) {
		printf("Cannot open file %s to write!\n", wordmapfile.c_str());
		return 1;
    }    
    
    mapword2atr::iterator it;
    fprintf(fout, "%d\n", (int)(pword2atr.size()));
    for (it = pword2atr.begin(); it != pword2atr.end(); it++) {
	    fprintf(fout, "%s %d\n", (it->first).c_str(), it->second.id);
    }
    
    fclose(fout);
    return 0;
}


int dataset::read_wordmap(string wordmapfile, mapid2word &pid2word) {
    pid2word.clear(); 
    
    FILE * fin = fopen(wordmapfile.c_str(), "r");
    if (!fin) {
		printf("Cannot open file %s to read!\n", wordmapfile.c_str());
		return 1;
    }    
    
    char buff[BUFF_SIZE_SHORT];
    string line;
    
    fgets(buff, BUFF_SIZE_SHORT - 1, fin);
    int nwords = atoi(buff);
    
    for (int i = 0; i < nwords; i++) {
		fgets(buff, BUFF_SIZE_SHORT - 1, fin);
		line = buff;
		strtokenizer strtok(line, " \t\r\n");
		if (strtok.count_tokens() != 2) {
			printf("Warning! Line %d in %s contains less than 2 words!\n", i+1, wordmapfile.c_str());
			continue;
		}
		
		pid2word.insert(pair<int, string>(atoi(strtok.token(1).c_str()), strtok.token(0)));
    }
    
    fclose(fin);
    return 0;
}


int dataset::read_wordmap(string wordmapfile, mapword2id& pword2id) {
    pword2id.clear();
    char buff[BUFF_SIZE_SHORT];
    string line;


    FILE * fin = fopen(wordmapfile.c_str(), "r");
    if (!fin) {
		printf("Cannot read file %s!\n", wordmapfile.c_str());
		return 1;
    }    
        
    fgets(buff, BUFF_SIZE_SHORT - 1, fin);
    int nwords = atoi(buff);
    
    for (int i = 0; i < nwords; i++) {
		fgets(buff, BUFF_SIZE_SHORT - 1, fin);
		line = buff;
		strtokenizer strtok(line, " \t\r\n");
		if (strtok.count_tokens() != 2) {
			continue;
		}
		pword2id.insert(pair<string, int>(strtok.token(0), atoi(strtok.token(1).c_str())));
    }
    
    fclose(fin);
    return 0;
}


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


#ifndef	_DATASET_H
#define	_DATASET_H

#include "constants.h"
#include "document.h"
#include "map_type.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
using namespace std; 


class dataset {

public:
    mapword2atr word2atr;
	mapid2word id2word; 
	mapword2prior sentiLex; // <word, polarity>
	
	document ** pdocs; // store training data vocab ID
	document ** _pdocs; // only use for inference, i.e., for storing the new/test vocab ID
    ifstream fin;
	
	string data_dir;
	string result_dir;
	string wordmapfile;

	int numDocs;
	int aveDocLength; // average document length
	int vocabSize;
	int corpusSize;
	
	vector<string> docs; // for buffering dataset
	vector<string> newWords;
		
	// functions 
	dataset();
	dataset(string result_dir);
	~dataset(void);
	
	int read_dataStream(ifstream& fin);
	int read_newData(string filename);
	int read_senti_lexicon(string sentiLexiconFileDir);
	int analyzeCorpus(vector<string>& docs);

	static int write_wordmap(string wordmapfile, mapword2atr& pword2atr);
	static int read_wordmap(string wordmapfile, mapid2word& pid2word);
	static int read_wordmap(string wordmapfile, mapword2id& pword2id); 

	int init_parameter();
	void deallocate();  
	void add_doc(document * doc, int idx);
	void _add_doc(document * doc, int idx);

};

#endif

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
   
   
#ifndef	_DOCUMENT_H
#define	_DOCUMENT_H

#include <vector>
#include <iostream>
using namespace std; 



class document {

public:
	int * words;
	int * priorSentiLabels;
	string docID;
	string rawstr;
	int length;
	
	document() {
		words = NULL;
		priorSentiLabels = NULL;
		docID = "";
		rawstr = "";
		length = 0;	
	}
    
	// Constructor. Retrieve the length of the document and allocate memory for storing the documents
    document(int length) {
		this->length = length;
		docID = "";
		rawstr = "";
		words = new int[length]; // words stores the word token ID, which is integer
		priorSentiLabels = new int[length];	
    }
    
    // Constructor. Retrieve the length of the document and store the element of words into the array
    document(int length, int * words) {
		this->length = length;
		docID = "";
		rawstr = "";
		this->words = new int[length];
		for (int i = 0; i < length; i++) {
			this->words[i] = words[i];
		}
		priorSentiLabels = new int[length];	
    }

    document(int length, int * words, string rawstr) {
		this->length = length;
		docID = "";
		this->rawstr = rawstr;
		this->words = new int[length];
		for (int i = 0; i < length; i++) {
			 this->words[i] = words[i];
		}
		priorSentiLabels = new int[length];
    }
    

    document(vector<int> & doc) {
		this->length = doc.size();
		docID = "";
		rawstr = "";
		this->words = new int[length];
		for (int i = 0; i < length; i++) {
			this->words[i] = doc[i];
		}
		priorSentiLabels = new int[length];	
    }


	document(vector<int> & doc, string rawstr) {
		this->length = doc.size();
		docID = "";
		this->rawstr = rawstr;
		this->words = new int[length];
		for (int i = 0; i < length; i++) {
			this->words[i] = doc[i];
		}
		priorSentiLabels = new int[length];
	}

    document(vector<int> & doc, vector<int> &priorSentiLab, string rawstr) {
		this->length = doc.size();
		docID = "";
		this->rawstr = rawstr;
		this->words = new int[length];
		this->priorSentiLabels = new int[length];
		for (int i = 0; i < length; i++) {
			this->words[i] = doc[i];
			this->priorSentiLabels[i] = priorSentiLab[i];
		}
    }
    
    ~document() {
		if (words != NULL){ 
			delete [] words;
			words = NULL;
		}
			
		if (priorSentiLabels != NULL){
			delete [] priorSentiLabels;
			priorSentiLabels = NULL;
		}
    }
};

#endif

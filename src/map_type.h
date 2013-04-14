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
   
   
#ifndef	_MAP_TYPE_H
#define	_MAP_TYPE_H
#include <map>
#include <iostream>
using namespace std;


struct Word_atr { 
	int id; // vocabulary index
	int polarity; // sentiment label
};

struct Word_Prior_Attr { 
	int id; // prior sentiment label
	vector<double> labDist; // label distribution
};

// map of words/terms [string => int]
typedef map<string, int> mapword2id;

// map of words/terms [int => string]
typedef map<int, string> mapid2word;

// map of words/attributes_of_words [string => word_attr]
typedef map<string, Word_atr> mapword2atr;

// map of word / word prior info [string => sentiment lab ID, sentiment label distribition]
typedef map<string, Word_Prior_Attr > mapword2prior;

// map of doc / doc label distribution [string => doc label distribition]
typedef map<string, vector<double> > mapname2labs;

#endif

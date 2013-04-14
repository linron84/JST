*****************************************************
         Joint Sentiment-Topic (JST) Model 
*****************************************************

(C) Copyright 2013, Chenghua Lin and Yulan He

Written by Chenghua Lin, University of Aberdeen, chenghua.lin@abdn.ac.uk, part of code
is from http://gibbslda.sourceforge.net/.

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


------------------------------------------------------------------------

This is a C++ implementation of the joint sentiment-topic (JST) model for  
sentiment classification and extracting sentiment-bearing topics from text copara.

------------------------------------------------------------------------


TABLE OF CONTENTS


A. COMPILING

B. ESTIMATION

C. INFERENCE

D. Data format

E. References 


------------------------------------------------------------------------

A. COMPILING

Type "make" in a shell.


------------------------------------------------------------------------

B. ESTIMATION

Estimate the model by executing:

	jst -est -config YOUR-PATH/train.properties	
	
Outputs of jst estimation include the following files:
	<iter>.others  // contains model parameter settings
	<iter>.pi      // contains the per-document sentiment distributions
	<iter>.phi     // contains the sentiment specific topic-word distributions
	<iter>.theta   // contains the per-document sentiment specific topic proportions
	<iter>.tassign // contains the sentiment label and topic assignments for words in training data
------------------------------------------------------------------------

C. INFERENCE

To perform inference on a different set of data (in the same format as
for estimation), execute:

    jst -inf -config YOUR-PATH/test.properties
    
Outputs of jst inference include the following files:
	<modelName_iter>.newothers 
	<modelName_iter>.newpi 
	<modelName_iter>.newphi 
	<modelName_iter>.newtheta 
	<modelName_iter>.newtassign
    
------------------------------------------------------------------------

D. Data format

(1) The input data format for estimation/inference is as follows, where each line is one document, preceded by the document ID.

    [Doc_1 name] [token_1] [token_2] ... [token_N]
     :
     :
    [Doc_M name] [token_1] [token_2] ... [token_N]

(2) Sentiment lexicon (mpqa.constraint)

    [word]	[neu prior prob.] [pos prior prob.] [neg prior prob.]
	
	
------------------------------------------------------------------------

E. References

[1] Lin, C., He, Y., Everson, R. and Reuger, S. Weakly-supervised Joint Sentiment-Topic Detection from Text, IEEE Transactions on Knowledge and Data Engineering (TKDE), 2011.

[2] Lin, C. and He, Y. Joint Sentiment/Topic Model for Sentiment Analysis, In Proceedings of the 18th ACM Conference on Information and Knowl- edge Management (CIKM), Hong Kong, China, 2009.



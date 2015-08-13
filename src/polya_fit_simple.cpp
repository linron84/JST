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
#include "polya_fit_simple.h"
#include "math_func.h"
#include <math.h>
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;


int polya_fit_simple(int ** data, double * alpha, int _K, int _nSample) {
	int K = _K;                 // hyperparameter dimension
	int nSample = _nSample;     // total number of samples, i.e.documents
	int polya_iter = 100000;    // maximum number of fixed point iterations
	int ifault1, ifault2;

	double sum_alpha_old;
	double * old_alpha = NULL;
	double sum_g = 0; // sum_g = sum_digama(data[i][k] + old_alpha[k]),
	double sum_h = 0; // sum_h + sum_digama(data[i] + sum_alpha_old) , where data[i] = sum_data[i][k] for all k,
	double * data_row_sum = NULL; // the sum of the counts of each data sample P = {P_1, P_2,...,P_k}
	bool sat_state = false;
	int i, k, j;
  
	old_alpha = new double[K];
	for (k = 0; k < K; k++) {
		old_alpha[k] = 0;
	}
  
	data_row_sum = new double[nSample];
	for (i = 0; i < nSample; i++) {
		data_row_sum[i] = 0;
	}

	// data_row_sum
	for (i = 0; i < nSample; i++) {
		for (k = 0; k < K; k++) {
			data_row_sum[i] += *(*(data+k)+i) ;
		}
	}

	// simple fix point iteration
	printf("Optimising parameters...\n");
	for (i = 0; i < polya_iter; i++) {  // reset sum_alpha_old
		sum_alpha_old = 0;
		// update old_alpha after each iteration
		for (j = 0; j < K; j++) {
			old_alpha[j] = *(alpha+j);
		}
 
		 // calculate sum_alpha_old
		 for (j = 0; j < K; j++) {
			 sum_alpha_old += old_alpha[j];
		 }

		 for (k = 0; k < K; k++) {
			 sum_g = 0;
			 sum_h = 0;

			 // calculate sum_g[k]
			 for (j = 0; j < nSample; j++) {
				 sum_g += digama( *(*(data+k)+j) + old_alpha[k], &ifault1);
			 }

			 // calculate sum_h
			 for (j = 0; j < nSample; j++) {
				 sum_h += digama(data_row_sum[j] + sum_alpha_old, &ifault1);
			 }

			 // update alpha (new)
			 *(alpha+k) = old_alpha[k] * (sum_g - nSample * digama(old_alpha[k], &ifault1)) / (sum_h - nSample * digama(sum_alpha_old, &ifault2));
		 }

		 // terminate iteration ONLY if each dimension of {alpha_1, alpha_2, ... alpha_k} satisfy the termination criteria,
		 for (j = 0; j < K; j++) {
			 if (fabs( *(alpha+j) - old_alpha[j]) > 0.000001) break;
			 if ( j == K-1) {
				 sat_state = true;
			 }
		 }

		// check whether to terminate the whole iteration
		if(sat_state) {
			cout<<"Terminated at iteration: "<<i<<endl;
			break;
		}
		else if(i == polya_iter-1)  cout<<"Haven't converged! Terminated at iteration: "<<i+1<<endl;

	}

	printf("Optimisation done!\n");

    return 0;
}

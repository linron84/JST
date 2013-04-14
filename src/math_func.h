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
   
   
#ifndef	_MATH_FUNC_H
#define	_MATH_FUNC_H


//*************************  asa032.h   ************************************//
double alngam ( double xvalue, int *ifault );
double gamain ( double x, double p, int *ifault );
void gamma_inc_values ( int *n_data, double *a, double *x, double *fx );
double r8_abs ( double x );
void timestamp ( void );


//*************************  asa103.cpp   ************************************//
double digama ( double x, int *ifault );
void psi_values ( int *n_data, double *x, double *fx );
//void timestamp ( void );


//*************************  asa121.cpp   ************************************//
//void timestamp ( void );
double trigam ( double x, int *ifault );
void trigamma_values ( int *n_data, double *x, double *fx );


#endif

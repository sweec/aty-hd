/*
 *  ATY_math.h
 *  ATY_HD
 *
 *  Created by Dong Luo on 3/11/09.
 *  Copyright 2009 Boston University. All rights reserved.
 *
 */
#ifndef _ATY_MATH_H
#define _ATY_MATH_H

#ifdef __cplusplus
//extern "C" {
#endif
	
double sin(double);
int __isfinitef(float);
int __isfinited(double);
int __isnanf(float);
int __isnand(double);
double fabs(double);
double floor(double);

#ifdef __cplusplus
//}
#endif

#endif
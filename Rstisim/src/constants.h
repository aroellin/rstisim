/***************************************************************************
 *   Copyright (C) 2008 by Adrian Roellin   *
 *   roellin@ispm.unibe.ch   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "R.h"

/** The number of full days in one year */
#define DAYSINYEAR 365
/** A constant of type int, representing the largest positive value
of a signed integer*/
#define MAXINT INT_MAX /* ((int)(((unsigned int)(-1))/2)) */
/** A constant of type double, representing the largest positive value of
a double*/
#define MAXDOUBLE R_PosInf

/** A constant of type int representing the value 'not available'. With the
current implementation of R 2.7 this is the minimal value of a signed integer,
i.e. -MAXINT-1 */ 
#define INTNA R_NaInt
/** A constant of type double, representing the value 'not available'. */
#define DOUBLENA R_NaReal

/** A constant representing the NA value for bins */
#define BINNA INTNA
/** A constant representing the NA value for types */
#define TYPENA INTNA
/** A constant representing the NA value for attributes */
#define ATTRIBUTENA INTNA
/** A constant representing the NA value for numbers */
#define NUMBERNA INTNA
/** A constant representing the NA value for time values */
#define TIMENA DOUBLENA
/** A constant representing the NA value for general values */
#define VALUENA DOUBLENA

/** The maximal number of possible types per category (ie. the maximal number 
of people types, maximal number of infection types, etc.*/
#define MAXTYPES 100
/** The maximal number of bins that each type can have */
#define MAXBINS 100
/** The maximal number of attribtues that can be installed in each type */
#define MAXNEWATTRIBUTES 200


/** A constant for object::idclass, representic the base class Object; also
used to represent a generic subclass of the classes in the following list. */
#define CLASSGENERIC                     0          

/** A constant for object::idclass */
#define CLASSPERSON                     10
/** A constant for object::idsubclass */
    #define CLASSPERSONGENERIC                  11
/** A constant for object::idsubclass */
    #define CLASSPERSONMALE                     12
/** A constant for object::idsubclass */
    #define CLASSPERSONFEMALE                   13    

/** A constant for object::idclass */
#define CLASSPERSONCREATOR              20
/** A constant for object::idsubclass */
    #define CLASSPERSONCREATORGENERIC           21
/** A constant for object::idsubclass */
    #define CLASSPERSONCREATORMALE              22
/** A constant for object::idsubclass */
    #define CLASSPERSONCREATORFEMALE            23    

/** A constant for object::idclass */
#define CLASSEVENT                      30
/** A constant for object::idsubclass */
    #define CLASSEVENTGENERIC                   31
/** A constant for object::idsubclass */
    #define CLASSEVENTIMMIGRATION               32
/** A constant for object::idsubclass */
    #define CLASSEVENTBIRTH                     33
/** A constant for object::idsubclass */
    #define CLASSEVENTPSINITIATE                34
/** A constant for object::idsubclass */
    #define CLASSEVENTAGEABLEDEATH              35
/** A constant for object::idsubclass */
    #define CLASSEVENTAGEABLEBINCHANGE          36
/** A constant for object::idsubclass */
    #define CLASSEVENTHAVESEX                   37
/** A constant for object::idsubclass */
    #define CLASSEVENTGETPREGNANT               38
/** A constant for object::idsubclass */
    #define CLASSEVENTREMOVEOLD                 39
/** A constant for object::idsubclass */
    #define CLASSEVENTINFECTPERSON              40
/** A constant for object::idsubclass */
    #define CLASSEVENTABORTION                  41
/** A constant for object::idsubclass */
    #define CLASSEVENTVISITGP                   42
/** A constant for object::idsubclass */
    #define CLASSEVENTTREAT                     43
/** A constant for object::idsubclass */
    #define CLASSEVENTPROVOKEVISITGP      44

/** A constant for object::idclass */
#define CLASSDISTRIBUTION               50
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONCONSTANT           51
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONDISCRETE           52
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONCONTINUOUS         53
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONEXPONENTIAL        54
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONWEIBULL            55
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONPOISSONPROCESS     56
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONUNIFORM            57
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONARRAY              58 
/** A constant for object::idsubclass */
    #define CLASSDISTRIBUTIONCONSTANTCONTINUOUS 59

/** A constant for object::idclass */
#define CLASSPARTNERSHIP                80
/** A constant for object::idsubclass */
    #define CLASSPARTNERSHIPGENERIC             81

/** A constant for object::idclass */
#define CLASSPARTNERSHIPCREATOR         90
/** A constant for object::idsubclass */
    #define CLASSPARTNERSHIPFORMERINDIVIDUAL    91
/** A constant for object::idsubclass */
    #define CLASSPARTNERSHIPFORMERPERBIN        92

/** A constant for object::idclass */
#define CLASSPARTNERSHIPFORMER         100    
/** A constant for object::idsubclass */
    #define CLASSPARTNERSHIPFORMERGENERIC      101

/** A constant for object::idclass */
#define CLASSINFECTION                  110
/** A constant for object::idsubclass */
    #define CLASSINFECTIONGENERIC              111
        

/** A constant for object::idclass */
#define CLASSINFECTIONCREATOR           120


/** A constant for object::idclass */
#define CLASSGPVISITCREATOR             130


/** A constant for object::idclass */
#define CLASSNOTIFIER                   140
#endif

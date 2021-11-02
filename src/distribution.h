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

#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include "object.h"

/*****************************************************************************
These two function create a new distribution. They differ in the second
argument which defines the context of how the distribution will be used. There
are two possiblities: an 'inner attribute' is defined within the same type as
it will be used (bin transitions, 'seekgeneraltreatment' for a person,
'immunity' for an infection, 'lifespan' for a person, etc.; here it does is not
allowed to use the 'bytype' argument, as the the type is already fixed to the
type in which the distribtion is defined;); the second possibility are 'outer
attributes', an attribute that will be evaluated against a different type than
the one that defines it; 'infectiousnessfactorinfector' for example is defined
in the 'infections' section but will be evaluated against a person, therefore
the 'bytype' argument is allowed.
******************************************************************************/
Distribution *createDistribution(ROBJ cfg, const Creator *creator = 0);
Distribution *createDistribution(ROBJ cfg, const CreatorCollection *collection);


/*****************************************************************************
This is the abstract class for a distribution and is inherited by any other
distribution. Atleast the method 'dsample' needs to be defined
******************************************************************************/

class Distribution : public Object
{
public:
    
    // Constructor
    // - randcore
    // This is the number of the random number generator that will be used
    // when sampling from this distribution; must satisfy 
    // 0 <= randcore < RANCORES
    // - cfg
    // This is the configuration list that defines the distribution; 
    // some elements of 'cfg' will be used already by the createDistribution
    // functions to decide which Distribution class to use
    // - cond
    // This defines the confitioning type; see 'typedefs.h' for details
    Distribution
    (unsigned int randcore, ROBJ cfg, ConditioningType cond = NoConditioning);
        
    // Destructor
    ~Distribution();

    //Sampling function,
    
    // Sample from this distribution without conditioning
    virtual double dsample() const = 0;
    // Sample from this distribution 
    virtual double dsample(double atleast) const; 
    virtual double dsample(const Ageable *obj, double now) const;
    virtual double dsample(
        const Ageable *obj, double now, double atleast
    ) const;
    
    virtual double dsamplefac(double factor) const;
    virtual double dsamplefac(double factor, double atleast) const; 
    virtual double dsamplefac(
        double factor, const Ageable *obj, double now
    ) const;
    virtual double dsamplefac(
        double factor, const Ageable *obj, double now, double atleast
    ) const;

    virtual void checkRange(double min, double max) const;
    
    virtual int isample() const;
    virtual int isample(double atleast) const;
    virtual int isample(const Ageable *obj, double now) const;
    virtual int isample(const Ageable *obj, double now, double atleast) const;
    
    virtual int isamplemax(int max) const;
    
    int    getiminx() const {return(iminx);};
    int    getimaxx() const {return(imaxx);};
    double getdminx() const {return(dminx);};
    double getdmaxx() const {return(dmaxx);};
    
    std::string str() const;

protected:
    ROBJ cfg;
    unsigned int randcore;
    ConditioningType cond;
    
    int    iminx, imaxx;
    double dminx, dmaxx;
    
    
};

class DistributionConstant : public Distribution {
public:
    DistributionConstant(
        int randcore, ROBJ cfg, ConditioningType cond, double value);
    ~DistributionConstant();
    
    int isample() const;
    double dsample() const;
    double dsample(double atleast) const;
    
    std::string str() const;
};

class DistributionExponential : public Distribution {
public:
    DistributionExponential(unsigned int randcore, ROBJ cfg, ConditioningType
cond,
        double shift, double rate, double cutat = MAXDOUBLE);
    ~DistributionExponential();
    
    double dsample() const;
    double dsample(double atleast) const;
   
    double dsamplefac(double factor) const;
    double dsamplefac(double factor, double atleast) const; 
   
    std::string str() const;
    
private:
    double rate;
};

class DistributionWeibull : public Distribution {
public:
    DistributionWeibull
    (unsigned int randcore, ROBJ cfg, ConditioningType cond,
        double shift, double scale, double shape, double cutat = MAXDOUBLE);
    ~DistributionWeibull();
    
    double dsample() const;
    double dsample(double atleast) const;
   
    std::string str() const;
    
private:
    double scale;
    double shape;
};

class DistributionUniform : public Distribution {
public:
    DistributionUniform
    (unsigned int randcore, ROBJ cfg, ConditioningType cond, 
        double min, double max);
    ~DistributionUniform();
    
    int isample() const;
    int isamplemax(int max) const;
    double dsample() const;
    double dsample(double atleast) const;
   
    std::string str() const;
    
private:
    int idiff;
};

class DistributionArray : public Distribution {
public:
    DistributionArray(ROBJ cfg, const Creator *creator);
    DistributionArray(ROBJ cfg, const CreatorCollection *collection);
    ~DistributionArray();
    
    int isample() const;
    int isample(double atleast) const;
    int isample(const Ageable *obj, double now) const;
    int isample(const Ageable *obj, double now, double atleast) const;
    
    double dsample() const;
    double dsample(double atleast) const;
    double dsample(const Ageable *obj, double now) const;
    double dsample(const Ageable *obj, double now, double atleast) const;
    
    double dsamplefac(double factor) const;
    double dsamplefac(double factor, double atleast) const;
    double dsamplefac(double factor, const Ageable *obj, double now) const;
    double dsamplefac
    (double factor, const Ageable *obj, double now, double atleast) const;

    std::string str() const;
    
private:
    Distribution **array;
    unsigned int len; 
    int min;
    NumberOf what;
};

class DistributionHost : public Distribution {
public:
    DistributionHost(ROBJ cfg, const CreatorCollection *hostcollector);
    ~DistributionHost();
    
    double dsample() const;
    
    int isample(const Ageable *obj, double now) const;
    int isample(const Ageable *obj, double now, double atleast) const;
    
    double dsample(const Ageable *obj, double now) const;
    double dsample(const Ageable *obj, double now, double atleast) const;
    
    double dsamplefac(double factor, const Ageable *obj, double now) const;
    double dsamplefac
        (double factor, const Ageable *obj, double now, double atleast) const;
    
    std::string str() const;
    
private:
    Distribution *dist;
    const CreatorCollection *hostcollection;
};

class DistributionDiscrete : public Distribution {
public:
    DistributionDiscrete
    (unsigned int randcore, ROBJ cfg, ConditioningType cond,
        PiecewiseFunction df);
    ~DistributionDiscrete();
    
    int isample() const;
    double dsample() const;
    double dsample(double atleast) const;
    
    std::string str() const;
    
private: 
    PiecewiseFunction df;
};

class DistributionContinuous : public Distribution {
public: 
    DistributionContinuous
    (unsigned int randcore, ROBJ cfg, ConditioningType cond, 
        PiecewiseFunction densf);
    ~DistributionContinuous();
    
    double dsample() const;
    
    std::string str() const;

private:
    PiecewiseFunction densf;
    PiecewiseFunction distf;
};

class DistributionConstantContinuous : public Distribution {
public: 
    DistributionConstantContinuous
    (unsigned int randcore, ROBJ cfg, ConditioningType cond, 
        PiecewiseFunction densf);
    ~DistributionConstantContinuous();
    
    double dsample() const;
    double dsample(double atleast) const;

    std::string str() const;

private:
    PiecewiseFunction densf;
    double rangemin, rangemax;

};


class DistributionPoissonProcess : public Distribution {
public: 
    DistributionPoissonProcess
    (unsigned int randcore, ROBJ cfg, ConditioningType cond,
        PiecewiseFunction densf);
    ~DistributionPoissonProcess();
    
    double dsample() const;
    double dsample(double atleast) const;
    
    double dsamplefac(double factor) const;
    double dsamplefac(double factor, double atleast) const;

    std::string str() const;

private:
    double _pointMeasureDensity(double x) const;
    PiecewiseFunction densf;
    double rangemin, rangemax;
    double densmax;
};



#endif

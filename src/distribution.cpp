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
#include "distribution.h"

#include <cmath>
#include <sstream>

#include "constants.h"
#include "rangen.h"

#include "person.h"
#include "infection.h"



using namespace std;


Distribution::Distribution(unsigned int _randcore, ROBJ _cfg, ConditioningType
_cond)
: Object()
{
    idclass = CLASSDISTRIBUTION;
    idsubclass = CLASSGENERIC;
    randcore = _randcore;
    cfg = _cfg;
    cond = _cond;
}

Distribution::~Distribution()
{
}

int Distribution::isample() const
{
    return(isample(0.0));
}

int Distribution::isample(double x) const
{
    return((int)floor(dsample(x)));
}

int Distribution::isample(const Ageable *obj, double now) const
{
    switch(cond) {
        case NoConditioning: 
            return(isample()); 
            break;
        case OnAge: {
            return(isample(obj->getAge(now)));
            break; }
        case OnBirth: 
            rif_error(cfg,"cannot sample 'frombirth' for integer sampling");
            break;
        case OnTime: {
            return(isample(now));
            break; }
        default: rif_error(cfg,"unknown conditioning type");
    };
    return(0); // Should never be reached
}

int Distribution::isample(const Ageable *obj, double now, double atleast) const
{
    switch(cond) {
        case NoConditioning: 
            return(isample(atleast)); 
            break;
        case OnAge: {
            return(isample(obj->getAge(now)+atleast));
            break; }
        case OnBirth: 
            rif_error(cfg,"cannot sample 'frombirth' for integer sampling");
            break;
        case OnTime: {
            return(isample(now+atleast));
            break; }
        default: rif_error(cfg,"unknown conditioning type");
    };
    return(0); // Should never be reached
}

int Distribution::isamplemax(int max) const
{
    error(
    "calling uninitialized function isamplemax(max) in Distribution class");
    return(0); // Should never be reached
}

double Distribution::dsample() const
{
    error("calling uninitialized function dsample in Distribution class");
    return(0.0); // Should never be reached
}

double Distribution::dsample(double atleast) const
{
    if (atleast > dmaxx) 
        error("Conditioning on atleast > maximum");
    if (atleast == dmaxx) return dmaxx;
    if (atleast <= dminx) return(dsample());
    double x;
    do {
        x = dsample();
    } while (x<atleast);
    return(x);
}
   
double Distribution::dsample(const Ageable *obj, double now) const
{
    switch(cond) {
        case NoConditioning: 
            return(dsample()); 
            break;
        case OnAge: {
            return(dsample(obj->getAge(now)));
            break; }
        case OnBirth: 
            return(obj->getTimeBirth() - now + dsample());
            break;
        case OnTime: {
            return(dsample(now));
            break; }
        default: rif_error(cfg,"unknown conditioning type");
    };
    return(0.0); // Should never be reached
}

double Distribution::dsample(const Ageable *obj, double now, double atleast)
const
{
    switch(cond) {
        case NoConditioning: 
            return(dsample(atleast)); 
            break;
        case OnAge: {
            return(dsample(obj->getAge(now)+atleast));
            break; }
        case OnBirth: 
            return(obj->getTimeBirth() - now + dsample(atleast));
            break;
        case OnTime:
            return(dsample(now+atleast));
            break;
        default: rif_error(cfg,"unknown conditioning type");
    };
    return(0.0); // Should never be reached
}

double Distribution::dsamplefac(double factor) const
{
    return(dsample()/factor);
}

double Distribution::dsamplefac(double factor, double atleast) const
{
    if (atleast > dmaxx) 
        error("Conditioning on atleast > maximum");
    if (atleast == dmaxx) return dmaxx;
    if (atleast <= dminx) return(dsamplefac(factor));
    double x;
    do {
        x = dsamplefac(factor);
    } while (x<atleast);
    return(x);
}

double Distribution::dsamplefac(double factor, const Ageable *obj, double now)
const
{
    switch(cond) {
        case NoConditioning: 
            return(dsamplefac(factor)); 
            break;
        case OnAge: {
            return(dsamplefac(factor,obj->getAge(now)));
            break; }
        case OnBirth: 
            return(obj->getTimeBirth() - now + dsamplefac(factor));
            break;
        case OnTime:
            return(dsamplefac(factor,now));
            break;
        default: rif_error(cfg,"unknown conditioning type");
    };
    return(0.0); // Should never be reached
}

double Distribution::dsamplefac(double factor, const Ageable *obj, double now,
double
atleast) const
{
    switch(cond) {
        case NoConditioning: 
            return(dsamplefac(factor, atleast)); 
            break;
        case OnAge: {
            return(dsamplefac(factor,obj->getAge(now)+atleast));
            break; }
        case OnBirth: 
            return(obj->getTimeBirth() - now + dsamplefac(factor,atleast));
            break;
        case OnTime:
            return(dsample(now+atleast));
            break;
        default: rif_error(cfg,"unknown conditioning type");
    };
    return(0.0); // Should never be reached
}

void Distribution::checkRange(double min, double max) const
{
    if (idsubclass == CLASSGENERIC) {
        error(
            "calling uninitialized function checkRange in Distribution class");
    }
    if (dminx < min || dmaxx > max) {
       if (cfg) {
            print();
            rif_error(cfg, string("values out of range: ") + str());
        } else {
            error("values out of range");
        } 
    }

}

string Distribution::str() const
{
    ostringstream s;
    s << "Distribution:"
        << "randcore=" << randcore
        << ",min=" << dminx
        << ",max=" << dmaxx
        << ",cond=" << cond
        << ",sourceline=" << rif_getSourceline(cfg)
        << ",path='" << rif_getPath(cfg) << "'";
    s << "|" << Object::str();
    return (s.str());
}


//// DistributionConstant
DistributionConstant::DistributionConstant(
    int randcore, ROBJ cfg, ConditioningType cond, double value)
 : Distribution(randcore, cfg, cond)
{
    idsubclass = CLASSDISTRIBUTIONCONSTANT;
    dminx = dmaxx = value;    
    iminx = imaxx = (int)floor(value);
//    dminy = 0.0;
//    dmaxy = 1.0;
}

DistributionConstant::~DistributionConstant()
{
}

int DistributionConstant::isample() const
{
    return (iminx);
}

double DistributionConstant::dsample() const
{
    return (dminx);
}

double DistributionConstant::dsample(double atleast) const
{
    if (atleast > dmaxx) {
        rif_error(cfg,"Conditioning on atleast > maximum");
        return(0.0); // Should never be reached
    } else return (dmaxx-atleast);
}

string DistributionConstant::str() const
{
    ostringstream s;
    s << "DistributionConstant";
    s << "|" << Distribution::str();
    return (s.str());
}

//// DistributionExponential
DistributionExponential::DistributionExponential(
    unsigned int randcore, ROBJ cfg, ConditioningType cond,
    double _shift, double _rate, double _cutat)
 : Distribution(randcore, cfg, cond)
{
    if (_shift > _cutat) 
        rif_error(cfg, "cut point must be bigger than shift");
    idsubclass = CLASSDISTRIBUTIONEXPONENTIAL;
    rate = _rate;
    dminx = _shift;
    iminx = (int)floor(_shift);
    imaxx = (int)floor(_cutat);
    dmaxx = _cutat;
    
/*    dminy = 0.0;
    dmaxy = 0.0;*/
}

DistributionExponential::~DistributionExponential()
{
}

double DistributionExponential::dsample() const
{
    if (rate == 0.0) return(dmaxx);
    return(min(dminx-log(ran::dran(randcore))/rate,dmaxx));
}

double DistributionExponential::dsample(double atleast) const
{
    if (atleast > dmaxx) 
        rif_error(cfg,"Conditioning on atleast > maximum", __LINE__);
        
    if (atleast <= dminx) {
        if (rate == 0.0) return(dmaxx-atleast);
        return(min(dminx-log(ran::dran(randcore))/rate,dmaxx)-atleast);
    } else {
        if (rate == 0.0) return(dmaxx-atleast);
        return(min(-log(ran::dran(randcore))/rate,dmaxx-atleast));
    }
}

double DistributionExponential::dsamplefac(double factor) const
{
    if (rate == 0.0) return(dmaxx);
    return(dsample()/factor);
}

double DistributionExponential::dsamplefac(double factor, double atleast) const
{
    if (atleast > dmaxx) 
        rif_error(cfg,"Conditioning on atleast > maximum", __LINE__);
        
    if (atleast <= dminx) {
        if (rate == 0.0) return(dmaxx - atleast);
        return(min(dminx-log(ran::dran(randcore))/(rate*factor),dmaxx)-atleast);
    } else {
        if (rate == 0.0) return(dmaxx-atleast);
        return(min(-log(ran::dran(randcore))/(rate*factor),dmaxx-atleast));
    }
    
}

string DistributionExponential::str() const
{
    ostringstream s;
    s << "DistributionExponential:";
    s << "shift=" << dminx;
    s << ",rate=1/" << 1/rate/DAYSINYEAR << "y";
    s << "|" << Distribution::str();
    return (s.str());
}







//// DistributionWeibull
DistributionWeibull::DistributionWeibull(
    unsigned int randcore, ROBJ cfg, ConditioningType cond,
    double _shift, double _scale, double _shape, double _cutat)
 : Distribution(randcore, cfg, cond)
{
    if (_shift > _cutat) 
        rif_error(cfg, "cut point must be bigger than shift");
    idsubclass = CLASSDISTRIBUTIONWEIBULL;
    dminx = _shift;
    iminx = (int)floor(_shift);
    imaxx = (int)floor(_cutat);
    dmaxx = _cutat;
    shape = _shape;
    scale = _scale;    
/*    dminy = 0.0;
    dmaxy = 0.0;*/
}

DistributionWeibull::~DistributionWeibull()
{
}

double DistributionWeibull::dsample() const
{
    return(min(dminx+pow(-log(ran::dran(randcore)),1/shape)*scale,dmaxx));
}

double DistributionWeibull::dsample(double atleast) const
{
    rif_error(cfg,"conditional sampling for Weibull not implemented yet");
    return(0.0);
}

string DistributionWeibull::str() const
{
    ostringstream s;
    s << "DistributionWeibull:"
        << "shift=" << dminx
        << ",scale=1/" << scale/DAYSINYEAR << "y"
        << ",shape=" << shape
        << "|" << Distribution::str();
    return (s.str());
}










//// DistributionDiscrete
DistributionDiscrete::DistributionDiscrete(
    unsigned int randcore, ROBJ cfg, ConditioningType cond,
    PiecewiseFunction _df)
 : Distribution(randcore, cfg, cond)
{
    idsubclass = CLASSDISTRIBUTIONDISCRETE;
    for (unsigned int i = 1; i < _df.n; i++) {
        if (_df.x[i-1]> _df.x[i]) {
            rif_error(cfg, "values must be given in non-decreasing order");
        }
    }

    if (_df.y[_df.n-1] != 1.0) 
            rif_error(cfg, "distribution function does not end in 1");
            
    df = _df;
    
    dminx = df.x[0];
    dmaxx = df.x[df.n-1];
    iminx = df.ix[0];
    imaxx = df.ix[df.n-1];
    
/*    dminy = dmaxy = 0.0;*/
    

}

DistributionDiscrete::~DistributionDiscrete()
{
    delete[] df.x;
    delete[] df.ix;
    delete[] df.y;
}

int DistributionDiscrete::isample() const
{
    double u = ran::dran(randcore);
    for (unsigned int i=0; i < df.n; i++) {
        if (u<=df.y[i]) return(df.ix[i]);                
    }
    rif_error(cfg,"Miss-specified distribution function (F[inf]<1)");
    return(0); // Should never be reached
}

double DistributionDiscrete::dsample() const
{
    double u = ran::dran(randcore);
    for (unsigned int i=0; i < df.n; i++) {
        if (u<=df.y[i]) return(df.x[i]);                
    }
    rif_error(cfg,"Miss-specified distribution function (F[inf]<1)");
    return(0.0); // Should never be reached
}

double DistributionDiscrete::dsample(double atleast) const
{
    if (dminx >= atleast) return(dsample()-atleast);
    if (dmaxx < atleast) rif_error(cfg, 
"Trying to sample from distribution conditioned on value bigger than maximum");
    // goto k such that dvalue[k] is the smallest value less or equal
    // to atleast; we can start with k=1, becase k=0 is captured by
    // first "if" above, which implies no conditioning
    int k = 1;
    while(df.x[k] < atleast) k++; 
    
    double cor = df.y[k-1];
    double u = ran::dran(randcore);
    for (unsigned int i=0; i < df.n; i++) {
        if (u<=(df.y[i]-cor)/(1.0-cor)) return(df.x[i]-atleast);  
    }
    rif_error(cfg,"Missspecified distribution function (F[inf]<1)");
    return(0.0); // Should never be reached
}


string DistributionDiscrete::str() const
{
    ostringstream s;
    unsigned int l = df.n < 5 ? df.n : 5;
    s << "DistributionDiscrete:";
    s << "length=" << df.n;
    s << ",values=[" << df.x[0]; 
        for (unsigned int i = 1; i < l; i++) s << "," << df.x[i];
        if (l < df.n) s << ",...";
        s << "]";
    s << ",dist=[" << df.y[0]; 
        for (unsigned int i = 1; i < l; i++) s << "," << df.y[i];
        if (l < df.n) s << ",...";
        s << "]";
    s << "|" << Distribution::str();
    return (s.str());
}

//// DistributionContinuous
DistributionContinuous::DistributionContinuous(
    unsigned int randcore, ROBJ cfg, ConditioningType cond,
    PiecewiseFunction _densf)
 : Distribution(randcore, cfg, cond)
{
    idsubclass = CLASSDISTRIBUTIONCONTINUOUS;
    
    if (_densf.n <= 1) 
        rif_error(cfg, "must be at least of length two");
    
    for (unsigned int i = 1; i < _densf.n; i++) {
        if (_densf.x[i-1]> _densf.x[i]) {
            rif_error(cfg, "values must be given in non-decreasing order");
        }
    }
    
    densf = _densf;           
    
    distf.x = densf.x;
    distf.ix = densf.ix;
    distf.n = densf.n - 1;
    distf.y = new double[distf.n];
    
    distf.y[0] = (densf.y[0]+densf.y[1])/2.0*(densf.x[1]-densf.x[0]);
    
    for (unsigned int i = 1; i < distf.n; i++) {
         distf.y[i] = distf.y[i-1] 
             + (densf.y[i]+densf.y[i+1])/2.0*(densf.x[i+1]-densf.x[i]);
    }

    distf.f = distf.y[distf.n-1];
    
    for (unsigned int i = 0; i < distf.n; i++) {
        distf.y[i] /= distf.f;  
    }
    
    dminx = densf.x[0];
    dmaxx = densf.x[densf.n-1];
    iminx = densf.ix[0];
    imaxx = densf.ix[densf.n-1];
}

DistributionContinuous::~DistributionContinuous()
{
    delete[] densf.x;
    delete[] densf.ix;
    delete[] densf.y;
    delete[] distf.y;
}

double DistributionContinuous::dsample() const
{
    double u = ran::dran(randcore);
    int i = 0;
    while(u>distf.y[i]) i++;
    u = ran::dran(randcore);
    double x = densf.y[i];
    double y = densf.y[i+1];
    double a = densf.x[i];
    double b = densf.x[i+1];
    double q = (y-x)/(b-a);

    if (q == 0.0) return(u*(b-a)+a);
    double c = 2.0/(b-a)/(x+y);
    double A = q/2.0;
    double B = (x-a*q);
    double C = a*(A*a-x)-u/c;
    double D = B*B-4.0*A*C; 
    return((-B+sqrt(D))/q);
}

string DistributionContinuous::str() const
{
    ostringstream s;
    unsigned int l = distf.n < 5 ? distf.n : 5;
    s << "DistributionContinuous:";
    s << "length=" << densf.n;
    s << ",values=[" << densf.x[0]; 
        for (unsigned int i = 1; i < l+1; i++) s << "," << densf.x[i];
        if (l < distf.n) s << ",...";
        s << "]";
    s << ",density=[" << densf.y[0]; 
        for (unsigned int i = 1; i < l+1; i++) s << "," << densf.y[i];
        if (l < distf.n) s << ",...";
        s << "]";
    s << ",dist=[" << distf.y[0]; 
        for (unsigned int i = 1; i < l; i++) s << "," << distf.y[i];
        if (l < distf.n) s << ",...";
        s << "]";
    s << "|" << Distribution::str();
    return (s.str());
}











DistributionConstantContinuous::DistributionConstantContinuous(
    unsigned int randcore, ROBJ cfg, ConditioningType cond, PiecewiseFunction
_densf)
 : Distribution(randcore, cfg, cond)
{
    idsubclass = CLASSDISTRIBUTIONCONSTANTCONTINUOUS;
    
    if (_densf.n <= 1) 
        rif_error(cfg, "must be at least of length two");
    
    for (unsigned int i = 1; i < _densf.n; i++) {
        if (_densf.x[i-1]>= _densf.x[i]) {
            rif_error(cfg, "values must be given in increasing order");
        }
    }
    
    if (_densf.f != 1.0) 
        rif_error(cfg,"density factor must be 1.0");
        
    densf = _densf;           
    
    dminx = 0.0;
    dmaxx = MAXDOUBLE;
    imaxx = MAXINT;
    iminx = 0;
    
    rangemin = densf.x[0];
    rangemax = densf.x[densf.n-1];
     
}

DistributionConstantContinuous::~DistributionConstantContinuous()
{
    delete[] densf.x;
    delete[] densf.ix;
    delete[] densf.y;
}
    
double DistributionConstantContinuous::dsample() const
{
    return(dsample(0.0));
}

double DistributionConstantContinuous::dsample(double atleast) const
{
    if (atleast >= rangemin && atleast < rangemax) {
        int i = 0;
        while (densf.x[i+1] <= atleast) i++;
        double width = densf.x[i+1] - densf.x[i];
        double lambda = (atleast - densf.x[i])/width;
        return(lambda*densf.y[i+1] + (1-lambda)*densf.y[i]);
    } else {
        return(0.0);
    }
}

string DistributionConstantContinuous::str() const
{
    ostringstream s;
    unsigned int l = densf.n < 5 ? densf.n : 5;
    s << "DistributionConstantContinuous:";
    s << "length=" << densf.n;
    s << ",values=[" << densf.x[0]; 
        for (unsigned int i = 1; i < l+1; i++) s << "," << densf.x[i];
        if (l < densf.n) s << ",...";
        s << "]";
    s << ",density=[" << densf.y[0]; 
        for (unsigned int i = 1; i < l+1; i++) s << "," << densf.y[i];
        if (l < densf.n) s << ",...";
        s << "]";
    s << "|" << Distribution::str();
    return (s.str());
}









//// DistributionPoissonProcess
DistributionPoissonProcess::DistributionPoissonProcess(
    unsigned int randcore, ROBJ cfg, ConditioningType cond,
    PiecewiseFunction _densf)
 : Distribution(randcore, cfg, cond)
{
    idsubclass = CLASSDISTRIBUTIONPOISSONPROCESS;
    
    if (_densf.n <= 1) 
        rif_error(cfg, "must be at least of length two");
    
    for (unsigned int i = 1; i < _densf.n; i++) {
        if (_densf.x[i-1]>= _densf.x[i]) {
            rif_error(cfg, "values must be given in increasing order");
        }
    }
    
    if (_densf.f != 1.0) 
        rif_error(cfg,"density factor must be 1.0");
        
    densf = _densf;           
    
    dminx = 0.0;
    dmaxx = MAXDOUBLE;
    iminx = 0;
    imaxx = MAXINT;
    
    rangemin = densf.x[0];
    rangemax = densf.x[densf.n-1];
     
    densmax = 0.0;
    for (unsigned int i = 1; i < densf.n; i++) {
        if (densf.y[i] > densmax) densmax = densf.y[i];
    }
}

DistributionPoissonProcess::~DistributionPoissonProcess()
{
    delete[] densf.x;
    delete[] densf.ix;
    delete[] densf.y;
}

double DistributionPoissonProcess::dsample() const
{
    return(dsample(0.0));
}

double DistributionPoissonProcess::dsamplefac(double factor) const
{
    return(dsamplefac(factor, 0.0));
}


double DistributionPoissonProcess::_pointMeasureDensity(double x) const
{
    if (x < rangemin || x >= rangemax) {
        return(0.0);
    }
    int i = 0;
    while (densf.x[i+1] <= x) i++;
    double width = densf.x[i+1] - densf.x[i];
    double lambda = (x - densf.x[i])/width;
    double ans = lambda*densf.y[i+1] + (1-lambda)*densf.y[i];
    return(ans);
}

double DistributionPoissonProcess::dsample(double atleast) const
{
    double t = atleast;
    while(t < rangemax) {
        t += -log(ran::dran(randcore))/densmax;
        if (ran::dran(randcore) < _pointMeasureDensity(t)/densmax)
return(t-atleast);
    }
    return(MAXDOUBLE);
}

double DistributionPoissonProcess::dsamplefac(double factor, double atleast)
const
{
    double t = atleast;
    while(t < rangemax) {
        t += -log(ran::dran(randcore))/densmax;
        if (ran::dran(randcore) < factor*_pointMeasureDensity(t)/densmax)
return(t-atleast);
    }
    return(MAXDOUBLE);
}

string DistributionPoissonProcess::str() const
{
    ostringstream s;
    unsigned int l = densf.n <= 9 ? densf.n : 9;
    s << "DistributionPoissonProcess:";
    s << "length=" << densf.n;
    s << ",values=[" << densf.x[0]; 
        for (unsigned int i = 1; i < l; i++) s << "," << densf.x[i];
        if (l < densf.n) s << ",...";
        s << "]";
    s << ",density=[" << densf.y[0]; 
        for (unsigned int i = 1; i < l; i++) s << "," << densf.y[i];
        if (l < densf.n) s << ",...";
        s << "]";
    s << ",measurerange=[" << rangemin << "," << rangemax << "]";
    s << "|" << Distribution::str();
    return (s.str());
}


void getWhatMinLen(ROBJ cfg, NumberOf &what, int &min, unsigned int &len, const
void *resolver, bool isCollector)
{
    min = 0;
    len = 0;
    what = NumberOfNothing;
    
    string str_what = rif_asString(cfg, 0, "depends");
        
    if (str_what == "currentpartners") {
        what = NumberOfPartnersCurrent;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
                     
    } else if (str_what == "currentpartnerstype0") {
        what = NumberOfPartnersCurrentType0;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;
        
    } else if (str_what == "currentpartnerstype1") {
        what = NumberOfPartnersCurrentType1;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;
        
    } else if (str_what == "currentpartnerstype2") {
        what = NumberOfPartnersCurrentType2;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;
        
    } else if (str_what == "currentpartnerstype3") {
        what = NumberOfPartnersCurrentType3;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;
        
    } else if (str_what == "currentpartnerstype4") {
        what = NumberOfPartnersCurrentType4;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;
        
    } else if (str_what == "totalpartners") {
        what = NumberOfPartnersTotal;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
        
    } else if (str_what == "withinpartners") {
        what = NumberOfPartnersWithin;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
        
    } else if (str_what == "currentinfections") {
        what = NumberOfInfectionsCurrent;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
            
    } else if (str_what == "totalinfections") {
        what = NumberOfInfectionsTotal;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
            
    } else if (str_what == "withininfections") {
        what = NumberOfInfectionsWithin;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
            
    } else if (str_what == "abortions") {
        what = NumberOfAbortions;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
            
    } else if (str_what == "pregnancies") {
        what = NumberOfPregnancies;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
            
    } else if (str_what == "contacts") {
        what = NumberOfContacts;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
    
    } else if (str_what == "linknumber") {
        what = NumberOfLink;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
    
    } else if (str_what == "unprotectedcontacts") {
        what = NumberOfContactsUnprotected;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
    
    } else if (str_what == "children") {
        what = NumberOfChildren;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
    
    } else if (str_what == "treatments") {
        what = NumberOfTreatments;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        

    } else if (str_what == "alreadynotified") {
        what = NumberOfAlreadyNotified;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
    
    } else if (str_what == "product") {
        what = NumberProduct;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
    
    } else if (str_what == "sum") {
        what = NumberSum;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
    
    } else if (str_what == "positivetests") {
        what = NumberOfPositiveTests;
        min = rif_asInteger(cfg, 0, "minimum");
        len = rif_asInteger(cfg, 0, "maximum") + 1;        
    
    } else if (str_what == "ispregnant") {
        what = IsPregnant;
        min = 0;
        len = 2;
    } else if (str_what == "isactive") {
        what = IsActive;
        min = 0;
        len = 2;
    } else if (str_what == "bin") {
        what = NumberOfBin;
        if (isCollector) {
            rif_error(cfg, "specify type first (use 'bytype' flag)");
        }
        if (!isCollector && !resolver) {
            rif_error(cfg, "must be called with Creator-object");
        }
        len = ((Creator*)resolver)->getNumberOfBins();
    } else if (str_what == "type") {
        what = NumberOfType;
        if (!isCollector) {
            rif_error(cfg, "type already specified");
        }
        if (isCollector && !resolver) {
            rif_error(cfg, "must be called with CreatorCollection-object");
        }
        len = ((CreatorCollection*)resolver)->getNumberOfCreators();
    } else {
        rif_error(cfg,"unknown size dependency");
    }
            
    if (min < 0) rif_error(cfg, "cannot have indices smaller than 0");
}

DistributionArray::DistributionArray(ROBJ cfg, const Creator *creator)
: Distribution(INTNA,cfg, NoConditioning)
{
    idsubclass = CLASSDISTRIBUTIONARRAY;
    array = 0;    
    dminx = 0;
    dmaxx = 0;
    min = 0;
    len = 0;
    what = NumberOfNothing;
    
    getWhatMinLen(cfg, what, min, len, creator, false);    
    
    if (min < 0) {
        rif_error(cfg,"must depend on non-negative values");
    }
    
    array = new Distribution*[len];
    
    for (unsigned int i = 0; i < len; i++) array[i] = 0;
    
    for (unsigned int i = min; i < len; i++) {
        ROBJ cfg_dist = 0;
        switch(what) {
            // !!!change at the other constructor as well!!!
            case NumberOfPartnersCurrent : 
            case NumberOfPartnersCurrentType0 :
            case NumberOfPartnersCurrentType1 :
            case NumberOfPartnersCurrentType2 :
            case NumberOfPartnersCurrentType3 :
            case NumberOfPartnersCurrentType4 :
            case NumberOfPartnersTotal :
            case NumberOfPartnersWithin :
            case NumberOfInfectionsCurrent : 
            case NumberOfInfectionsTotal : 
            case NumberOfInfectionsWithin : 
            case NumberOfContacts : 
            case NumberOfContactsUnprotected : 
            case NumberOfAbortions : 
            case NumberOfPregnancies : 
            case NumberOfChildren : 
            case NumberOfTreatments :
            case NumberOfAlreadyNotified :
            case NumberOfPositiveTests :
            case NumberOfLink :  
            case NumberSum :
            case NumberProduct :
            {    
                ostringstream s;
                s << i;
                cfg_dist = rif_trylookup(cfg, s.str().c_str());
                break;
            }
            case NumberOfType: {
                // Should never be reached
                rif_error(cfg, "internal error", __LINE__);                
                break;
            }
            case IsPregnant : {
                if (i == 0) {
                    cfg_dist = rif_lookup(cfg,"no");
                } else {
                    cfg_dist = rif_lookup(cfg,"yes");
                }
                break;
            }
            case IsActive : {
                if (i == 0) {
                    cfg_dist = rif_lookup(cfg,"no");
                } else {
                    cfg_dist = rif_lookup(cfg,"yes");
                }
                break;
            }
            case NumberOfBin: {
                string name = creator->getBinName(i);
                cfg_dist = rif_trylookup(cfg, name.c_str());
                if (rif_isNull(cfg_dist)) {
                    cfg_dist = rif_trylookup(cfg, "*");
                }
                if (rif_isNull(cfg_dist)) {
                    // cause a 'bin not found' error
                    rif_lookup(cfg, name.c_str());
                }
                break;
            }
            default : rif_error(cfg,"unknown number");
        }
        
        if (!rif_isNull(cfg_dist)) {
            array[i] = createDistribution(cfg_dist, creator);   
            // array[i]->print();
            dminx = array[i]->getdminx();
            dmaxx = array[i]->getdmaxx();
        } else {
            rif_error(cfg,"internal: missing configuration", __LINE__);
        }
    }
    
    for (unsigned int i = min; i < len; i++) {
        if (array[i]) {
            if (array[i]->getdminx() < dminx)
                dminx = array[i]->getdminx();
            if (array[i]->getdmaxx() > dmaxx)
                dmaxx = array[i]->getdmaxx();
        }
    }
    
    iminx = (int)floor(dminx);
    imaxx = (int)floor(dmaxx);
}

DistributionArray::DistributionArray(ROBJ cfg, const CreatorCollection
*collection)
: Distribution(INTNA, cfg, NoConditioning)
{
    idsubclass = CLASSDISTRIBUTIONARRAY;
    array = 0;    
        
    getWhatMinLen(cfg, what, min, len, collection, true);
    
    array = new Distribution*[len];
    for (unsigned int i = 0; i < len; i++) array[i] = 0;
    
    for (unsigned int i = min; i < len; i++) {
        ROBJ cfg_dist = 0;
        switch(what) {
            // !!!change at the other constructor as well!!!
            case NumberOfPartnersCurrent : 
            case NumberOfPartnersCurrentType0 :
            case NumberOfPartnersCurrentType1 :
            case NumberOfPartnersCurrentType2 :
            case NumberOfPartnersCurrentType3 :
            case NumberOfPartnersCurrentType4 :
            case NumberOfPartnersTotal :
            case NumberOfPartnersWithin :
            case NumberOfInfectionsCurrent : 
            case NumberOfInfectionsTotal : 
            case NumberOfInfectionsWithin : 
            case NumberOfContacts : 
            case NumberOfContactsUnprotected : 
            case NumberOfAbortions : 
            case NumberOfPregnancies : 
            case NumberOfChildren :
            case NumberOfTreatments :
            case NumberOfAlreadyNotified :
            case NumberOfPositiveTests :
            case NumberOfLink : 
            case NumberProduct :
            case NumberSum :  
            {
                ostringstream s;
                s << i;
                cfg_dist = rif_trylookup(cfg, s.str().c_str());
                break;
            }
            case NumberOfType: {
                string name = collection->getCreatorName(i);
                cfg_dist = rif_trylookup(cfg, name.c_str());
                if (rif_isNull(cfg_dist)) {
                    cfg_dist = rif_trylookup(cfg, "*");
                }
                if (rif_isNull(cfg_dist)) {
                    // cause an error
                    rif_lookup(cfg, name.c_str());
                }
                break;
            }
            case IsPregnant : {
                if (i == 0) {
                    cfg_dist = rif_lookup(cfg,"no");
                } else {
                    cfg_dist = rif_lookup(cfg,"yes");
                }
                break;
            }
            case IsActive : {
                if (i == 0) {
                    cfg_dist = rif_lookup(cfg,"no");
                } else {
                    cfg_dist = rif_lookup(cfg,"yes");
                }
                break;
            }
            case NumberOfBin: {
                // Should never be reached
                rif_error(cfg, "internal error", __LINE__);
                break;
            }
            default : rif_error(cfg,"unknown number");
        }
        
        if (!rif_isNull(cfg_dist)) {
            if (what == NumberOfType) {
                array[i] = createDistribution(cfg_dist,
collection->getCreator(i));   
            } else {
                array[i] = createDistribution(cfg_dist, collection);   
            }
            dminx = array[i]->getdminx();
            dmaxx = array[i]->getdmaxx();
        }
    }
    
    for (unsigned int i = min; i < len; i++) {
        if (array[i]) {
            if (array[i]->getdminx() < dminx)
                dminx = array[i]->getdminx();
            if (array[i]->getdmaxx() > dmaxx)
                dmaxx = array[i]->getdmaxx();
        }
    }
    
    iminx = (int)floor(dminx);
    imaxx = (int)floor(dmaxx);
}

DistributionArray::~DistributionArray()
{
    if (array) {
        for (unsigned int i = 0; i < len; i++) 
            if (array[i]) delete array[i];
        delete[] array;
    }
}
    
double DistributionArray::dsample() const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->dsample();
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->dsample();
            return(res);
        }
        default :return(array[min]->dsample());
    }
}

double DistributionArray::dsample(double atleast) const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->dsample(atleast);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->dsample(atleast);
            return(res);
        }
        default : return(array[min]->dsample(atleast));
    }
}

double DistributionArray::dsample(const Ageable *obj, double now) const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->dsample(obj, now);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->dsample(obj, now);
            return(res);
        }
        default : {    
            int index = obj->getNumber(what);
            if (index < 0) {
                rif_error(cfg,
        "internal distribution.cpp: getNumber returned negative value",__LINE__);
            }
            if ((unsigned int)index >= len) {
                index = len-1;
            } else if (index < min) {
                index = min;
            }
            return(array[index]->dsample(obj, now));
        }
    }
        
}

double DistributionArray::dsample(const Ageable *obj, double now, double
atleast)
const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->dsample(obj, now, atleast);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->dsample(obj, now, atleast);
            return(res);
        }
        default : {    
            int index = obj->getNumber(what);
            if (index < 0) {
                rif_error(cfg,
        "internal distribution.cpp: getNumber returned negative value",__LINE__);
            }
            if ((unsigned int)index >= len) {
                index = len-1;
            } else if (index < min) {
                index = min;
            }
            return(array[index]->dsample(obj, now, atleast));
        }
    }
}

int DistributionArray::isample() const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->isample();
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->isample();
            return(res);
        }
        default : {    
            return(array[0]->isample());
        }
    }
}

int DistributionArray::isample(double atleast) const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->isample(atleast);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->isample(atleast);
            return(res);
        }
        default : {    
            return(array[0]->isample(atleast));
        }
    }
}

int DistributionArray::isample(const Ageable *obj, double now) const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->isample(obj, now);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->isample(obj, now);
            return(res);
        }
        default : {    
            int index = obj->getNumber(what);
            if (index < 0) {
                rif_error(cfg,
        "internal distribution.cpp: getNumber returned negative value",__LINE__);
            }
            if ((unsigned int)index >= len) {
                index = len-1;
            } else if (index < min) {
                index = min;
            }
            return(array[index]->isample(obj, now));
        }
    }
}

int DistributionArray::isample(const Ageable *obj, double now, double atleast)
const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->isample(obj, now, atleast);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->isample(obj, now, atleast);
            return(res);
        }
        default : {    
            int index = obj->getNumber(what);
            if (index < 0) {
                rif_error(cfg,
        "internal distribution.cpp: getNumber returned negative value",__LINE__);
            }
            if ((unsigned int)index >= len) {
                index = len-1;
            } else if (index < min) {
                index = min;
            }
            return(array[index]->isample(obj, now, atleast));
        }
    }
}

double DistributionArray::dsamplefac(double factor) const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->dsamplefac(factor);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->dsamplefac(factor);
            return(res);
        }
        default : {    
            return(array[min]->dsamplefac(factor));
        }
    }
}

double DistributionArray::dsamplefac(double factor, double atleast) const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->dsamplefac(factor, atleast);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->dsamplefac(factor, atleast);
            return(res);
        }
        default : {    
            return(array[min]->dsamplefac(factor,atleast));
        }
    }
}

double DistributionArray::dsamplefac(double factor, const Ageable *obj, double
now)
const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->dsamplefac(factor, obj, now);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->dsamplefac(factor, obj, now);
            return(res);
        }
        default : {    
            int index = obj->getNumber(what);
            if (index < 0) {
                rif_error(cfg,
        "internal distribution.cpp: getNumber returned negative value",__LINE__);
            }
            if ((unsigned int)index >= len) {
                index = len-1;
            } else if (index < min) {
                index = min;
            }
            return(array[index]->dsamplefac(factor,obj, now));
        }
    }
}

double DistributionArray::dsamplefac(double factor, const Ageable *obj, double
now,
double atleast) const
{
    switch(what) {
        case NumberProduct: {
            double res = 1;
            for (unsigned int i = min; i < len; i++)
                res *= array[i]->dsamplefac(factor, obj, now, atleast);
            return(res);
        }
        case NumberSum: {
            double res = 0;
            for (unsigned int i = min; i < len; i++)
                res += array[i]->dsamplefac(factor, obj, now, atleast);
            return(res);
        }
        default : {    
            int index = obj->getNumber(what);
            if (index < 0) {
                rif_error(cfg,
        "internal distribution.cpp: getNumber returned negative value",__LINE__);
            }
            if ((unsigned int)index >= len) {
                index = len-1;
            } else if (index < min) {
                index = min;
            }
            return(array[index]->dsamplefac(factor,obj, now, atleast));
        }
    }
}


string DistributionArray::str() const
{
    ostringstream s;
    s << "DistributionArray:"
        << "length=" << len
        << ",what=" << what,
    s << "|" << Distribution::str();
    for (unsigned int i = min; i < len; i++) {
        s << endl << "\t";
        s << array[i]->str();
    }
    return (s.str());
}












DistributionHost::DistributionHost(ROBJ cfg, const CreatorCollection
*_hostcollection)
: Distribution(INTNA,cfg,NoConditioning)
{
    hostcollection = _hostcollection;
    dist = 0;
    dist = createDistribution(rif_lookup(cfg,"host"),hostcollection);
}

DistributionHost::~DistributionHost()
{
    if (dist) delete dist;
}
    
double DistributionHost::dsample() const
{
    error("DistributionHost has not implemented 'dsample'");
    return(0.0);
}

int DistributionHost::isample(const Ageable *obj, double now) const
{
    if (obj->idclass == CLASSINFECTION) {
        return(dist->isample(((Infection*)obj)->getHost(),now));
    } else {
        rif_error(cfg, "cannot get host from non-infection object");
        return(0);
    }
}

int DistributionHost::isample(const Ageable *obj, double now, double atleast)
const
{
    if (obj->idclass == CLASSINFECTION) {
        return(dist->isample(((Infection*)obj)->getHost(),now, atleast));
    } else {
        rif_error(cfg, "cannot get host from non-infection object");
        return(0);
    }
}
    
double DistributionHost::dsample(const Ageable *obj, double now) const
{
    if (obj->idclass == CLASSINFECTION) {
        return(dist->dsample(((Infection*)obj)->getHost(),now));
    } else {
        rif_error(cfg, "cannot get host from non-infection object");
        return(0);
    }
}

double DistributionHost::dsample(const Ageable *obj, double now, double atleast)
const
{
    if (obj->idclass == CLASSINFECTION) {
        return(dist->dsample(((Infection*)obj)->getHost(),now, atleast));
    } else {
        rif_error(cfg, "cannot get host from non-infection object");
        return(0);
    }
}
    
double DistributionHost::dsamplefac(double factor, const Ageable *obj, double
now)
const
{
    if (obj->idclass == CLASSINFECTION) {
        return(dist->dsamplefac(factor, ((Infection*)obj)->getHost(),now));
    } else {
        rif_error(cfg, "cannot get host from non-infection object");
        return(0);
    }
}

double DistributionHost::dsamplefac(double factor, const Ageable *obj, double
now,
double atleast) const
{
    if (obj->idclass == CLASSINFECTION) {
        return(dist->dsamplefac(factor, ((Infection*)obj)->getHost(),now,
atleast));
    } else {
        rif_error(cfg, "cannot get host from non-infection object");
        return(0);
    }
}

string DistributionHost::str() const
{
    ostringstream s;
    s << "DistributionHost"
    << ",hostcreatorcollection='" << hostcollection->getName() << "'";    
    s << "|" << Distribution::str();
    return (s.str());
 
}
















DistributionUniform::DistributionUniform(
    unsigned int randcore, ROBJ cfg, ConditioningType cond, double min, double
max)
: Distribution(randcore, cfg, cond)
{
    idsubclass = CLASSDISTRIBUTIONUNIFORM;
    dminx = min;
    dmaxx = max;
    iminx = (int)floor(min);
    imaxx = (int)floor(max);
    
    idiff = imaxx - iminx + 1;
}

DistributionUniform::~DistributionUniform()
{
}
    
int DistributionUniform::isample() const
{
    return(iminx + ran::ran(randcore, idiff));
}

int DistributionUniform::isamplemax(int max) const
{
    return(ran::ran(randcore, max));
}

double DistributionUniform::dsample() const
{
    return(ran::dran(randcore,dminx,dmaxx));
}

double DistributionUniform::dsample(double atleast) const
{
    if (atleast > dmaxx)
        rif_error(cfg,
"Trying to sample from distribution conditioned on value bigger than maximum");
    if (atleast <= dminx) { 
        return(dsample() - atleast);
    }
    return(ran::dran(randcore,0,dmaxx-atleast));
}
   
string DistributionUniform::str() const
{
    ostringstream s;
    s << "DistributionUniform";
    s << "|" << Distribution::str();
    return (s.str());

}


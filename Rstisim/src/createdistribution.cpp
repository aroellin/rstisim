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


PiecewiseFunction val2val(ROBJ val)
{
    PiecewiseFunction pw;
    pw.n = rif_getLength(val);
    pw.x = new double[pw.n];
    pw.ix = new int[pw.n];
    pw.y = 0;
    pw.f = 1.0;
    for (unsigned int i = 0; i < pw.n; i++) {
        pw.x[i] = rif_asDouble(val,i);
        pw.ix[i] = (int)floor(pw.x[i]);
    }
    return(pw); // pw.y is not valid array!
}

PiecewiseFunction prob2dist(ROBJ prob)
{
    PiecewiseFunction pw; 
    pw.n = rif_getLength(prob);
    pw.x = 0;
    pw.ix = 0;
    pw.y = new double[pw.n];
    pw.f = 0.0;
    
    pw.f = pw.y[0] = rif_asDouble(prob,0);
    
    if (pw.y[0] < 0) 
        rif_error(prob,"invalid probabilities");
    for (unsigned int i = 1; i < pw.n; i++) {
        double p = rif_asDouble(prob,i);
        if (p < 0) 
            rif_error(prob,"invalid probabilities");
        pw.y[i] = pw.y[i-1] + p;
        pw.f += p;
    }
    
    if (pw.f != 1.0) {
        rif_warning(prob, "rescaling probabilities");
        for (unsigned int i = 0; i < pw.n; i++) {
            pw.y[i] /= pw.f;
        }
    }
    
    return(pw); // pw.x and pw.ix are not valid arrays!
}

PiecewiseFunction surv2dist(ROBJ surv)
{
    PiecewiseFunction pw;
    pw.n = rif_getLength(surv);
    pw.x = 0;
    pw.ix = 0;
    pw.y = new double[pw.n];

    for(unsigned int i = 0; i < pw.n; i++) {
        pw.y[i] = rif_asDouble(surv,i);
        if ((i > 0 && pw.y[i] > pw.y[i-1]) || pw.y[i]<0)
            rif_error(surv, "invalid survival function");
    }
    
    pw.f = pw.y[0];

    if (pw.f != 1.0) {
        rif_warning(surv, "rescaling survival function");
        for (unsigned int i = 0; i < pw.n; i++) {
            pw.y[i] /= pw.f;
        }
    }
    
    for (unsigned int i = 0; i < pw.n-1; i++) {
        pw.y[i] = 1.0 - pw.y[i+1];
    }
    pw.y[pw.n-1] = 1.0;
    
    return(pw);
}

PiecewiseFunction dens2dens(ROBJ dens)
{
    PiecewiseFunction pw;
    pw.n = rif_getLength(dens);
    pw.x = 0;
    pw.ix = 0;
    pw.y = new double[pw.n];
    pw.f = 1.0;
    
    for (unsigned int i = 0; i < pw.n; i++) {
        pw.y[i] = rif_asDouble(dens,i);
        if (pw.y[i] < 0)
            rif_error(dens,"invalid density function");
    }
    
    return(pw);
}

// The following two functions are the same except for the difference
// creator/collector; this is messy and unelegant so need rewriting
Distribution *_createDistribution(ROBJ cfg, const Creator *creator)
{
    Distribution *d = 0;    
    PiecewiseFunction df;
    int randcore = 0;
    ConditioningType cond = NoConditioning;
    
    ROBJ attr_randcore = getAttrib(cfg, install("randcore"));
    if (rif_isNull(attr_randcore)) {
        rif_error(cfg,"attribute 'randcore' is missing");
    } else {
        randcore = rif_asInteger(attr_randcore, 0);
    }

    // One number -> DistributionConstant
    if ((rif_isDouble(cfg) || rif_isInteger(cfg)) && rif_getLength(cfg) == 1) {
        d = new DistributionConstant(randcore, cfg, cond, rif_asDouble(cfg,0));
        return(d);
    }
    
    // An array of numbers
    if (rif_isDouble(cfg) || rif_isInteger(cfg)) {
        df = prob2dist(cfg);
        df.x = new double[df.n];
        df.ix = new int[df.n];
        for (unsigned int i = 0; i < df.n; i++) {
            df.ix[i] = i;
            df.x[i] = (double)i;
        }
        
        d = new DistributionDiscrete(randcore, cfg, cond, df);
        return(d); 
    }
    
    // A list...
    string stype;
    

    if (rif_exists(cfg,"type")) {
        stype = rif_asString(cfg, 0, "type");
    } else if (rif_exists(cfg,"rate")) {
        stype = "exponential";
    } else if (rif_exists(cfg,"density")) {
        stype = "linear";
    } else if (rif_exists(cfg,"depends")) {
        stype = "array";
    } else if (rif_exists(cfg,"host")) {
        stype = "host";
    } else {
        stype = "simple";
    }
    
    if (rif_exists(cfg, "relativeto")) {
        ROBJ cfg_relativeto = rif_lookup(cfg,"relativeto");
        string relativeto = rif_asString(cfg_relativeto, 0);
        if (relativeto == "now") {
            cond = NoConditioning;
        } else if (relativeto == "age") {
            cond = OnAge;        
        } else if (relativeto == "birth") {
            cond = OnBirth;
        } else if (relativeto == "time") {
            cond = OnTime;
        } else {
            rif_error(cfg_relativeto, "unknown conditioning type");
        }
    }
    
    if (stype == "simple") {
        ROBJ cfg_prob = rif_trylookup(cfg,"probabilities");
        //ROBJ cfg_dist = rif_trylookup(cfg,"distribution");
        ROBJ cfg_surv = rif_trylookup(cfg,"survival");
        ROBJ cfg_val  = rif_trylookup(cfg,"values");
        ROBJ cfg_from = rif_trylookup(cfg,"from");
        ROBJ cfg_step = rif_trylookup(cfg,"step");
        
        PiecewiseFunction v;

        if (!rif_isNull(cfg_val)) {
            v = val2val(cfg_val);
        } else { // no values given
            if (!rif_isNull(cfg_prob)) {
                v.n = rif_getLength(cfg_prob);
/*            } else if (!rif_isNull(cfg_dist)) {
                v.n = rif_getLength(cfg_dist);*/
            } else if (!rif_isNull(cfg_surv)) {
                v.n = rif_getLength(cfg_surv);
            } else {
                rif_error(cfg,"missing arguments", __LINE__);
                v.n = 0; // Unreachable
            }
            
            double from = !rif_isNull(cfg_from) ? 
                rif_asDouble(cfg_from, 0) : 0.0;
            double step = !rif_isNull(cfg_step) ? 
                rif_asDouble(cfg_step, 0) : 1.0;
                
            v.x = new double[v.n];
            v.ix = new int[v.n];
            for (unsigned int i = 0; i < v.n; i++) {
                v.x[i] = from + (double)i*step;
                v.ix[i] = (int)floor(v.x[i]);
            }
        }
       
        if (!rif_isNull(cfg_prob)) {
            df = prob2dist(cfg_prob);
/*        } else if (!rif_isNull(cfg_dist)) {
            df = dist2dist(cfg_dist);*/
        } else if (!rif_isNull(cfg_surv)) {
            df = surv2dist(cfg_surv);
        } else { 
            df.n = v.n;
            df.y = new double[df.n];
            for (unsigned int i = 0; i < df.n; i++) 
                df.y[i] = (double)(i+1)/(double)df.n;
            df.f = 1.0;
        }
        
        if (df.n != v.n) 
            rif_error(cfg,"incompatible arguments", __LINE__);
        
        df.x = v.x;
        df.ix = v.ix;
        d = new DistributionDiscrete(randcore, cfg, cond, df);
        return(d);
    } else if (stype == "linear") {
        ROBJ cfg_dens = rif_trylookup(cfg,"density");
        ROBJ cfg_val  = rif_trylookup(cfg,"values");
        ROBJ cfg_from = rif_trylookup(cfg,"from");
        ROBJ cfg_step = rif_trylookup(cfg,"step");
        
        PiecewiseFunction v;

        if (!rif_isNull(cfg_val)) {
            v = val2val(cfg_val);
        } else { // no values given
            if (!rif_isNull(cfg_dens)) {
                v.n = rif_getLength(cfg_dens);
            } else {
                rif_error(cfg,"missing arguments", __LINE__);
                v.n = 0; //Unreachable
            }
            
            double from = !rif_isNull(cfg_from) ? 
                rif_asDouble(cfg_from, 0) : 0.0;
            double step = !rif_isNull(cfg_step) ? 
                rif_asDouble(cfg_step, 0) : 1.0;
                
            v.x = new double[v.n];
            v.ix = new int[v.n];
            for (unsigned int i = 0; i < v.n; i++) {
                v.x[i] = from + (double)i*step;
                v.ix[i] = (int)floor(v.x[i]);
            }
        }
       
        if (!rif_isNull(cfg_dens)) {
            df = dens2dens(cfg_dens);
        } else { 
            df.n = v.n;
            df.y = new double[df.n];
            for (unsigned int i = 0; i < df.n; i++) 
                df.y[i] = 1.0;
        }
        
        if (df.n != v.n) 
            rif_error(cfg,"incompatible arguments", __LINE__);
        
        df.x = v.x;
        df.ix = v.ix;
        d = new DistributionContinuous(randcore, cfg, cond, df);
        return(d);
    } else if (stype == "exponential") {
        ROBJ cfg_rate = rif_trylookup(cfg,"rate");
        ROBJ cfg_shift = rif_trylookup(cfg,"shift");
        ROBJ cfg_cutat = rif_trylookup(cfg,"cutat");
        ROBJ cfg_val = rif_trylookup(cfg,"values");
        ROBJ cfg_dens = rif_trylookup(cfg,"density");
        ROBJ cfg_from = rif_trylookup(cfg,"from");
        ROBJ cfg_step = rif_trylookup(cfg,"step");
        
        if (!rif_isNull(cfg_rate)) {
            double rate = rif_asDouble(cfg_rate,0);
            double shift = !rif_isNull(cfg_shift) ?
                rif_asDouble(cfg_shift,0) : 0.0;
            if (!rif_isNull(cfg_cutat)) {
                double cutat = rif_asDouble(cfg_cutat,0);
                d = new DistributionExponential(
                    randcore,cfg,cond,shift,rate,cutat
                );
            } else {
                d = new DistributionExponential(randcore,cfg,cond,shift,rate);
            }
            return(d);
        } else if (!rif_isNull(cfg_dens)) {
            
            PiecewiseFunction v;

            if (!rif_isNull(cfg_val)) {
                v = val2val(cfg_val);
            } else { // no values given
                if (!rif_isNull(cfg_dens)) {
                    v.n = rif_getLength(cfg_dens);
                } else {
                    rif_error(cfg,"missing arguments", __LINE__);
                    v.n = 0; //Unreachable
                }
            
                double from = !rif_isNull(cfg_from) ? 
                    rif_asDouble(cfg_from, 0) : 0.0;
                double step = !rif_isNull(cfg_step) ? 
                    rif_asDouble(cfg_step, 0) : 1.0;
                
                v.x = new double[v.n];
                v.ix = new int[v.n];
                for (unsigned int i = 0; i < v.n; i++) {
                    v.x[i] = from + (double)i*step;
                    v.ix[i] = (int)floor(v.x[i]);
                }
            }
       
            if (!rif_isNull(cfg_dens)) {
                df = dens2dens(cfg_dens);
            } else { 
                df.n = v.n;
                df.y = new double[df.n];
                for (unsigned int i = 0; i < df.n; i++) 
                    df.y[i] = 1.0;
            }
        
            if (df.n != v.n) 
                rif_error(cfg,"incompatible arguments", __LINE__);
        
            df.x = v.x;
            df.ix = v.ix;
            d = new DistributionPoissonProcess(randcore, cfg, cond, df);
            return(d);
        } else {
            rif_error(cfg, 
                "must give either rate or density for exponential sampling");
        }
        
    } else if (stype == "constant") {
        ROBJ cfg_val = rif_trylookup(cfg,"values");
        ROBJ cfg_dens = rif_trylookup(cfg,"density");
        ROBJ cfg_from = rif_trylookup(cfg,"from");
        ROBJ cfg_step = rif_trylookup(cfg,"step");
        
        if (!rif_isNull(cfg_dens)) {
            
            PiecewiseFunction v;

            if (!rif_isNull(cfg_val)) {
                v = val2val(cfg_val);
            } else { // no values given
                v.n = rif_getLength(cfg_dens);
            
                double from = !rif_isNull(cfg_from) ? 
                    rif_asDouble(cfg_from, 0) : 0.0;
                double step = !rif_isNull(cfg_step) ? 
                    rif_asDouble(cfg_step, 0) : 1.0;
                
                v.x = new double[v.n];
                v.ix = new int[v.n];
                for (unsigned int i = 0; i < v.n; i++) {
                    v.x[i] = from + (double)i*step;
                    v.ix[i] = (int)floor(v.x[i]);
                }
            }
       
            df = dens2dens(cfg_dens);
        
            if (df.n != v.n) 
                rif_error(cfg,"incompatible arguments", __LINE__);
        
            df.x = v.x;
            df.ix = v.ix;
            d = new DistributionConstantContinuous(randcore, cfg, cond, df);
            return(d);
        } else {
            rif_error(cfg, 
                "must give density argument");
        }
        
    } else if (stype=="array") {
        d = new DistributionArray(cfg, creator);
        return(d);
    } else if (stype=="host") {
        if (creator->getCollector() == global::infectiontypes) {
            d = new DistributionHost(cfg, global::persontypes);
        } else {
            rif_error(cfg,"type does not have an associated host", __LINE__);
        }
        return(d);
    } else if (stype == "weibull") {
        ROBJ cfg_shift = rif_trylookup(cfg,"shift");
        ROBJ cfg_cutat = rif_trylookup(cfg,"cutat");
        ROBJ cfg_shape = rif_trylookup(cfg,"shape");
        ROBJ cfg_scale = rif_trylookup(cfg,"scale");
        
        double shift = !rif_isNull(cfg_shift) ?
            rif_asDouble(cfg_shift,0) : 0.0;
        double shape = !rif_isNull(cfg_shape) ?
            rif_asDouble(cfg_shape,0) : 1.0;
        double scale = !rif_isNull(cfg_scale) ?
            rif_asDouble(cfg_scale,0) : 1.0;
            
        if (!rif_isNull(cfg_cutat)) {
            double cutat = rif_asDouble(cfg_cutat,0);
            d = new DistributionWeibull(
                randcore,cfg,cond,shift,scale,shape,cutat
            );
        } else {
            d = new DistributionWeibull(randcore,cfg,cond,shift,scale,shape);
        }
        return(d);
    } else if (stype=="uniform") {
        double min = 0; double max = 1;
        ROBJ cfg_min = rif_trylookup(cfg,"min");
        if (!rif_isNull(cfg_min)) min = rif_asDouble(cfg_min,0);
        ROBJ cfg_max = rif_trylookup(cfg,"max");
        if (!rif_isNull(cfg_max)) max = rif_asDouble(cfg_max,0);
        d = new DistributionUniform(randcore, cfg, cond, min, max);
        return(d);
    } else rif_error(cfg, "unknown distribution type", __LINE__);
    
    error("should not come here");
    return(0); // Should never be reached
}

Distribution *_createDistribution(ROBJ cfg, const CreatorCollection *collection)
{
    Distribution *d = 0;    
    PiecewiseFunction df;
    int randcore = 0;
    ConditioningType cond = NoConditioning;
    
    ROBJ attr_randcore = getAttrib(cfg, install("randcore"));
    if (rif_isNull(attr_randcore)) {
        rif_error(cfg,"attribute 'randcore' is missing");
    } else {
        randcore = rif_asInteger(attr_randcore, 0);
    }

    // One number -> DistributionConstant
    if ((rif_isDouble(cfg) || rif_isInteger(cfg)) && rif_getLength(cfg) == 1) {
        d = new DistributionConstant(randcore, cfg, cond, rif_asDouble(cfg,0));
        return(d);
    }
    
    // An array of numbers
    if (rif_isDouble(cfg) || rif_isInteger(cfg)) {
        df = prob2dist(cfg);
        df.x = new double[df.n];
        df.ix = new int[df.n];
        for (unsigned int i = 0; i < df.n; i++) {
            df.ix[i] = i;
            df.x[i] = (double)i;
        }
        
        d = new DistributionDiscrete(randcore, cfg, cond, df);
        return(d); 
    }
    
    // A list...
    string stype;
    

    if (rif_exists(cfg,"type")) {
        stype = rif_asString(cfg, 0, "type");
    } else if (rif_exists(cfg,"rate")) {
        stype = "exponential";
    } else if (rif_exists(cfg,"density")) {
        stype = "linear";
    } else if (rif_exists(cfg,"depends")) {
        stype = "array";
    } else if (rif_exists(cfg,"host")) {
        stype = "host";
    } else {
        stype = "simple";
    }
    
    if (rif_exists(cfg, "relativeto")) {
        ROBJ cfg_relativeto = rif_lookup(cfg,"relativeto");
        string relativeto = rif_asString(cfg_relativeto, 0);
        if (relativeto == "now") {
            cond = NoConditioning;
        } else if (relativeto == "age") {
            cond = OnAge;        
        } else if (relativeto == "birth") {
            cond = OnBirth;
        } else if (relativeto == "time") {
            cond = OnTime;
        } else {
            rif_error(cfg_relativeto, "unknown conditioning type");
        }
    }
    
    if (stype == "simple") {
        ROBJ cfg_prob = rif_trylookup(cfg,"probabilities");
        //ROBJ cfg_dist = rif_trylookup(cfg,"distribution");
        ROBJ cfg_surv = rif_trylookup(cfg,"survival");
        ROBJ cfg_val  = rif_trylookup(cfg,"values");
        ROBJ cfg_from = rif_trylookup(cfg,"from");
        ROBJ cfg_step = rif_trylookup(cfg,"step");
        
        PiecewiseFunction v;

        if (!rif_isNull(cfg_val)) {
            v = val2val(cfg_val);
        } else { // no values given
            if (!rif_isNull(cfg_prob)) {
                v.n = rif_getLength(cfg_prob);
/*            } else if (!rif_isNull(cfg_dist)) {
                v.n = rif_getLength(cfg_dist);*/
            } else if (!rif_isNull(cfg_surv)) {
                v.n = rif_getLength(cfg_surv);
            } else {
                rif_error(cfg,"missing arguments", __LINE__);
                v.n = 0; // Unreachable
            }
            
            double from = !rif_isNull(cfg_from) ? 
                rif_asDouble(cfg_from, 0) : 0.0;
            double step = !rif_isNull(cfg_step) ? 
                rif_asDouble(cfg_step, 0) : 1.0;
                
            v.x = new double[v.n];
            v.ix = new int[v.n];
            for (unsigned int i = 0; i < v.n; i++) {
                v.x[i] = from + (double)i*step;
                v.ix[i] = (int)floor(v.x[i]);
            }
        }
       
        if (!rif_isNull(cfg_prob)) {
            df = prob2dist(cfg_prob);
/*        } else if (!rif_isNull(cfg_dist)) {
            df = dist2dist(cfg_dist);*/
        } else if (!rif_isNull(cfg_surv)) {
            df = surv2dist(cfg_surv);
        } else { 
            df.n = v.n;
            df.y = new double[df.n];
            for (unsigned int i = 0; i < df.n; i++) 
                df.y[i] = (double)(i+1)/(double)df.n;
            df.f = 1.0;
        }
        
        if (df.n != v.n) 
            rif_error(cfg,"incompatible arguments", __LINE__);
        
        df.x = v.x;
        df.ix = v.ix;
        d = new DistributionDiscrete(randcore, cfg, cond, df);
        return(d);
    } else if (stype == "linear") {
        ROBJ cfg_dens = rif_trylookup(cfg,"density");
        ROBJ cfg_val  = rif_trylookup(cfg,"values");
        ROBJ cfg_from = rif_trylookup(cfg,"from");
        ROBJ cfg_step = rif_trylookup(cfg,"step");
        
        PiecewiseFunction v;

        if (!rif_isNull(cfg_val)) {
            v = val2val(cfg_val);
        } else { // no values given
            if (!rif_isNull(cfg_dens)) {
                v.n = rif_getLength(cfg_dens);
            } else {
                rif_error(cfg,"missing arguments", __LINE__);
                v.n = 0; //Unreachable
            }
            
            double from = !rif_isNull(cfg_from) ? 
                rif_asDouble(cfg_from, 0) : 0.0;
            double step = !rif_isNull(cfg_step) ? 
                rif_asDouble(cfg_step, 0) : 1.0;
                
            v.x = new double[v.n];
            v.ix = new int[v.n];
            for (unsigned int i = 0; i < v.n; i++) {
                v.x[i] = from + (double)i*step;
                v.ix[i] = (int)floor(v.x[i]);
            }
        }
       
        if (!rif_isNull(cfg_dens)) {
            df = dens2dens(cfg_dens);
        } else { 
            df.n = v.n;
            df.y = new double[df.n];
            for (unsigned int i = 0; i < df.n; i++) 
                df.y[i] = 1.0;
        }
        
        if (df.n != v.n) 
            rif_error(cfg,"incompatible arguments", __LINE__);
        
        df.x = v.x;
        df.ix = v.ix;
        d = new DistributionContinuous(randcore, cfg, cond, df);
        return(d);
    } else if (stype == "exponential") {
        ROBJ cfg_rate = rif_trylookup(cfg,"rate");
        ROBJ cfg_shift = rif_trylookup(cfg,"shift");
        ROBJ cfg_cutat = rif_trylookup(cfg,"cutat");
        ROBJ cfg_val = rif_trylookup(cfg,"values");
        ROBJ cfg_dens = rif_trylookup(cfg,"density");
        ROBJ cfg_from = rif_trylookup(cfg,"from");
        ROBJ cfg_step = rif_trylookup(cfg,"step");
        
        if (!rif_isNull(cfg_rate)) {
            double rate = rif_asDouble(cfg_rate,0);
            double shift = !rif_isNull(cfg_shift) ?
                rif_asDouble(cfg_shift,0) : 0.0;
            if (!rif_isNull(cfg_cutat)) {
                double cutat = rif_asDouble(cfg_cutat,0);
                d = new DistributionExponential(
                    randcore,cfg,cond,shift,rate,cutat
                );
            } else {
                d = new DistributionExponential(randcore,cfg,cond,shift,rate);
            }
            return(d);
        } else if (!rif_isNull(cfg_dens)) {
            
            PiecewiseFunction v;

            if (!rif_isNull(cfg_val)) {
                v = val2val(cfg_val);
            } else { // no values given
                if (!rif_isNull(cfg_dens)) {
                    v.n = rif_getLength(cfg_dens);
                } else {
                    rif_error(cfg,"missing arguments", __LINE__);
                    v.n = 0; //Unreachable
                }
            
                double from = !rif_isNull(cfg_from) ? 
                    rif_asDouble(cfg_from, 0) : 0.0;
                double step = !rif_isNull(cfg_step) ? 
                    rif_asDouble(cfg_step, 0) : 1.0;
                
                v.x = new double[v.n];
                v.ix = new int[v.n];
                for (unsigned int i = 0; i < v.n; i++) {
                    v.x[i] = from + (double)i*step;
                    v.ix[i] = (int)floor(v.x[i]);
                }
            }
       
            if (!rif_isNull(cfg_dens)) {
                df = dens2dens(cfg_dens);
            } else { 
                df.n = v.n;
                df.y = new double[df.n];
                for (unsigned int i = 0; i < df.n; i++) 
                    df.y[i] = 1.0;
            }
        
            if (df.n != v.n) 
                rif_error(cfg,"incompatible arguments", __LINE__);
        
            df.x = v.x;
            df.ix = v.ix;
            d = new DistributionPoissonProcess(randcore, cfg, cond, df);
            return(d);
        } else {
            rif_error(cfg, 
                "must give either rate or density for exponential sampling");
        }
        
    } else if (stype == "constant") {
        ROBJ cfg_val = rif_trylookup(cfg,"values");
        ROBJ cfg_dens = rif_trylookup(cfg,"density");
        ROBJ cfg_from = rif_trylookup(cfg,"from");
        ROBJ cfg_step = rif_trylookup(cfg,"step");
        
        if (!rif_isNull(cfg_dens)) {
            
            PiecewiseFunction v;

            if (!rif_isNull(cfg_val)) {
                v = val2val(cfg_val);
            } else { // no values given
                v.n = rif_getLength(cfg_dens);
            
                double from = !rif_isNull(cfg_from) ? 
                    rif_asDouble(cfg_from, 0) : 0.0;
                double step = !rif_isNull(cfg_step) ? 
                    rif_asDouble(cfg_step, 0) : 1.0;
                
                v.x = new double[v.n];
                v.ix = new int[v.n];
                for (unsigned int i = 0; i < v.n; i++) {
                    v.x[i] = from + (double)i*step;
                    v.ix[i] = (int)floor(v.x[i]);
                }
            }
       
            df = dens2dens(cfg_dens);
        
            if (df.n != v.n) 
                rif_error(cfg,"incompatible arguments", __LINE__);
        
            df.x = v.x;
            df.ix = v.ix;
            d = new DistributionConstantContinuous(randcore, cfg, cond, df);
            return(d);
        } else {
            rif_error(cfg, 
                "must give density argument");
        }
        
    } else if (stype=="array") {
        d = new DistributionArray(cfg, collection);
        return(d);
    } else if (stype=="host") {
        if (collection == global::infectiontypes) {
            d = new DistributionHost(cfg, global::persontypes);
        } else {
            rif_error(cfg,"type does not have an associated host", __LINE__);
        }
        return(d);
    } else if (stype == "weibull") {
        ROBJ cfg_shift = rif_trylookup(cfg,"shift");
        ROBJ cfg_cutat = rif_trylookup(cfg,"cutat");
        ROBJ cfg_shape = rif_trylookup(cfg,"shape");
        ROBJ cfg_scale = rif_trylookup(cfg,"scale");
        
        double shift = !rif_isNull(cfg_shift) ?
            rif_asDouble(cfg_shift,0) : 0.0;
        double shape = !rif_isNull(cfg_shape) ?
            rif_asDouble(cfg_shape,0) : 1.0;
        double scale = !rif_isNull(cfg_scale) ?
            rif_asDouble(cfg_scale,0) : 1.0;
            
        if (!rif_isNull(cfg_cutat)) {
            double cutat = rif_asDouble(cfg_cutat,0);
            d = new DistributionWeibull(
                randcore,cfg,cond,shift,scale,shape,cutat
            );
        } else {
            d = new DistributionWeibull(randcore,cfg,cond,shift,scale,shape);
        }
        return(d);
    } else if (stype=="uniform") {
        double min = 0; double max = 1;
        ROBJ cfg_min = rif_trylookup(cfg,"min");
        if (!rif_isNull(cfg_min)) min = rif_asDouble(cfg_min,0);
        ROBJ cfg_max = rif_trylookup(cfg,"max");
        if (!rif_isNull(cfg_max)) max = rif_asDouble(cfg_max,0);
        d = new DistributionUniform(randcore, cfg, cond, min, max);
        return(d);
    } else rif_error(cfg, "unknown distribution type", __LINE__);
    
    error("should not come here");
    return(0); // Should never be reached
}


Distribution *createDistribution(ROBJ cfg, const Creator *creator)
{
    ROBJ cfg_dist = rif_trylookup(cfg, "distribution");
    if (!rif_isNull(cfg_dist)) return(createDistribution(cfg_dist,creator));
    
    Distribution *d = _createDistribution(cfg, creator);
//     d->print();
    return(d);
}

Distribution *createDistribution(ROBJ cfg, const CreatorCollection *collection)
{
    ROBJ cfg_dist = rif_trylookup(cfg, "distribution");
    if (!rif_isNull(cfg_dist)) return(createDistribution(cfg_dist,collection));
    
    Distribution *d = _createDistribution(cfg, collection);
//     d->print();
    return(d);
}


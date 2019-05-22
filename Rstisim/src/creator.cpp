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
#include "object.h"

#include "global.h"
#include "distribution.h"
#include "event.h"
#include "scheduler.h"

#include <sstream>

using namespace std;

//
// BEGIN IMPLEMENTATION OF CLASS Creator
// 

Creator::Creator(
    CreatorCollection *_collection, std::string _name, ROBJ _cfg,
    CreatorCollection *_bindistreferencecollector
) : Object()
{
    // store and initialise some variables
    name = _name;
    cfg = _cfg;
    collection = _collection;
    
    // Bins
    binnum = 0;
    binname = 0;
    bindist = 0;
    
    bintransnum = 0;
    bintransname = 0;
    bintransfrom = 0;
    bintransto = 0;
    bintransdist = 0;
    
    // get the bin configuration part
    ROBJ cfg_bins = rif_trylookup(cfg,"bins");
    // is there any?
    if (!rif_isNull(cfg_bins)) {
        // ...yes
        // get the bin names (error if not present)
        ROBJ cfg_binsnames = rif_lookup(cfg_bins,"names");
        // get the number of bins
        binnum = rif_getLength(cfg_binsnames);
        // create the array for the bin names and assign it
        binname = new string[binnum];
        for (int k = 0; k < binnum; k++) {
            binname[k] = rif_asString(cfg_binsnames,k);
        }
        // register at the responsible collection
        type = collection->registerCreator(this, name, binnum); 
        
        // see if there is a starting bin distribution defined...
        if (rif_exists(cfg_bins,"distribution")) {
            // ...yes, so get it
            bindist = createDistribution(
                rif_lookup(cfg_bins,"distribution"), _bindistreferencecollector
            );
            // make sure there that the values stay within the range of bins
            bindist->checkRange(0,binnum-1);
        } else {
            // ...no distribution defined, so create one that returns always 0
            ROBJ cfg_const = rif_lookup(cfg_bins,"constantdistribution");
            REAL(cfg_const)[0] = 0;
            bindist = createDistribution(cfg_const, _bindistreferencecollector);
        }
    } else {
        // ...no, so define a generic standard bin
        // just 1 bin
        binnum = 1;
        // make an array for the names
        binname = new string[binnum];
        // set the standard name
        binname[0] = "generic";
        // register this creator at its collection
        type = collection->registerCreator(this, name, binnum);
        // as there is no bin distribution for the starting bin of the ageable
        // objects, we create one, returning always 0
        ROBJ cfg_const = rif_lookup(cfg,"constantdistribution");
        REAL(cfg_const)[0] = 0;
        bindist = createDistribution(cfg_const, _bindistreferencecollector);
    }

    // read bin transitions from the configuration //
    // get the transition part
    ROBJ cfg_trans = rif_trylookup(cfg_bins, "transitions");
    // is one defines?
    if (!rif_isNull(cfg_trans)) {
        // ...yes
        // get the transition names (must be given)
        ROBJ cfg_transnames = rif_lookup(cfg_trans, "names");
        // get the number of transitions
        bintransnum = rif_getLength(cfg_transnames);
        // create the arrays to store all the information
        bintransname = new string[bintransnum];
        bintransfrom = new Bin[bintransnum];
        bintransto = new Bin[bintransnum];
        bintransdist = new Distribution*[bintransnum];            
        for(int i = 0; i < bintransnum; i++) bintransdist[i]=0; // init array
        
        // go through every transition
        for (int trans = 0; trans < bintransnum; trans++) {
            // store the name
            bintransname[trans] = rif_asString(cfg_transnames,trans);
            // get the definition in the configuration
            ROBJ cfg_thistrans = 
                rif_lookup(cfg_trans, bintransname[trans].c_str());
            // get the 'from' bin (mandatory)
            bintransfrom[trans] =           
                getBinByName(rif_asString(cfg_thistrans,0,"from"));     
            // get the 'to' bin (mandatory)
            bintransto[trans] =
                getBinByName(rif_asString(cfg_thistrans,0,"to"));     
            // get the distribution (mandatory)
            bintransdist[trans] = 
                createDistribution(rif_lookup(cfg_thistrans,"at"), this);
        }
    }
}

Creator::~Creator()
{
    if (bindist) delete bindist;
    if (binname) delete[] binname;
    if (bintransname) delete[] bintransname;
    if (bintransfrom) delete[] bintransfrom;
    if (bintransto) delete[] bintransto;
    for (Bin trans = 0; trans < bintransnum; trans++) {
        if (bintransdist && bintransdist[trans]) 
            delete bintransdist[trans];
    }
    if (bintransdist) delete[] bintransdist;
}
    
string Creator::str() const
{
    ostringstream s;
    s << "Creator:"
     << "name='" << getTypeName() << "'"
     << "basetype='" << rif_asString(cfg,0,"basetype")<< "'";
    s << "|" << Object::str();
    return (s.str());
}

Bin Creator::getBinByName(std::string _binname) const
{
    for (Bin bin = 0; bin < binnum; bin++) {
        if (binname[bin] == _binname) return(bin);
    }
    rif_error(cfg, 
        string("internal error in type '")
        + name + "': bin '"
        + _binname+"' not found", __LINE__);
    return(BINNA); // will never be reached but avoids warning when compiling
}

 
PairAttribute Creator::installPairAttribute(
    CreatorCollection *collection, std::string msg, std::string attrname, 
    Value initval, Value defaultval
) const
{
    // THIS CODE IS QUITE MESSY
    
    // initialise some stuff
    Distribution ***param = 0; // this will give a double array
    bool symmetricbytype = false;
    
    param = new Distribution**[collection->getNumberOfCreators()];
    for (int i = 0; i < collection->getNumberOfCreators(); i++) {
        param[i] = new Distribution*[collection->getTotalNumberOfBins()];
        for (int k = 0; k < collection->getTotalNumberOfBins(); k++) {
            param[i][k] = 0;
        }
    } // this makes sure that the whole double array is initialised
    
    // some output
    if (!global::keepquiet) {
    Rprintf("| Pair attribute '%s' (wrt %s): %s",attrname.c_str(),
        collection->getName().c_str(), msg.c_str());
    }
    
    // get the configuration for a constant distribution
    ROBJ defaultdist = rif_lookup(getCfg(),"constantdistribution");
    // get the configuration for the attribute (no path feature here, yet)
    ROBJ cfg = rif_trylookup(getCfg(),attrname.c_str());
    
    // check if we should throw an error because attribute is missing and no
    // default value is given
    if ((rif_isNull(cfg)) & (defaultval==DOUBLENA)) {
        // this is just to provoke an error, as no default is given
        rif_lookup(getCfg(),attrname.c_str());
    }
    
    // attribute is missing?
    if (rif_isNull(cfg)) {
        // ...yes, so the default distribution is set to the default value
        REAL(defaultdist)[0] = defaultval;
    } else {
        // ..no, so see if the attribute is just a number
        if (rif_isDouble(cfg)) {
            // ...yes, so take this as the distribution for everything
            REAL(defaultdist)[0] = rif_asDouble(cfg,0);
            // adjust the sourceline attribute if there should ever occur an
            // error where this distribution is involved
            setAttrib(
                defaultdist, install("sourceline"), 
                getAttrib(cfg, install("sourceline"))
            );
        } else {
            // ...no, so everything will be initialised with a distribution
            // using the the initval and some of these will be overridden by the
            // definitinons in the configuration file
            REAL(defaultdist)[0] = initval;
        }
    }

    // create a constant distribution for every possible combination of type /
    // type*bin    
    for (int i = 0; i < collection->getNumberOfCreators(); i++) {
        for (int k = 0; k < collection->getTotalNumberOfBins(); k++) {
            param[i][k] = createDistribution(defaultdist);
        }
    }
    
    // some output
    if (rif_isNull(cfg)) {
        if (!global::keepquiet) Rprintf(" (set to default: %f)\n",defaultval);
        return(param);
    } else if (rif_isDouble(cfg) || rif_isInteger(cfg)) {
        if (!global::keepquiet) Rprintf(" (all set to %f)\n", rif_asDouble(cfg,0));
        return(param);
    } else {
        if (!global::keepquiet) Rprintf(" (not explicitely specified values set to %f) ",initval);
    }
    
    
    // check if symmetric feature should be used
    ROBJ cfg_sym = rif_trylookup(cfg,"symmetric");
    if (!rif_isNull(cfg_sym) && rif_asString(cfg_sym,0) == "type") {  
        symmetricbytype = true;
        if (!global::keepquiet) Rprintf(" (type symmetric)");
    }
    if (!global::keepquiet) Rprintf(":\n");
    // get the names of the single definitions
    ROBJ cfg_names = rif_lookup(cfg,"names");
    // go through all the definitions
    for (int i = 0; i < rif_getLength(cfg_names); i++) {
        // store the name
        string name = rif_asString(cfg_names,i);
        if (!global::keepquiet) Rprintf("|\t'%s'\n",name.c_str());
        // get the definition
        ROBJ thiscfg = rif_lookup(cfg,name.c_str());
        // get the type of the first Ageable
        Type typefrom = collection->getCreatorTypeByName(
            thiscfg, rif_asString(thiscfg,0,"from")
        );
        // get the type or type and bin of the second Ageable
        ROBJ cfg_to = rif_lookup(thiscfg, "to");
        Type typeto;
        if (rif_isString(cfg_to)) {
            // type is directly given...
            typeto = collection->getCreatorTypeByName(
                cfg_to,rif_asString(cfg_to,0)
            );
            if (symmetricbytype && 
                collection->getCreator(typeto)->getNumberOfBins() !=
                collection->getCreator(typefrom)->getNumberOfBins())
                rif_error(thiscfg,
"symmetry not possible if number of bins are different for the types");
            int numbins = collection->getCreator(typeto)->getNumberOfBins();
            // ...so go thorugh all the bins and install the same distribution
            for (Bin b = 0; b < numbins; b++) {
                delete param[typefrom][collection->linearise(typeto,b)];
                param[typefrom][collection->linearise(typeto,b)] = 0;
                param[typefrom][collection->linearise(typeto,b)] 
                = createDistribution(
                    rif_lookup(thiscfg,"value"),
                    collection->getCreator(typefrom)
                );
                if (symmetricbytype) {
                    delete param[typeto][collection->linearise(typefrom,b)];
                    param[typeto][collection->linearise(typefrom,b)]
                    = createDistribution(
                        rif_lookup(thiscfg,"value"),
                        collection->getCreator(typeto)
                    );
                }
            }
        } else {
            // 'to' is not just a type but a type and a bin...
            typeto = collection->getCreatorTypeByName(
                cfg_to,rif_asString(cfg_to,0,"type")
            );
            if (symmetricbytype && 
                collection->getCreator(typeto)->getNumberOfBins() !=
                collection->getCreator(typefrom)->getNumberOfBins()
                ) {
                rif_error(thiscfg,
"symmetry not possible if number of bins are different for the types"
                );
            }
            Bin binto = collection->getCreator(typeto)->getBinByName(
                rif_asString(cfg_to,0,"bin")
            );
            delete param[typefrom][collection->linearise(typeto,binto)];
            param[typefrom][collection->linearise(typeto,binto)] = 0;
            param[typefrom][collection->linearise(typeto,binto)]
            = createDistribution(
                rif_lookup(thiscfg,"value"), collection->getCreator(typefrom)
            );
            if (symmetricbytype) {
                binto = collection->getCreator(typefrom)->getBinByName(
                    rif_asString(cfg_to,0,"bin"));
                delete param[typeto][collection->linearise(typefrom,binto)];
                param[typeto][collection->linearise(typefrom,binto)] = 0;
                param[typeto][collection->linearise(typefrom,binto)]
                = createDistribution(
                    rif_lookup(thiscfg,"value"),collection->getCreator(typeto)
                );
            }
        }
    }
    return(param);
}

void Creator::deletePairAttribute(
    CreatorCollection *collection, PairAttribute pairattr
) const
{
    for (int i = 0; i < collection->getNumberOfCreators(); i++) {
        for (int k = 0; k < collection->getTotalNumberOfBins(); k++) {
            if (pairattr && pairattr[i] && pairattr[i][k]) {
                delete pairattr[i][k];
            }
        }
        if (pairattr && pairattr[i]) delete[] pairattr[i];
    }
    if (pairattr) delete[] pairattr;
}

Attribute Creator::installAttribute(
    CreatorCollection *collection, std::string msg, std::string attrname,
    std::string path, Value defaultval
) const
{
    // THIS CODE IS QUITE MESSY
    
    if (path != "") attrname = path + "." + attrname;
    Attribute attr;
    ROBJ cfg_attr;
    bool settodefault = false;
    if (defaultval == DOUBLENA) {
        cfg_attr = rif_lookup(getCfg(),attrname.c_str());
    } else {
        cfg_attr = rif_trylookup(getCfg(),attrname.c_str());
        if (rif_isNull(cfg_attr)) {
            cfg_attr = rif_lookup(getCfg(),"constantdistribution");
            REAL(cfg_attr)[0] = defaultval;
            settodefault = true;
        }
    }
    // Decide whether the collector is the one that manages this Creator...
    if (collection == getCollector()) {
        // ...if so, the attribute will be evaluated directly with respect to
        // this Creator, so 'bytype' is not allowed (and not necessary for now)
        attr = collection->installAttribute(this, attrname, cfg_attr, this);
    } else {
        // ...if not, the attribute will be evaluated with respect to
        // some other CreatorCollection and 'bytype' is therefore allowed
        attr = collection->installAttribute(this, attrname.c_str(), cfg_attr);
    }
    if (!global::keepquiet) Rprintf("| Attribute '%s' (wrt %s): %s ", 
        collection->getAttributeName(attr).c_str(),        
        collection->getName().c_str(), msg.c_str()
    );
    if (collection->getAttributesIsFixedArray()[attr]) {
        if (!global::keepquiet) Rprintf("(fixed at birth");
        if (collection->getAttributesIsFixedPerBinArray()[attr]) 
            if (!global::keepquiet) Rprintf(" per bin");
        if (!global::keepquiet) Rprintf(")");
    } 
    if (settodefault) 
        if (!global::keepquiet) Rprintf(" (set to default: %f)", defaultval);
    if (!global::keepquiet) Rprintf("\n");
    return(attr);
}
 
int Creator::linearise(Bin bin) const
{
    return(collection->linearise(type, bin));
}

ROBJ Creator::getCfg() const
{
    return(cfg);
}

Type Creator::getType() const
{
    return(type);
}

std::string Creator::getTypeName() const
{
    return(name);
}

std::string Creator::getBinName(Bin bin) const
{
    return(binname[bin]);
}

 Number Creator::getNumberOfBins() const
{
    return(binnum);
};

Distribution *Creator::getBinDist() const
{
    return(bindist);
};
    
CreatorCollection *Creator::getCollector() const
{
    return(collection);
}

//
// END IMPLEMENTATION OF CLASS Creator
// 

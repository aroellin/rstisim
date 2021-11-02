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
// BEGIN IMPLEMENTATION OF CLASS CreatorCollection
//

CreatorCollection::CreatorCollection(
    ROBJ names, std::string _name, Distribution *_dist
)
: Object()
{
    // store and init some variables
    name = _name;
    len = 0;
    dist = 0;
    // ...if an optional distribution is given...
    if (_dist) {
        // ...store it
        dist = _dist;
    } else {
        // ...or create a new one always returning 0
        dist = new DistributionConstant(0, names, NoConditioning, 0);
    }
    // ...initialise the arrays
    for (Type type = 0; type < MAXTYPES; type++) {
        binnum[type] = 0;
        creators[type] = 0;
        for (Bin bin = 0; bin < MAXBINS; bin++) {
            map[type][bin] = INTNA;
        }
    }
    attrnum = 0;
    for (int i = 0; i < MAXNEWATTRIBUTES; i++) 
        attr[i] = 0;
        
    // ...get the number of types that will be installed (i.e. that will
    // register themselves later
    typenum = rif_getLength(names);
    
    // ...store the creator names
    for(Type type = 0; type < typenum; type++) {
        creatorname[type] = rif_asString(names, type);
    }
    
    // check that the distribution will not return a type that does not exist
    if (typenum > 0) {
        dist->checkRange(0,typenum-1);
    }
}

CreatorCollection::~CreatorCollection()
{
    // nothing to do; all is done by preDelete()
}

void CreatorCollection::preDelete()
{
    // delete all attached creators
    for (Type type = 0; type < typenum; type++) {
        if (creators[type]) delete creators[type];
    }
    // delete all attribute distributions
    for (int i = 0; i < attrnum; i++) {
        if (attr[i]) delete attr[i];
    }
    // delete the distribution used by for getRandomCreator()
    if (dist) delete dist;
}

Attribute CreatorCollection::installAttribute(
    const Creator *installer, std::string name, ROBJ cfg, const Creator
*creator) 
{
    // check if maximum is reached
    if (attrnum >= MAXNEWATTRIBUTES) {
        rif_error(cfg,"reached maximum number of installable attributes");
    }
    // assign a new number and increase the number of attributes
    Attribute a = attrnum++;
    
    // set the attribute name with some additional information...maybe too much
    attrname[a] = getName() + "." + installer->getTypeName() + "." + name;
    // see if this attribute is fixed at birth by bin
    attrisfixedperbin[a] = rif_exists(cfg, "fixatbirthbybin") ? true : false;
    // see if this attribute is fixed at birth 
    attrisfixed[a] = 
        attrisfixedperbin[a] || rif_exists(cfg, "fixatbirth") ? true : false;
    // see if a reference creator is given
    if (creator) {
        // if yes, evaluate with respect to that creator ('bytype' not allowed)
        attr[a] = createDistribution(cfg, creator);
    } else {
        // if not, evaluate with respect to this collection ('bytype' allowed)
        attr[a] = createDistribution(cfg, this);
    }
    return(a);
}

Type CreatorCollection::registerCreator(
    Creator *creator, std::string name, int _binnum
)
{
    // get the type of the creator from its name; if not found this method
    // throws an error
    Type type = getCreatorTypeByName(name);
    // for safety reasons make sure each creator only registers once
    if (binnum[type]>0) {
        rif_error(creator->getCfg(), "internal error: type already assigned");
    }
    // store the creator
    creators[type] = creator;
    // store the number of bins of the creator
    binnum[type] = _binnum;
    // update the map for the linearisation functionality
    for (Bin bin = 0; bin < binnum[type]; bin++) {
        map[type][bin] = len++;
    }
    
    return(type);
}

Creator *CreatorCollection::getCreatorByName(std::string name) const
{
    for (Type type = 0; type < typenum; type++) {
        if (creatorname[type] == name) return(creators[type]);
    }
    error((string("internal: could not find type '")+name+"'").c_str());
    return(0); // this is never reached but avoids a warning when compiling
}

Type CreatorCollection::getCreatorTypeByName(std::string name) const
{
    for (Type type = 0; type < typenum; type++) {
        if (creatorname[type] == name) return(type);
    }
    error((string("internal: could not find type '")+name+"'").c_str());
    return(TYPENA); // this is never reached but avoids a warning when compiling
}

Type CreatorCollection::getCreatorTypeByName(ROBJ cfg, std::string name) const
{
    for (Type type = 0; type < typenum; type++) {
        if (creatorname[type] == name) return(type);
    }
    // it's important here to use the cfg of the calling object to give the
    // user a sourceline of the error
    rif_error(cfg,(string("person type not found: ") + name).c_str());
    return(TYPENA); // this is never reached but avoids a warning when compiling
}

Creator *CreatorCollection::getRandomCreator() const 
{
    return(creators[dist->isample()]);
};

std::string CreatorCollection::getCreatorName(Type type) const
{
    if (type < 0 || type >= typenum) {
        error("internal: type out of bound");        
    } 
    return(creatorname[type]);
}

std::string CreatorCollection::str() const
{
    ostringstream s;
    s << "CreatorCollection:"
        << "name='" << getName() << "'"
        << ",#creators=" << typenum 
        << "|";
    s << Object::str();
    return(s.str());
}

std::string CreatorCollection::getName() const 
{
    return(name);
}; 
    
int CreatorCollection::linearise(Type type, Bin bin) const
{
    return(map[type][bin]);
};

Number CreatorCollection::getTotalNumberOfBins() const 
{
    return(len);
};
    
Creator *CreatorCollection::getCreator(Type type) const
{
    return(creators[type]);
};
    
Number CreatorCollection::getNumberOfCreators() const
{
    return(typenum);
}
    
Number CreatorCollection::getNumberOfAttributes() const
{
    return(attrnum);
};
        
const bool *CreatorCollection::getAttributesIsFixedArray() const
{
    return(attrisfixed);
};
        
const bool *CreatorCollection::getAttributesIsFixedPerBinArray() const
{
    return(attrisfixedperbin);
};
        
const Distribution * const *CreatorCollection::getAttributesArray() const
{
    return(attr);
};
        
const Distribution *CreatorCollection::getAttribute(Attribute a) const
{
    return(attr[a]);
};
        
std::string CreatorCollection::getAttributeName(Attribute attr) const
{
    return(attrname[attr]);
};

//
// BEGIN IMPLEMENTATION OF CLASS CreatorCollection
//




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

#include "partnership.h"

#include "global.h"
#include "rif.h"
#include "person.h"
#include "infection.h"
#include "scheduler.h"
#include "distribution.h"
#include "population.h"
#include "event.h"

using namespace std;


PSCreator::PSCreator(CreatorCollection *collection, std::string name, ROBJ cfg)
: Creator(collection, name, cfg, global::persontypes)
{
    idclass = CLASSPARTNERSHIPFORMER;
    idsubclass = CLASSPARTNERSHIPFORMERGENERIC;
    breakup = 0;
    contactcouplefactor = 0;
    unprotectedcouplefactor = 0;
    
    if (!global::keepquiet) Rprintf("Scanning partnership type %d='%s'\n",type,getTypeName().c_str());
    
    if (!global::keepquiet) Rprintf("| Basetype: '%s'\n",getBasetype().c_str());      
 
    if (!global::keepquiet) Rprintf("| Bins:\n");
    for (int k = 0; k < binnum; k++) {
        if (!global::keepquiet) Rprintf("|\t'%s'\n", binname[k].c_str());
    }

    if (!global::keepquiet) Rprintf("| Bin transitions:\n");
    for (int k = 0; k < bintransnum; k++) {
        if (!global::keepquiet) Rprintf("|\t'%s' (from '%s' to '%s')\n",
            bintransname[k].c_str(), 
            binname[bintransfrom[k]].c_str(),
            binname[bintransto[k]].c_str());
    }
    
    uniform = createDistribution(rif_lookup(cfg,"uniformdistribution"), this);
    
    breakup = installPairAttribute(global::persontypes, 
        "breakup parameters", "breakup", 0, 365);
    
    breakupfactor1 = installAttribute(global::persontypes, 
        "Breakup factor person 1", "breakupfactorperson1", "", 1);
    breakupfactor2 = installAttribute(global::persontypes, 
        "Breakup factor person 2", "breakupfactorperson2", "", 1 );
    
    
    contact = installAttribute(global::psctypes, 
        "contact distribution", "contact", "", 7);
    contactfactor = installAttribute(global::psctypes, 
        "contact factor", "contactfactor", "", 1 );
    contactcouplefactor = installPairAttribute(global::persontypes, 
        "contact factor of couple", "contactcouplefactor", 0, 1);
    contactfactorperson1 = installAttribute(global::persontypes, 
        "contact factor of person 1", "contactfactorperson1", "", 1);
    contactfactorperson2 = installAttribute(global::persontypes, 
        "contact factor of person 2", "contactfactorperson2", "", 1 );
        
    unprotected = installAttribute(global::psctypes, 
        "probability for unprotected contact", "unprotected", "", 1);
    unprotectedfactor = installAttribute(global::psctypes, 
        "unprotected factor", "unprotectedfactor", "", 1);
    unprotectedcouplefactor = installPairAttribute(global::persontypes, 
        "unprotected factor of couple", "unprotectedfactorcouple", 0, 1);
    unprotectedfactorperson1 = installAttribute(global::persontypes, 
        "unprotected factor of person 1", "unprotectedfactorperson1", "", 1);
    unprotectedfactorperson2 = installAttribute(global::persontypes, 
        "unprotected factor of person 2", "unprotectedfactorperson2", "", 1);
        
}

PSCreator::~PSCreator()
{
    deletePairAttribute(global::persontypes, breakup);
    deletePairAttribute(global::persontypes, contactcouplefactor);
    deletePairAttribute(global::persontypes, unprotectedcouplefactor);
    if (uniform) delete uniform;
}
    
Partnership *PSCreator::createPartnership(Person *p1, Person *p2, PSFInfo
psfinfo)
{
    double factor = 
        p1->getAttribute(breakupfactor1, global::abstime)
        *p2->getAttribute(breakupfactor2, global::abstime);
    double length = min(
        p1->getTimeDeath()-global::abstime,
        p2->getTimeDeath()-global::abstime
    );
    length = min(
        breakup[p1->getType()][global::persontypes->linearise(p2->getType(),
            p2->getBin())]->dsamplefac(factor,p1,global::abstime)
            ,length
    ) - 0.000001;
    if (length <= 0.0) length = 0.0000001;
    Partnership *ps = 
        new Partnership(this, p1, p2, global::abstime, length, psfinfo);    
    return(ps);
}

string PSCreator::str() const
{
    ostringstream s;
    s << "PSCreator:"
    << "name='" << getTypeName() << "'"
    << ",basetype='" << rif_asString(cfg,0,"basetype")<< "'";
    s << "|" << Creator::str();
    return (s.str());
}





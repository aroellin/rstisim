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
#include <string>

#include "distribution.h"
#include "event.h"

#include "person.h"
#include "partnership.h"
#include "infection.h"

#include "population.h"
#include "scheduler.h"

using namespace std;



InfectionCreator::InfectionCreator(CreatorCollection *collection, std::string
name, ROBJ cfg)
: Creator(collection, name, cfg, global::persontypes)
{
    idclass = CLASSINFECTIONCREATOR;
    
    uniform = 0;
    testuniform = 0;
    
    inffaccouple = 0;
    
    if (!global::keepquiet) Rprintf("Scanning infection type %d='%s'\n",type,getTypeName().c_str());

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

    uniform = createDistribution(rif_lookup(cfg,"uniformdistribution"));
    
    prevpopulating = installAttribute(global::persontypes, 
        "prevalence when populating", "prevalencewhenpopulating", "", 0.1);
    previmmigrating = installAttribute(global::persontypes, 
        "prevalence when immigrating", "prevalencewhenimmigrating", "", 0);
    immunity = installAttribute(global::infectiontypes, 
        "immunity", "immunity", "", 0);

    provokegpvisit = installAttribute(global::infectiontypes,
        "provoke seek testing", "provokegpvisit", "", MAXDOUBLE);
    provokegpvisittype = installAttribute(global::infectiontypes,
        "provoke seek general testing", "provokegpvisittype",
        "", 0);
    if (global::gpvisittypesnum > 0) {
        global::infectiontypes->getAttribute(provokegpvisittype)->
            checkRange(0,global::gpvisittypesnum-1);
    }
        
    inf = installAttribute(global::infectiontypes,
        "infectiousness", "infectiousness", "", 0.1);
    inffac = installAttribute(global::infectiontypes,
        "infectiousness factor", "infectiousnessfactor", "", 1);
    inffaccouple = installPairAttribute(global::persontypes,
        "infectiousness factor infector/susceptible",
"infectiousnessfactorcouple", 0, 1);
    inffacinfector = installAttribute(global::persontypes,
        "infectiousness factor infector", "infectiousnessfactorinfector", "",1);
    inffacsusceptible = installAttribute(global::persontypes,
        "infectiousness factor susceptible","infectiousnessfactorsusceptible","",1);
    inffacps = installAttribute(global::psctypes,
        "infectiousness factor partnership","infectiousnessfactorpartnership","",1);

    ROBJ cfg_uniform = rif_trylookup(cfg, "test");
    if (!rif_isNull(cfg_uniform)) {
        testuniform = createDistribution(
            rif_lookup(cfg_uniform,"uniformdistribution")
        );
    } else {
        testuniform = createDistribution(rif_lookup(cfg,"uniformdistribution"));
    }
    
    testsensitivity = installAttribute(global::infectiontypes, 
        "test sensitivity", "sensitivity", "test", 1);
    testspecificity = installAttribute(global::persontypes,
        "test specificity", "specificity", "test", 1);
    testwaitforresult = installAttribute(global::persontypes,
        "wait for test result", "waitforresult", "test", 1);
        

    clearedstate = getBinByName("cleared");
    treatedstate = getBinByName("treated");
}

InfectionCreator::~InfectionCreator()
{
    deletePairAttribute(global::persontypes, inffaccouple);
    if (uniform) delete uniform;
    if (testuniform) delete testuniform;    
}

bool InfectionCreator::randomlyInfectPerson(Person *person, CauseCreation why)
{
    double p = 0.0;
    switch(why) {
        case CausePopulate: {
            p = person->getAttribute(prevpopulating, global::abstime);
            break;
        }
        case CauseImmigration: {
            p = person->getAttribute(previmmigrating, global::abstime);
            break;
        }
        default: rif_error(getCfg(),
"cannot randomly infect person except when populating or immigrating");
    }
    
    if (uniform->dsample() < p) {
        global::scheduler->insert(new EventInfectPerson(
            global::abstime, this, person));
        return(true);
    } else {
        return(false);
    }
}

bool InfectionCreator::getTestResult(Person *person) const
{
    global::statstests++;
    Infection *infection = person->getInfection(type);
    Bin bin;
    if (infection 
        && (bin = infection->getBin()) != treatedstate 
        && bin != clearedstate) {
        double u = testuniform->dsample();
        double p = infection->getAttribute(testsensitivity, global::abstime);
        return(u < p);
    } else {
        double u = testuniform->dsample();
        double p = person->getAttribute(testspecificity, global::abstime);
        return(u > p);
    }
}


Time InfectionCreator::getWaitForTestResult(Person *person) const
{
    return(person->getAttribute(testwaitforresult, global::abstime));
}

void InfectionCreator::slotInfectPerson(Person *person) 
{
        Infection *d = new Infection(this, person);
        person->slotRegisterInfection(d);
}

string InfectionCreator::str() const
{
    ostringstream s;
    s << "InfectionCreator:"
    << "name='" << getTypeName() << "'"
    << ",basetype='" << rif_asString(cfg,0,"basetype")<< "'";
    s << "|" << Object::str();
    return (s.str());
}


Bin InfectionCreator::getClearedBin() const
{ 
    return(clearedstate);
};
    
Bin InfectionCreator::getTreatedBin() const
{
    return(treatedstate);
};


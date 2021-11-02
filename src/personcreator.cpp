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
#include "person.h"
#include "distribution.h"
#include <string>
#include "rif.h"
#include "constants.h"
#include "event.h"
#include "rangen.h"
#include "partnership.h"
#include "infection.h"
#include "scheduler.h"
#include "global.h"
#include "population.h"

using namespace std;




PersonCreator::PersonCreator(
    CreatorCollection *collection, std::string name, ROBJ cfg
)
: Creator(collection, name, cfg)
{
    idclass = CLASSPERSONCREATOR;
    uniform = 0;
    gpvisitsuniform = 0;
    
    if (!global::keepquiet) Rprintf(
        "Scanning people type %d='%s'\n",
        type,global::persontypes->getCreatorName(type).c_str()
    );

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
    
    lifespan = installAttribute(
        global::persontypes, "lifespan", "lifespan", "", 80*DAYSINYEAR
    );
    
    birthshift = installAttribute(
        global::persontypes, "shift at birth", "birthshift", "", 0
    );
    
    initran = installAttribute(
        global::persontypes, "initial randomisation","initialrandomisation","",0
    );

    if (rif_exists(getCfg(),"initialrandomisation")) {
        initranpresent = true;
    } else {
        initranpresent = false;
        uniform = createDistribution(
            rif_lookup(getCfg(),"uniformdistribution"),this
        );
    }
    
    gpvisitsuniform = createDistribution(
        rif_lookup(getCfg(),"uniformdistribution"),this
    );
    
    immigrage = installAttribute(
        global::persontypes, "age at immigration", "immigrationage", "", 20*365
    );
        
    visitgp = installAttribute(
        global::persontypes, "seek general testing", "visitgp",
        "", MAXDOUBLE
    );
    
    visitgpfactor = installAttribute(
        global::persontypes, "seek general testing factor", 
        "visitgpfactor", "", 1
    );

    visitgpfactor2 = installAttribute(
        global::persontypes, "seek general testing factor 2", 
        "visitgpfactor2", "", 1
    );
    
    visitgpprobability = installAttribute(
        global::persontypes, 
        "probability of actually going to gp when 'visitgp' activates a visit", 
        "visitgpprobability", "", 1
    );

    visitgpprobabilityfactor = installAttribute(
        global::persontypes, 
"probability factor of actually going to gp when 'visitgp' activates a visit", 
        "visitgpprobabilityfactor", "", 1
    );
    
    visitgpprobabilityfactor2 = installAttribute(
        global::persontypes, 
"probability factor 2 of actually going to gp when 'visitgp' activates a visit", 
        "visitgpprobabilityfactor2", "", 1
    );

    gpvisittype = installAttribute(
        global::persontypes, "seek general testing", "gpvisittype",
        "", 0
    );
    
    if (global::gpvisittypesnum>0) {
        global::persontypes->getAttribute(gpvisittype)->
            checkRange(0,global::gpvisittypesnum-1);
    } else {
      warning(
      "No 'gpvisits' specified. Simulation may crash if 'visitgp' is activated!"
      );
    } 
    
    global::peoplemaxage = max(
        global::peoplemaxage,global::persontypes->    
            getAttribute(lifespan)->getdmaxx()
    );
    
    global::peopleminage = min(
        global::peopleminage,global::persontypes->
            getAttribute(birthshift)->getdminx()
    );

}

PersonCreator::~PersonCreator()
{   
    if (uniform) delete uniform;
    if (gpvisitsuniform) delete gpvisitsuniform;
}


Person *PersonCreator::createPerson(
    CauseCreation why, PersonFemale *mother, PersonMale *father
)
{
    return(new Person(this, why, mother, father));
}

string PersonCreator::str() const
{
    ostringstream s;
    s << "PersonCreator" 
        << "|";
    s << Creator::str();
    return (s.str());
    
}

PersonCreatorMale::PersonCreatorMale(
    CreatorCollection *collection, std::string name, ROBJ cfg
)
: PersonCreator(collection, name, cfg)
{
    idsubclass = CLASSPERSONCREATORMALE;
}

PersonCreatorMale::~PersonCreatorMale()
{
}

Person *PersonCreatorMale::createPerson(
    CauseCreation why, PersonFemale *mother, PersonMale *father
)
{
    return(new PersonMale(this, why, mother, father));
}



PersonCreatorFemale::PersonCreatorFemale(
    CreatorCollection *collection, std::string name, ROBJ cfg
)
: PersonCreator(collection, name, cfg)
{
    
    idsubclass = CLASSPERSONCREATORFEMALE;
    
    uniform = 0;
    
    ROBJ cfg_pregnancy = rif_trylookup(cfg,"pregnancy");
    
    if (!rif_isNull(cfg_pregnancy)) {
        uniform = createDistribution(
            rif_lookup(cfg_pregnancy,"uniformdistribution"), this
        );
    } else {
        uniform = createDistribution(
            rif_lookup(cfg,"uniformdistribution"), this
        );
    } 
    
    pregnancygeneralrate = installAttribute(
        global::persontypes, "general rate of pregnancy", "generalrate",
        "pregnancy", MAXDOUBLE
    );
    
    pregnancygeneralfactor = installAttribute(
        global::persontypes, "general factor of pregnancy", "generalfactor",
        "pregnancy", 1
    );
    
    probpregnancypercontact = installAttribute(
        global::persontypes, "probability of pregnancy per unprotected contact",
        "probabilitypercontact", "pregnancy", 0
    ); 
    probpregnancypercontactfactor = installAttribute(
        global::persontypes,
        "probability factor of pregnancy per unprotected contact",
        "probabilitypercontactfactor", "pregnancy", 1
    );
    pregnancyduration = installAttribute(
        global::persontypes, "duration of pregnancy", "duration",
        "pregnancy", 260
    );
;
}

PersonCreatorFemale::~PersonCreatorFemale()
{
    if (uniform) delete uniform;
}

Person *PersonCreatorFemale::createPerson(
    CauseCreation why, PersonFemale *mother, PersonMale *father
)
{
    return(new PersonFemale(this, why, mother, father));
}


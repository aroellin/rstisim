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
#include "distribution.h"

#include "rif.h"

#include "constants.h"
#include "person.h"
#include "partnership.h"
#include "infection.h"
#include "gpvisitcreator.h"
#include "notifier.h"

#include <iostream>

using namespace std;

void personPreInitVars()
{
    ROBJ cfg_people = rif_trylookup(global::cfg,"model.population.people");
    if (rif_isNull(cfg_people)) {
        error("missing people types definition (model.population.people)");
    }
    
    ROBJ cfg_persontypenames = rif_lookup(cfg_people,"names");
    
    global::persontypes = new CreatorCollection(
        cfg_persontypenames, "person",
        createDistribution(rif_lookup(cfg_people,"distribution")));
    global::persontypesnum = global::persontypes->getNumberOfCreators();
}


void personInitVars()
{
    if (!global::keepquiet) Rprintf("READING PERSON TYPES\n");

    ROBJ cfg_people = rif_lookup(global::cfg,"model.population.people");
    ROBJ cfg_persontypenames = rif_lookup(cfg_people,"names");
    ROBJ cfg = rif_lookup(global::cfg,"model.people");
    
    for (Type type = 0; type < global::persontypesnum; type++) {
        string name = rif_asString(cfg_persontypenames, type);
        ROBJ cfg_type = rif_lookup(cfg,name.c_str());
        ROBJ cfg_basetype = rif_trylookup(cfg_type,"basetype");
        string basetype =   
                rif_isNull(cfg_basetype) ? "GENERIC" : rif_asString(cfg_basetype);
        if (basetype=="GENERIC") {
            new PersonCreator(global::persontypes, name, cfg_type);
        } else if (basetype=="MALE") {
            new PersonCreatorMale(global::persontypes, name, cfg_type);
        } else if (basetype=="FEMALE") {
            new PersonCreatorFemale(global::persontypes, name, cfg_type);
        } else {
            rif_error(cfg_basetype,"unknown basetype");
        }
       
if (!global::keepquiet) Rprintf("+------------------------------------------------------------\n");
    };
    
    if (!global::keepquiet) Rprintf(
"READ %d PERSON\
TYPES\n*************************************************************\n",
        global::persontypesnum);

}

void personPreDelVars()
{
    if (global::persontypes) {
        global::persontypes->preDelete();
    }
}

void personDelVars()
{
    if (global::persontypes) { 
        delete global::persontypes; 
        global::persontypes = 0; 
    }
}




void partnershipPreInitVars()
{
    ROBJ cfg_names = rif_trylookup(
        global::cfg, "model.population.partnershipformers.names"
    );
    
    
    global::psftypes = new CreatorCollection(cfg_names, "partnership former");
    global::psftypesnum = global::psftypes->getNumberOfCreators();
    
    cfg_names = rif_trylookup(
         global::cfg, "model.population.partnerships.names"
    );
    global::psctypes = new CreatorCollection(cfg_names, "partnership");
    global::psctypesnum = global::psctypes->getNumberOfCreators();
}

void partnershipInitVars()
{
    if (!global::keepquiet) Rprintf("READING PARTNERSHIP FORMERS\n");
    
    ROBJ cfg_names = rif_trylookup(
        global::cfg, "model.population.partnershipformers.names"
    );
    ROBJ cfg = rif_trylookup(global::cfg, "model.partnershipformers");
 
    for (int psf = 0; psf < global::psftypesnum; psf++) {
        string name = rif_asString(cfg_names,psf);
        ROBJ thiscfg = rif_lookup(cfg, name.c_str());
        ROBJ cfg_basetype = rif_trylookup(thiscfg,"basetype");
        string basetype =   
              rif_isNull(cfg_basetype) ? "INDIVIDUALSEARCH" : rif_asString(cfg_basetype);
        if (basetype=="INDIVIDUALSEARCH") {
            new PSFormerIndivSearch(global::psftypes, name, thiscfg);
        } else {
            rif_error(cfg_basetype,"unknown basetype");
        }                
       
if (!global::keepquiet) Rprintf("+------------------------------------------------------------\n");
    }
    
    if (!global::keepquiet) Rprintf(
"READ %d PARTNERSHIP\
FORMERS\n*************************************************************\n",
        global::psftypesnum);
    
    
    if (!global::keepquiet) Rprintf("READING PARTNERSHIP TYPES\n");
    
    cfg_names = rif_trylookup(
        global::cfg, "model.population.partnerships.names"
    );
    cfg = rif_trylookup(global::cfg, "model.partnerships");
    
    for (int psc = 0; psc < global::psctypesnum; psc++) {
        string name = rif_asString(cfg_names,psc);
        ROBJ thiscfg = rif_lookup(cfg, name.c_str());
        ROBJ cfg_basetype = rif_trylookup(thiscfg,"basetype");
        string basetype =   
                rif_isNull(cfg_basetype) ? "GENERIC" : rif_asString(cfg_basetype);
        if (basetype=="GENERIC") {
            new PSCreator(global::psctypes, name, thiscfg);
        } else {
            rif_error(cfg_basetype,"unknown basetype");
        }                
       
if (!global::keepquiet) Rprintf("+------------------------------------------------------------\n");
    }
    
    if (!global::keepquiet) Rprintf(
"READ %d PARTNERSHIP\
TYPES\n*************************************************************\n",
        global::psctypesnum);
    
}

void partnershipPreDelVars()
{
    if (global::psftypes) global::psftypes->preDelete();
    if (global::psctypes) global::psctypes->preDelete();
}

void partnershipDelVars()
{
    if (global::psftypes) { 
        delete global::psftypes; 
        global::psftypes = 0;
    }
    if (global::psctypes) { 
        delete global::psctypes; 
        global::psctypes = 0;
    }
}






void infectionPreInitVars()
{
    ROBJ cfg_names = rif_trylookup(
        global::cfg, "model.population.infections.names"
    );
    global::infectiontypes = new CreatorCollection(cfg_names, "infections");
    global::infectiontypesnum = global::infectiontypes->getNumberOfCreators();
}

void infectionInitVars()
{
    if (!global::keepquiet) Rprintf("READING INFECTION TYPES\n");
    
    ROBJ cfg_names = rif_trylookup(
        global::cfg, "model.population.infections.names"
    );    
    ROBJ cfg = rif_trylookup(global::cfg, "model.infections");
    
    for (int infection = 0; infection < global::infectiontypesnum; infection++)
{
        string name = rif_asString(cfg_names,infection);
        ROBJ thiscfg = rif_lookup(cfg, name.c_str());
        ROBJ cfg_basetype = rif_trylookup(thiscfg,"basetype");
        string basetype =   
                rif_isNull(cfg_basetype) ? "GENERIC" : rif_asString(cfg_basetype);
        if (basetype=="GENERIC") {
            new InfectionCreator(global::infectiontypes, name, thiscfg);
        } else {
            rif_error(cfg_basetype,"unknown basetype");
        }                
       
if (!global::keepquiet) Rprintf("+------------------------------------------------------------\n");
    }
    
        if (!global::keepquiet) Rprintf(
"READ %d INFECTION\
TYPES\n*************************************************************\n",
            global::infectiontypesnum);

}

void infectionPreDelVars()
{
    if (global::infectiontypes) global::infectiontypes->preDelete();
}

void infectionDelVars()
{
    if (global::infectiontypes) {
        delete global::infectiontypes;
        global::infectiontypes = 0;
    }
}








void gpvisitPreInitVars()
{
    ROBJ cfg_names = rif_trylookup(
        global::cfg, "model.population.gpvisits.names"
    );
    global::gpvisittypes = new CreatorCollection(cfg_names, "gpvisits");
    global::gpvisittypesnum = global::gpvisittypes->getNumberOfCreators();
}

void gpvisitInitVars()
{
    if (!global::keepquiet) Rprintf("READING GPVISIT TYPES\n");
    
    ROBJ cfg_names = rif_trylookup(
        global::cfg, "model.population.gpvisits.names"
    );    
    ROBJ cfg = rif_trylookup(global::cfg, "model.gpvisits");
    
    for (int gpvisit = 0; gpvisit < global::gpvisittypesnum; gpvisit++)
    {
        string name = rif_asString(cfg_names,gpvisit);
        ROBJ thiscfg = rif_lookup(cfg, name.c_str());
        ROBJ cfg_basetype = rif_trylookup(thiscfg,"basetype");
        string basetype =   
                rif_isNull(cfg_basetype) ? "GENERIC" : rif_asString(cfg_basetype);
        if (basetype=="GENERIC") {
            new GPVisitCreator(global::gpvisittypes, name, thiscfg);
        } else {
            rif_error(cfg_basetype,"unknown basetype");
        }                
       
if (!global::keepquiet) Rprintf("+------------------------------------------------------------\n");
    }
    
        if (!global::keepquiet) Rprintf(
"READ %d GPVISITS\
TYPES\n*************************************************************\n",
            global::gpvisittypesnum);

}

void gpvisitPreDelVars()
{
    if (global::gpvisittypes) global::gpvisittypes->preDelete();
}

void gpvisitDelVars()
{
    if (global::gpvisittypes) {
        delete global::gpvisittypes;
        global::gpvisittypes = 0;
    }
}








void notifyPreInitVars()
{
    ROBJ cfg_names = rif_trylookup(
        global::cfg, "model.population.partnernotification.names"
    );
    global::notifiertypes = new CreatorCollection(
        cfg_names, "partnernotification"
    );
    global::notifiertypesnum = global::notifiertypes->getNumberOfCreators();
}

void notifyInitVars()
{
    if (!global::keepquiet) Rprintf("READING PARTNER NOTIFICATION TYPES \n");
    
    ROBJ cfg_names = rif_trylookup(
        global::cfg, "model.population.partnernotification.names"
    );    
    ROBJ cfg = rif_trylookup(global::cfg, "model.partnernotification");
    
    for (int notifier = 0; notifier < global::notifiertypesnum; notifier++)
{
        string name = rif_asString(cfg_names,notifier);
        ROBJ thiscfg = rif_lookup(cfg, name.c_str());
        ROBJ cfg_basetype = rif_trylookup(thiscfg,"basetype");
        string basetype =   
                rif_isNull(cfg_basetype) ? "GENERIC" : rif_asString(cfg_basetype);
        if (basetype=="GENERIC") {
            new Notifier(global::notifiertypes, name, thiscfg);
        } else {
            rif_error(cfg_basetype,"unknown basetype");
        }                
       
if (!global::keepquiet) Rprintf("+------------------------------------------------------------\n");
    }
    
        if (!global::keepquiet) Rprintf(
"READ %d PARTNER NOTIFICATION TYPES\n\
*************************************************************\n",
            global::notifiertypesnum);

}

void notifyPreDelVars()
{
    if (global::notifiertypes) global::notifiertypes->preDelete();
}

void notifyDelVars()
{
    if (global::notifiertypes) {
        delete global::notifiertypes;
        global::notifiertypes = 0;
    }
}





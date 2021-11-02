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
#include "gpvisitcreator.h"
#include "distribution.h"
#include "infection.h"
#include "person.h"
#include "notifier.h"

#include <iostream>

using namespace std;

GPVisitCreator::GPVisitCreator(
    CreatorCollection *collection, std::string name, ROBJ cfg
)
: Creator(collection, name, cfg)
{
    // set the class id
    idclass = CLASSGPVISITCREATOR;

    // Initialize some variables;
    uniform = 0;
    typeistarget = new bool[global::infectiontypesnum];
    
    if (!global::keepquiet) Rprintf(
        "Scanning gpvisit type %d='%s'\n",
        type,global::gpvisittypes->getCreatorName(type).c_str()
    );

    if (!global::keepquiet) Rprintf("| Basetype: '%s'\n",getBasetype().c_str());      
     
    
    ROBJ cfg_active = rif_lookup(cfg,"targets");
    for (int i = 0; i < global::infectiontypesnum; i++) 
        typeistarget[i]=false;
    for (int i = 0; i < rif_getLength(cfg_active); i++) {
        string name = rif_asString(cfg_active,i);
        Type infectiontype =
            global::infectiontypes->getCreatorTypeByName(cfg_active,name);
        typeistarget[infectiontype] = true;
    }
        
    if (!global::keepquiet) Rprintf("| Infection types that are treated:\n");
    for (int i = 0; i < global::infectiontypesnum; i++) {
        if (typeistarget[i]) {
        if (!global::keepquiet) Rprintf(
            "|\t'%s'\n", global::infectiontypes->getCreatorName(i).c_str()
        );
        }
    }
    
    uniform = createDistribution(rif_lookup(cfg,"uniformdistribution"));

    probdirtreatall = installAttribute(global::persontypes,
        "probability direct treatment overall",
        "probabilitydirecttreatment", "", 0);
        
    probdirtreatspec = installAttribute(global::persontypes,
        "probability direct treatment specific",
        "probabilitydirecttreatmentspecific", "", 0);

    probtestall = installAttribute(global::persontypes,
        "probability for testing overall",
        "probabilitytesting", "", 1);
        
    probtestspec = installAttribute(global::persontypes,
        "probability for testing specific",
        "probabilitytestingspecific", "", 0);

    probobeyall = installAttribute(global::persontypes,
        "probability for obeying overall",
        "probabilityobey", "", 1);
        
    probobeyspec = installAttribute(global::persontypes,
        "probability for obeying specific",
        "probabilityobeyspecific", "", 0);

    probnotify = installAttribute(global::persontypes,
        "probability of notifying partners",
        "probabilitynotifypartners", "", 0);
         
    notifiertype = installAttribute(global::persontypes,
        "which notifier should be used",
        "partnernotifiertype", "", 0);
    if (global::notifiertypesnum > 0) {
        global::persontypes->getAttribute(notifiertype)
            ->checkRange(0,global::notifiertypesnum-1);
    }

    probnotify2 = installAttribute(global::persontypes,
        "probability of notifying partners (2)",
        "probabilitynotifypartners2", "", 0);
         
    notifiertype2 = installAttribute(global::persontypes,
        "which notifier should be used (2)",
        "partnernotifiertype2", "", 0);
    if (global::notifiertypesnum > 0) {
        global::persontypes->getAttribute(notifiertype2)
            ->checkRange(0,global::notifiertypesnum-1);
    }

    probnotify3 = installAttribute(global::persontypes,
        "probability of notifying partners (3)",
        "probabilitynotifypartners3", "", 0);
         
    notifiertype3 = installAttribute(global::persontypes,
        "which notifier should be used (3)",
        "partnernotifiertype3", "", 0);
    if (global::notifiertypesnum > 0) {
        global::persontypes->getAttribute(notifiertype3)
            ->checkRange(0,global::notifiertypesnum-1);
    }


}


GPVisitCreator::~GPVisitCreator()
{
    if (typeistarget) delete[] typeistarget;
    if (uniform) delete uniform;
}


SingleGPVisit GPVisitCreator::makeVisit(Person *person, Notification notif)
{
    double p,u;
    bool dirtreated[global::infectiontypesnum];
    bool dotest[global::infectiontypesnum];
    bool result[global::infectiontypesnum];
    bool waitforresult[global::infectiontypesnum];
    for (Type i = 0; i < global::infectiontypesnum; i++) {
        dirtreated[i] = false;
        dotest[i] = false;
        result[i] = false;
        waitforresult[i] = 0;
    }

    // If the linknumber is = 0, then this is index case, so create new NID
    if (notif.linknumber == 0) {
        notif.nid = ++global::counternotifid;
    }
    
    SingleGPVisit gpv;
    gpv.gptype = getType();
    gpv.nid = notif.nid;
    gpv.linknumber = notif.linknumber;
    gpv.time = global::abstime;
    gpv.ntype = TYPENA; // will be set later if notification is started
    gpv.puid = person->puid;
    gpv.ptype = person->getType();
    gpv.pbin = person->getBin();
    
    // Set the linknumber of the person
    person->setLinkNumber(notif.linknumber);
    
    // Should be treated directly overall?
    p = person->getAttribute(probdirtreatall);
    u = uniform->dsample();
    if (u < p) {
        for (Type i = 0; i < global::infectiontypesnum; i++) {
            if (typeistarget[i]) {
                person->slotTreat(i);   
                dirtreated[i] = true;
            }
        }
        
    } else {
        for (Type i = 0; i < global::infectiontypesnum; i++) {
            if (typeistarget[i]) {
                p = person->getAttribute(probdirtreatspec);
                u = uniform->dsample();
                if (u < p) {
                    person->slotTreat(i);
                    dirtreated[i] = true;
                }
            }
        }        
    }
    
    // So, eventual direct treatment done, check whether should do a test
    p = person->getAttribute(probtestall);
    u = uniform->dsample();
    if (u < p) {
        for (Type i = 0; i < global::infectiontypesnum; i++) {
            if (typeistarget[i] && !dirtreated[i]) {
                dotest[i] = true;
            }
        }
        
    } else {
        for (Type i = 0; i < global::infectiontypesnum; i++) {
            if (typeistarget[i]) {
                p = person->getAttribute(probtestspec);
                u = uniform->dsample();
                if (u < p) {
                    dotest[i] = true;
                }
            }
        }        
    }
    
    // get results and waiting times
    for (Type i = 0; i < global::infectiontypesnum; i++) {
        if (dotest[i]) {
            InfectionCreator *ic =
                (InfectionCreator *)global::infectiontypes->getCreator(i);
            result[i] = ic->getTestResult(person);
            if (result[i]) {
                person->slotTestedPositive(i);   
            }
            waitforresult[i] = ic->getWaitForTestResult(person);
        }
    }        

    // if person obeys, install treatment events
    p = person->getAttribute(probobeyall);
    u = uniform->dsample();
    if (u < p) {
        for (Type i = 0; i < global::infectiontypesnum; i++) {
            if (result[i]) {
                person->slotTreat(i,waitforresult[i]);
            }
        }                
    } else {
        for (Type i = 0; i < global::infectiontypesnum; i++) {
            if (result[i]) {
                p = person->getAttribute(probobeyspec);
                u = uniform->dsample();
                if (u < p) {
                    person->slotTreat(i,waitforresult[i]);
                }
            }
        }                
    }
    
    
    // partnernotification
    if (global::gpvisittypesnum>0) {
        person->slotNotificationStarts(notif);
        p = person->getAttribute(probnotify);
        u = uniform->dsample();
        if (u < p) {
            gpv.ntype = (int)floor(person->getAttribute(notifiertype));
            Notifier *nc = 
                (Notifier*)(global::notifiertypes->getCreator(gpv.ntype));
            nc->notifyPartners(person, notif);
        }
        p = person->getAttribute(probnotify2);
        u = uniform->dsample();
        if (u < p) {
            gpv.ntype = (int)floor(person->getAttribute(notifiertype2));
            Notifier *nc = 
                (Notifier*)(global::notifiertypes->getCreator(gpv.ntype));
            nc->notifyPartners(person, notif);
        }
        p = person->getAttribute(probnotify3);
        u = uniform->dsample();
        if (u < p) {
            gpv.ntype = (int)floor(person->getAttribute(notifiertype3));
            Notifier *nc = 
                (Notifier*)(global::notifiertypes->getCreator(gpv.ntype));
            nc->notifyPartners(person, notif);
        }
    }
    
    gpv.dirtreated = 0;
    gpv.tested = 0;
    gpv.posresults = 0;
    
    for (Type i = 0; i < global::infectiontypesnum; i++) {
        if (dirtreated[i]) gpv.dirtreated++;
        if (dotest[i]) gpv.tested++;
        if (result[i]) gpv.posresults++;
    }
   
    return(gpv);
}

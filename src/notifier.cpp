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
#include "typedefs.h"
#include "notifier.h"
#include "distribution.h"
#include "person.h"
#include "partnership.h"
#include <iostream>
using namespace std;

Notifier::Notifier(CreatorCollection *collection, std::string name, ROBJ cfg)
 : Creator(collection, name, cfg)
{
    idclass = CLASSNOTIFIER;
    
    uniform = 0;
    probabilitycouple = 0;
    waitreaction = 0;
    
    if (!global::keepquiet) Rprintf(
        "Scanning partner notifier type %d='%s'\n",
        type, global::notifiertypes->getCreatorName(type).c_str()
    );
    
    if (!global::keepquiet) Rprintf("| Basetype: '%s'\n",getBasetype().c_str());      
     
    uniform = createDistribution(rif_lookup(cfg,"uniformdistribution"));

    useorforgoback = installAttribute(global::persontypes,
        "the probability of using 'or' instead of 'and' to stop the 'goback' procedure", 
        "probabilitygobackuseor", "", 0
    );
    
    gobacktime = installAttribute(global::persontypes,
        "how far in time should the notification go", "gobacktime", "", 0
    );
    
    gobackpartners = installAttribute(global::persontypes,
        "how many partners back should the notification go", 
        "gobackpartners", "", 0
    );
    
    probabilitycouple = installPairAttribute(global::persontypes,
        "probability that treatment is notified from parter to partner",
        "probabilitycouple", 0, 1);
        
    probabilityfactorsender = installAttribute(global::persontypes, 
        "probability factor of sender of notifying",
        "probabilityfactorsender", "", 1
    );
    
    probabilityfactorreceiver = installAttribute(global::persontypes, 
        "probability factor of receiver being notified",
        "probabilityfactorreceiver", "", 1
    );
    
    probabilityfactorpartnership = installAttribute(global::psctypes, 
            "probability factor of partnership letting notification",
            "probabilityfactorpartnership", "", 1 
    );
    
    waitreaction = installPairAttribute(global::persontypes,
        "how long to wait until partner goes to GP", "waitforreactioncouple",
        MAXDOUBLE, 0);

    waitfacsender = installAttribute(global::persontypes,
        "how long to wait factor sender", "waitforreactionfactorsender", "", 1);
    
    waitfacreceiver = installAttribute(global::persontypes,
       "how long to wait factor receiver","waitforreactionfactorreceiver","",1);
        
    gpvisittype = installAttribute(global::persontypes,
        "type to take for next GPVisit", "gpvisittype", "",0);
   
}


Notifier::~Notifier()
{
    if (probabilitycouple) {
        deletePairAttribute(global::persontypes, probabilitycouple);
    }
    if (waitreaction) {
        deletePairAttribute(global::persontypes, waitreaction);
    }
}

void Notifier::notifyPartners(Person *person, Notification notif)
{
    if (notif.linknumber && !notif.nid) {
        rif_error(getCfg(), "internal: linknumber >0 but nid not set");
    }

    Notification newnotif = notif;
    newnotif.linknumber = notif.linknumber+1;

    // This is for the partners; the link number of the person is already 
    // set at the GP visit


        
    double puseor = person->getAttribute(useorforgoback);
    double howmuch = person->getAttribute(gobacktime);
    int howmany = floor(person->getAttribute(gobackpartners));
   
    const PartnershipList *ps = person->getPartnerships();
    for(PartnershipList::const_iterator it = ps->begin(); 
        it != ps->end(); it++) {
        Person *partner = (*it)->getPartner(person);
        if (!partner->isNotifiedAlready(notif)) {
            partner->setLinkNumber(newnotif.linknumber);
            double facsender = person->getAttribute(probabilityfactorsender);
            double facreceiver =  partner->getAttribute(probabilityfactorreceiver);
            double facpartnership = (*it)->getAttribute(probabilityfactorpartnership);
            double prob = probabilitycouple[person->getType()]
                [partner->getBinLinearised()]->dsample(person, global::abstime);
            prob *= facsender*facreceiver*facpartnership;
            double u = uniform->dsample();
            if (u < prob) {
                double fac = 
                    person->getAttribute(waitfacsender)
                    *partner->getAttribute(waitfacreceiver);
                Time wait = waitreaction[person->getType()]
                    [partner->getBinLinearised()]->dsamplefac(
                        fac,person,global::abstime
                    );
                Type gpv = (int)floor(partner->getAttribute(gpvisittype));
    /*            cout << "(active) ";*/
                person->slotNotifyingPartner(partner, *it, this, notif);
                partner->slotVisitGPNotified(gpv, wait, newnotif);
            }
        }
    }
    
    ps = person->getPartnershipsOld();
    
    int countps = 0;
    
    double uuseor = uniform->dsample();
    
    for(PartnershipList::const_iterator it = ps->begin(); 
        it != ps->end(); it++) {
        countps++;
        /* check if partnership complies neither with gobacktime 
         nor gobackpartners */
        if (
            ( (uuseor > puseor) 
              && ((countps > howmany) 
                  && ((*it)->getTimeDeath() < global::abstime - howmuch)) 
            ) 
            || 
            (
              (uuseor <= puseor)          
              && ((countps > howmany) 
                  || ((*it)->getTimeDeath() < global::abstime - howmuch))
            )
           ) 
        {
            break; // we can stop here, because if this ps does not comply    
                   // neither won't earlier ones
        }
                
        Person *partner = (*it)->getPartner(person);
        if (!partner->isNotifiedAlready(notif)) {        
            partner->setLinkNumber(newnotif.linknumber);
            double facsender = person->getAttribute(probabilityfactorsender);
            double facreceiver =  partner->getAttribute(probabilityfactorreceiver);
            double facpartnership = (*it)->getAttribute(probabilityfactorpartnership);
            double prob = probabilitycouple[person->getType()]
                [partner->getBinLinearised()]->dsample(person, global::abstime);
            prob *= facsender*facreceiver*facpartnership;
            double u = uniform->dsample();
            if (u < prob) {
                double fac = 
                    person->getAttribute(waitfacsender)
                    *partner->getAttribute(waitfacreceiver);
                Time wait = waitreaction[person->getType()]
                    [partner->getBinLinearised()]->dsamplefac(
                        fac,person,global::abstime
                    );
                Type gpv = (int)floor(partner->getAttribute(gpvisittype));
                person->slotNotifyingPartner(partner, *it, this, notif);
                partner->slotVisitGPNotified(gpv, wait, newnotif);
            }
        }
    }
}


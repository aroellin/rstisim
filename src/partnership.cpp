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


Partnership::Partnership(
    PSCreator *_psc, 
    Person  *_p1, Person *_p2, 
    double _start, double _breakup, 
    PSFInfo _psfinfo, Bin _bin)
 : Ageable(_psc)
{
    idclass = CLASSPARTNERSHIP;
    idsubclass = CLASSPARTNERSHIPGENERIC;
    psuid = ++global::counterpsuid;
    contacts = 0;
    unprotectedcontacts = 0;
    p1 = _p1;
    p2 = _p2;
    bin = _bin >= 0 ? _bin : creator->getBinDist()->isample(p1,global::abstime);
    timebirth = _start;
    timedeath = _start+_breakup;
    psfinfo = _psfinfo;
    psid = global::population->slotRegisterPartnership(this);
    p1->slotRegisterPartnership(this);
    p2->slotRegisterPartnership(this);
    throwEventDeath();
    throwEventHaveSex();
    throwEventBinChange();
}


Partnership::~Partnership()
{
}

Number Partnership::getNumber(NumberOf what) const
{ 
    switch(what) {
        case NumberOfContacts: 
 	    return(contacts);
        case NumberOfContactsUnprotected: 
            return(unprotectedcontacts);
	case IsActive : 
            return(isAlive());
        default: return(Ageable::getNumber(what));
    }
    
};

void Partnership::slotDeath() 
{   
    Ageable::slotDeath();
    p1->slotDeregisterPartnership(this);
    p2->slotDeregisterPartnership(this);    
    psid = global::population->slotDeregisterPartnership(psid);
}

void Partnership::slotHaveSex(bool unprotected) 
{
    contacts++;
    global::statscontacts++;
    if (unprotected) { unprotectedcontacts++; global::statscontactsunprot++; }
    const InfectionList *dl = p1->getInfections();
    for (InfectionList::const_iterator it = dl->begin(); 
        it != dl->end(); it++) {
        (*it)->slotTryToProgress(p2, this);   
    }
    dl = p2->getInfections();
    for (InfectionList::const_iterator it = dl->begin(); 
        it != dl->end(); it++) {
        (*it)->slotTryToProgress(p1, this);   
    }
    p1->slotNotifyHaveContact(this, unprotected);
    p2->slotNotifyHaveContact(this, unprotected);
    throwEventHaveSex();
}

void Partnership::throwEventBinChange(bool conditional)
{
    Ageable::throwEventBinChange(conditional);
}

void Partnership::slotBinChange(Bin from, Bin to)
{
    Ageable::slotBinChange(from, to);
    global::statspartnershipbinchanges++;
    throwEventHaveSex(true);
}

void Partnership::throwEventHaveSex(bool conditional)
{
    Time time;
    
    double factor = getAttribute(
        ((PSCreator*)creator)->contactfactor, global::abstime
    );
    Type p1type = p1->getType();
    int p2index = global::persontypes->linearise(p2->getType(),p2->getBin());
    
    double factor2 = ((PSCreator*)creator)->contactcouplefactor[p1type][p2index]
        ->dsample(p1,global::abstime);
    factor *= factor2;
    
    double factor3 = p1->getAttribute(
        ((PSCreator*)creator)->contactfactorperson1, global::abstime
    );
    factor *= factor3;
    
    double factor4 = p2->getAttribute(
        ((PSCreator*)creator)->contactfactorperson2, global::abstime
    );
    factor *= factor4;
    
    if (conditional) {
        Time timediff = global::abstime - procHaveSex.lastTime();
        if (timediff < 0) {
            rif_error(creator->getCfg(),
"internal: cannot sample conditional having sex if last sex is later than now"
            );
        }
        time = global::abstime 
                + getAttributeFac(((PSCreator*)creator)->contact,
                    procHaveSex.lastTime(), factor, timediff);
    } else {
        time = global::abstime 
                + getAttributeFac(((PSCreator*)creator)->contact,
                    global::abstime, factor);
    }
    
    if (time < getTimeDeath()) {
        // Sample whether unprotected
        double prob = getAttribute(
            ((PSCreator*)creator)->unprotected, global::abstime
        );
        prob *= getAttribute(
            ((PSCreator*)creator)->unprotectedfactor, global::abstime
        );
        prob *= ((PSCreator*)creator)->unprotectedcouplefactor[p1type][p2index]
                ->dsample(p1,global::abstime);
        prob *= p1->getAttribute(
            ((PSCreator*)creator)->unprotectedfactorperson1, global::abstime
        );
        prob *= p2->getAttribute(
            ((PSCreator*)creator)->unprotectedfactorperson2, global::abstime
        );
        
        if (((PSCreator*)creator)->uniform->dsample(this, global::abstime)<prob)
        {
            procHaveSex.replace(new EventHaveSex(time, this, true));
        } else {
            procHaveSex.replace(new EventHaveSex(time, this, false));
        }
        
    } else {
        procHaveSex.clear();
    }
}

string Partnership::str() const
{
    ostringstream s;
    s << "Partnership:"
    << "type='" << creator->getTypeName() << "'"
    << ",from=" << p1->puid
    << ",to=" << p2->puid;
    s << "|" << Ageable::str();
    return (s.str());
}






PartnershipID Partnership::getPartnershipID() const
{
    return(psid);
}    
    
Person *Partnership::getPerson1() const
{
    return(p1);
}
    
Person *Partnership::getPerson2() const
{
    return(p2);
}


Person *Partnership::getPartner(Person *p) const
{ 
    if (p==p1) {
        return(p2);
    } else if (p==p2) {
        return(p1);
    } else {
        rif_error(creator->getCfg(),
            "internal, 'getPartner': argument is not part of partnership"
        );
        return(0);
    }
}
        
PSFInfo Partnership::getPSFInfo() const
{
    return(psfinfo);
}
 


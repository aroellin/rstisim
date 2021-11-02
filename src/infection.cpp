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



Infection::Infection(InfectionCreator *creator, Person *_host, 
    const Infection *_parent, const Partnership *_ps)
 : Ageable(creator)
{
    idclass = CLASSINFECTION;
    idsubclass = CLASSINFECTIONGENERIC;
    infuid = ++global::counterinfuid;
    
    host = _host;
    parent = _parent;
    ps = _ps;
    
    timeofimmunity = MAXDOUBLE;
    
    if (parent) {
        strainid = parent->getStrainID();
    } else {
        strainid = ++global::counterstrainid;
    }
    
    timebirth = global::abstime;
    timedeath = MAXDOUBLE;

    bin = creator->getBinDist()->isample(host,global::abstime);
    
    throwEventBinChange();
    throwEventProvokeGPVisit(CauseSymptomsSpecific);
    throwEventProvokeGPVisit(CauseSymptomsGeneral);
}


Infection::~Infection()
{
}

bool Infection::isInfectious() const
{
    Bin clearedstate = ((InfectionCreator*)creator)->clearedstate;
    Bin treatedstate = ((InfectionCreator*)creator)->treatedstate;
    if (bin == clearedstate || bin == treatedstate) {
        return(false);
    } else {
        return(true);
    }
}

Number Infection::getNumber(NumberOf what) const
{ 
    switch(what) {
        case IsActive :
            return(isInfectious());
        default: 
            return(Ageable::getNumber(what));
    }; 
}

Person * Infection::getHost() const 
{ 
    return(host);
};

const Infection * Infection::getParent() const
{
    return(parent);
};

void Infection::throwEventBinChange(bool conditional)
{
    if (bin != ((InfectionCreator*)creator)->clearedstate 
        && bin != ((InfectionCreator*)creator)->treatedstate) {
        Ageable::throwEventBinChange(conditional);
    }
    if (procBinChange.nextTime() >= host->getTimeDeath()) { 
        procBinChange.clear();
    }
}

void Infection::throwEventProvokeGPVisit(bool conditional)
{
    Time time;
    if (conditional) {
        time = global::abstime + getAttribute(
            ((InfectionCreator*)creator)->provokegpvisit, 
            global::abstime,global::abstime-procProvokeGPVisit.lastTime()
        );
    } else {
        time = global::abstime + getAttribute(
            ((InfectionCreator*)creator)->provokegpvisit
        );
    }
    if (time < host->getTimeDeath()) {
        procProvokeGPVisit.replace(new EventProvokeGPVisit(time, this));
    } else {
        procProvokeGPVisit.clear();
    }
}

Time Infection::getTimeOfImmunity()
{
    return(timeofimmunity);
}

void Infection::slotBinChange(Bin from, Bin to)
{
    Bin clearedstate = ((InfectionCreator*)creator)->clearedstate;
    Bin treatedstate = ((InfectionCreator*)creator)->treatedstate;
    
    Ageable::slotBinChange(from,to);
    global::statsinfectionbinchanges++;
    
    host->slotInfectionChangedState(this,from,to);
    if (to == clearedstate || to == treatedstate) {
        // Delete any possible transition out of the these two special states
        timeofimmunity = global::abstime;
        
        procBinChange.clear();
        procProvokeGPVisit.clear();
        
        if (to == ((InfectionCreator*)creator)->clearedstate) {
            global::statsclearances++;
        } else {
            // global::statstreatments++; this is now counted in 
            // Person::slotTreat
        }
        Time immunity =
getAttribute(((InfectionCreator*)creator)->immunity,global::abstime);
        timedeath = global::abstime + immunity;

        throwEventDeath();
    } else {
        throwEventProvokeGPVisit(true);
    }
}

void Infection::slotClear()
{
    Bin treatedstate = ((InfectionCreator*)creator)->treatedstate;
    Bin clearedstate = ((InfectionCreator*)creator)->clearedstate;
    if (bin != treatedstate && bin != treatedstate)
        slotBinChange(bin, clearedstate);
}

void Infection::slotTreated()
{
    Bin treatedstate = ((InfectionCreator*)creator)->treatedstate;
    Bin clearedstate = ((InfectionCreator*)creator)->clearedstate;
    if (bin != treatedstate && bin != clearedstate)
        slotBinChange(bin, treatedstate);
}

void Infection::slotProvokeGPVisit()
{
    procProvokeGPVisit.setLastTime();
    Type gpt =(int)floor(
        getAttribute(((InfectionCreator*)creator)->provokegpvisittype)
    );
    host->slotVisitGP(gpt, CauseSymptomsSpecific);
    throwEventProvokeGPVisit();
}


void Infection::slotDeath()
{
    procProvokeGPVisit.clear();
  
    host->slotDeregisterInfection(this);
    
    Ageable::slotDeath();
    
}

void Infection::throwEventDeath(bool conditional)
{
    Ageable::throwEventDeath(conditional);
}

void Infection::slotHostDies()
{
    slotDeath();
}

Infection *Infection::clone(Person *newhost, const Partnership *_ps) const
{
    return(new Infection((InfectionCreator*)(
        getCreator()), newhost, this, _ps
    ));
}


void Infection::slotTryToProgress(Person *victim, const Partnership *ps) 
{
    Bin treatedstate = ((InfectionCreator*)creator)->treatedstate;
    Bin clearedstate = ((InfectionCreator*)creator)->clearedstate;
    if (bin == treatedstate || bin == clearedstate) return;
    
    double factor = getAttribute(
        ((InfectionCreator*)creator)->inffac, global::abstime
    );
    factor *= ((InfectionCreator*)creator)->inffaccouple
        [host->getType()]
        [victim->getBinLinearised()]
        ->dsample(host,global::abstime);
    factor *= host->getAttribute(
        ((InfectionCreator*)creator)->inffacinfector,global::abstime
    );
    factor *= victim->getAttribute(
        ((InfectionCreator*)creator)->inffacsusceptible,global::abstime
    );
    factor *= ps->getAttribute(
        ((InfectionCreator*)creator)->inffacps,global::abstime
    );
    
    double p = factor * getAttribute(
        ((InfectionCreator*)creator)->inf, global::abstime
    );
    if (((InfectionCreator*)creator)->uniform->dsample() < p) {
        victim->slotInfect(this, ps);
    }
}

string Infection::str() const
{
    ostringstream s;
    s << "Infection:"
     << "infuid=" << infuid 
     << ",hostpuid=" << host->puid
     << ",parentinfuid=" << (parent ? parent->infuid : 0);
    s << "|" << Ageable::str();
    return (s.str());
   
}


const Partnership *Infection::getPartnership() const
{ 
    return(ps); 
}

void Infection::clearParent()
{ 
    parent = 0;
}

void Infection::clearPartnership() 
{
    ps = 0;
};

Counter Infection::getStrainID() const
{
    return(strainid);
};


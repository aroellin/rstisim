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
#include "event.h"
#include "person.h"
#include "partnership.h"
#include "infection.h"
#include "scheduler.h"
#include "population.h"

using namespace std;

// 
// BEGIN IMPLEMENTATION OF BASE CLASS Event
// 

Event::Event(double _time)
{
    // update and set the unique event ID
    euid = ++global::countereuid;    
    // set class and subclass ID
    idclass = CLASSEVENT;
    idsubclass = CLASSEVENTGENERIC;
    // store the time
    time = _time;
    // set to active by default
    active = true;
    // this event has not been executed yet
    executed = false;
    // store the time of creation
    timecreated = global::abstime;
}

Event::~Event()
{
    // nothing to do
}

void Event::execute()
{
    // update the global time
    global::abstime = time;
    // to make sure this event is not executed again, set to inactive
    active = false;
    // mark as executed
    executed = true;
    // update the global statistics
    global::statsevents++;
}

string Event::str() const 
{
    ostringstream s;
    s << "Event:"
        << "at=" << time
        << ",active=" << active
        << ",timecreated=" << timecreated
        << ",euid=" << euid;
    return (s.str());
}

void Event::print() const
{
    Rprintf("%s\n", str().c_str());
}

//
// END IMPLEMENTATION OF BASE CLASS Event
//


// 
// BEGIN IMPLEMENTATION OF CLASS EventDeath
// 

EventDeath::EventDeath(Time time, Ageable *_target) 
// Call the constructor of the superclass
: Event(time) 
{
    // set the subclass ID
    idsubclass = CLASSEVENTAGEABLEDEATH;
    // store variables
    target = _target;
}

EventDeath::~EventDeath()
{
    // nothing to do
}

void EventDeath::execute()
{
    // call superclass method
    Event::execute();
    // inform target ageable about death
    target->slotDeath();
}

string EventDeath::str() const
{
    ostringstream s;
    s << "EventDeath:"
        << "targetuid=" << target->uid
        << ",class='" << target->getCreator()->getCollector()->getName() << "'"
        << ",type='" << target->getCreator()->getTypeName() << "'"
        << "|";
    s << Event::str();
    return (s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventDeath
// 

// 
// BEGIN IMPLEMENTATION OF CLASS EventImmigration
// 

EventImmigration::EventImmigration(Time time)
// Call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTIMMIGRATION;
}

EventImmigration::~EventImmigration()
{
    // nothing to do
}

void EventImmigration::execute()
{
    // call superclass method
    Event::execute();
    // inform populatio object about immigration
    global::population->slotImmigration();
}

string EventImmigration::str() const
{
    ostringstream s;
    s << "EventImmigration"
        << "|";
    s << Event::str();
    return (s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventImmigration
// 

// 
// BEGIN IMPLEMENTATION OF CLASS EventBirth
// 

EventBirth::EventBirth(
    Time time, CauseCreation _why, PersonFemale *_mother, PersonMale *_father
)
// call superuser constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTBIRTH;
    // store variables
    why = _why;
    mother = _mother;
    father = _father;
}

EventBirth::~EventBirth()
{
}

void EventBirth::execute()
{
    // call the superclass method
    Event::execute();
    // if a mother is available inform her about birth
    if (mother) {
        mother->slotBabyIsBorn(father); 
        // (if 'father' is not 0 he will be informed by the mother)
    }
    // inform the population object about birth; the object will be created
    // there and not by the mother 
    global::population->slotBirth(why, mother, father);
}

string EventBirth::str() const
{
    ostringstream s;
    s << "EventBirth:"
        << "motherpuid=" << (mother ? mother->puid : 0)
        << ",fatherpuid=" << (father ? father->puid : 0)
        << "|";
    s << Event::str();
    return (s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventBirth
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventGetPregnant
// 

EventGetPregnant::EventGetPregnant(
    Time time, PersonFemale *_mother, PersonMale *_father
)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTGETPREGNANT;
    // store variables
    mother = _mother;
    father = _father;
}

EventGetPregnant::~EventGetPregnant()
{
    // nothing to do
}

void EventGetPregnant::execute()
{
    // call superclass method
    Event::execute();
    // inform mother that she gets pregnant and from whom
    mother->slotGetPregnant(father);
}

string EventGetPregnant::str() const
{
    ostringstream s;
    s << "EventGetPregnant:"
        << "motherpuid=" << (mother ? mother->puid : 0)
        << ",fatherpuid=" << (father ? father->puid : 0)
        << "|";
    s << Event::str();
    return (s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventGetPregnant
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventAbortion
// 

EventAbortion::EventAbortion(
    Time time, CauseAbortion _why, PersonFemale *_mother, PersonMale *_father
)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTABORTION;
    // store variables
    why = _why;
    mother = _mother;
    father = _father;
}

EventAbortion::~EventAbortion()
{
}

void EventAbortion::execute()
{
    // call the superclass method
    Event::execute();
    // inform mother about abortion
    mother->slotAbortion(why);
}

string EventAbortion::str() const
{
    ostringstream s;
    s << "EventAbortion:"
        << "motherpuid=" << (mother ? mother->puid : 0)
        << ",fatherpuid=" << (father ? father->puid : 0)
        << ",cause=" << why
        << "|";
    s << Event::str();
    return (s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventAbortion
// 

// 
// BEGIN IMPLEMENTATION OF CLASS EventBinChange
// 

EventBinChange::EventBinChange(
    Time time, Ageable *_target, Bin _from, Bin _to
)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTAGEABLEBINCHANGE;
    // store variables
    target = _target;
    from = _from;
    to = _to;
}

EventBinChange::~EventBinChange()
{
}

void EventBinChange::execute()
{
    // call the superclass method
    Event::execute();
    // inform target about bin change
    target->slotBinChange(from, to);
}

string EventBinChange::str() const
{
    ostringstream s;
    s << "EventBinChange:"
        << "targetuid=" << target->uid
        << ",class='" << target->getCreator()->getCollector()->getName() << "'"
        << ",type='" << target->getCreator()->getTypeName() << "'"
        << ",from='"
    << target->getCreator()->getBinName(from)
        << "'"
        << ",to='" 
    << target->getCreator()->getBinName(to)
        << "'"
        << "|";
    s << Event::str();
    return (s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventBinChange
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventPSInitiate
// 

EventPSInitiate::EventPSInitiate(Time time, 
    PSFormerIndivSearch *_psf, Person *_person)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTPSINITIATE;
    // store variables
    psf = _psf;
    person = _person;
}

EventPSInitiate::~EventPSInitiate()
{
}
    
void EventPSInitiate::execute()
{
    // call the superclass method
    Event::execute();
    // inform partnership creator that this person wants to form a partnership
    psf->slotPSInitiate(person);
}

std::string EventPSInitiate::str() const
{
    ostringstream s;
    s << "EventPSInitiate:"
        << "psformer='" << global::psftypes->getCreatorName(psf->getType())
            << "'"
        << ",puid=" << person->puid << "'";
    s << "|" << Event::str();
    return (s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventPSInitiate
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventHaveSex
// 

EventHaveSex::EventHaveSex(Time time, Partnership *_ps, bool _unprotected)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTHAVESEX;
    // store variables
    ps = _ps;
    unprotected = _unprotected;
}

EventHaveSex::~EventHaveSex()
{
}

void EventHaveSex::execute() 
{
    // call the superclass method
    Event::execute();
    // inform partnership about sexual contact, also whether unprotected or not
    ps->slotHaveSex(unprotected);
}

string EventHaveSex::str() const
{
    ostringstream s;
    s << "EventHaveSex:"
        << "psuid=" << ps->psuid
        << ",unprot=" << (unprotected ? "yes" : "no");
    s << "|" << Event::str();
    return(s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventHaveSex
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventRemoveOld
// 

EventRemoveOld::EventRemoveOld(Time time, EnumRemove _what)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTREMOVEOLD;
    // stor variables
    what = _what;
}

EventRemoveOld::~EventRemoveOld()
{  
    // nothing to dp
}

void EventRemoveOld::execute() 
{
    // call the superclass method
    Event::execute();
    // check what type of objects should be removed
    switch(what) {
        case RemovePartnerships: {
            // tell population object to remove old partnerships
            global::population->internalRemoveOldPartnerships();
            // install next event partnership removal
            global::scheduler->insert(
                new EventRemoveOld(
                    global::abstime+global::removepsolderthan, what
                )
            );
            break;
        }
        case RemoveInfections: {
            // tell population object to remove old infections
            global::population->internalRemoveOldInfections();
            // install next event for infection removal
            global::scheduler->insert(
                new EventRemoveOld(
                    global::abstime+global::removeinfolderthan, what
                )
            );
            break;
        }
    }
}

string EventRemoveOld::str() const
{
    ostringstream s;
    s << "EventRemoveOld:"
        << "what=" << what;
    s << "|" << Event::str();
    return(s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventRemoveOld
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventVisitGP
// 

EventVisitGP::EventVisitGP(
    Time time, Person *_person, Type _gpvisittype, Notification _notif,
    enum CauseVisitGP _cause
)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTVISITGP;
    
    // store variables
    person = _person;
    gpvisittype = _gpvisittype;
    notif = _notif;
    cause = _cause;
}

EventVisitGP::~EventVisitGP()
{
    // nothing to do
}
    
void EventVisitGP::execute()
{
    // call the superclass method
    Event::execute();
    person->slotVisitGP(gpvisittype, cause, notif);
}

std::string EventVisitGP::str() const
{
    ostringstream s;
    s << "EventVisitGP:"
      << ",puid=" << person->puid
      << ",gpvisittype='" 
        << global::gpvisittypes->getCreatorName(gpvisittype) << "'";
    s << "|" << Event::str();
    return(s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventVisitGP
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventProvokeGPVisit
// 

EventProvokeGPVisit::EventProvokeGPVisit(
    Time time, Infection *_infection
)
// call superclass constructor
: Event(time)
{   
    // set subclass ID
    idsubclass = CLASSEVENTPROVOKEVISITGP;
    // store variables
    infection = _infection;
}

EventProvokeGPVisit::~EventProvokeGPVisit()
{
}
    
void EventProvokeGPVisit::execute()
{
    // call the superclass method
    Event::execute();
    // tell infection to provoke seek for treatment in its host
    infection->slotProvokeGPVisit();
}

std::string EventProvokeGPVisit::str() const
{
    ostringstream s;
    s << "EventProvokeGPVisit:"
        << "infuid=" << infection->infuid
        << ",hostpuid=" << ((Person*)(infection->getHost()))->puid
        << ",infectiontype='" << infection->getCreator()->getTypeName() << "'";
    s << "|" << Event::str();
    return(s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventProvokeGPVisit
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventTreat
// 

EventTreat::EventTreat(
    Time time, Person *_person, Type _infectiontype
)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTTREAT;
    
    // store variables
    person = _person;
    infectiontype = _infectiontype;
}

EventTreat::~EventTreat()
{
    // nothing to do
}
    
void EventTreat::execute()
{
    // call the superclass method
    Event::execute();
    // tell person to treat; if treatment is specific we also need to pass the
    // infection type; otherwise this argument will not be used
    person->slotTreat(infectiontype);
}

std::string EventTreat::str() const
{
    ostringstream s;
    s << "EventTreat:"
        << ",puid=" << person->puid;
    if (infectiontype != TYPENA) {
        s << ",infectiontype='" 
            << global::infectiontypes->getCreatorName(infectiontype) << "'";
    }
    s << "|" << Event::str();
    return(s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventTreat
// 


// 
// BEGIN IMPLEMENTATION OF CLASS EventInfectPerson
// 

EventInfectPerson::EventInfectPerson(
    Time time, InfectionCreator *_inftype, Person *_person
)
// call superclass constructor
: Event(time)
{
    // set subclass ID
    idsubclass = CLASSEVENTINFECTPERSON;
    // store variables
    inftype = _inftype;
    person = _person;
}

EventInfectPerson::~EventInfectPerson()
{
    // nothing to do
}

void EventInfectPerson::execute() 
{
    // call the superclass method
    Event::execute();
    // inform the infection creator about infection
    inftype->slotInfectPerson(person);
}

string EventInfectPerson::str() const
{
    ostringstream s;
    s << "EventInfectPerson:"
        << "infection='" << inftype->getTypeName() << "'"
        << ",puid=" << person->uid;
    s << "|" << Event::str();
    return(s.str());
}

// 
// END IMPLEMENTATION OF CLASS EventInfectPerson
// 

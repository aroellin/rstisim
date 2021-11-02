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
#include "gpvisitcreator.h"
#include "notifier.h"

using namespace std;

//
// BEGIN IMPLEMENTATION OF CLASS Person
//

Person::Person(PersonCreator *pc, CauseCreation why, PersonFemale *_mother,
PersonMale *_father)
: Ageable(pc)
{
    idclass = CLASSPERSON;
    idsubclass = CLASSPERSONGENERIC;
    puid = ++global::counterpuid;
    causeCreated = why;
    mother = _mother;
    father = _father;
    partnerscurrent = 0;
    partnerstotal = 0;
    contacts = 0;
    contactsunprot = 0;
    infectionscurrent = 0;
    infectionstotal = 0;
    gpvisits = 0;
    notifications = 0;
    alreadynotified = 0;
    treatments = 0;
    positivetests = 0;
    procSpecTreatment = 0;
    
    // Sample time birth and age
    double lifespan = 0.0;
    
    switch(why) {
        case CauseBirth: {
            double birthshift = 
                getAttribute(pc->birthshift, global::abstime);
            do {
                lifespan = birthshift + getAttribute(
                    pc->lifespan, global::abstime, birthshift
                );
            } while (lifespan == birthshift);
            timebirth = timecreated - birthshift;
            break;        
        }
        case CausePopulate: {
            double birthshift = getAttribute(
                pc->birthshift, global::abstime);
            do {
                lifespan = birthshift + getAttribute(
                    pc->lifespan, global::abstime, birthshift
                );
            } while (lifespan == birthshift);
            double initran = pc->initranpresent ?
                getAttribute(pc->initran, global::abstime) :
                pc->uniform->dsample();
            timebirth = timecreated - birthshift;
            timebirth -= initran*(lifespan-birthshift);
            break;
        }
        case CauseImmigration: {
            double immigrationage = getAttribute(
                pc->immigrage, global::abstime
            );
            timebirth = timecreated - immigrationage;
            do {
                lifespan = immigrationage + getAttribute(
                    pc->lifespan, global::abstime, immigrationage
                );
            } while (lifespan == immigrationage);
            break;
        }
        default: error("unknown creation type");
    }
    
    timedeath = timebirth + lifespan;
    if (timedeath == timebirth) {
        error("timedeath == timebirth");
    }
    
    // Need to register here to get a popid
    popid = global::population->slotRegisterPerson(this,causeCreated);
    
    // Install relevant events
    throwEventDeath();

    // Cannot just throw throwEventBinChange(), we have to simulate from the
    // time of
    // birth
    // first bin is already sampled by Ageable-class constructor
    double nexttranstime = timebirth;
    procBinChange.setLastTime(timebirth);
    BinChange bc;
    while(1) {
        bc = sampleBinChange(bin, procBinChange.lastTime()); 
/*        cout << puid << "(" << nexttranstime/365 << "): " << bc.from << ", "
<< bc.to <<            ", "   << bc.reltime/365 << endl;*/
        // No further transition possible?
        if (bc.to == BINNA) break; 
        // Next transition is in the future?
        if ((nexttranstime += bc.reltime) > global::abstime) break; 
        procBinChange.setLastTime(nexttranstime);
        bin = bc.to;
    } 
    if (bc.to != BINNA && nexttranstime < timedeath) {
        procBinChange.replace(
            new EventBinChange(nexttranstime, this, bin, bc.to));
    } else {
        // No transition at all or next transition beyond death
    }
  
    // Register for Partnership Formation
    global::population->slotRegisterForPSFormation(this);
    
    // Infection related stuff
  

    procSpecTreatment = new Process[global::infectiontypesnum];
    throwEventVisitGP();
}

Person::~Person()
{
    for (InfectionList::iterator it = infections.begin();
        it != infections.end(); it++) {
        delete (*it);   
    }
    for (InfectionList::iterator it = infectionsold.begin();
        it != infectionsold.end(); it++) {
//         cout << *it << endl;
//         (*it)->print();
        delete (*it);   
    }
    
    delete[] procSpecTreatment;
}

PopID Person::getPopID() const
{ 
    return(popid);
}

void Person::setPopID(PopID _popid) 
{
    popid = _popid;
}

void Person::setLinkNumber(Number _linknumber)
{
    linknumber = _linknumber;
}

const InfectionList *Person::getInfections() const
{
    return(&infections);
}
        
const InfectionList *Person::getInfectionsOld() const
{
    return(&infectionsold);
}

const PartnershipList *Person::getPartnerships() const
{
    return(&partnerships);
}

const PartnershipList *Person::getPartnershipsOld() const
{
    return(&partnershipsold);
}

const SingleNotificationList *Person::getSingleNotifications() const
{
    return(&singlenotiflist);
}

const SingleGPVisitList *Person::getSingleGPVisits() const
{
    return(&singlegpvisitlist);
}

Number Person::getNumber(NumberOf what) const
{ 
    switch(what) {
        case NumberOfTreatments :
            return(treatments);
        case NumberOfPartnersCurrent: 
            return(partnerscurrent);
        case NumberOfPartnersCurrentType0:
        case NumberOfPartnersCurrentType1:
        case NumberOfPartnersCurrentType2:
        case NumberOfPartnersCurrentType3:
        case NumberOfPartnersCurrentType4: {
            Type pstype;
            switch(what) {
                case NumberOfPartnersCurrentType0: {
                    pstype = 0; break;
                }
                case NumberOfPartnersCurrentType1: {
                    pstype = 1; break;
                }
                case NumberOfPartnersCurrentType2: {
                    pstype = 2; break;
                }
                case NumberOfPartnersCurrentType3: {
                    pstype = 3; break;
                }
                case NumberOfPartnersCurrentType4: {
                    pstype = 4; break;
                }
                default : pstype = 0;
            }
            Number n = 0;
            for(PartnershipList::const_iterator it = partnerships.begin();
                it != partnerships.end(); it++) {
                if ((*it)->getType() == pstype) {
                    n++;
                }
            }
            return(n);
        }
        case NumberOfPartnersTotal:
            return(partnerstotal);
        case NumberOfPartnersWithin: {
            Number n = partnerscurrent;
            for(PartnershipList::const_iterator it = partnershipsold.begin();
                it != partnershipsold.end(); it++) {
                if ((global::abstime-(*it)->getTimeDeath())
                      <global::withintimelag)
                  n++;
            }
            return(n);
        }
        case NumberOfContacts :
            return(contacts);
        case NumberOfContactsUnprotected : 
            return(contactsunprot);
        case NumberOfInfectionsCurrent :
            return(infectionscurrent);
        case NumberOfInfectionsWithin : {
            Number n = infectionscurrent;
            for(InfectionList::const_iterator it = infectionsold.begin();
                it != infectionsold.end(); it++) {
                if (global::abstime-(*it)->getTimeDeath()<global::withintimelag) n++;
            }
            return(n);
        }
        case NumberOfInfectionsTotal :
            return(infectionstotal);
        case NumberOfLink : 
            return(linknumber);
        case NumberOfGPVisits :
            return(gpvisits);
        case NumberOfNotifications :
            return(notifications);
        case NumberOfAlreadyNotified : 
            return(alreadynotified);
        case NumberOfPositiveTests :
            return(positivetests);
        default: 
            return(Ageable::getNumber(what));
    }; 
}

void Person::slotDeath()
{
    // deregister from partner formation
    global::population->slotDeregisterFromPSFormation(this);
    
    // check if there are still active partnerships present
    if (partnerscurrent>0) 
        rif_error(creator->getCfg(),
            string("active partnerships present when person dies: ")+str());
     
    // inform all the infections that the host dies; this is a bit tricky: 
    // we eed to make a copy of list 'infections' because "slotHostDies" will
    // call "slotDeregisterInfection" and that method manipulates the infection
    // list, which would mess up the loop below.
     
    InfectionList dl(infections);
                  
    for (InfectionList::iterator it = dl.begin(); 
        it != dl.end(); it++) {
        if (*it) {
            (*it)->slotHostDies();
        } else {
            error("empty infection in list 'infections' (3)");
        }
    }
    // clear any treatment appointments
    for (int i = 0; i < global::infectiontypesnum; i++) {
        procSpecTreatment[i].clear();
    }
    // deregister from the population and get new popid; this must be at the
    // end because popid may be used by functions above
    popid = global::population->slotDeregisterPerson(popid, CauseDeath);

    // call superclass method
    Ageable::slotDeath();

}

void Person::slotTreat(Type infectiontype, Time wait)
{
    if (infectiontype == TYPENA) {
        rif_error(creator->getCfg(), 
            "internal: cannot treat specifically without infection given"
        );
    }
    
    if (wait < 0) {
        rif_error(creator->getCfg(), 
            "internal: cannot set treatment event into the past"
        );
    }
        
    if (wait == 0) {
        procSpecTreatment[infectiontype].clear();
        Infection * infection = getInfection(infectiontype);
        if (infection) {           
            infection->slotTreated();
        } else {
            // infection already cleared (probably naturaly)
            // so don't call infection->slotTreat
            // or the treament was because of a false positive test result
            global::statstreatmentsvain++;
        }
        // Increase the two counters even if infection was cleared
        // naturalybetween testing and treament or not present 
        treatments++; 
        global::statstreatments++;
    } else {
        if (procSpecTreatment[infectiontype].empty()) {
            procSpecTreatment[infectiontype].replace(new EventTreat(
                global::abstime + wait, this, infectiontype
            ));
        } else {
            // do nothing, as a treatment for this infection type is already
            // scheduled
        }
    }
}

bool Person::isNotifiedAlready(Notification notif) {
    for (NotificationList::iterator it = notiflist.begin();
        it != notiflist.end(); it++) {
        if ((*it).nid == notif.nid) {
            return(true);   
        }
    }
    return(false);
}

void Person::slotNotificationStarts(Notification notif)
{
    alreadynotified = 0;
    if (!isNotifiedAlready(notif)) {
        notiflist.push_front(notif);
        notifications++;
    }    
}    
   
void Person::slotNotifyingPartner(
    Person *partner, Partnership *ps, Notifier *notifier, Notification notif)
{
    SingleNotification sn;
    
    sn.ntype = notifier->getType();
    sn.nid = notif.nid;
    sn.linknumber = notif.linknumber;
    sn.time = global::abstime;

    sn.puid1 = puid;
    sn.ptype1 = getType();
    sn.pbin1 = getBinLinearised();

    sn.puid2 = partner->puid;
    sn.ptype2 = partner->getType();
    sn.pbin2 = partner->getBinLinearised();

    sn.psuid = ps->psuid;
    sn.pstype = ps->getType(); 
    sn.psbin = ps->getBinLinearised();
    
    singlenotiflist.push_front(sn);

    alreadynotified++;
}

void Person::slotVisitGPNotified(
    Type gpvisittype, Time wait, Notification notif
)
{
    
    if (isNotifiedAlready(notif)) return;
    
    // no, so store
    notiflist.push_front(notif);
    notifications++;
    
    // set next event
    global::statsfollowupvisits++;
    procVisitGP.update(new EventVisitGP(
        global::abstime + wait, this, gpvisittype, notif, CauseNotified
    ));
    
    // ToDo throw event to visit gp
}


void Person::slotVisitGP(
    Type gpvisittype, CauseVisitGP cvgp, Notification notif
)
{
  if (cvgp==CauseSymptomsGeneral) {
    double prob = getAttribute(
                    ((PersonCreator*)getCreator())->visitgpprobability, 
                     global::abstime) *
                  getAttribute(
                    ((PersonCreator*)getCreator())->visitgpprobabilityfactor,
                     global::abstime) *
                  getAttribute(
                    ((PersonCreator*)getCreator())->visitgpprobabilityfactor2,
                     global::abstime
                  );
    
    if (((PersonCreator*)(getCreator()))->gpvisitsuniform->dsample()<prob) {
      positivetests = 0;   
      GPVisitCreator *gc = 
          (GPVisitCreator*)(global::gpvisittypes->getCreator(gpvisittype));
      SingleGPVisit gpv = gc->makeVisit(this, notif);
      gpv.cause = cvgp;
      singlegpvisitlist.push_front(gpv);
      gpvisits++;
    }
    throwEventVisitGP();
  } else if (cvgp==CauseSymptomsSpecific) {
    positivetests = 0;
    GPVisitCreator *gc = 
        (GPVisitCreator*)(global::gpvisittypes->getCreator(gpvisittype));
    SingleGPVisit gpv = gc->makeVisit(this, notif);
    gpv.cause = cvgp;
    singlegpvisitlist.push_front(gpv);
    gpvisits++;
  } else if (cvgp==CauseNotified) {
    positivetests = 0;
    GPVisitCreator *gc = 
        (GPVisitCreator*)(global::gpvisittypes->getCreator(gpvisittype));
    SingleGPVisit gpv = gc->makeVisit(this, notif);
    gpv.cause = cvgp;
    singlegpvisitlist.push_front(gpv);
    gpvisits++;
    throwEventVisitGP();
  } else {
    rif_error(creator->getCfg(),"internal: unknown CauseVisitGP in Person::slotVisitGP");
  }
}
    
void Person::slotTestedPositive(Type infectiontype) 
{
    positivetests++;   
}

void Person::throwEventVisitGP(bool conditional)
{
    Time time;
    Type gpt = (int)floor(getAttribute(((PersonCreator*)creator)->gpvisittype));
    
    double facvisitgp = getAttribute(
                            ((PersonCreator*)getCreator())->visitgpfactor,
                            global::abstime
                        ) *
                        getAttribute(
                            ((PersonCreator*)getCreator())->visitgpfactor2,
                            global::abstime
                        ); 
    if (conditional) {
        time = global::abstime + getAttributeFac(
            ((PersonCreator*)getCreator())->visitgp, 
            global::abstime, facvisitgp, global::abstime-procVisitGP.lastTime()
        );
        if (time < getTimeDeath()) { 
          procVisitGP.update(new EventVisitGP(time, this, gpt));
        } else {
          procVisitGP.clear();
        }
    } else {
        time = global::abstime + getAttributeFac(
            ((PersonCreator*)getCreator())->visitgp, 
            global::abstime, facvisitgp
        );
        if (time < getTimeDeath()) { 
          procVisitGP.replace(new EventVisitGP(time, this, gpt));
        } else {
          procVisitGP.clear();
        }
    }
}

void Person::slotBinChange(Bin from, Bin to)
{
    Ageable::slotBinChange(from, to);
    
    global::statspersonbinchanges++;
    
    global::population->slotUpdatePSFormation(this);
    
    for(PartnershipList::iterator it = partnerships.begin();
        it != partnerships.end(); it++) {
        (*it)->throwEventHaveSex(true);
    }
}

void Person::slotRegisterPartnership(Partnership *ps)
{
    partnerscurrent++;
    partnerstotal++;
    partnerships.push_front(ps);
    global::population->slotUpdatePSFormation(this);
}

void Person::slotDeregisterPartnership(Partnership *ps)
{
    partnerscurrent--;
    bool found = false;
    for(PartnershipList::iterator it = partnerships.begin();
        it != partnerships.end(); it++) {
        if (*it == ps) {
            *it = 0;
            if (found) 
                rif_error(creator->getCfg(),
"internal while deregistering partnership: partnership was registered more than\
once");
            found = true;
//             break;
        }
    }
    if (!found) 
        rif_error(creator->getCfg(),
"internal while deregistering partnership: not found in active partnerships");
    partnerships.remove(0);
    partnershipsold.push_front(ps);
    global::population->slotUpdatePSFormation(this);
}

void Person::internalRemovePartnership(Partnership *ps)
{
    for(PartnershipList::iterator it = partnershipsold.begin();
        it != partnershipsold.end(); it++) {
        if (*it == ps) {
            *it = 0;
            break;
        }
    }
    partnershipsold.remove(0);
    
    for(InfectionList::iterator it = infections.begin();
        it != infections.end(); it++) {
        if ((*it)->getPartnership() == ps) {
            (*it)->clearPartnership();
        }
    }
    for(InfectionList::iterator it = infectionsold.begin();
        it != infectionsold.end(); it++) {
        if ((*it)->getPartnership() == ps) {
            (*it)->clearPartnership();
        }
    }
    
}


void Person::internalRemoveInfection(Infection *infection)
{
    for(InfectionList::iterator it = infectionsold.begin();
        it != infectionsold.end(); it++) {
        if (*it == infection) {
            *it = 0;
            break;
        }
    }
    infectionsold.remove(0);
    
    for(InfectionList::iterator it = infections.begin();
        it != infections.end(); it++) {
        if ((*it)->getParent() == infection) {
            (*it)->clearParent();
        }
    }
    for(InfectionList::iterator it = infectionsold.begin();
        it != infectionsold.end(); it++) {
        if ((*it)->getParent() == infection) {
            (*it)->clearParent();
        }
    }
    
}

void Person::slotRegisterInfection(Infection *infection)
{
//     cout << "infected: " << uid << endl;
    infectionscurrent++;
    infectionstotal++;
    
    if (!infection) {
        error("internal: trying to register NULL infection");
    }
    
    for (InfectionList::iterator it = infections.begin();
        it != infections.end(); it++) {
        if (infection == *it) {
            error("internal: infection is already in list 'infections'");  
        }
    }
    if (!infection) {
        error("internal: trying to register NULL infection");
    }
    infections.push_front(infection);
    global::population->slotUpdatePSFormation(this);
}

void Person::slotDeregisterInfection(Infection *infection)
{
    bool found = false;
    for(InfectionList::iterator it = infections.begin();
        it != infections.end(); it++) {
        if (*it == infection) {
            *it = 0;
            if (!found) {
                found = true;
            } else {
                error(
"internal: found infection more than once in list 'infections'");
            }
        }
    }
    if (!found) {
        error(
            "internal: infection not found in the list of current infections"
        );
    }
    infections.remove(0);
    
    for (InfectionList::iterator it = infectionsold.begin();
        it != infectionsold.end(); it++) {
        if (infection == *it) {
            error("internal: infection is already in list 'infectionsold'");  
        }
    }
    infectionsold.push_front(infection);
}

Infection *Person::getInfection(Type infectiontype) const
{
    for(InfectionList::const_iterator it = infections.begin();
        it != infections.end(); it++) {
        if ((*it)->getType() == infectiontype) return(*it);
    }
    return(0);    
}

bool Person::isInfected(Type infectiontype) const
{
    for(InfectionList::const_iterator it = infections.begin();
        it != infections.end(); it++) {
        if ((*it)->getType() == infectiontype) {
            return((*it)->isInfectious());
        }
    }
    return(false);    
}

bool Person::isInfected() const
{
    return(infectionscurrent>0);
}

bool Person::wasEverInfected(Type infectiontype) const
{
    for(InfectionList::const_iterator it = infections.begin();
        it != infections.end(); it++) {
        if ((*it)->getType() == infectiontype) return(true);
    }
    for(InfectionList::const_iterator it = infectionsold.begin();
        it != infectionsold.end(); it++) {
        if ((*it)->getType() == infectiontype) return(true);
    }
    return(false);
}

bool Person::wasEverInfected() const
{
    return(infectionstotal>0);
}

void Person::slotInfect(const Infection *infection, const Partnership *ps)
{
    if (!getInfection(infection->getType())) {
        Infection *d = infection->clone(this, ps);
        slotRegisterInfection(d);
        global::statsinfections++;
    }
}


void Person::slotInfectionChangedState(const Infection *infection, Bin from, Bin
to)
{
    Bin clearedstate =        
        ((InfectionCreator*)(infection->getCreator()))->getClearedBin();
    Bin treatedstate =
        ((InfectionCreator*)(infection->getCreator()))->getTreatedBin();
    if (to == clearedstate || to == treatedstate) {
        infectionscurrent--;
    }
    // ToDo: throwNextDoctorVisit
}


const Partnership * Person::getLastPartnershipWith(const Person *person) const
{
    Counter uid = person->uid;
    for(PartnershipList::const_iterator it = partnerships.begin();
        it != partnerships.end(); it++) {
            Counter puid1 = (*it)->getPerson1()->puid;
            if (uid == puid1) return(*it);
            Counter puid2 = (*it)->getPerson2()->puid;
            if (uid == puid2) return(*it);
    }
    for(PartnershipList::const_iterator it = partnershipsold.begin();
        it != partnershipsold.end(); it++) {
            if (*it == 0) error("internal error", __LINE__);
            Counter puid1 = (*it)->getPerson1()->puid;
            if (puid == puid1) return(*it);
            Counter puid2 = (*it)->getPerson2()->puid;
            if (puid == puid2) return(*it);
    }
    return(0);
}

void Person::slotNotifyHaveContact(Partnership *ps, bool unprotected)
{
    contacts++;
    if (unprotected) contactsunprot++;
    // Todo:: Maybe progress infections here?
    
}


string Person::str() const
{
    ostringstream s;
    s << "Person:"
        << "puid=" << puid
        << ",motherpuid=" << (mother ? mother->puid : 0)
        << ",fatherpuid=" << (father ? father->puid : 0)
        << ",partners=" << partnerscurrent 
        << ",causeCreate=" <<(int)causeCreated
        << "|";
    s << Ageable::str();
    return (s.str());
}

//
// END IMPLEMENTATION OF CLASS Person
//


//
// BEGIN IMPLEMENTATION OF CLASS PersonMale
//


PersonMale::PersonMale(PersonCreatorMale *pc, CauseCreation why, 
    PersonFemale *mother, PersonMale *father)
 : Person(pc, why, mother, father)
{
    idsubclass = CLASSPERSONMALE;
    
    numchildren = 0;
}

PersonMale::~PersonMale()
{
}

void PersonMale::slotBabyIsBorn(Person *mother)
{
    numchildren++;
}

Number PersonMale::getNumber(NumberOf what) const
{ 
    switch(what) {
        case NumberOfChildren:
            return(numchildren);
        default: 
            return(Person::getNumber(what));
    }; 
}

//
// END IMPLEMENTATION OF CLASS PersonMale
//


//
// BEGIN IMPLEMENTATION OF CLASS PersonFemale
//

PersonFemale::PersonFemale(PersonCreatorFemale *pc, CauseCreation why, 
    PersonFemale *mother, PersonMale *father)
 : Person(pc, why, mother, father)
{
    idsubclass = CLASSPERSONFEMALE;
    pregnant = false;
    numchildren = 0;
    numabortions = 0;
    numpregnancies = 0;
    
    procGetPregnant.setLastTime(getTimeBirth());
    throwEventGetPregnant();
}


PersonFemale::~PersonFemale()
{
}
    
bool PersonFemale::isPregnant() const 
{
    return(pregnant);
}

Number PersonFemale::getNumber(NumberOf what) const
{ 
    switch(what) {
        case IsPregnant: 
            return(pregnant);
        case NumberOfChildren:
            return(numchildren);
        case NumberOfAbortions:
            return(numabortions);
        case NumberOfPregnancies: 
            return(numpregnancies);
        default: 
            return(Person::getNumber(what));
    }; 
}


void PersonFemale::slotRegisterPartnership(Partnership *ps)
{
    Person::slotRegisterPartnership(ps);
    throwEventGetPregnant(true);
}

void PersonFemale::slotDeregisterPartnership(Partnership *ps)
{
    Person::slotDeregisterPartnership(ps);
    throwEventGetPregnant(true);
}

void PersonFemale::slotBinChange(Bin from, Bin to)
{
    Person::slotBinChange(from, to);
    throwEventGetPregnant(true);    
}

void PersonFemale::slotBabyIsBorn(PersonMale *father)
{
    if (!pregnant) {
        rif_error(creator->getCfg(), 
            "internal: female was not pregnant but got baby");
    }
    numchildren++;
    pregnant = false;
    if (father) {
        father->slotBabyIsBorn(this);
    }
//     throwEventBinChange(true);
    throwEventGetPregnant();
}

void PersonFemale::slotAbortion(CauseAbortion why)
{
    //ToDo: count numabortions
    numabortions++;
    pregnant = false;
//     throwEventBinChange(true);
    throwEventGetPregnant();
}

void PersonFemale::slotGetPregnant(PersonMale *father)
{
    if (pregnant) {
        rif_error(creator->getCfg(),
            "received EventGetPregnant, but female already pregnant");
        return;
    }
    procGetPregnant.setLastTime();
    procGetPregnant.clear();
    
    pregnant = true;
    numpregnancies++;
    global::statspregnancies++;
    
    Time timeofbirth = global::abstime + getAttribute(
        ((PersonCreatorFemale*)creator)->pregnancyduration, global::abstime
    );
    
    if (timeofbirth < getTimeDeath()) {
        global::scheduler->insert(new EventBirth(
            timeofbirth, CauseBirth, this, father
        ));
    } else {
        timeofbirth = getTimeDeath()-0.00000001;
        global::scheduler->insert(
            new EventAbortion(timeofbirth, CauseDeathMother, this, father
        ));
    }
}

void PersonFemale::throwEventGetPregnant(bool conditional)
{
    procGetPregnant.clear();
    
    Time time;
    
    double factor = getAttribute(
        ((PersonCreatorFemale*)creator)->pregnancygeneralfactor, global::abstime
    );
    if (conditional) {
        Time timediff = global::abstime - procGetPregnant.lastTime();
        if (timediff < 0) {
            rif_error(creator->getCfg(),
"internal: cannot sample conditional having sex if last sex is later than now"
            );
        }
        time = global::abstime + getAttributeFac(
            ((PersonCreatorFemale*)creator)->pregnancygeneralrate,
            procGetPregnant.lastTime(), factor, timediff
        );
    } else {
        time = global::abstime + getAttributeFac(
            ((PersonCreatorFemale*)creator)->pregnancygeneralrate,
            global::abstime, factor
        );
    }
    
    if (time < getTimeDeath()) {
        procGetPregnant.replace(new EventGetPregnant(time, this));
    } else {
        procGetPregnant.clear();
    }
}

void PersonFemale::slotNotifyHaveContact(Partnership *ps, bool unprotected)
{
    Person::slotNotifyHaveContact(ps, unprotected);
    
    if (unprotected &&
        !pregnant 
        && ps->getPartner(this)->idsubclass==CLASSPERSONMALE
        ) {
        double factor = getAttribute(
            ((PersonCreatorFemale*)creator)->probpregnancypercontactfactor, 
            global::abstime
        );
        double prob = factor * getAttribute(
            ((PersonCreatorFemale*)creator)->probpregnancypercontact,
            global::abstime
        );
        PersonCreatorFemale *pcf = (PersonCreatorFemale*)creator;
        if (pcf->uniform->dsample(this, global::abstime)<prob) {
            slotGetPregnant((PersonMale*)(ps->getPartner(this)));
        }
    }
    // no throwEventBinChange here, because too many resampling of events would
    //happen
}


string PersonFemale::str() const
{
    ostringstream s;
    s << "PersonFemale:"
        << "pregnant=" << (pregnant ? "yes" : "no")
        << "|";
    s << Person::str();
    return (s.str());
}

//
// END IMPLEMENTATION OF CLASS PersonFemale
//


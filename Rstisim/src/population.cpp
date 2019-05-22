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
#include "population.h"
#include "person.h"
#include "event.h"
#include "distribution.h"
#include "partnership.h"
#include "infection.h"
#include "scheduler.h"
#include "global.h"

using namespace std;


Population::Population(ROBJ cfg)
 : Object()
{
    popsize = rif_asInteger(cfg,0,"size");
    people_v.resize(popsize);
    for (int i = 0; i < popsize; i++) {
        emptypos_q.push(i);
        people_v[i] = 0;
    }
    popsize = 0;
    atDeathReplace = true;
    if (rif_exists(cfg,"atdeath")) {
        atDeathReplace = rif_asString(cfg, 0, "atdeath") == "replace" ? 
                        true : false;
    }
    if (atDeathReplace) {
        if (rif_exists(cfg,"timeofreplacement")) {
            timeofreplacement = 
                    createDistribution(rif_lookup(cfg,"timeofreplacement"));
        } else {
            ROBJ cfg_cd = rif_lookup(cfg,"constantdistribution");
            REAL(cfg_cd)[0] = 1;
            timeofreplacement = createDistribution(cfg_cd);
        }        
    } else {
        timeofreplacement = 0;
    }
    atBirthInsert = false;
    if (rif_exists(cfg,"atbirth")) {
        atBirthInsert = rif_asString(cfg, 0, "atbirth") == "insert" ? true : false;
    }
    immigration = 0; // by default no immigration
    if (rif_exists(cfg,"immigration")) {
        immigration = createDistribution(rif_lookup(cfg,"immigration"));
        if (immigration->getdmaxx() <= 0.0) {
            delete immigration;
            immigration = 0; //no immigration
        }
    }
    
    throwEventImmigration();
}


Population::~Population()
{
    for(PeopleList::iterator it = peopledead_l.begin(); 
        it != peopledead_l.end(); it++) {
        delete *it;
    }
    for(PeopleList::iterator it = peoplegone_l.begin(); 
        it != peoplegone_l.end(); it++) {
        delete *it;
    }
    for(PeopleList::iterator it = people_l.begin(); 
        it != people_l.end(); it++) {
        delete *it;
    }
    for(PartnershipList::iterator it = partnerships_l.begin();
        it != partnerships_l.end(); it++) {
        delete *it;   
    }
    for(PartnershipList::iterator it = partnershipsended_l.begin();
        it != partnershipsended_l.end(); it++) {
        delete *it;   
    }
    if (immigration) delete immigration;
    if (timeofreplacement) delete timeofreplacement;
}

bool comparePersons(Person *p1, Person*p2)
{
    return(p1->getTimeBirth() > p2->getTimeBirth());
}

void Population::slotPopulate() 
{
    for (unsigned int i = 0; i < people_v.size(); i++) {
        PersonCreator *pc =
(PersonCreator*)(global::persontypes->getRandomCreator());
        pc->createPerson(CausePopulate);
    }
    people_l.sort(comparePersons);
}

void Population::slotImmigration()
{
    ((PersonCreator*)(global::persontypes->getRandomCreator()))
        ->createPerson(CauseImmigration);
    global::statsimmigrations++;
    throwEventImmigration();
}

void Population::slotBirth(CauseCreation why, PersonFemale *mother,
PersonMale *father)
{
    switch(why) {
        case CauseBirth : {
            if (atBirthInsert) {
                ((PersonCreator*)(global::persontypes->getRandomCreator()))
                    ->createPerson(CauseBirth, mother, father);
                global::statsbirths++;
            }
            break;
        }
        case CauseReplacement : {
            if (atDeathReplace) {
                ((PersonCreator*)(global::persontypes->getRandomCreator()))
                    ->createPerson(CauseBirth, mother, father);
                global::statsbirths++;
            } else {
                error
("received EventBirth with CauseReplacement but atDeathReplace=false");
            }
            break;
        }
        default: error
("internal: EventBirth happend but not CauseBirth orCauseReplacement");
    }
}

void Population::throwEventImmigration() 
{
    if (immigration) {
        Time time = global::abstime + immigration->dsample();
        global::scheduler->insert(new EventImmigration(time));
    }
}

PopID Population::slotRegisterPerson(Person *p, CauseCreation why)
{
    // 'why' not needed for now
    PopID popid;
    if (emptypos_q.empty()) {
        // Resize vector by 1
        int cursize = people_v.size();
        people_v.resize(cursize+100);
        for (int i = cursize; i < cursize + 100; i++) {
            emptypos_q.push(i);
            people_v[i] = 0;
        }
    }
    
    popid.pos_v = emptypos_q.front();
    emptypos_q.pop();
    people_v[popid.pos_v] = p;
        
    people_l.push_front(p);
    popid.pos_l = people_l.begin();
    
    // We need to do this to make sure that popid is set as early as possible 
    // For infecting a person, popid needs to be set, because an new infection
    // will call some
    // "throw..." methods
    p->setPopID(popid);
    
    popsize++;
    
    switch(why) {
        case CausePopulate:
        case CauseImmigration: {
            for (int i = 0; i < global::infectiontypesnum; i++) {
                ((InfectionCreator*)(global::infectiontypes->getCreator(i)))->
                    randomlyInfectPerson(p, why);        
            }
            break;
        }
        default: break;
    }
    return(popid);
}

PopID Population::slotDeregisterPerson(PopID popid, CauseDeletion why)
{
    // Push into corresponding list of non-active people
    switch(why) {
        case CauseDeath : {
            peopledead_l.push_front(people_v[popid.pos_v]);
            global::statsdeaths++;
            break;
        }
        case CauseEmigration: {
            peoplegone_l.push_front(people_v[popid.pos_v]);    
            global::statsemigrations++;
            break;
        }
        default:
            error("unknown CauseDeletion");
    }
    
    // Delete in people list and vector
    people_l.erase(popid.pos_l);
    people_v[popid.pos_v] = 0;   
    
    // Construct new popid
    emptypos_q.push(popid.pos_v);  // Make slot in people_v available
    popid.pos_v = INTNA;
    switch(why) {
        case CauseDeath : {
            popid.pos_l = peopledead_l.begin();
            break;
        }
        case CauseEmigration: {
            popid.pos_l = peoplegone_l.begin();
            break;
        }
        default:
            error("unknown CauseDeletion");
    }
    
    if (atDeathReplace) {
        Time time =global::abstime + timeofreplacement->dsample();
        global::scheduler->insert(new EventBirth(time, CauseReplacement));
    }
    popsize--;
    return(popid);
}

void Population::slotRegisterForPSFormation(Person *person)
{
    for (int i = 0; i < global::psftypesnum; i++) {
       
((PSFormer*)global::psftypes->getCreator(i))->slotPersonRegister(person);
    }
}

void Population::slotUpdatePSFormation(Person *person)
{
    for (int i = 0; i < global::psftypesnum; i++) {
        ((PSFormer*)global::psftypes->getCreator(i))->slotPersonUpdate(person);
    }
}

void Population::slotDeregisterFromPSFormation(Person *person)
{
    for (int i = 0; i < global::psftypesnum; i++) {
       
((PSFormer*)global::psftypes->getCreator(i))->slotPersonDeregister(person);
    }
}

PartnershipID Population::slotRegisterPartnership(Partnership *ps)
{
    PartnershipID psid;
    partnerships_l.push_front(ps);
    psid.pos_l = partnerships_l.begin();
    global::statspscreated++;
    return(psid);
}

PartnershipID Population::slotDeregisterPartnership(PartnershipID psid)
{
    partnershipsended_l.push_front(getPartnership(psid));
    partnerships_l.erase(psid.pos_l);

    psid.pos_l = partnershipsended_l.begin();
    global::statspsended++;
    return(psid);
}

Person *Population::getRandomPerson(const Distribution *uniform) const
{
    int index;
    int size = people_v.size();
    do {
        index = uniform->isamplemax(size);
    } while (!people_v[index]);
    return(people_v[index]);
}


void Population::internalRemoveOldPartnerships()
{    
    for (PartnershipList::iterator it = partnershipsended_l.begin();
        it != partnershipsended_l.end(); it++) {
           if ((*it)->getTimeDeath() < global::abstime -
global::removepsolderthan) {
                Partnership *ps = *it;
//                ps->print();
                ps->getPerson1()->internalRemovePartnership(ps);
                ps->getPerson2()->internalRemovePartnership(ps);
                delete ps;
                *it = 0;
           }
    }
    partnershipsended_l.remove(0);
}

void Population::internalRemoveOldInfections()
{
    PeopleList *people;
    InfectionList dl;
    
    people = (PeopleList *)(global::population->getActivePeopleList());
    
    for (PeopleList::iterator it = people->begin();
        it != people->end(); it++) {
            const InfectionList *dltmp = (*it)->getInfectionsOld();
            for (InfectionList::const_iterator itd = dltmp->begin();
                itd != dltmp->end(); itd++) {
                    if ((*itd)->getTimeDeath() < global::abstime -
global::removeinfolderthan) {
                        dl.push_front(*itd);
                    }
            }
    }
    
    for (InfectionList::iterator itd = dl.begin();
        itd != dl.end(); itd++) {
        for (PeopleList::iterator it = people->begin();
            it != people->end(); it++) {
                (*it)->internalRemoveInfection(*itd);
            }
        delete *itd;
    }
 
}


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
#ifndef POPULATION_H
#define POPULATION_H

#include <queue>

#include "object.h"

class Population : public Object
{
public:
    Population(ROBJ cfg);
    ~Population();

    const PeopleList* getActivePeopleList() const
        { return(&people_l); };
    const PeopleList* getDeadPeopleList() const
        { return(&peopledead_l); };
    int getActivePopulationSize() const
        { return(popsize); };
    const PartnershipList* getActivePartnershipList() const
        { return(&partnerships_l); };
    const PartnershipList* getEndedPartnershipList() const
        { return(&partnershipsended_l); };

    Person * getRandomPerson(const Distribution *uniform) const;
    
    void slotPopulate();
    void slotImmigration();
    void slotBirth(CauseCreation why, PersonFemale *mother = 0, PersonMale
*father = 0);
    
    PopID slotRegisterPerson(Person *p, CauseCreation why);
    PopID slotDeregisterPerson(PopID popid, CauseDeletion why);
    Person *getPerson(PopID popid) const { return (*(popid.pos_l)); };
   
    PartnershipID slotRegisterPartnership(Partnership *ps);
    PartnershipID slotDeregisterPartnership(PartnershipID psid);
    Partnership *getPartnership(PartnershipID psid) const { return
(*(psid.pos_l)); };
    
    void slotRegisterForPSFormation(Person *person); // At birth register for
                                            //partnership formation
    void slotUpdatePSFormation(Person *person); // A person calles this if Bin
                                            //or numpartner changes
    void slotDeregisterFromPSFormation(Person *person); // At death or
                                                    //emigration deregister   
    
    void internalRemoveOldPartnerships();
    void internalRemoveOldInfections();

private:
    // Storing the people
    int popsize;

    PeopleVector people_v; 
    std::queue<int> emptypos_q;
    
    PeopleList people_l;
    
    PeopleList peopledead_l;
    PeopleList peoplegone_l;
    
    // Array 
    PartnershipList partnerships_l;
    PartnershipList partnershipsended_l;
    
    // Flags
    bool atDeathReplace;
    bool atBirthInsert;
    
    // Distribution
    Distribution *immigration;
    Distribution *timeofreplacement;
  
    void throwEventImmigration();
};


#endif

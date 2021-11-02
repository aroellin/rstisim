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
#ifndef INFECTIONINFECTION_H
#define INFECTIONINFECTION_H

#include "object.h"



class Infection : public Ageable
{
public:
    Counter infuid;

    Infection(InfectionCreator *creator, Person *host, 
        const Infection *parent = 0, const Partnership *ps = 0);

    ~Infection();
        
    Person *getHost() const;
    const Partnership *getPartnership() const;
    const Infection *getParent() const;
    void clearParent();
    void clearPartnership();
    Time getTimeOfImmunity();
    
    Counter getStrainID() const;
        
    std::string str() const;
    
    bool isInfectious() const;

    virtual Number getNumber(NumberOf what) const;     
    virtual void slotBinChange(Bin from, Bin to);
    virtual void slotDeath();
    virtual void slotHostDies();
    virtual void slotClear();
    virtual void slotTreated();
    virtual void slotTryToProgress(Person *victim, const Partnership *ps);
    virtual void slotProvokeGPVisit();

    virtual Infection *clone(Person *newhost, const Partnership *ps = 0) const;
    
protected:
    
    Person *host;
    const Infection *parent;
    const Partnership *ps;
    
    Counter strainid;
    
    virtual void throwEventProvokeGPVisit(bool conditional = false);        
    virtual void throwEventBinChange(bool conditional = false);
    virtual void throwEventDeath(bool conditional = false);
    
    Process procProvokeGPVisit;
        
    Time timeofimmunity;
};

class InfectionCreator : public Creator
{
friend class Infection;
public:
    InfectionCreator(CreatorCollection *collection, std::string name, ROBJ cfg);
    ~InfectionCreator();

    Bin getClearedBin() const;
    Bin getTreatedBin() const;
        
    std::string str() const;

    virtual bool getTestResult(Person *person) const;
    
    virtual Time getWaitForTestResult(Person *person) const;
    
    virtual bool randomlyInfectPerson(Person *person, CauseCreation why);
    
    virtual void slotInfectPerson(Person *person);
    
    std::string getBasetype() const { return("GENERIC"); };
    
private:
    Distribution * uniform;
    Attribute prevpopulating;
    Attribute previmmigrating;
    Attribute immunity;
    Attribute provokegpvisit;
    Attribute provokegpvisittype;
    
    Attribute inf;
    Attribute inffac;
    PairAttribute inffaccouple;
    Attribute inffacinfector; 
    Attribute inffacsusceptible;
    Attribute inffacps;
    
    Distribution *testuniform;    
    Attribute testsensitivity;
    Attribute testspecificity;
    Attribute testwaitforresult;

    Bin clearedstate;
    Bin treatedstate;
};

#endif

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

#ifndef PARTNERSHIP_H
#define PARTNERSHIP_H

#include <vector>
#include <string>

#include "object.h"


class PSCreator : public Creator
{
friend class Partnership;
public: 
    PSCreator(CreatorCollection *collection, std::string name, ROBJ cfg);
    ~PSCreator();
    
    Partnership *createPartnership(Person *p1, Person *p2, PSFInfo psfinfo);
    
     
    std::string getBasetype() const { return("GENERIC"); };
    
    std::string str() const;
   
private:
    // breakup distributions
    PairAttribute breakup; // pairparameter[p1][p2], then wrt p1
    Attribute breakupfactor1;  // wrt to person1
    Attribute breakupfactor2;  // wrt to person2
    
    // unprotected sex
    Attribute contact;              // dsamplefac wrt to this, where factor is
                                    //product of 4 factors below
    Attribute contactfactor;        // dsample wrt this
    PairAttribute contactcouplefactor;  // pairparameter, sampled wrt. p1/p2 and
                                        //then p1
    Attribute contactfactorperson1; // dsample wrt p1
    Attribute contactfactorperson2; // dsample wrt p2
        
    Attribute unprotected;          // pointMeasureDensity wrt this
    Attribute unprotectedfactor;        // dsample wrt this
    PairAttribute unprotectedcouplefactor;  // pairparameter, sampled wrt. p1/p2
                                            //and then p1
    Attribute unprotectedfactorperson1; // dsample wrt p1
    Attribute unprotectedfactorperson2; // dsample wrt p2
    
    Distribution *uniform;
     
};

class PSFormer : public Creator
{
friend class Partnership;
public:
    PSFormer(CreatorCollection *collection, std::string name, ROBJ cfg);
    ~PSFormer();
    
    virtual void slotPersonRegister(Person *person) = 0;
    virtual void slotPersonDeregister(Person *person) = 0;
    virtual void slotPersonUpdate(Person *person) = 0;
  
    virtual std::string str() const;
    
    std::string getBasetype() const { return("GENERIC"); };
};

class PSFormerIndivSearch : public PSFormer
{
public:
    PSFormerIndivSearch(CreatorCollection *lin, std::string name, ROBJ cfg);
    ~PSFormerIndivSearch();

    virtual void slotPersonRegister(Person *person);
    virtual void slotPersonDeregister(Person *person);
    virtual void slotPersonUpdate(Person *person);

    void slotPSInitiate(Person *person);
    void throwEventPSInitiate(Person *person);
    
    virtual std::string str() const;
    
    std::string getBasetype() const { return("INDIVIDUALSEARCH"); };
    
private:    
    std::vector<Process *> procPSForm;
    
//     std::vector<Event *> nextpsformevent;
    bool *typeisactive;
    
    Attribute seek;
    Attribute seekfactor;
    Attribute accept;
    Attribute acceptfactor;
    
    Attribute blockpartners;
    
    PairAttribute agediffmean;
    PairAttribute agediffsd;
    PairAttribute mixingfac;
    PairAttribute partnershiptype;
    
    Distribution *uniform;
};

class Partnership : public Ageable
{
public:
    Counter psuid;

    Partnership(PSCreator * psc, 
        Person *p1, Person *p2, 
        double start, double breakup, 
        PSFInfo psfinfo, Bin _bin = BINNA);

    ~Partnership();

    PartnershipID getPartnershipID() const;    
    Person *getPerson1() const;
    Person *getPerson2() const;
    Person *getPartner(Person *p) const;
        
    PSFInfo getPSFInfo() const;
    
    virtual Number getNumber(NumberOf what) const;
        
    virtual void slotDeath();
    virtual void slotHaveSex(bool unprotected);
    virtual void slotBinChange(Bin from, Bin to);
        
    virtual void throwEventHaveSex(bool conditional = false);
    virtual void throwEventBinChange(bool conditional = false);

    virtual std::string str() const;
    
private:
    Person *p1, *p2;
    PartnershipID psid;
    PSFInfo psfinfo;
    Process procHaveSex;
    Number contacts;
    Number unprotectedcontacts;
    
};


#endif

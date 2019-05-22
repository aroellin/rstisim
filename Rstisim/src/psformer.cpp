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






PSFormer::PSFormer(CreatorCollection *collection, std::string name, ROBJ cfg)
: Creator(collection, name, cfg, global::persontypes)
{
    idclass = CLASSPARTNERSHIPFORMER;
    
    if (!global::keepquiet) Rprintf(
"Scanning partnership formation type %d='%s'\n",type,getTypeName().c_str());
    
    if (!global::keepquiet) Rprintf("| Basetype: '%s'\n",getBasetype().c_str());      
 
    
}

PSFormer::~PSFormer()
{
}


string PSFormer::str() const 
{
    ostringstream s;
    s << "PSFormer";
    s << "|" << Creator::str();
    return (s.str());
}





PSFormerIndivSearch::PSFormerIndivSearch(CreatorCollection *lin, std::string
name, ROBJ cfg)
: PSFormer(lin, name, cfg)
{
    
    idsubclass = CLASSPARTNERSHIPFORMERINDIVIDUAL;
   
    // to start with, set vector to population size 
    procPSForm.resize(global::population->getActivePopulationSize());
    
    for(unsigned int i = 0; i < procPSForm.size(); i++) {
        procPSForm[i] = 0;
    }
    
    typeisactive = 0;
    agediffmean = 0;
    agediffsd = 0;
    mixingfac = 0;
    uniform = 0;
    agediffmean = 0;
    agediffsd = 0;
    mixingfac = 0;
    partnershiptype = 0;

    uniform = createDistribution(rif_lookup(cfg,"uniformdistribution"));
    
    
    seek = installAttribute(global::persontypes, "seek", "seek", "", MAXDOUBLE);
    
    seekfactor = installAttribute(global::persontypes,"seek factor",
        "seekfactor", "", 1);
    
    accept = installAttribute(global::persontypes, "accept", "accept", "", 1);
    
    acceptfactor = installAttribute(global::persontypes,
        "accept factor", "acceptfactor", "", 1);
    
    blockpartners = installAttribute(global::persontypes,
        "block past partners", "blockpreviouspartners", "", 0);
   
    
    ROBJ cfg_active = rif_lookup(cfg,"active");
    typeisactive = new bool[global::persontypesnum];
    for (int i = 0; i < global::persontypesnum; i++) typeisactive[i]=false;
    for (int i = 0; i < rif_getLength(cfg_active); i++) {
        string name = rif_asString(cfg_active,i);
        Type persontype =
global::persontypes->getCreatorTypeByName(cfg_active,name);
        typeisactive[persontype] = true;
    }
        
    if (!global::keepquiet) Rprintf("| Person types that actively search:\n");
    for (int i = 0; i < global::persontypesnum; i++) {
        if (typeisactive[i]) {
            if (!global::keepquiet) Rprintf("|\t'%s'\n",global::persontypes->getCreatorName(i).c_str());
        }
    }
    
    agediffmean = installPairAttribute(global::persontypes, 
        "average of age difference parameters", "agedifferencemean", 0, 0);
    agediffsd = installPairAttribute(global::persontypes,
        "variation in age difference parameters","agedifferencesd", 0.00000001,
MAXDOUBLE);
    mixingfac = installPairAttribute(global::persontypes,
        "mixing factors","mixingfactors", 0, 1);
    partnershiptype = installPairAttribute(global::persontypes,
        "which-partnership type","partnershiptype", 0, 0);
    for (int i = 0; i < global::persontypesnum; i++) 
        for (int k = 0; k < global::persontypes->getTotalNumberOfBins(); k++) 
            partnershiptype[i][k]->checkRange(0, global::psctypesnum-1);

}

PSFormerIndivSearch::~PSFormerIndivSearch()
{
    deletePairAttribute(global::persontypes, agediffmean);
    deletePairAttribute(global::persontypes, agediffsd);
    deletePairAttribute(global::persontypes, mixingfac);
    deletePairAttribute(global::persontypes, partnershiptype);
    if (typeisactive) delete[] typeisactive;
    if (uniform) delete uniform;
}

string PSFormerIndivSearch::str() const 
{
    ostringstream s;
    s << "PSFormerIndivSearch";
    s << "|" << PSFormer::str();
    return (s.str());
}


void PSFormerIndivSearch::slotPersonRegister(Person *person)
{
    int pos_v = person->getPopID().pos_v;
    
    if (pos_v < 0) {
        error(
"internal: trying to register person for PSFormerIndivSearch, but person not a\
valid popid");
    }
    if ((unsigned int)pos_v >= procPSForm.size()) {
        int size = procPSForm.size();
        procPSForm.resize(pos_v+100);
        for (unsigned int i = size; i < procPSForm.size(); i++) 
            procPSForm[i] = 0;
    }
    
    if (typeisactive[person->getType()]) {
        if (procPSForm[pos_v]) {
          error(
"internal: trying to assign new partnership formation process, but possion is\
occupied already");
        }
        procPSForm[pos_v] = new Process();
        throwEventPSInitiate(person);    
    }
}

void PSFormerIndivSearch::slotPersonDeregister(Person *person)
{
    if (typeisactive[person->getType()]) {
      if (procPSForm[person->getPopID().pos_v]) {
          delete procPSForm[person->getPopID().pos_v];
      } else {
            error(
"internal: trying to remove partnership formation process, but there is no\
active process present at this position");
      }
      procPSForm[person->getPopID().pos_v] = 0;
    }
}

void PSFormerIndivSearch::slotPersonUpdate(Person *person)
{
    if (typeisactive[person->getType()])
        throwEventPSInitiate(person);    
}

void PSFormerIndivSearch::throwEventPSInitiate(Person *person)
{
    int pos_v = person->getPopID().pos_v;
    if (!procPSForm[pos_v]) {
        error("internal: no Process object present", __LINE__);
    }
    if (!person->isAlive()) {
        person->print();
        error(
            "internal: cannot throw Partnership forming event for dead person"
        );
    }
    double wf = person->getAttribute(seekfactor,global::abstime);
    double wait = person->getAttributeFac(seek, global::abstime, wf);
    double time = global::abstime + wait;
    if (time < person->getTimeDeath()) {
        procPSForm[pos_v]->replace(new EventPSInitiate(
            global::abstime+wait, this, person
        ));
    } else {
        procPSForm[pos_v]->clear();
    }
}

void PSFormerIndivSearch::slotPSInitiate(Person *p1)
{
    Type p1type = p1->getType();
    double p1birth = p1->getTimeBirth();
    Distribution **agediffmeanp1type = agediffmean[p1type];
    Distribution **agediffsdp1type = agediffsd[p1type];
    Distribution **mixingfacp1type = mixingfac[p1type];

    int size = 100*global::population->getActivePopulationSize();

    double acc1 = p1->getAttribute(blockpartners, global::abstime);       

    double p;
    Person *ptry;
    double u;
    int ptrytype, ptryindex;
    double mixingfacp1typeptryindex, accptry;
    const Partnership *ps;
    Time back; 

   
    bool found = false;
    Person *p2;
    int i;
    {
    for (i = 0; i < size; i++) {
        u = uniform->dsample(); 
        ptry = global::population->getRandomPerson(uniform);
        ptrytype = ptry->getType();
        ptryindex = global::persontypes->linearise(ptrytype,ptry->getBin());
        mixingfacp1typeptryindex =
            mixingfacp1type[ptryindex]->dsample(p1,global::abstime);
        if (mixingfacp1typeptryindex == 0.0) {continue;}
        p = p1birth - ptry->getTimeBirth();
        p -= agediffmeanp1type[ptryindex]->dsample(p1,global::abstime);
        p /= agediffsdp1type[ptryindex]->dsample(p1,global::abstime);
        p *= (-0.5)*p;
        p = exp(p);
        if (u>p) continue; // if u is bigger than p already, skip rest and try
                            //new person
        p *= mixingfacp1typeptryindex;

        p *= ptry->getAttribute(acceptfactor, global::abstime);
                
        p *= ptry->getAttribute(accept, global::abstime);
        
        if (u<p) {
            if (p1==ptry) continue;
                
            ps = p1->getLastPartnershipWith(ptry);
            if (ps == 0) {
                found = true;
                p2 = ptry;
                break;
            }
            back = global::abstime - ps->getTimeDeath();
            if (back < 0) {
                found = true;
                p2 = ptry;
                break;
            }
            accptry = ptry->getAttribute(blockpartners, global::abstime);
            if (back > max(acc1, accptry)) {
                found = true;
                p2 = ptry;
                break;
            }
        }
    }
    }
    
    
    if (found) {
        PSFInfo info;
        info.fitness = p;
        info.tries = i+1;
        info.formertype = type;
        Type pstype = partnershiptype[p1->getType()]
            [global::persontypes->linearise(p2->getType(),p2->getBin())]
            ->isample(p1,global::abstime);
       
        ((PSCreator*)(global::psctypes->getCreator(pstype)))
            ->createPartnership(p1, p2,info);
    } else {
        if (!global::keepquiet) Rprintf("Formation of partnership failed\n");
        if (!global::keepquiet) Rprintf("\t"); p1->print();
        // Only necessary here, otherwise automatically called through update
        // functionalism
    }
    
    throwEventPSInitiate(p1); 
}



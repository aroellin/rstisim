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

#include "distribution.h"
#include "event.h"

#include "person.h"
#include "partnership.h"
#include "infection.h"

#include "population.h"
#include "scheduler.h"

#include "initdel.h"

using namespace std;

extern Distribution **typesinitran;

extern "C" {

ROBJ rif_omp(SEXP num)
{
    SEXP ans;
    PROTECT(ans = allocVector(INTSXP, 1));
#ifdef _OPENMP
    if (!isNull(num)) {
        omp_set_num_threads(INTEGER(num)[0]);
    }
    INTEGER(ans)[0] = omp_get_max_threads();
#else
    INTEGER(ans)[0] = -1;
#endif
    UNPROTECT(1);
    return(ans);
}

ROBJ rif_charToInt(ROBJ str)
{
    int len = LENGTH(str);
    
    ROBJ ans;
    PROTECT(ans = allocVector(INTSXP, len));
    
    for (int i = 0; i < len; i++) {
        const char * s = CHAR(STRING_ELT(str,i));
        INTEGER(ans)[i] = (int)(s[0]);
    }
    UNPROTECT(1);
    return(ans);
}

ROBJ rif_intToChar(ROBJ vec)
{
    ROBJ ans;
    int len = LENGTH(vec);
    PROTECT(ans = allocVector(STRSXP,len));
    for (int i = 0; i < len; i++) {
        char s[2]; 
        s[0] = (char)(INTEGER(vec)[i]);
        s[1] = 0;
        SET_STRING_ELT(ans, i, mkChar(s));
    }
    UNPROTECT(1);
    return(ans);
}

ROBJ rif_getStatistics()
{
    ROBJ num;
    
    PROTECT(num = allocVector(REALSXP, 21));
    
    int i = 0;
    REAL(num)[i++] = global::abstime;
    REAL(num)[i++] = (double)(global::population->getActivePopulationSize());
    REAL(num)[i++] = (double)global::statsevents;
    REAL(num)[i++] = (double)global::statsbirths;
    REAL(num)[i++] = (double)global::statsdeaths;
    REAL(num)[i++] = (double)global::statsimmigrations;
    REAL(num)[i++] = (double)global::statsemigrations;
    REAL(num)[i++] = (double)global::statspersonbinchanges;
    REAL(num)[i++] = (double)global::statspartnershipbinchanges;
    REAL(num)[i++] = (double)global::statsinfectionbinchanges;
    REAL(num)[i++] = (double)global::statspscreated;
    REAL(num)[i++] = (double)global::statspsended;
    REAL(num)[i++] = (double)global::statscontacts;
    REAL(num)[i++] = (double)global::statscontactsunprot;
    REAL(num)[i++] = (double)global::statspregnancies;
    REAL(num)[i++] = (double)global::statsinfections;
    REAL(num)[i++] = (double)global::statsclearances;
    REAL(num)[i++] = (double)global::statstests;
    REAL(num)[i++] = (double)global::statstreatments;
    REAL(num)[i++] = (double)global::statstreatmentsvain;
    REAL(num)[i++] = (double)global::statsfollowupvisits;
    
    UNPROTECT(1);
    return(num);
}

ROBJ rif_initModel(ROBJ obj, ROBJ _keepquiet)
{
    global::cfg = obj;
    
    bool keepquiet = (INTEGER(_keepquiet)[0] != 0);
        
    global::globalInitVars(keepquiet);
    
    personPreInitVars();
    partnershipPreInitVars();
    infectionPreInitVars();
    gpvisitPreInitVars();
    notifyPreInitVars();
       
    personInitVars();
    partnershipInitVars();
    infectionInitVars();
    gpvisitInitVars();
    notifyInitVars();
     
    return(R_NilValue);
}

ROBJ rif_removeModel()
{
    global::globalDelVars();
    
    notifyPreDelVars();
    gpvisitPreDelVars();
    partnershipPreDelVars();
    infectionPreDelVars();
    personPreDelVars();
    
    notifyDelVars();
    gpvisitDelVars();
    partnershipDelVars();
    infectionDelVars();
    personDelVars();
    
    return(R_NilValue);
}

ROBJ rif_populate()
{
    global::population->slotPopulate();
    return(R_NilValue);
}

ROBJ rif_advanceBy(ROBJ _time)
{
    Time time = REAL(_time)[0];
    ROBJ ans;
    PROTECT(ans = allocVector(INTSXP,1));
    INTEGER(ans)[0] = global::scheduler->executeBy(time);
    UNPROTECT(1);
    return(ans);
}

ROBJ rif_executeN(ROBJ _n, ROBJ _verbose)
{
    int n = INTEGER(_n)[0];
    bool verbose = (INTEGER(_verbose)[0] != 0);
    ROBJ ans;
    PROTECT(ans = allocVector(INTSXP,1));
    INTEGER(ans)[0] = global::scheduler->executeEvents(n,verbose);
    UNPROTECT(1);
    return(ans);
}

ROBJ rif_getSeed() 
{
    ROBJ seed;
    PROTECT(seed = allocVector(INTSXP,1));
    INTEGER(seed)[0] = global::originalseed;
    UNPROTECT(1);
    return(seed);
}

ROBJ rif_getAgeRange()
{
    ROBJ ans;
    PROTECT(ans = allocVector(REALSXP,2));
    REAL(ans)[0] = global::peopleminage;
    REAL(ans)[1] = global::peoplemaxage;
    UNPROTECT(1);
    return(ans);
}


ROBJ rif_testDistribution(ROBJ _n, ROBJ _atleast)
{
    int n = INTEGER(_n)[0];
    
    ROBJ ans;
        
    PROTECT(ans = allocVector(REALSXP, n));
    
    if (!global::testdistribution) error("'test.distribution' not defined");
    
    global::testdistribution->print();
    
    if (rif_isNull(_atleast)) {
        for (int i = 0; i < n; i++) {
            REAL(ans)[i] = global::testdistribution->dsample();
        }
    } else {
        double atleast = REAL(_atleast)[0];
        for (int i = 0; i < n; i++) {
            REAL(ans)[i] = global::testdistribution->dsample(atleast);
        }
    }
    
    UNPROTECT(1);
    return(ans);
}

void enumPeople()
{
    const PeopleList * people = global::population->getActivePeopleList();
    
    int pos = 0;
    for (PeopleList::const_iterator it = people->begin();
                            it != people->end(); it++) {
        // PopID *popid = (*it)->getPopID_p();
        (*it)->popid.adhoc = ++pos;
    }
    
    people = global::population->getDeadPeopleList();
    for (PeopleList::const_iterator it = people->begin();
                            it != people->end(); it++) {
        (*it)->popid.adhoc = INTNA;
    }
}

ROBJ rif_getPeople(ROBJ _old)
{
    enumPeople();
    
    int old = rif_asInteger(_old,0);
   
    const PeopleList * people;
     
    if (old) {
        people = global::population->getDeadPeopleList();
    } else {
        people = global::population->getActivePeopleList();
    }
    
    int popsize = people->size();
    
    ROBJ ans;
    PROTECT(ans = allocVector(VECSXP,21));
    
    ROBJ puid, adhocid, type, bin, timebirth, timedeath,
        curnumpartners, totnumpartners, oneypartners, contacts, contactsunprot,
        pregnant, children, abortions, pregnancies,
        infectionscur, infectionstot, infectionswithin, treatments,
        gpvisits, notifications;
    PROTECT(puid = allocVector(INTSXP, popsize));
    PROTECT(adhocid = allocVector(INTSXP, popsize));
    PROTECT(type = allocVector(INTSXP, popsize));
    PROTECT(bin = allocVector(INTSXP, popsize));
    PROTECT(timebirth = allocVector(REALSXP, popsize));
    PROTECT(timedeath = allocVector(REALSXP, popsize));
    PROTECT(curnumpartners = allocVector(INTSXP, popsize));
    PROTECT(totnumpartners = allocVector(INTSXP, popsize));
    PROTECT(oneypartners = allocVector(INTSXP, popsize));
    PROTECT(contacts = allocVector(INTSXP, popsize));
    PROTECT(contactsunprot = allocVector(INTSXP, popsize));
    PROTECT(pregnant = allocVector(INTSXP, popsize));
    PROTECT(children = allocVector(INTSXP, popsize));
    PROTECT(abortions = allocVector(INTSXP, popsize));
    PROTECT(pregnancies = allocVector(INTSXP, popsize));
    PROTECT(infectionscur = allocVector(INTSXP, popsize));
    PROTECT(infectionstot = allocVector(INTSXP, popsize));
    PROTECT(infectionswithin = allocVector(INTSXP, popsize));
    PROTECT(treatments = allocVector(INTSXP, popsize));
    PROTECT(gpvisits = allocVector(INTSXP, popsize));
    PROTECT(notifications = allocVector(INTSXP, popsize));
    
    int pos = 0;
    
    for (PeopleList::const_iterator it = people->begin();
                            it != people->end(); it++) {
        PopID popid = (*it)->getPopID();
        if (!old && popid.adhoc != pos+1) 
            error("living people are not enumerated");
            
        INTEGER(puid)[pos] = (*it)->puid;
        int adhoc = popid.adhoc;
        if (adhoc != INTNA) {
            INTEGER(adhocid)[pos] = adhoc;
        } else {
            INTEGER(adhocid)[pos] = R_NaInt;
        }
        INTEGER(type)[pos] = (*it)->getType();
        INTEGER(bin)[pos] = (*it)->getBinLinearised();
        REAL(timebirth)[pos] = (*it)->getTimeBirth();
        REAL(timedeath)[pos] = (*it)->getTimeDeath();
        INTEGER(curnumpartners)[pos] =
(*it)->getNumber(NumberOfPartnersCurrent);
        INTEGER(totnumpartners)[pos] = (*it)->getNumber(NumberOfPartnersTotal);
        INTEGER(oneypartners)[pos] = (*it)->getNumber(NumberOfPartnersWithin);
        INTEGER(contacts)[pos] = (*it)->getNumber(NumberOfContacts);
        INTEGER(contactsunprot)[pos] =
(*it)->getNumber(NumberOfContactsUnprotected);
        INTEGER(infectionscur)[pos] =
(*it)->getNumber(NumberOfInfectionsCurrent);
        INTEGER(infectionstot)[pos] = (*it)->getNumber(NumberOfInfectionsTotal);
        INTEGER(infectionswithin)[pos] =
(*it)->getNumber(NumberOfInfectionsWithin);
        
        if ((*it)->idsubclass == CLASSPERSONFEMALE) {
            INTEGER(pregnant)[pos] = ((PersonFemale*)(*it))->isPregnant();
            INTEGER(children)[pos] =
((PersonFemale*)(*it))->getNumber(NumberOfChildren);
            INTEGER(abortions)[pos] =
((PersonFemale*)(*it))->getNumber(NumberOfAbortions);
            INTEGER(pregnancies)[pos] =
((PersonFemale*)(*it))->getNumber(NumberOfPregnancies);
        } else if ((*it)->idsubclass == CLASSPERSONMALE) {
            INTEGER(pregnant)[pos] = R_NaInt;
            INTEGER(children)[pos] =
((PersonMale*)(*it))->getNumber(NumberOfChildren);;
            INTEGER(abortions)[pos] = R_NaInt;
            INTEGER(pregnancies)[pos] = R_NaInt;
        } else {
            INTEGER(pregnant)[pos] = R_NaInt;
            INTEGER(children)[pos] = R_NaInt;
            INTEGER(abortions)[pos] = R_NaInt;
            INTEGER(pregnancies)[pos] = R_NaInt;
        }

        INTEGER(treatments)[pos] = (*it)->getNumber(NumberOfTreatments);
        INTEGER(gpvisits)[pos] = (*it)->getNumber(NumberOfGPVisits);
        INTEGER(notifications)[pos] = (*it)->getNumber(NumberOfNotifications);
        pos++;
    }
    
    ROBJ typelabels;
    PROTECT(typelabels = allocVector(STRSXP,global::persontypesnum));
    ROBJ binlabels;
    PROTECT(binlabels =
allocVector(STRSXP,global::persontypes->getTotalNumberOfBins()));
    for (int type = 0; type < global::persontypesnum; type++) {
        SET_STRING_ELT(typelabels, type,
            mkChar(global::persontypes->getCreatorName(type).c_str()));
        for (int bin = 0; bin <
global::persontypes->getCreator(type)->getNumberOfBins(); bin++) 
            SET_STRING_ELT(binlabels, global::persontypes->linearise(type,bin), 
               
mkChar(global::persontypes->getCreator(type)->getBinName(bin).c_str()));
    }
    setAttrib(type, install("levels"), typelabels);
    setAttrib(bin, install("levels"), binlabels);
    
    ROBJ yesnolabels;
    PROTECT(yesnolabels = allocVector(STRSXP, 2));
    SET_STRING_ELT(yesnolabels, 0, mkChar("not pregnant"));
    SET_STRING_ELT(yesnolabels, 1, mkChar("pregnant"));
    setAttrib(pregnant, install("levels"), yesnolabels);    
    
    pos = 0;
    SET_VECTOR_ELT(ans,pos++, puid);
    SET_VECTOR_ELT(ans,pos++, adhocid);
    SET_VECTOR_ELT(ans,pos++, type);
    SET_VECTOR_ELT(ans,pos++, bin);
    SET_VECTOR_ELT(ans,pos++, timebirth);
    SET_VECTOR_ELT(ans,pos++, timedeath);
    SET_VECTOR_ELT(ans,pos++, curnumpartners);
    SET_VECTOR_ELT(ans,pos++, totnumpartners);
    SET_VECTOR_ELT(ans,pos++, oneypartners);
    SET_VECTOR_ELT(ans,pos++, contacts);
    SET_VECTOR_ELT(ans,pos++, contactsunprot);
    SET_VECTOR_ELT(ans,pos++, pregnant);
    SET_VECTOR_ELT(ans,pos++, pregnancies);
    SET_VECTOR_ELT(ans,pos++, abortions);
    SET_VECTOR_ELT(ans,pos++, children);
    SET_VECTOR_ELT(ans,pos++, infectionscur);
    SET_VECTOR_ELT(ans,pos++, infectionstot);
    SET_VECTOR_ELT(ans,pos++, infectionswithin);
    SET_VECTOR_ELT(ans,pos++, treatments);
    SET_VECTOR_ELT(ans,pos++, gpvisits);
    SET_VECTOR_ELT(ans,pos++, notifications);
    
    UNPROTECT(25);
    return(ans);
}

ROBJ rif_getPartnerships(ROBJ _old)
{
    enumPeople();
    
    int old = rif_asInteger(_old,0);
   
    const PartnershipList *psl;
     
    if (old) {
        psl = global::population->getEndedPartnershipList();
    } else {
        psl = global::population->getActivePartnershipList();
    }

    int size = psl->size();
    
    ROBJ ans;
    PROTECT(ans = allocVector(VECSXP,14));
    
    ROBJ psuid, p1, p2, puid1, puid2, 
        type, bin, formertype,  
        begin, end, fitness, tries, 
        contacts, contactsunprot;
    PROTECT(psuid = allocVector(INTSXP, size));    
    PROTECT(p1 = allocVector(INTSXP, size));
    PROTECT(p2 = allocVector(INTSXP, size));
    PROTECT(puid1 = allocVector(INTSXP, size));
    PROTECT(puid2 = allocVector(INTSXP, size));
    PROTECT(begin = allocVector(REALSXP, size));
    PROTECT(end = allocVector(REALSXP, size));
    PROTECT(fitness = allocVector(REALSXP, size));
    PROTECT(tries = allocVector(INTSXP, size));
    PROTECT(formertype = allocVector(INTSXP, size));
    PROTECT(type = allocVector(INTSXP, size));
    PROTECT(bin = allocVector(INTSXP, size));
    PROTECT(contacts = allocVector(INTSXP, size));
    PROTECT(contactsunprot = allocVector(INTSXP, size));
    
    int pos = 0;
    for (PartnershipList::const_iterator it = psl->begin();
            it != psl->end(); it++) {
        INTEGER(psuid)[pos] = (*it)->psuid;
        int adhocid = (*it)->getPerson1()->getPopID().adhoc;
        if (adhocid != INTNA) {
            INTEGER(p1)[pos] = adhocid;
        } else {
            INTEGER(p1)[pos] = R_NaInt;
        }
        adhocid = (*it)->getPerson2()->getPopID().adhoc;
        if (adhocid != INTNA) {
            INTEGER(p2)[pos] = adhocid;
        } else {
            INTEGER(p2)[pos] = R_NaInt;
        }
        
        INTEGER(puid1)[pos] = (*it)->getPerson1()->puid;
        INTEGER(puid2)[pos] = (*it)->getPerson2()->puid;
        REAL(begin)[pos] = (*it)->getTimeBirth();
        REAL(end)[pos] = (*it)->getTimeDeath();
        PSFInfo info = (*it)->getPSFInfo();
        REAL(fitness)[pos] = info.fitness;
        INTEGER(tries)[pos] = info.tries;
        INTEGER(formertype)[pos] = info.formertype;
        INTEGER(type)[pos] = (*it)->getType();
        INTEGER(bin)[pos] = (*it)->getBin();
        INTEGER(contacts)[pos] = (*it)->getNumber(NumberOfContacts);
        INTEGER(contactsunprot)[pos] =
(*it)->getNumber(NumberOfContactsUnprotected);
        pos++;
    }
    
    ROBJ formertypelabels;
    PROTECT(formertypelabels = allocVector(STRSXP,global::psftypesnum));
    for (int type = 0; type < global::psftypesnum; type++) {
        SET_STRING_ELT(formertypelabels, type,
            mkChar(global::psftypes->getCreatorName(type).c_str()));
    }
    
    ROBJ typelabels;
    PROTECT(typelabels = allocVector(STRSXP,global::psctypesnum));
    ROBJ binlabels;
    PROTECT(binlabels =
allocVector(STRSXP,global::psctypes->getTotalNumberOfBins()));
    for (int type = 0; type < global::psctypesnum; type++) {
        SET_STRING_ELT(typelabels, type,
            mkChar(global::psctypes->getCreatorName(type).c_str()));
        for (int bin = 0; bin <
global::psctypes->getCreator(type)->getNumberOfBins(); bin++) 
            SET_STRING_ELT(binlabels, global::psctypes->linearise(type,bin), 
               
mkChar(global::psctypes->getCreator(type)->getBinName(bin).c_str()));
    }
    
    setAttrib(formertype, install("levels"), formertypelabels);    
    setAttrib(type, install("levels"), typelabels);
    setAttrib(bin, install("levels"), binlabels);
    
    pos = 0;
    SET_VECTOR_ELT(ans,pos++,psuid);
    SET_VECTOR_ELT(ans,pos++,type);
    SET_VECTOR_ELT(ans,pos++,bin);
    SET_VECTOR_ELT(ans,pos++,begin);
    SET_VECTOR_ELT(ans,pos++,end);
    SET_VECTOR_ELT(ans,pos++,puid1);
    SET_VECTOR_ELT(ans,pos++,p1);
    SET_VECTOR_ELT(ans,pos++,puid2);
    SET_VECTOR_ELT(ans,pos++,p2);
    SET_VECTOR_ELT(ans,pos++,formertype);
    SET_VECTOR_ELT(ans,pos++,fitness);
    SET_VECTOR_ELT(ans,pos++,tries);
    SET_VECTOR_ELT(ans,pos++,contacts);
    SET_VECTOR_ELT(ans,pos++,contactsunprot);
    
    UNPROTECT(18);
    return(ans);
}


ROBJ rif_getInfections(ROBJ _oldinfections, ROBJ _oldpeople)
{
    enumPeople();
    
    int oldinfections = rif_asInteger(_oldinfections,0);
    int oldpeople = rif_asInteger(_oldpeople,0);
   
    const PeopleList * people;
     
    if (oldpeople) {
        people = global::population->getDeadPeopleList();
    } else {
        people = global::population->getActivePeopleList();
    }
    
    int pos = 0;
    for (PeopleList::const_iterator it = people->begin();
            it != people->end(); it++) {
        const InfectionList * infections;
        if (oldinfections) {
            infections = ((*it)->getInfectionsOld());
        } else {
            infections = (*it)->getInfections();
        }
        pos += infections->size();
    }
    
    
    ROBJ ans;
    PROTECT(ans = allocVector(VECSXP,11));
    
    ROBJ infuid, strainid, parentinfuid, hostpuid, hostadhocid, type, bin,
birth, endinfection, death, psuid;
    
    PROTECT(infuid = allocVector(INTSXP,pos));
    PROTECT(strainid = allocVector(INTSXP,pos));
    PROTECT(parentinfuid = allocVector(INTSXP,pos));
    PROTECT(hostpuid = allocVector(INTSXP,pos));
    PROTECT(hostadhocid = allocVector(INTSXP,pos));
    PROTECT(type = allocVector(INTSXP,pos));
    PROTECT(bin = allocVector(INTSXP,pos));
    PROTECT(birth = allocVector(REALSXP,pos));
    PROTECT(endinfection = allocVector(REALSXP,pos));
    PROTECT(death = allocVector(REALSXP,pos));
    PROTECT(psuid = allocVector(INTSXP,pos));
    
    pos = 0;
    for (PeopleList::const_iterator it = people->begin();
            it != people->end(); it++) {
        
        const InfectionList * infections;
        if (oldinfections) {
            infections = ((*it)->getInfectionsOld());
        } else {
            infections = (*it)->getInfections();
        }
        
        int adhocid = (*it)->getPopID().adhoc;
                
        for (InfectionList::const_iterator itinf = infections->begin();
            itinf != infections->end(); itinf++) {
            INTEGER(infuid)[pos] = (*itinf)->infuid;
            INTEGER(strainid)[pos] = (*itinf)->getStrainID();
            const Infection *parent = (*itinf)->getParent();
            if (parent) {
                INTEGER(parentinfuid)[pos] = parent->infuid;
            } else {
                INTEGER(parentinfuid)[pos] = R_NaInt;
            }
            INTEGER(hostpuid)[pos] = (*it)->puid;
            if (adhocid != INTNA) {
                INTEGER(hostadhocid)[pos] = adhocid;
            } else {
                INTEGER(hostadhocid)[pos] = R_NaInt;
            }
            
            INTEGER(type)[pos] = (*itinf)->getType();
            INTEGER(bin)[pos] = (*itinf)->getBinLinearised();
            REAL(birth)[pos] = (*itinf)->getTimeBirth();
            if ((*itinf)->getTimeOfImmunity() < (*it)->getTimeDeath()) {
                REAL(endinfection)[pos] = (*itinf)->getTimeOfImmunity();
            } else {
                REAL(endinfection)[pos] = R_NaReal;
            }
            if ((*itinf)->getTimeDeath() < (*it)->getTimeDeath()) {
                REAL(death)[pos] = (*itinf)->getTimeDeath();
            } else {
                REAL(death)[pos] = R_NaReal;
            }
            if ((*itinf)->getPartnership()) {
                INTEGER(psuid)[pos] = (*itinf)->getPartnership()->psuid;
            } else {
                INTEGER(psuid)[pos] = R_NaInt;
            }
            pos++;        
        }
    }
    
    ROBJ typelabels;
    PROTECT(typelabels = allocVector(STRSXP,global::infectiontypesnum));
    ROBJ binlabels;
    PROTECT(binlabels =
allocVector(STRSXP,global::infectiontypes->getTotalNumberOfBins()));
    for (int type = 0; type < global::infectiontypesnum; type++) {
        SET_STRING_ELT(typelabels, type,
            mkChar(global::infectiontypes->getCreatorName(type).c_str()));
        for (int bin = 0; bin <
global::infectiontypes->getCreator(type)->getNumberOfBins(); bin++) 
            SET_STRING_ELT(binlabels,
global::infectiontypes->linearise(type,bin), 
               
mkChar(global::infectiontypes->getCreator(type)->getBinName(bin).c_str()));
    }
    
    setAttrib(type, install("levels"), typelabels);
    setAttrib(bin, install("levels"), binlabels);
    
    pos = 0;
    SET_VECTOR_ELT(ans,pos++, infuid);
    SET_VECTOR_ELT(ans,pos++, strainid);
    SET_VECTOR_ELT(ans,pos++, type);
    SET_VECTOR_ELT(ans,pos++, bin);
    SET_VECTOR_ELT(ans,pos++, birth);
    SET_VECTOR_ELT(ans,pos++, endinfection);
    SET_VECTOR_ELT(ans,pos++, death);
    SET_VECTOR_ELT(ans,pos++, parentinfuid);
    SET_VECTOR_ELT(ans,pos++, hostpuid);
    SET_VECTOR_ELT(ans,pos++, hostadhocid);
    SET_VECTOR_ELT(ans,pos++, psuid);

    UNPROTECT(1+11+2);
    
    return(ans);
}





ROBJ rif_getNotifications(ROBJ _oldpeople)
{
    enumPeople();
    
    int oldpeople = rif_asInteger(_oldpeople,0);
   
    const PeopleList * people;
     
    if (oldpeople) {
        people = global::population->getDeadPeopleList();
    } else {
        people = global::population->getActivePeopleList();
    }
    
    int pos = 0;
    for (PeopleList::const_iterator it = people->begin();
            it != people->end(); it++) {
        pos += (*it)->getSingleNotifications()->size();
    }
    
    
    ROBJ ans;
    PROTECT(ans = allocVector(VECSXP,13));
    
    ROBJ ntype, nid, linknumber, time, 
        puid1, ptype1, pbin1, 
        puid2, ptype2, pbin2,
        psuid, pstype, psbin;

    PROTECT(ntype = allocVector(INTSXP,pos));
    PROTECT(nid = allocVector(INTSXP,pos));
    PROTECT(linknumber = allocVector(INTSXP,pos));
    PROTECT(time = allocVector(REALSXP,pos));
    PROTECT(puid1 = allocVector(INTSXP,pos));
    PROTECT(ptype1 = allocVector(INTSXP,pos));
    PROTECT(pbin1 = allocVector(INTSXP,pos));
    PROTECT(puid2 = allocVector(INTSXP,pos));
    PROTECT(ptype2 = allocVector(INTSXP,pos));
    PROTECT(pbin2 = allocVector(INTSXP,pos));
    PROTECT(psuid = allocVector(INTSXP,pos));
    PROTECT(pstype = allocVector(INTSXP,pos));
    PROTECT(psbin = allocVector(INTSXP,pos));

    pos = 0;
    for (PeopleList::const_iterator it = people->begin();
            it != people->end(); it++) {
        
        const SingleNotificationList *snl = (*it)->getSingleNotifications();
        
        // int adhocid = (*it)->getPopID().adhoc;
                
        for (SingleNotificationList::const_iterator itsn = snl->begin();
                itsn != snl->end(); itsn++) {

            INTEGER(ntype)[pos] = (*itsn).ntype;
            INTEGER(nid)[pos] = (*itsn).nid;
            INTEGER(linknumber)[pos] = (*itsn).linknumber;            
            REAL(time)[pos] = (*itsn).time;
            INTEGER(puid1)[pos] = (*itsn).puid1;
            INTEGER(puid2)[pos] = (*itsn).puid2;
            INTEGER(ptype1)[pos] = (*itsn).ptype1;
            INTEGER(ptype2)[pos] = (*itsn).ptype2;
            INTEGER(pbin1)[pos] = (*itsn).pbin1;
            INTEGER(pbin2)[pos] = (*itsn).pbin2;
            INTEGER(psuid)[pos] = (*itsn).psuid;
            INTEGER(pstype)[pos] = (*itsn).pstype;
            INTEGER(psbin)[pos] = (*itsn).psbin;

            pos++;        
        }
    }

    ROBJ ntypelabels;
    PROTECT(ntypelabels = allocVector(STRSXP,global::notifiertypesnum));
    for (int type = 0; type < global::notifiertypesnum; type++) {
        SET_STRING_ELT(ntypelabels, type,
            mkChar(global::notifiertypes->getCreatorName(type).c_str()));
    }
    
    setAttrib(ntype, install("levels"), ntypelabels);

    ROBJ ptypelabels;
    PROTECT(ptypelabels = allocVector(STRSXP,global::persontypesnum));
    ROBJ pbinlabels;
    PROTECT(pbinlabels =
        allocVector(STRSXP,global::persontypes->getTotalNumberOfBins()));
    for (int type = 0; type < global::persontypesnum; type++) {
        SET_STRING_ELT(ptypelabels, type,
            mkChar(global::persontypes->getCreatorName(type).c_str()));
        for (int bin = 0; bin <
            global::persontypes->getCreator(type)->getNumberOfBins(); bin++) 
            SET_STRING_ELT(pbinlabels,
            global::persontypes->linearise(type,bin), 
    mkChar(global::persontypes->getCreator(type)->getBinName(bin).c_str()));
    }

    
    setAttrib(ptype1, install("levels"), ptypelabels);
    setAttrib(ptype2, install("levels"), ptypelabels);
    setAttrib(pbin1, install("levels"), pbinlabels);
    setAttrib(pbin2, install("levels"), pbinlabels);


    ROBJ pstypelabels;
    PROTECT(pstypelabels = allocVector(STRSXP,global::psctypesnum));
    ROBJ psbinlabels;
    PROTECT(psbinlabels =
        allocVector(STRSXP,global::psctypes->getTotalNumberOfBins()));
    for (int type = 0; type < global::psctypesnum; type++) {
        SET_STRING_ELT(pstypelabels, type,
            mkChar(global::psctypes->getCreatorName(type).c_str()));
        for (int bin = 0; bin <
            global::psctypes->getCreator(type)->getNumberOfBins(); bin++) 
            SET_STRING_ELT(psbinlabels,
            global::psctypes->linearise(type,bin), 
    mkChar(global::psctypes->getCreator(type)->getBinName(bin).c_str()));
    }

    
    setAttrib(pstype, install("levels"), pstypelabels);
    setAttrib(psbin, install("levels"), psbinlabels);

    
    pos = 0;
    SET_VECTOR_ELT(ans,pos++, ntype);
    SET_VECTOR_ELT(ans,pos++, nid);
    SET_VECTOR_ELT(ans,pos++, linknumber);
    SET_VECTOR_ELT(ans,pos++, time);
    SET_VECTOR_ELT(ans,pos++, puid1);
    SET_VECTOR_ELT(ans,pos++, ptype1);
    SET_VECTOR_ELT(ans,pos++, pbin1);
    SET_VECTOR_ELT(ans,pos++, puid2);
    SET_VECTOR_ELT(ans,pos++, ptype2);
    SET_VECTOR_ELT(ans,pos++, pbin2);
    SET_VECTOR_ELT(ans,pos++, psuid);
    SET_VECTOR_ELT(ans,pos++, pstype);
    SET_VECTOR_ELT(ans,pos++, psbin);
        
    UNPROTECT(1+13+5);

    
    return(ans);
}
















ROBJ rif_getGPVisits(ROBJ _oldpeople)
{
    enumPeople();
    
    int oldpeople = rif_asInteger(_oldpeople,0);
   
    const PeopleList * people;
     
    if (oldpeople) {
        people = global::population->getDeadPeopleList();
    } else {
        people = global::population->getActivePeopleList();
    }
    
    int pos = 0;
    for (PeopleList::const_iterator it = people->begin();
            it != people->end(); it++) {
        pos += (*it)->getSingleGPVisits()->size();
    }
    
    
    ROBJ ans;
    PROTECT(ans = allocVector(VECSXP,12));
    
    ROBJ gptype, nid, linknumber, time, ntype, cause,
         dirtreated, tested, posresults,
         puid, ptype, pbin;
    
    PROTECT(gptype = allocVector(INTSXP,pos));
    PROTECT(nid = allocVector(INTSXP,pos));
    PROTECT(linknumber = allocVector(INTSXP,pos));
    PROTECT(time = allocVector(REALSXP,pos));
    PROTECT(ntype = allocVector(INTSXP,pos));
    PROTECT(cause = allocVector(INTSXP,pos));
    PROTECT(dirtreated = allocVector(INTSXP,pos));
    PROTECT(tested = allocVector(INTSXP,pos));
    PROTECT(posresults = allocVector(INTSXP,pos));
    PROTECT(puid = allocVector(INTSXP,pos));
    PROTECT(ptype = allocVector(INTSXP,pos));
    PROTECT(pbin = allocVector(INTSXP,pos));

    pos = 0;
    
    for (PeopleList::const_iterator it = people->begin();
            it != people->end(); it++) {
        
        const SingleGPVisitList *gpvl = (*it)->getSingleGPVisits();
        
        for (SingleGPVisitList::const_iterator itgpv = gpvl->begin();
                itgpv != gpvl->end(); itgpv++) {

            INTEGER(gptype)[pos] = (*itgpv).gptype;
            INTEGER(nid)[pos] = (*itgpv).nid;
            INTEGER(linknumber)[pos] = (*itgpv).linknumber;            
            REAL(time)[pos] = (*itgpv).time;
            INTEGER(ntype)[pos] = (*itgpv).ntype;
            INTEGER(cause)[pos] = (*itgpv).cause;
            INTEGER(dirtreated)[pos] = (*itgpv).dirtreated;
            INTEGER(tested)[pos] = (*itgpv).tested;
            INTEGER(posresults)[pos] = (*itgpv).posresults;
            INTEGER(puid)[pos] = (*itgpv).puid;
            INTEGER(ptype)[pos] = (*itgpv).ptype;
            INTEGER(pbin)[pos] = (*itgpv).pbin;

            pos++;        
        }
    }

    ROBJ gptypelabels;
    PROTECT(gptypelabels = allocVector(STRSXP,global::gpvisittypesnum));
    for (int type = 0; type < global::gpvisittypesnum; type++) {
        SET_STRING_ELT(gptypelabels, type,
            mkChar(global::gpvisittypes->getCreatorName(type).c_str()));
    }
    
    setAttrib(gptype, install("levels"), gptypelabels);


    ROBJ ntypelabels;
    PROTECT(ntypelabels = allocVector(STRSXP,global::notifiertypesnum));
    for (int type = 0; type < global::notifiertypesnum; type++) {
        SET_STRING_ELT(ntypelabels, type,
            mkChar(global::notifiertypes->getCreatorName(type).c_str()));
    }
    
    setAttrib(ntype, install("levels"), ntypelabels);


    ROBJ ptypelabels;
    PROTECT(ptypelabels = allocVector(STRSXP,global::persontypesnum));
    ROBJ pbinlabels;
    PROTECT(pbinlabels =
        allocVector(STRSXP,global::persontypes->getTotalNumberOfBins()));
    for (int type = 0; type < global::persontypesnum; type++) {
        SET_STRING_ELT(ptypelabels, type,
            mkChar(global::persontypes->getCreatorName(type).c_str()));
        for (int bin = 0; bin <
            global::persontypes->getCreator(type)->getNumberOfBins(); bin++) 
            SET_STRING_ELT(pbinlabels,
            global::persontypes->linearise(type,bin), 
    mkChar(global::persontypes->getCreator(type)->getBinName(bin).c_str()));
    }

    
    setAttrib(ptype, install("levels"), ptypelabels);
    setAttrib(pbin, install("levels"), pbinlabels);


    ROBJ causelabels;
    PROTECT(causelabels=allocVector(STRSXP,3));
    SET_STRING_ELT(causelabels, 0, mkChar("general"));
    SET_STRING_ELT(causelabels, 1, mkChar("symptoms"));
    SET_STRING_ELT(causelabels, 2, mkChar("notified"));
    
    setAttrib(cause, install("levels"), causelabels);
    
    pos = 0;
    SET_VECTOR_ELT(ans,pos++, gptype);
    SET_VECTOR_ELT(ans,pos++, time);
    SET_VECTOR_ELT(ans,pos++, cause);
    SET_VECTOR_ELT(ans,pos++, puid);
    SET_VECTOR_ELT(ans,pos++, ptype);
    SET_VECTOR_ELT(ans,pos++, pbin);
    SET_VECTOR_ELT(ans,pos++, linknumber);
    SET_VECTOR_ELT(ans,pos++, nid);
    SET_VECTOR_ELT(ans,pos++, dirtreated);
    SET_VECTOR_ELT(ans,pos++, tested);
    SET_VECTOR_ELT(ans,pos++, posresults);
    SET_VECTOR_ELT(ans,pos++, ntype);
        
    UNPROTECT(1+12+5);

    
    return(ans);
}

















ROBJ rif_getSchedulerSizes()
{
    ROBJ ans;
    PROTECT(ans = allocVector(INTSXP, 3));
    int *sizes = global::scheduler->getSizes();
    for (int i = 0; i < 3; i++) 
        INTEGER(ans)[i] = sizes[i];
    delete[] sizes;
    UNPROTECT(1);
    return(ans);
}

ROBJ rif_getEventQueue(ROBJ _number, ROBJ _includetext, ROBJ _activeonly)
{
    int number = INTEGER(_number)[0];
    bool includetext = INTEGER(_includetext)[0];
    bool activeonly = INTEGER(_activeonly)[0]; 
    
    ROBJ ans;
    PROTECT(ans = allocVector(VECSXP,4));
    
    ROBJ idclass, time, active, text;
    int *sizes = global::scheduler->getSizes();
    if (activeonly && sizes[2] < number) number = sizes[2];
    if (!activeonly && sizes[1] < number) number = sizes[1];
    
    PROTECT(idclass = allocVector(INTSXP, number));
    PROTECT(time = allocVector(REALSXP, number));
    PROTECT(active = allocVector(LGLSXP, number));
    PROTECT(text = allocVector(STRSXP, number));
        
    Event *el[sizes[1]];
    
    int pos = 0;
    int len = 0;
    
    while(pos < number) {
        Event *e = global::scheduler->internalGetEvent();
        if (e->active || !activeonly) {
            INTEGER(idclass)[pos] =  e->idsubclass - CLASSEVENT;
            REAL(time)[pos] = e->time;
            LOGICAL(active)[pos] = e->active ? TRUE : FALSE;
            if (includetext) {
                SET_STRING_ELT(text, pos, mkChar(e->str().c_str()));            
       
            } else {
                SET_STRING_ELT(text, pos, R_NaString);
            }
            pos++;
        }
        el[len++] = e;
    }
    
    for (int i = 0; i < len; i++) {
        global::scheduler->insert(el[i]);
    }
    
    ROBJ classlabels;
    PROTECT(classlabels = allocVector(STRSXP,14));
    
    pos = 0;
    SET_STRING_ELT(classlabels, pos++, mkChar("GENERIC"));
    SET_STRING_ELT(classlabels, pos++, mkChar("IMMIGRATION"));
    SET_STRING_ELT(classlabels, pos++, mkChar("BIRTH"));
    SET_STRING_ELT(classlabels, pos++, mkChar("PSINITIATE"));
    SET_STRING_ELT(classlabels, pos++, mkChar("AGEABLEDEATH"));
    SET_STRING_ELT(classlabels, pos++, mkChar("AGEABLEBINCHANGE"));
    SET_STRING_ELT(classlabels, pos++, mkChar("HAVESEX"));
    SET_STRING_ELT(classlabels, pos++, mkChar("GETPREGNANT"));
    SET_STRING_ELT(classlabels, pos++, mkChar("REMOVEOLD"));
    SET_STRING_ELT(classlabels, pos++, mkChar("INFECTPERSON"));
    SET_STRING_ELT(classlabels, pos++, mkChar("ABORTION"));
    SET_STRING_ELT(classlabels, pos++, mkChar("VISITGP"));
    SET_STRING_ELT(classlabels, pos++, mkChar("TREAT"));
    SET_STRING_ELT(classlabels, pos++, mkChar("PROVOKEVISITGP"));

    setAttrib(idclass, install("levels"), classlabels);

    SET_VECTOR_ELT(ans, 0, time);
    SET_VECTOR_ELT(ans, 1, idclass);
    SET_VECTOR_ELT(ans, 2, active);
    SET_VECTOR_ELT(ans, 3, text);
    
    UNPROTECT(6);
    delete[] sizes;
    return(ans);    
}

}

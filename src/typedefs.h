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

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <list>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

class Ageable;
class Creator;
class CreatorCollection;

class Distribution;
class DistributionConstant;
class DistributionExponential;
class DistributionWeibull;
class DistributionUniform;
class DistributionArray;
class DistributionHost;
class DistributionDiscrete;
class DistributionContinuous;
class DistributionConstantContinuous;
class DistributionPoissonProcess;


class Process;
class Event;
class EventDeath;
class EventBirth;
class EventGetPregnant;
class EventAbortion;
class EventImmigration;
class EventBinChange;
class EventPSInitiate;
class EventHaveSex;
class EventRemoveOld;
class EventVisitGP;
class EventProvokeGPVisit;
class EventTreat;
class EventInfectPerson;

class Notifier;

class Person; 
class PersonFemale; 
class PersonMale; 
class PersonCreator;
class PersonCreatorFemale;
class PersonCreatorMale;

class Partnership;
class PSCreator;
class PSFormer;
class PSFormerIndivSearch;

class Infection;
class InfectionCreator;

class Scheduler;
class Population;

class GPVisitCreator;

enum ConditioningType {
    NoConditioning = 0,
    OnAge,
    OnBirth,
    OnTime
};

enum TypeOfTreatment {
    TreatmentGeneral = 0,
    TreatmentSpecific    
};

enum CauseVisitGP {
    CauseSymptomsGeneral = 0,
    CauseSymptomsSpecific,
    CauseNotified
};

enum NumberOf {
    NumberOfNothing = 0,
    NumberOfBin,             // The bin in which 'Ageable' is now
    NumberOfType,            // The type of the Ageable
    IsPregnant,               // 1 if pregnant, 0 otherwise
    IsActive,
    NumberOfPregnancies,    
    NumberOfChildren,
    NumberOfAbortions,
    NumberOfContacts,
    NumberOfContactsUnprotected,
    NumberOfPartnersCurrent, // Number of current partners
    NumberOfPartnersCurrentType0, // Number of current partners of Type 0
    NumberOfPartnersCurrentType1, // Number of current partners of Type 1
    NumberOfPartnersCurrentType2, // Number of current partners of Type 2
    NumberOfPartnersCurrentType3, // Number of current partners of Type 3
    NumberOfPartnersCurrentType4, // Number of current partners of Type 4
    NumberOfPartnersWithin, // Number of partners that ended with last year
                            //+current partners
    NumberOfPartnersTotal,   // Number of total partners
    NumberOfInfectionsCurrent,
    NumberOfInfectionsWithin,
    NumberOfInfectionsTotal,
    NumberOfTreatments,
    NumberOfLink,
    NumberOfGPVisits,
    NumberOfNotifications,
    NumberOfAlreadyNotified,
    NumberOfPositiveTests,
    NumberProduct,
    NumberSum
};

enum CauseDeletion {
    CauseDeath = 0,
    CauseEmigration
};

enum CauseCreation {
    CausePopulate = 0,
    CauseBirth,
    CauseImmigration,
    CauseReplacement
};

enum CauseAbortion {
    CauseDeathMother = 0,
    CauseChlamydia
};

enum EnumRemove{
    RemovePartnerships = 0,
    RemoveInfections
};

/** The type to move events around. At the moment just a pointer to the class
Event */
typedef Event* EventID;

/** The type to represent a 'type'. At the moment just an integer, ranging from
0 to the actual number of types - 1 (eg types of people: 0='male',
1='female', eg types of infections: 0='chlamydia', 1='gonorrhoea',
2='HIV'). There is a constant  defined in constants.h which represents the
NA in R and stands for 'not available' */
typedef int Type;

/** The type to represent a 'type'. At the moment just an integer. There is a 
constant BINNA defined in constants.h which represents the NA in R and stands
for 'not available' */
typedef int Bin;
/** The type to represent an attribute. At the moment just an integer. There is
a constant ATTRIBUTENA defined in constants.h which represents the NA in R and
stands for 'not available' */
typedef int Attribute;

/** A type which represents a time point or a duration. At the moment just a
double. There is a constant TIMENA defined in constants.h which represents the
NA in R and stands for 'not available' */
typedef double Time;

/** A type which represents a scalar value, such as sampling from a
distribution, a probability, etc. At the moment just a double */
typedef double Value;

/** A type for things that are counted. At the moment just an integer (signed)
*/
typedef int Number;
typedef Distribution *** PairAttribute;
typedef unsigned long long int Counter;

typedef struct {
    Time reltime;
    Bin from, to;
} BinChange;

typedef struct {
    Type formertype;
    double fitness;
    int tries;
} PSFInfo;

typedef struct {
    Counter nid;
    Number linknumber;
} Notification;

typedef struct {
    // information about the notification
    Type ntype;
    Counter nid;
    Number linknumber;   // link number of the sender
    Time time;
    // infos about sender
    Counter puid1;
    Type ptype1;
    Bin  pbin1; //at the time of notification
    // infos about receiver
    Counter puid2;
    Type ptype2;
    Bin  pbin2; //at the time of notification
    // infos about partnership
    Counter psuid;
    Type pstype; 
    Bin  psbin;
} SingleNotification;

typedef struct {
    // information about the gpvisit
    Type gptype;
    Counter nid; // ID of the notification chain
    Number linknumber;   // link number of the person
    Time time;
    Type ntype; // the notification type if started, otherwise TYPENA
    enum CauseVisitGP cause;
    Counter dirtreated;
    Counter tested;
    Counter posresults;
    // infos about visiting person
    Counter puid;
    Type ptype;  
    Bin  pbin;  //at the time of visit
} SingleGPVisit;

typedef struct {
    unsigned int n;      //length
    int *ix;    //integer version of x-values
    double *x;  // x-values
    double *y;  // y-values
    double f;   // a factor before the y values
} PiecewiseFunction;



typedef struct {
    int pos_v;
    std::list<Person*>::iterator pos_l;
    int adhoc; // this is used to number the population from 1 to size and is
             //needed to produce R network objects
} PopID;

typedef struct {
    std::list<Partnership*>::iterator pos_l;
} PartnershipID;

typedef std::list<Person*> PeopleList;
typedef std::vector<Person*> PeopleVector;
typedef std::list<Partnership*> PartnershipList;
typedef std::list<Infection*> InfectionList;
typedef std::list<Notification> NotificationList;
typedef std::list<SingleNotification> SingleNotificationList;
typedef std::list<SingleGPVisit> SingleGPVisitList;

#endif

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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "typedefs.h"
#include "rif.h"

namespace global {

/**
 * Initialises the global variables, such as all the counters, the seed, the
 * variables of type CreatorCollection (but without reading those sections, yet;
 * this is done individually with the functions personInitVars(),
 * infectionInitVars(), etc.; see initdel.h). This function is called before 
 * anything else; see entrypoints.cpp
 * @see rif_initModel(), rif_removeModel()
 */
void globalInitVars(bool keepquiet);
/**
 * Removes the variables of type CreatorCollection from the memory. This
 * function is called after the other [...]DelVars() functions, such as
 * personDelVars(), infectionDelVars(), etc. are called; see initdel.h 
 * @see rif_initModel(), rif_removeModel()
 */
void globalDelVars();
// Global classes
/** 
 * The variable that point to the single instance of class Scheduler which
 * represents and manages the event queue.
 * @see class Scheduler
 */ 
extern Scheduler *scheduler;
/** 
 * The variable that point to the single instance of class Population which
 * manages the population.
 * @see class Population
 */ 
extern Population *population;

/**
 * A pointer to a distribution that can be used for testing and is constructed
from the section 'test' in the configuration file.
 * @see implementation of globalInitVars()
 */
extern Distribution *testdistribution;

/**
 * The variable containing the nested list structure representing the
 * configuration file; this variable can be obtained by the 'sti.config()'
 * function in R
 */
extern ROBJ cfg;

/** 
 * The variable containing the current time of the simulation; updated only by
 * Event::execute()
 */ 
extern Time abstime;

/**
 * The seed that is beeing used for the run; the value of this variable is only
 * used  to initialise the actual seed of the random number generator and does
 * not change, so that it can be queried by the 'sti.seed()' function in R
 * @see implementation of globalInitVars()
 */
extern int originalseed;


/** 
 * Flag whether there should be output generated
 */
extern bool keepquiet;

/**
 * Counter for the universal id of instances of the Event class and its progeny
 */
extern Counter countereuid;
/**
 * Counter for the universal id of instances of the Object class and its progeny
 */
extern Counter counteruid;
/**
 * Counter for the universal id of instances of the Person class and its progeny
 */
extern Counter counterpuid;
/**
 * Counter for the universal id of instances of the Partnerhsip class and its
 * progeny
 */
extern Counter counterpsuid;
/**
 * Counter for the universal id of instances of the Infection class and its
 * progeny
 */
extern Counter counterinfuid;
/**
 * Counter for the strain id that each infection has; this number is not unique,
 * but inherited from infection to infection 
 */
extern Counter counterstrainid;
/**
 * Counter for the id of a notification tree; this number is not unique,
 * but inherited from notification to notification
 */
extern Counter counternotifid;


// Count the number of events, total and relative
/** The total number of handled events until now */
extern Counter statsevents;
/** The total number of births of people until now; this includes natural
births or *  replacements, but not immigration and initial population */
extern Counter statsbirths;
/** The total number of deaths of people until now; not including emigrations*/
extern Counter statsdeaths;
/** The total number of immigrations of people until now*/
extern Counter statsimmigrations;
/** The total number of emmigrations until now*/
extern Counter statsemigrations;
/** The total number of bin changes of people until now*/
extern Counter statspersonbinchanges;
/** The total number of bin changes of partnerships until now*/
extern Counter statspartnershipbinchanges;
/** The total number of bin changes of infections until now*/
extern Counter statsinfectionbinchanges;
/** The total number of created partnerships until now*/
extern Counter statspscreated;
/** The total number of parterships that ended until now*/
extern Counter statspsended;
/** The total number sexual contacts within partnerships */
extern Counter statscontacts;
/** The total number of unprotected contacts within partnerships */
extern Counter statscontactsunprot;
/** The total number of pregnancies until now; this includes the females that
 * are pregnant now; that is, the counter is updated at the begining of each
 * pregnancy */
extern Counter statspregnancies;
/** The total number of newly people that were infected until now; this does not
include people that are initially infected */
extern Counter statsinfections; 
/** The total number of natural clearances in the population */
extern Counter statsclearances; 
/** The total number of tests conducted in the population */
extern Counter statstests;
/** The total number of treatments in the population; if a infection was
 * cleared between a person seeks treament and the actual treatment, it is still
 * counted as a treatment, because the treatment is nevertheless assumend to
 * take place */
extern Counter statstreatments; 
/** The number of treatments in the population that were conducted although
the specific infection was not present (either cleared naturaly in the
meantime or due to a false positive test result */
extern Counter statstreatmentsvain;
/** The number of notifications that led to a GP visit of the partner */
extern Counter statsfollowupvisits;

/** A variable containing the Time after which partnerships should be
 * definitely removed from the memory; see
 * 'simulation.remove.partnershipsolderthan' in the configuration file */
extern Time removepsolderthan;
/** A variable containing the Time after which infections should be
 * definitely removed from the memory; see
 * 'simulation.remove.partnershipsolderthan' in the configuration file; if the
 infection is still active after this time, it will not be removed, however */
extern Time removeinfolderthan;
/** A variable that stores the duration for the 'within' property of number of
infections, number of partnerships */
extern Time withintimelag;

/***************************************************************************
 *  These parameters can be changed through the config file
 ***************************************************************************/

/** The collection of infectio types */
extern CreatorCollection *infectiontypes;
/** The number of different infection types
 * @see class Person */
extern Number infectiontypesnum;

/** A variables that stores the maximal age that people may have; calculated
 * from the configuration file */
extern double peoplemaxage;
/** A variables that stores the minimum age that people may have; calculated
 * from the configuration file */
extern double peopleminage;

/** The collection of person types
 * @see class Person */
extern CreatorCollection *persontypes;
/** The number of person types */
extern Number persontypesnum;

/** The collection of partnerships formers */
extern CreatorCollection *psftypes;
/** The number of partnership formers */
extern Number psftypesnum;
/** The collection of partnership creators */
extern CreatorCollection *psctypes;
/** The number of partnership creators */
extern Number psctypesnum;

/** The collection of infection types */ 
extern CreatorCollection *infectiontypes;
/** The number of infection types */
extern Number infectiontypesnum;

/** The collection of GPVisit types */
extern CreatorCollection *gpvisittypes;
/** The number of GPVisit types */
extern Number gpvisittypesnum;

/** A constant representing the state "not notified" */
extern Notification NOTNOTIFIED;

/** The collection of Partnernotifiers */
extern CreatorCollection *notifiertypes;
/** The number of partner notifiers */
extern Number notifiertypesnum;

}

#endif


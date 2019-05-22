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

#include "person.h"
#include "global.h"
#include "rangen.h"
#include "scheduler.h"
#include "population.h"
#include "rif.h"
#include "distribution.h"
#include "event.h"

#include <time.h>

using namespace std;

namespace global {

Scheduler *scheduler;
Population *population;

Distribution *testdistribution;

ROBJ cfg;

double abstime;

int originalseed;

bool keepquiet;

Counter countereuid;

Counter counteruid;
Counter counterpuid;
Counter counterpsuid;
Counter counterinfuid;
Counter counterstrainid;
Counter counternotifid;

Counter statsevents;
Counter statsdeaths;
Counter statsbirths;
Counter statsimmigrations;
Counter statsemigrations;
Counter statspersonbinchanges;
Counter statspartnershipbinchanges;
Counter statsinfectionbinchanges;
Counter statspscreated;
Counter statspsended;
Counter statscontacts;
Counter statscontactsunprot;
Counter statspregnancies;
Counter statsinfections;
Counter statsclearances;
Counter statstests;
Counter statstreatments;
Counter statstreatmentsvain;
Counter statsfollowupvisits;

double removepsolderthan;
double removeinfolderthan;
double withintimelag;

Notification NOTNOTIFIED;

CreatorCollection *persontypes;
Number persontypesnum;

double peoplemaxage;
double peopleminage;

CreatorCollection *psftypes;
Number psftypesnum;
CreatorCollection *psctypes;
Number psctypesnum;

CreatorCollection *infectiontypes;
Number infectiontypesnum;

CreatorCollection *gpvisittypes;
Number gpvisittypesnum;

CreatorCollection *notifiertypes;
Number notifiertypesnum;

void globalDelVars() {
    // Delete the population, scheduler and test distribution objects from the
    // memory
    if (population) { delete population; population = 0; }
    if (scheduler) { delete scheduler; scheduler = 0; }
    if (testdistribution) { delete testdistribution; testdistribution = 0; }
}

void globalInitVars(bool _keepquiet)
{
    /* The following variables are set to 0 before loading anything from the
     * configuration file, so that if an error occurs somewhere, we still know
     * which parts actually were initiated and have to be deleted before trying
     * again 
     */ 
     
    // Set pointers to 0
    scheduler = 0;
    population = 0;
    testdistribution = 0;

    // Set time to 0
    abstime = 0.0;

    // Set seed for random number generation to 0
    originalseed = 0;

    // Set message output to true
    keepquiet = _keepquiet;
    
    // Set global counters to 0
    countereuid     = 0;
    counteruid      = 0;
    counterpuid     = 0;
    counterpsuid    = 0;
    counterinfuid   = 0;
    counterstrainid = 0;
    counternotifid = 0;

    // Set counters for global statistics to 0
    statsevents             = 0;
    statsdeaths             = 0;
    statsbirths             = 0;
    statsimmigrations       = 0;
    statsemigrations        = 0;
    statspersonbinchanges   = 0;
    statspartnershipbinchanges= 0;
    statsinfectionbinchanges= 0;
    statspscreated          = 0;
    statspsended            = 0;
    statscontacts           = 0;
    statscontactsunprot     = 0;
    statspregnancies        = 0;
    statsinfections         = 0;
    statsclearances         = 0;
    statstests              = 0;
    statstreatments         = 0;
    statstreatmentsvain     = 0;
    statsfollowupvisits     = 0;
    
    // Initialise 'removeolder' feature variables
    removepsolderthan = MAXDOUBLE;
    removeinfolderthan = MAXDOUBLE;
    withintimelag = 365.0;

    // Set the person type collection to 0
    persontypes = 0;
    persontypesnum = 0;

    // Set minimal and maximal age to most extreme values
    peoplemaxage = 0;
    peopleminage = MAXDOUBLE;
    
    // Set partnership former type collection to 0
    psftypes = 0;
    psftypesnum = 0;
    
    // Set partnership creator type collection to 0
    psctypes = 0;
    psctypesnum = 0;

    // Set infection type collection to 0
    infectiontypes = 0;
    infectiontypesnum = 0;
    
    // Set gpvisit type collection to 0
    gpvisittypes = 0;
    gpvisittypesnum = 0;

    // Set notifier type collection to 0
    notifiertypes = 0;
    notifiertypesnum = 0;
    
    // Initialise the global variable for "not notified"
    NOTNOTIFIED.nid = 0;
    NOTNOTIFIED.linknumber = 0;
    
    // Load the seed from the configuration file...
    ROBJ cfg_seed = rif_trylookup(cfg, "simulation.seed");
    if (!rif_isNull(cfg_seed)) {
        originalseed = rif_asInteger(cfg_seed);
    } else {
    // ... or initialise with current time
        originalseed = time(0);
    }
    
    // Init the actual random number generator with the above seed
    ran::sran((unsigned int)originalseed);
        
    // Init the scheduler
    // Get the 
    int elsf = 50; //default 50 times the population size
    if (rif_exists(cfg,"simulation.eventlistsizefactor")) {
        elsf = rif_asInteger(cfg,0,"simulation.eventlistsizefactor"); 
    }     
    scheduler = new Scheduler(
        elsf *rif_asInteger(cfg,0,"model.population.size"));
    
    // Init the population object 
    population = new Population(rif_lookup(cfg,"model.population"));
    
    // Init the 'removeolderthan' feature
    // ... for partnerships
    if (rif_exists(cfg,"simulation.remove.partnershipsolderthan")) {
        removepsolderthan =
            rif_asDouble(cfg,0,"simulation.remove.partnershipsolderthan");
    } else {
        warning("keeping all partnerships in memory");
    }
    //  ... install the corresponding event
    if (removepsolderthan < MAXDOUBLE) {
        global::scheduler->insert(new EventRemoveOld(
            abstime+removepsolderthan, RemovePartnerships));
    }
    
    // ... and for infections
    if (rif_exists(cfg,"simulation.remove.infectionsolderthan")) {
        removeinfolderthan =
            rif_asDouble(cfg,0,"simulation.remove.infectionsolderthan");
    } else {
        warning("keeping all infections in memory");        
    }
    //  ... install the corresponding event
    if (removeinfolderthan < MAXDOUBLE) {
        global::scheduler->insert(new EventRemoveOld(
            abstime+removeinfolderthan, RemoveInfections));
    }
    
    // load the withintime lag
    if (rif_exists(cfg,"simulation.withintimelag")) {
        withintimelag = rif_asDouble(cfg,0,"simulation.withintimelag");
    }
    
    // Load the distribution defined in the 'test' section
    ROBJ cfg_testdist = rif_trylookup(cfg,"test.distribution");
    
    testdistribution = !rif_isNull(cfg_testdist) ?
        createDistribution(cfg_testdist) : 0;
}

} // end namespace global

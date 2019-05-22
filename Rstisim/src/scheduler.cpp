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
#include "scheduler.h"

#include <string>

#include "global.h"
#include "event.h"


using namespace std;

Scheduler::Scheduler(int maxsize)
{
    sizemax = maxsize;
    sizeactive = 0;
    sizeall = 0;
    el = new BinaryHeap<Event*>(maxsize);
}


Scheduler::~Scheduler()
{
    Event *e;
    
    try {
        while(1){
            el->deleteMin(e);
            delete e;
        }   
    } catch (Underflow) {}
    delete el;
}

int* Scheduler::getSizes() const
{
    int *sizes = new int[3];
    sizes[0] = sizemax;
    sizes[1] = sizeall;
    sizes[2] = sizeactive;
    return(sizes);
}

EventID Scheduler::insert(Event *e) 
{   
    if (e->time < global::abstime) {
        string msg("Event inserted before current time, ");
        msg += e->str();
        error(msg.c_str());
    } else {
        try {
            el->insert(e);
        } catch(Overflow of) {
            error((string("event list overflow; try to increase 'eventlistsizefactor' in the 'simulation' section\n"
            + e->str())).c_str());
        } 
        sizeall++;
        if (e->active) {
            sizeactive++;
        }
    }
    return((EventID)e);
}

EventID Scheduler::remove(EventID e)
{
    e->active = false;
    if (!e->executed) sizeactive--;
    return(e);
}

Event *Scheduler::internalGetEvent()
{
    Event *e;
    el->deleteMin(e);
    sizeall--;
    if (e->active) sizeactive--;
    return(e);    
}

void Scheduler::executeNext(bool verbose)
{
    Event *e;
   
    while(1) {
        el->deleteMin(e);
        sizeall--;
        if (e->active) {
            sizeactive--;
            break;
        }
        delete e;
    } 
    
    if (verbose) {
        Rprintf("Executing %s\n",e->str().c_str());
    }
    e->execute();
    delete e;
}

int Scheduler::executeBy(Time time)
{
    double stoptime = global::abstime + time;
    int counts = 0;
    
    Event *e;
    try {
        e = el->findMin();
    } catch (Underflow) {
            global::abstime = stoptime;
            return(counts);
    }
    
    while (e->time <= stoptime) {
        try {
            executeNext();
            counts++;
        } 
        catch (Underflow) {
            break;
        }
        try {
            e = el->findMin();
        } catch (Underflow) {
            global::abstime = stoptime;
            return(counts);
        }
    }
    global::abstime = stoptime;
    return(counts);
}

int Scheduler::executeEvents(int number, bool verbose)
{
    int i;
    
    for (i = 1; i <= number; i++) {
        try {
            executeNext(verbose);
        }
        catch (Underflow) {
            break;
        }
    }
    i--;
    return(i);
}


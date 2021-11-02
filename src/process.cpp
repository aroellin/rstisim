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

#include "event.h"
#include "scheduler.h"
#include "global.h"

using namespace std;


Process::Process() 
{
    nextevent = 0;
    lastexecution = global::abstime;
}

Process::~Process() 
{
    if (nextevent) {
        global::scheduler->remove(nextevent);
    }
}

void Process::update(EventID event) 
{
    if (nextevent) {
        global::scheduler->remove(nextevent);
    }
    nextevent = global::scheduler->insert(event);
}

void Process::replace(EventID event) 
{
    if (nextevent) {
        global::scheduler->remove(nextevent);
    }
    nextevent = global::scheduler->insert(event);
    lastexecution = global::abstime;
}

Time Process::lastTime() const
{
    return(lastexecution);
}

void Process::setLastTime(Time time)
{
    if (time > global::abstime) {
        error("internal: cannot set 'lastexecution' time into the future");
    }
    lastexecution = time;    
}

Time Process::nextTime() const
{
    if (nextevent) {
        return(nextevent->time);
    } else {
        return(MAXDOUBLE);
    }
}

bool Process::empty() const
{
    return(nextevent == 0);
}
void Process::clear()
{
    if (nextevent) {
        global::scheduler->remove(nextevent);
    }
    nextevent = 0;
}

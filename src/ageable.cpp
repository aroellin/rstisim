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
#include "object.h"

#include "global.h"
#include "distribution.h"
#include "event.h"
#include "scheduler.h"

#include <sstream>

using namespace std;

Ageable::Ageable(Creator *_creator)
: Object()
{
    creator = _creator;
    timebirth = global::abstime;
    timedeath = global::abstime;
/*    nextBinChange = 0;
    nextDeath = 0;*/
    binnum = creator->binnum;
    attr = 0;
    attrfixed = 0;
    attrisfixed = 0;
    
    attrfixed = new double*[creator->getCollector()->getNumberOfAttributes()];
    for(int i = 0; i < creator->getCollector()->getNumberOfAttributes(); i++) 
        attrfixed[i] = 0; 
    
    attrisfixed = 
        creator->getCollector()->getAttributesIsFixedArray();
    const bool * attrisfixedperbin = 
        creator->getCollector()->getAttributesIsFixedPerBinArray();
    attr = creator->getCollector()->getAttributesArray();
    
    
    for(int a = 0; a < creator->getCollector()->getNumberOfAttributes(); a++) {
        attrfixed[a] = new double[binnum];
        for (Bin b = 0; b < binnum; b++) {
            if (attrisfixed[a]) {
                if (!attrisfixedperbin[a] && b > 0) {
                    attrfixed[a][b] = attrfixed[a][0];
                } else {
                    bin = b; 
                    attrfixed[a][b] = attr[a]->dsample(this, global::abstime);
                }
            } else {
                attrfixed[a][b] = 0.0;
            }
        }
    }
    
    bin = creator->bindist->isample();
}

Ageable::~Ageable()
{
    for(Attribute attr = 0; 
        attr < creator->getCollector()->getNumberOfAttributes(); attr++) {
        if (attrfixed && attrfixed[attr]) delete[] attrfixed[attr];
    }
    if (attrfixed) delete[] attrfixed;
}

Number Ageable::getNumber(NumberOf what) const
{ 
    switch(what) {
        case NumberOfBin: 
            return(bin);
        case NumberOfType:
            return(creator->getType());
	case IsActive:
	    return(isAlive());
        default: 
            return(0);
    }; 
}

Value Ageable::getAttribute(Attribute a, Time now) const
{
    if (attrisfixed[a]) { 
        return(attrfixed[a][bin]);
    } else { 
        return(attr[a]->dsample(this, now));
    }
}

Value Ageable::getAttribute(Attribute a, Time now, Value atleast) const
{
    if (attrisfixed[a]) {
        if (atleast > attrfixed[a][bin]) {    
            rif_error(creator->getCfg(), 
"trying to condition on attribute that was fixed at birth, but conditioning\
value is bigger than attribute");
            return(0);
        } else {
            return(attrfixed[a][bin]-atleast);
        };
    } else { 
        return(attr[a]->dsample(this, now, atleast));
    }
}

Value Ageable::getAttributeFac(Attribute a, Time now, double factor) const
{
    if (attrisfixed[a]) { 
        rif_error(creator->getCfg(), 
            "cannot sample with factor if attribute was fixed at birth",
            __LINE__);
        return(0);
    } else { 
        return(attr[a]->dsamplefac(factor, this, now));
    }
}

Value Ageable::getAttributeFac(Attribute a, Time now, double factor, Value
atleast) const
{
    if (attrisfixed[a]) {
        rif_error(creator->getCfg(), 
            "cannot sample with factor if attribute was fixed at birth",
__LINE__);
        return(0);
    } else { 
        return(attr[a]->dsamplefac(factor, this, now, atleast));
    }
}


std::string Ageable::str() const
{
    ostringstream s;
    CreatorCollection *collection = creator->getCollector();
    s << "Ageable:"
        << "birth=" << timebirth
        << ",death=" << timedeath
        << ",age=" << getAge()/DAYSINYEAR << "y" 
        << ",lifespan=" << (timedeath-timebirth)/DAYSINYEAR << "y"
        << ",type='" << creator->getTypeName() << "'"
        << ",bin='" << creator->getBinName(bin) << "'";
    if (collection->getNumberOfAttributes()>0) {
        s << ",attribtes=[";
        for (Attribute attr = 0; attr < collection->getNumberOfAttributes();
attr++) {
            s << "'" << collection->getAttributeName(attr) << "'";
            if (collection->getAttributesIsFixedPerBinArray()[attr]) {
                s << "(fixedperbin)=(";
                for (Bin b = 0; b < binnum; b++) {
                    s << attrfixed[attr][b] << ",";
                }
                int pos = s.tellp();
                s.seekp(pos-1);
                s << ")";
            } else if (attrisfixed[attr]) {
                s << "(fixed)=" << attrfixed[attr][0];
            } else {
                s << "(random)";
            }
            s << ",";
        }
        int pos = s.tellp();
        s.seekp(pos-1);
        s << "]";
    }        
    s << "|" << Object::str();
    return (s.str());

}

        
void Ageable::slotDeath()
{
/*    if (nextBinChange) {
        global::scheduler->remove(nextBinChange);
        nextBinChange = 0;
    }    
    if (nextDeath) {
        global::scheduler->remove(nextDeath);
        nextDeath = 0;
    }*/
    procDeath.clear();
    procBinChange.clear();
    timedeath = global::abstime;
}

void Ageable::slotBinChange(Bin from, Bin to)
{
    if (bin != from) {
        error("trying to change from bin person is not in");
    }
    bin = to;
    procBinChange.setLastTime();
    throwEventBinChange();
}

    
void Ageable::throwEventBinChange(bool conditional)
{
    BinChange bc = sampleBinChange(bin, global::abstime, conditional);
    if (bc.to != BINNA && global::abstime + bc.reltime < timedeath) {
        procBinChange.update(new EventBinChange(
                        global::abstime + bc.reltime,this, 
                        bc.from, bc.to));
    } else { 
        procBinChange.clear();
    }
}

BinChange Ageable::sampleBinChange(Bin from, Time now, bool conditional)
{
    BinChange bc;
    bc.reltime = MAXDOUBLE;
    bc.from = from;
    bc.to = BINNA;
        
    if (conditional) {
        Time timediff = now - procBinChange.lastTime();
        if (timediff < 0) {
            rif_error(creator->getCfg(),
"internal: cannot sample conditional bin change if last change is\
later than now"
            );
        }
        for (int trans = 0; trans < creator->bintransnum; trans++) {
            if (creator->bintransfrom[trans] == from) {
                double nexttime = 
                    creator->bintransdist[trans]->
                    dsample(this, procBinChange.lastTime(),timediff);
                if (nexttime > 0 && nexttime < bc.reltime) {
                    bc.reltime = nexttime;
                    bc.to = creator->bintransto[trans];
                }
            }
        }
    } else {
        for (int trans = 0; trans < creator->bintransnum; trans++) {
            if (creator->bintransfrom[trans] == from) {
                Time nexttime = 
                    creator->bintransdist[trans]->dsample(this, now);
                if (nexttime > 0 && nexttime < bc.reltime) {
                    bc.reltime = nexttime;
                    bc.to = creator->bintransto[trans];
                }
            }
        }
    }
    return(bc);
}

void Ageable::throwEventDeath(bool conditional)
{
        
    if (conditional) {
        rif_error(creator->getCfg(), 
            "internal: conditional sampling for death not implemented yet");
    }
    procDeath.replace(new EventDeath(timedeath,this));
}



Time Ageable::getTimeBirth() const 
{
    return(timebirth);
}
        
Time Ageable::getTimeDeath() const 
{
    return(timedeath);
}
        
Time Ageable::getAge() const 
{
    return(global::abstime - timebirth);
}
        
Time Ageable::getAge(Time now) const 
{
    return(now - timebirth);
}
        
bool Ageable::isAlive() const
{
    return(timebirth <= global::abstime && global::abstime<timedeath);
}
        
Bin Ageable::getBin() const
{
    return(bin);
}
        
int Ageable::getBinLinearised() const 
{
    return(creator->linearise(bin));
}

Type Ageable::getType() const 
{
    return(creator->getType());
}

Creator * Ageable::getCreator() const 
{
    return(creator);
}
    

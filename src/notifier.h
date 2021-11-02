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
#ifndef NOTIFIER_H
#define NOTIFIER_H

#include "typedefs.h"
#include "object.h"
#include "global.h"

class Notifier : public Creator
{
public:
    Notifier(CreatorCollection *collection, std::string name, ROBJ cfg);
    ~Notifier();

    void notifyPartners(
        Person *person, Notification notif = global::NOTNOTIFIED
    );
    
    std::string getBasetype() const { return("GENERIC"); };
    
protected : 
    Attribute useorforgoback;
    Attribute gobacktime;
    Attribute gobackpartners;
    PairAttribute probabilitycouple;
    Attribute probabilityfactorsender;
    Attribute probabilityfactorreceiver;
    Attribute probabilityfactorpartnership;
    
    PairAttribute waitreaction;
    Attribute waitfacsender;
    Attribute waitfacreceiver;
    
    Attribute gpvisittype;
    
    Distribution *uniform;
};

#endif

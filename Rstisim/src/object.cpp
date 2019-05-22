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
#include <sstream>

#include "object.h"
#include "global.h"

using namespace std;

// 
// BEGIN IMPLEMENTATION OF BASE CLASS Object
// 

Object::Object()
{
    // update and set the universal object ID
    uid = ++global::counteruid;
    // set the creation time
    timecreated = global::abstime;
    // set class and subclass IDs
    idclass = CLASSGENERIC;
    idsubclass = CLASSGENERIC;
}

Object::~Object()
{
    // nothing to do
}

string Object::str() const
{
    ostringstream s;
    s << "Object:"
        << "UID=" << uid 
        << ",created=" << timecreated
        << ",idclass=" << idclass
        << ",idsubclass=" << idsubclass;
    return(s.str());
}

void Object::print() const
{
    if (!global::keepquiet) Rprintf("%s\n", str().c_str());
}

// 
// END IMPLEMENTATION OF BASE CLASS Object
// 



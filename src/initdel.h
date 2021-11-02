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
#ifndef INITDEL_H
#define INITDEL_H


#include "object.h"

void personPreInitVars();
void personInitVars();
void personPreDelVars();                 
void personDelVars();                 

void partnershipPreInitVars();
void partnershipInitVars();
void partnershipPreDelVars();
void partnershipDelVars();

void infectionPreInitVars();
void infectionInitVars();
void infectionPreDelVars();                 
void infectionDelVars();                 

void gpvisitPreInitVars();
void gpvisitInitVars();
void gpvisitPreDelVars();                 
void gpvisitDelVars();                 

void notifyPreInitVars();
void notifyInitVars();
void notifyPreDelVars();                 
void notifyDelVars();                 

#endif

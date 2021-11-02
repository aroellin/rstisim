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
#include "rif.h"
#include "constants.h"

using namespace std;


bool rif_isNull(ROBJ obj) 
{
    return(isNull(obj));
}

ROBJ rif_getListElement(ROBJ list, const char *name) 
{
    if (*name == 0) return(list);
    
    ROBJ elmt = R_NilValue, names = getAttrib(list, R_NamesSymbol);
    
    if (!isString(names)) return(R_NilValue);
    
    for (int i = 0; i < length(list); i++) {
        if (!strcmp(CHAR(STRING_ELT(names,i)), name)) {
            elmt = VECTOR_ELT(list,i);
            break;
        }
    }
    return(elmt);
}

ROBJ rif__lookup(ROBJ list, char *path) 
{
    char *nextpoint = strchr(path,'.');
    if (nextpoint) {
        *nextpoint = 0;
        ROBJ sublist = rif_getListElement(list, path);
        return(rif__lookup(sublist,nextpoint+1));
    } else {
        return(rif_getListElement(list,path));
    }
}

ROBJ rif_lookup(ROBJ list, const char* path)
{
    char cpath[200];
    strcpy(cpath,path);
    ROBJ ans = rif__lookup(list,cpath);
    if (isNull(ans)) {
        rif_error(list, string("element not found: ") + path);
    } 
    ROBJ used;
    PROTECT(used = allocVector(LGLSXP,1));
    LOGICAL(used)[0] = TRUE;
    setAttrib(ans, install("used"), used);
    UNPROTECT(1);
    return(ans);
}

ROBJ rif_trylookup(ROBJ list, const char* path)
{
    char cpath[200];
    strcpy(cpath,path);
    ROBJ ans = rif__lookup(list,cpath);
    if (!isNull(ans)) {
        ROBJ used;
        PROTECT(used = allocVector(LGLSXP,1));
        LOGICAL(used)[0] = TRUE;
        setAttrib(ans, install("used"), used);
        UNPROTECT(1);
    }
    return(ans);
}

int rif_asInteger(ROBJ obj, int index, const char *path)
{
    ROBJ value = path ? rif_trylookup(obj,path) : obj;
    if (isNull(value)) {
        rif_error(obj, string("rif_asInteger: Missing value '") + path + string("'"));
    }
    return(INTEGER(value)[index]);
}

double rif_asDouble(ROBJ obj, int index, const char *path)
{
    ROBJ value = path ? rif_trylookup(obj,path) : obj;
    if (isNull(value)) {
        rif_error(obj, string("rif_asDouble: Missing value '") + path + string("'"));
    }

    if (rif_isInteger(value)) {
        return((double)(INTEGER(value)[index]));
    } else {
        return(REAL(value)[index]);
    }
}

string rif_asString(ROBJ obj, int index, const char *path)
{
    ROBJ value = path ? rif_trylookup(obj,path) : obj;

    if (isNull(value)) {
        rif_error(obj, string("rif_asString: Missing value ") + path + string("'"));
    }

    return(string(CHAR(STRING_ELT(value,index))));
}

bool rif_exists(ROBJ obj, const char *path) 
{
    char cpath[200];
    strcpy(cpath,path);
    ROBJ ans = rif__lookup(obj,cpath);
    if (!isNull(ans)) {
        ROBJ used;
        PROTECT(used = allocVector(LGLSXP,1));
        LOGICAL(used)[0] = TRUE;
        setAttrib(ans, install("used"), used);
        UNPROTECT(1);
        return(true);
    } else {
        return(false);
    }
}

bool rif_isInteger(ROBJ obj, const char *path)
{
    ROBJ value = path ? rif_lookup(obj,path) : obj;
    return(isInteger(value));
}

bool rif_isDouble(ROBJ obj, const char *path)
{
    ROBJ value = path ? rif_lookup(obj,path) : obj;
    return(isReal(value) || isInteger(value));
}

bool rif_isString(ROBJ obj, const char *path)
{
    ROBJ value = path ? rif_lookup(obj,path) : obj;
    return(isString(value));
}

bool rif_isList(ROBJ obj, const char *path)
{
    ROBJ value = path ? rif_lookup(obj,path) : obj;
    return(value == R_NilValue ? false : isNewList(value));
}

int rif_getLength(ROBJ obj, const char *path)
{
    ROBJ value = path ? rif_lookup(obj,path) : obj;
    int len = length(value);
    return(len);
}

int rif_getSourceline(ROBJ obj)
{
    ROBJ s = getAttrib(obj, install("sourceline"));
    if (!isNull(s)) {
        return(INTEGER(s)[0]);
    } else {
        return(INTNA);
    }
}

string rif_getPath(ROBJ obj)
{
    ROBJ s = getAttrib(obj, install("path"));
    if (!isNull(s)) {
        return(string(CHAR(STRING_ELT(s,0))));
    } else {
        return(string("?"));
    }
}

void rif_error(ROBJ obj, std::string msg, int sourceline)
{
        ostringstream e;
        int configline = 0;
        ROBJ s = getAttrib(obj, install("sourceline"));
        if (!isNull(s)) {
            configline = INTEGER(s)[0];
        }
        e << msg;
        if (sourceline || configline) {
            e << " (";
        }
        if (configline) {
            e << "configuration line " << configline;
        }
        if (sourceline && configline) {
            e << ", ";
        }
        if (sourceline) {
            e << "source line " << sourceline;
        }
        if (sourceline || configline) {
            e << ")";
        }
        error(e.str().c_str());   
}

void rif_warning(ROBJ obj, std::string msg, int sourceline)
{
        ostringstream e;
        int configline = 0;
        ROBJ s = getAttrib(obj, install("sourceline"));
        if (!isNull(s)) {
            configline = INTEGER(s)[0];
        }
        e << msg;
        if (sourceline || configline) {
            e << " (";
        }
        if (configline) {
            e << "configuration line " << configline;
        }
        if (sourceline && configline) {
            e << ", ";
        }
        if (sourceline) {
            e << "source line " << sourceline;
        }
        if (sourceline || configline) {
            e << ")";
        }
        warning(e.str().c_str());   
}


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

#ifndef RIF_H
#define RIF_H

#include <string>

#include <R.h>
#include <Rinternals.h>

/** This is the basic type to handle R objects and just a wrapper for the R
internal SEXP type, which itself is a pointer. Never manipulate this objet
directly, but use the R interface functions defined in rif.h. */
typedef SEXP ROBJ;

/**
 * Returns a ROBJ object of the element specified by 'path' with respect to 
 * 'list' argument. If the element is not found, an error is thrown. Use this
 * function for elements that are mandatory in the configuration file
 * @param list the configuration list or a part of it in which the element is 
 * @param path the relative name of the element 
 * @return the ROBJ object corresponding to 'path'
 */
extern ROBJ rif_lookup(ROBJ list, const char* path);
/**
 * Same as rif_lookup() but, instead of an error, returns a R_NilValue if the
 * element is not found; use rif_isNull() to check if the element was found
 * @param list the configuration list or a part of it in which the element is 
 * @param path the relative name of the element 
 * @return the ROBJ object corresponding to 'path' or R_NilValue
 */
extern ROBJ rif_trylookup(ROBJ list, const char* path);
/**
 * Checks whether the passed object is R_NilValue or not
 * @param obj the object that needs to be checked, allowed to be R_NilValue
 * @return 'true' if obj equals to R_NilValue, 'false' else
 */
extern bool rif_isNull(ROBJ obj);
/**
 * Extract the integer value out of a ROBJ object, possibly with an index for
 * arrays and a path if the value is with respect to a subelement of 'obj' is
 * requested. If the value is not an integer an error is thrown; if the index is
 * out of bound an error is thrown; if the element specified in 'path' is not
 * found an error is thrown
 * @param obj the object that should be converted to an integer
 * @param index the index of the element if object is an array
 * @param path optional: the relative path, if a subelement of 'obj' is
 * requested
 * @return the value of the element as an integer
 */
extern int rif_asInteger(ROBJ obj, int index = 0, const char *path = 0);
/**
 * Extract the double value out of a ROBJ object, possibly with an index for
 * arrays and a path if the value is with respect to a subelement of 'obj' is
 * requested. If the value is not a REAL, then it is checked whether it is an 
 * INTEGER in which case it is automatically converted to a double, otherwise an
 * error is thrown; if the index is * out of bound an error is thrown; if the
 * element specified in 'path' is not * found an error is thrown
 * @param obj the object that should be converted to a double
 * @param index optional: the index of the element if object is an array
 * @param path optional: the relative path, if a subelement of 'obj' is
 * requested
 * @return the value of the element as a double
 */
extern double rif_asDouble(ROBJ obj, int index = 0, const char *path = 0);
/**
 * Extract the string value out of a ROBJ object, possibly with an index for
 * arrays and a path if the value is with respect to a subelement of 'obj' is
 * requested. If the value is not of STRING type an error is thrown; if the
 * index is out of bound an error is thrown; if the element specified in
 * 'path' is not found, an error is thrown
 * @param obj the object that should be converted to string
 * @param index optional: the index of the element if object is an array
 * @param path optional: the relative path, if a subelement of 'obj' is
 * requested
 * @return the value of the element as an object of standard C++ type
 * std::string
 */
extern std::string rif_asString(ROBJ obj, int index = 0, const char *path = 0);
/**
 * Check whether an element exists in a configuration list
 * @param obj the list in which to search, allowed to be R_NilValue
 * @param path the path of the element that should be searched
 * @return 'true', if the element was found, 'false' otherwise
 */
extern bool rif_exists(ROBJ obj, const char *path);
/**
 * Checks whether an element is an integer. Throws an error if path is given
 * but element not found.
 * @param obj the list in which to search, allowed to be R_NilValue
 * @param path optional: the relative path of the element within 'obj'
 * @return true, if element is an integer, false otherwise
 */
extern bool rif_isInteger(ROBJ obj, const char *path = 0);
/**
 * Checks whether an element is a double or an integer (i.e. convertable into a
 * double). Throws an error if path * is given but element not found.
 * @param obj the list in which to search, allowed to be R_NilValue
 * @param path optional: the relative path of the element within 'obj'
 * @return true, if element is a double or an integer, false otherwise
 */
extern bool rif_isDouble(ROBJ obj, const char *path = 0);
/**
 * Checks whether an element is a string. Throws an error if path is given
 * but element not found.
 * @param obj the list in which to search, allowed to be R_NilValue
 * @param path optional: the relative path of the element within 'obj'
 * @return true, if element is a string, false otherwise
 */
extern bool rif_isString(ROBJ obj, const char *path = 0);
/**
 * Checks whether an element is a R list. Throws an error if path is given
 * but element not found.
 * @param obj the list in which to search, allowed to be R_NilValue
 * @param path optional: the relative path of the element with in 'obj'
 * @return true, if element is a R list, false otherwise
 */
extern bool rif_isList(ROBJ obj, const char *path = 0);
/**
 * Returns the length of the object. If object is double, integer or string, it
 * returns the length of the array (which is 1 if just one element is in the
 * array), and the number of elements if 'obj' is an R list. Throws an error if
 * path is given but element not found
 * @param obj the ROBJ that should be checked, or a list if path is given
 * @param path optional: the relative path of the element within 'obj'
 * @return 
 */
extern int rif_getLength(ROBJ obj, const char *path = 0);
/**
 * Returns the sourceline where 'obj' was defined in the configuration file,
 * returns INTNA (see constants.h) if the 'obj' has no sourceline attached to
 * it.
 * @param obj the obj of which the sourceline is requested
 * @return the sourceline as an integer or INTNA
 */
extern int rif_getSourceline(ROBJ obj);
/**
 * Returns the absolute path of the object 'obj' in the configuration file,
 * returns "?" if the 'obj' has no absolute path attached to it.
 * @param obj the obj of which the absolute path is requested
 * @return the absolute path as an integer or "?" as a standard C++ string
 */
extern std::string rif_getPath(ROBJ obj);
/**
 * Throw an R error. The execution of the C++ code is stoped. 
 * @param obj The ROBJ object that is related to this error
 * @param msg an error message (begin with lowercase)
 * @param sourceline an optional sourceline, i.e. the sourceline in the C++
 * code that throws the error (simply use __LINE__)
 */
extern void rif_error(ROBJ obj, std::string msg, int sourceline = 0);
/**
 * Throw an R warning. The execution of the C++ code is not interrupted. At the
 * end of execution all the warnings are returned to the user (if the warning
 * option is turned on).
 * @param obj The ROBJ object that is related to this error
 * @param msg an error message (begin with lowercase)
 * @param sourceline an optional sourceline, i.e. the sourceline in the C++
 * code that throws the warning (simply use __LINE__)
 */
extern void rif_warning(ROBJ obj, std::string msg, int sourceline = 0);

#endif


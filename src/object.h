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
#ifndef OBJECT_H
#define OBJECT_H

#include <sstream>

#include "constants.h"
#include "typedefs.h"
#include "global.h"
#include "event.h"

/**
 * This is the base class for practically all objects in the software (except
 * Event and its subclasses). It provides only the most rudimentary
 * capabilities.
 */ 
class Object {
public:
    /**
     * Constructor
     */
    Object();
    /**
     * Destructor<br>Note that it is virtual.
     */
    virtual ~Object();

    /** 
     * An identifier, unique among all instances of Object and its subclasses
     */ 
    Counter uid;
    /**
     * The time when this object was created
     */
    Time timecreated;
    /** 
     * The class id of the object, see constants.h
     */
    int idclass;
    /** 
     * The subclass id of the object, see constants.h
     */
    int idsubclass;
    
    /**
     * A method to output some information about the object; needs to be
     * overwritten by subclasses with more specific information; see
     * implementation of the different subclasses such as Person or Infection
     * how to do this
     */
    virtual std::string str() const;
    /**
     * The method to actually print the string returned by Object::str() using
     * the R interface (Rprintf). Although virtual, this function usially needs
     * not be overwritten
     */ 
    virtual void print() const;
};

/**
 * This is the base class for a Creator (synonyme: Type). This class is
 * abstract and musst be sub-classed, such as done by PersonCreator,
 * InfectionCreator, * etc.
 */ 
class Creator : public Object { 
friend class Ageable;
public:
    /**
     * Constructor<br>
     * The constructor essentialy reads the bin definitions along with the
     * transition information. 
     * @param collection the CreatorCollection that manages this Creator
     * @param name the name of this Creator
     * @param cfg the part of the R configuration list defining this type
     * @param bindistreferencecollector optional; if given, the (random)
     * starting bin that is chosen when an Ageable objects is created is sampled
     * with respect to this CreatorCollection; for example when an Infection is
     * created the starting bin is evaluated with respect to its host (and thus
     * may depend on the host's type, bin, age, etc).
     */
    Creator(
        CreatorCollection *collection, std::string name, 
        ROBJ cfg, CreatorCollection *bindistreferencecollector = 0
    );
    /**
     * Destructor<br>
     * Deletes the definitions of the bin transitions.
     */
    ~Creator();
    
    /**
     * Returns the part of the R configuration list that defines this type.
     * @return An object of type ROBJ
     */
    ROBJ getCfg() const;
            
    /**
     * Returns the basetype of this Creator; this corresponds to the 'basetype'
     * element in the configuration file. Need
     * @return A C++ string
     */
    virtual std::string getBasetype() const = 0;
    
    // Type
    /**
     * Returns the Type of this Creator
     * @return A Type object
     */
    Type getType() const;
    /**
     * Returns the name of this Creator
     * @return A C++ string
     */
    std::string getTypeName() const;
        
    // Bins
    /**
     * Find a bin by its name; throws an error if not found
     * @param binname the bin name
     * @return the corresponding bin
     */
    Bin getBinByName(std::string binname) const;
    /**
     * Returns the name of a Bin
     * @param bin the Bin
     * @return A C++ string
     */
    std::string getBinName(Bin bin) const;
    /**
     * Returns the number of Bins that this type defines.
     * @return the number of bins
     */
    Number getNumberOfBins() const;
    /**
     * Returns the Distribution that is used to initialise the starting bin of
     * an Ageable of this Type (should be 'protected' because Ageable is a
     * friend of this class but I couldn't access the protected method
     * from child classes of the Ageable class)
     * @return 
     */
    Distribution *getBinDist() const;
    
    /**
     * Returnes the linearised number of the given Bin of this Type.
     * @param bin a bin of this Creator
     * @return an integer
     * @see CreatorCollection::linearise()
     */
    int linearise(Bin bin) const;
    
    /**
     * Returns the collection that manages this Creator.
     * @return A pointer to the CreatorCollection object
     */
    CreatorCollection *getCollector() const;
        
    /**
     * Returns some human readable information about this Creator
     * @return a C++ string
     */
    virtual std::string str() const;
    
 
protected:
    /** The part of the configuration list that defines this Creator. It is
    stored only for later reference when an error occures (sourceline) */
    ROBJ cfg;
    /** A pointer to the the CreatorCollection that manages this Creator */
    CreatorCollection *collection; 
    
    /** The Type of this Creator */
    Type type;
    /** The name of this creator (eg "female", "chlamydia", etc.) */
    std::string name;
    /** The number of bins that Ageable objects of this Creator can have */
    Number binnum;
    /** An array with the binnames (con- and destructed by this Creator) */
    std::string *binname;
    /** A pointer to the distribution that is used to sample the starting bin
    when an Ageable object is created (con- and destructed by this Creator)*/
    Distribution *bindist;
    
    /** The number of bin transition that were defined */
    Number bintransnum;
    /** An array with the names of the bin transitions (con- and destructed by
    this Creator) */
    std::string *bintransname;
    /** An array with the 'from' bin (con- and destructed by this Creator) */
    Bin *bintransfrom;
     /** An array with the 'to' bin (con- and destructed by this Creator) */
    Bin *bintransto;
    /** An array with pointers to the distributions that are used to sample the
    next bin transition (con- and destructed by this Creator) */
    Distribution **bintransdist;
   
    /**
     * Created a PairAttribute. These attributes are not installed at the
     * CreatorCollection but at the calling Creator (as an double indexed
     * array); therefore use deletePairAttribute() to delete the Attribute in
     * the destructor;
     * @param collection the CreatorCollection with respect the distributions
     * will be evaluated (need here to resolve the type names if the 'bytype'
     * feature is used)
     * @param msg a message with detailed description of the attribute; this
     * will be shown when initialising the model with sti.init()
     * @param attrname an name for the attribute
     * @param initvalue if atleast one value is given for the PairAttribute,
     * then the others will be initialised with the 'initvalue' (for factors
     * usually 0, so that any combination that has not been specified will be
     * downweighted to 0)
     * @param defaultval if the element is not found in the configuration it
     * will be set to this value (for factors usually 1, so that it has no
     * effect)
     * @return A PairAttribute object
     */
    PairAttribute installPairAttribute(
        CreatorCollection *collection, std::string msg, std::string attrname, 
        Value initvalue, Value defaultval = DOUBLENA
    ) const;
    
    /**
     * Use this function in the destructor of a child class of the Creator
     * class to destruct a PairAttribute.
     * @param collection a pointer to the CreatorCollection with respect to
     * which the PairAttribute was created (used only to get the sizes of the
     * array)
     * @param pairattr the PairAttribute that is to be deleted
     */
    void deletePairAttribute(
        CreatorCollection *collection, PairAttribute pairattr
    ) const;
    
    /**
     * This is the interface that should be used by any child of a Creator
     * class to install new attributes. If no default value is given and the
     * element is not specified by the user, an error is thrown.
     * @param collection a pointer to the CreatorCollection where the attribue
     * will be installed; every Ageable of every Creator managed by that
     * collection will have this attribute; when sampling from that attribute it
     * has to be sampled with respect to an Ageable managed by a Creator
     * of this CreatorCollection. 
     * @param msg a message with detailed description of the attribute; this
     * will be shown when initialising the model with sti.init()
     * @param name a name for the attribute
     * @param path an optional path that leads to the element (such as for
     * pregnancy parameters that are encapsuled in a subsection 'pregnancy');
     * if this is not used but a defaultvalue is needed, set this argument to ""
     * @param defaultval a possible default value; this value is used if the
     * element is not specified in the configuration
     * @return A Attribute object; store it for future reference
     */
    Attribute installAttribute(
        CreatorCollection *collection, std::string msg, std::string name,
        std::string path = "", Value defaultval = DOUBLENA
    ) const;
};


/**
 * This is the base class for all object that can have an age (persons,
 * infections, partnerships). Although not abstract it is not meant to be used
 * directly. The implementation provides the basic mechanism of bin changes.
 */ 
class Ageable : public Object {
public:
    /**
     * Constructor<br>
     * The constructor initialises and samples the fixed Attributes which then
     * stay fixed for the lifetime of this Ageable; if 'fixatbirthbybin' is
     * used, it goes through all possible bins and samples for each bin such a
     * fixed Attriute, otherwise sampling is done with respect to bin 0 and this
     * value is then used for the other bins as well. The events for death and
     * bin changes are not installed, this is the duty of the subclasses if
     * needed (eg. the death of an Infection is known only when going into one
     * of the immune state 'cleared' or 'treated'; only then the death event is
     * installed, not before)
     * @param creator a pointer to the Creator class that creates this Ageable
     */
    Ageable(Creator *creator);
    /**
     * Destructor<br>
     * Releases the array that contains the values of the fixed Attributes
     */
    ~Ageable();
    
    /**
     * Returns the time when this ageable was born; this is not necessarily the
     * time when it was created in the memory but maybe earlier.
     * @return a Time object
     */
    Time getTimeBirth() const;
    /**
     * Returns the time when this ageable will die or died.
     * @return a Time object
     */
    Time getTimeDeath() const;
    /**
     * Return the age of this Ageable with respect to the current time
     * (global::abstime)
     * @return a Time object
     */
    Time getAge() const;
    /**
     * Return the age of this Ageable with respect to time 'now'
     * @param now the reference time
     * @return a Time object
     */
    Time getAge(Time now) const;
    /**
     * Returns whether this object is alive with respect to the current time
     * (global::abstime).
     * @return 'true' if Ageable is alive, 'false' otherwise
     */
    bool isAlive() const;
    
    /**
     * Get the current Bin in which the Ageable is at the moment
     * @return a Bin object
     */
    Bin getBin() const;
    /**
     * Get the number of bin in which the Ageable is at the moment, but
     * enumerated with respect to all bins of the same class as this ageable;
     * see CreatorCollection for more details.
     * @return an integer representing the absolut bin number
     */
    int getBinLinearised() const;
    /**
     * Returns the Type of this Ageable (note that the Type of an Ageable is
    * fixed when created)
     * @return a Type object
     */
    Type getType() const;
    /**
     * Get the Creator that created this Ageable (and represents its Type)
     * @return a pointer to the Creator object
     */
    Creator * getCreator() const;
    
    /**
     * Sample from an Attribute belonging to this Ageable, assuming the current
     * time being 'now'
     * @param attribute an Attribute object representing the attribute
     * @param now the 'now' time
     * @return a sample from this Attribute
     * @see Ageable::installAttribute(); Distribution::dsample()
     */
    Value getAttribute(Attribute attribute, Time now = global::abstime) const;
    /**
     * Sample from an Attribute belonging to this Ageable, assuming the current
     * time being 'now' and given it is atleast 'atleast'
     * @param attribute an Attribute object representing the attribute
     * @param now the 'now' time
     * @param atleast the value on which to condition
     * @return a sample from this Attribute (>= atleast)
     * @see Ageable::installAttribute(); Distribution::dsample()
     */
    Value getAttribute(Attribute attribute, Time now, Value atleast) const;
    /**
     * Sample from an Attribute belonging to this Ageable, assuming the current
     * time being 'now', scaled by a factor; it depends on the distribution
     * what the effect of factor is, but usually a smaller factor value means a
     * bigger Attribute value (e.g. a smaller rate for an exponential
     * distribution means bigger values)
     * @param attribute an Attribute object representing the attribute
     * @param now the 'now' time
     * @param factor the scaling factor
     * @return a sample from this Attribute
     * @see Ageable::installAttribute(); Distribution::dsamplefac()
     */
    Value getAttributeFac(Attribute attribute, Time now, double factor) const;
    /**
     * Sample from an Attribute belonging to this Ageable, assuming the current
     * time being 'now', scaled by a factor and given it is atleast 'atleast'
     * (conditioning after scaling); it depends on the distribution
     * what the effect of factor is, but usually a smaller factor value means a
     * bigger Attribute value (e.g. a smaller rate for an exponential
     * @param attribute an Attribute object representing the attribute
     * @param now the 'now' time
     * @param factor the scaling factor
     * @param atleast the value on which to condition
     * @return a sample from this Attribute (>= atleast)
     * @see Ageable::installAttribute(); Distribution::dsample()
     */
    Value getAttributeFac(
        Attribute attribute, Time now, double factor, Value atleast
    ) const;
    
    /**
     * Returns a Number, depending on 'what'; in the Ageable only
     * 'NumberOfType' and 'NumberOfBin' is implemented
     * @param what the property 
     * @return a Number object of what is requested, 0 if the NumberOf type is
     * not supported
     * @see NumberOf type
     */
    virtual Number getNumber(NumberOf what) const;
    
    /**
     * Returns some human readable information about this ageable
     * @return a C++ string
     */
    virtual std::string str() const;
        
    /**
     * This function is called if this Ageable is supposed to die; in general
     * by a previously installed EventDeath event; if this method is
     * overwritten you need to call this version nevertheless
     */
    virtual void slotDeath();
    /**
     * This function is called if this Ageable is supposed to change its
     * current bin; in general by a previously installed EventBinChange event;
     * the 'from' argument is only for safety reasons; an error is thrown if the
     * current bin is not the same as the 'from' bin; if this method is
     * overwritten you need to call this version nevertheless
     * @param from the bin in which the Ageable is supposed to be currently
     * @param to the bin in which the Ageable should change
     */
    virtual void slotBinChange(Bin from, Bin to);
    
protected:
    /** the time of birth
     * @see getTimeBirth() */
    Time timebirth;
    /** the time of death
     * @see getTimeDeath() */
    Time timedeath;
    /** the number of bins of this type; copied from the corresponding Creator
     * object (for higher performance) 
     * @see Creator->getNumberOfBins() */
    int binnum;
    /** the bin in which the Ageable is currently 
     * @see Ageable::getBin() */
    Bin bin;
    /** a pointer to the Creator object that created this Ageable 
     * @see Ageable::getCreator() */
    Creator *creator;    
    
    /** 
     * An array with the values with the 'fixatbirth' or 'fixatbirthbybin'
     * features; if an Attribute has this feature activated, the value is not
     * sampled from the corresponding distributino but taken from this double
     * array; first index is the Attribute number the second index the current
     * bin of the Ageable (array is created and destructed by the Ageable) */
    Value **attrfixed;
    /** 
     * A pointer to an array specifying which Attribute is fixed at birth; the
     * array is managed by the corresponding CreatorCollection object
     * @see CreatorCollection::getAttributesIsFixedArray()
     */
    const bool *attrisfixed;
    /**
     * A pointer to an array representing the distribution from which to
     * sample; the index is the Attribute number; the array is managed by the
     * corresponding CreatorCollection object 
     */
    const Distribution * const *attr;
    
    /** A Process object which manages the event flow of bin changes 
     * @see Ageable::throwEventBinChange() 
     */
    Process procBinChange;
    /** 
     * This method installs a new EventBinChange; 
     * @param conditional if 'true', the time of the next event is sampled,
     * starting from the last time 'slotBinChange' was called, conditioned
     * on the time that has already passed since then; if 'false' unconditional
     * sampling is used and it is therefore assumed that a bin change just
     * happend
     */
    virtual void throwEventBinChange(bool conditional = false);
    /**
     * An internal method to actually sample from the corresponding
     * distribution   
     */
    BinChange sampleBinChange(Bin from, Time now, bool conditional = false);
    
    /** A Process object which manages the event flow of deaths. Usually, only
     * one death occures 
     * @see Ageable::throwEventDeath() 
     */
    Process procDeath;
    /** 
     * This method installs a new EventDeath; 
     * @param conditional if 'true', the time of the next event is sampled,
     * starting from the last time 'slotDeath' was called, conditioned
     * on the time that has already passed since then (not supported in the
     * current implementation as more than one death does not make much sense);
     * if 'false', unconditional sampling is used
     */
    void throwEventDeath(bool conditional = false);
};

/**
 * This is the class to manage a collection of types, such as 'female' and
 * 'male' types for the category 'person'. This class does all that is needed
 * to manage Creator object, so subclasses are probably not necessary. 
 */
class CreatorCollection : public Object {
public:
    /**
     * Constructor<br>
     * The constructor does not more than store the names of the types and
     * initialise some arrays; the actual work is done when the Creators
     * register themselves here using the CreatorCollection::registerCreator
     * method
     * @param names An R array of strings representing the types of this
     * category (eg c("female","male") for the person types)
     * @param name the name of this collection (eg. "person", "partnerships")
     * @param dist an optional pointer to a distribution from which to sample;
     * see CreatorCollection::getRandomCreator()
     */
    CreatorCollection(ROBJ names, std::string name, Distribution *dist = 0);
    /**
     * Destructor<br>
     * Nothing to be done here; 
     * @see CreatorCollection::preDelete()
     */
    ~CreatorCollection();
    /**
     * PreDestructor<br>
     * Deletes the attached Creators; we need to keep the CreatorCollection
     * object in the memory as some other Creator object belonging to another
     * CreatorCollection may still need the number of types or so; might by
     * superfluous now, as the number of types is separately stored in a
     * variable (such as 'global::persontypesnum' for 'global::persontypes'
     */
    void preDelete();
    
    /**
     * Retusn the name of this CreatorCollection
     * @return a C++ string
     */
    std::string getName() const;
    
    /**
     * Returns the number of the bin, enumerated among all the bins of all the
     * types attached to this CreatorCollection; for example if we defined two
     * person types 'female' and 'male' and female has the bins 'low' and 'mid'
     * and 'male' has the bins 'low', 'mid' and 'high' then the 'low' bin of the
     * 'female' type gets the number 0 the 'mid' bin gets the number 1, the
     * 'low' bin of the 'male' type gets number 2, the 'mid' bin number 3,
     * etc.; this feature is needed by PairAttibutes
     * @param type the type 
     * @param bin the bin
     * @return an integer, a unique integer for each combination of type and bin
     * @see CreatorCollection::getTotalNumberOfBins()
     */
    int linearise(Type type, Bin bin) const;
    /**
     * Returns the total number of bins in all types managed by this
     * CreatorCollection; 
     * @see CreatorCollection::linearise()
     * @return 
     */
    Number getTotalNumberOfBins() const;
    /**
     * This is the method were Creators register themselves;
     * @param creator a pointer to the calling Creator object
     * @param name the name of the calling Creator object (to compare with the
     * already stored names; see CreatorCollection::CreatorCollection())
     * @param binsnum the number of bins that the calling Creator has; we could
     * get this number by calling the Creator::getNumberOfBins() method, but the
     * Creator is probably calling from its constructor and we don't want to
     * nest too much; this explict parameter is cleaner
     * @return the Type that the Creator is assigned to by the CreatorCollection
     */
    Type registerCreator(Creator *creator, std::string name, int binsnum);
    /**
     * Returns a pointer to the Creator object corresponding to the type name
     * @param name the name of the Creator
     * @return a pointer to a Creator object
     * @see Creator::getCreatorTypeByName()
     */
    Creator *getCreatorByName(std::string name) const;
    /**
     * Returns the Type corresponding to the Creator given by its name; if not
     * found, an error is thrown
     * @param  name the name of the Creator
     * @return a Type object 
     */
    Type getCreatorTypeByName(std::string name) const;
    /** 
     * Returns the Type corresponding to the Creator given by its name; if not
     * found, an error is thrown, but with respect to the cfg argument as the
     * sourceline in the configuration file
     * @param cfg a part of the R configuration list object; if there is an
     * error, the sourceline is taken from this object; otherwise this object
     * remains untouched
     * @param  name the name of the Creator
     * @return a Type object 
     */        
    Type getCreatorTypeByName(ROBJ cfg, std::string name) const;
    /**
     * Returns the Creator that represent the Type given by the 'type' argument
     * @param type Type of the Creator
     * @return a pointer to a Creator object
     */
    Creator *getCreator(Type type) const;
    /**
     * Returns the name of the Creator specified by the 'type' argument.
     * @param type the Type of the Creator
     * @return a C++ string
     */
    std::string getCreatorName(Type type) const;
    /**
     * Returns the number of Creator/Types that this CreatorCollection manages;
     * this number is available even before the Creator objects are initialised
     * themselves; this method essentially returns the lenght of the array
     * passed to constructor CreatorCollection::CreatorCollection()
     * @return the number of Creators
     */
    Number getNumberOfCreators() const;
    /**
     * Returns a random Creator according to a specified distribution; at
     * the moment only used to sample a random person when populating, at birth
     * or when immigrating
     * @return A pointer to a Creator object
     */
    Creator *getRandomCreator() const;
    
    /**
     * Returns some human readable information about this CreatorCollection
     * @return a C++ string
     */
    std::string str() const;
    
    /**
     * This is the main method to install an Attribute for this
     * CreatorCollection; it is also used if the Attribute will only be
     * available to one type of the collection (such as 'pregnancy', which is
     * only installed by PersonCreatorFemale.This method should only be called
     * by Creator::installAttribute(), so use that interface to install an
     * Attribute.
     * @param installer the calling Creator; not necessarily a Creator managed
     * by this CreatorCollection (this is only used to produce human readable
     * output)
     * @param name the name of the Attribute (this is only used to produce human
     * readable output)
     * @param cfg the part of the R configuration that defines the distribution
     * for this attribute
     * @param creator optional; if not given, the distribution will be resolved
     * with respect to this collection (ie. the feature 'bytype' is allowed and
     * is resolved with respect to this collector); if given, the 'bytype'
     * feature is not allowed as the distribuion is evaluated directly with
     * respect to the given creator; this feature is usually only used by
     * creators that install an attribute in the collection; ie. a
     * PersonCreatorFemale Creator installing the pregnancy attributes, which
     * are always resolved with respect to this type (it does not make sense a
     * possible other type, say 'male', to use this Attribute); if a Creator
     * installs an Attribute at another collection, it usually doesn't make
     * sense to pass this argument.
     * @return an Attribute object
     * @see Creator::installAttribute(); Ageable::getAttribute();
     * Ageable::getAttributeFac()
     */
    Attribute installAttribute(
        const Creator *installer, std::string name, 
        ROBJ cfg, const Creator *creator = 0
    );
    /**
     * Returns the number of installed attributes
     * @return the number of attributes
     */
    Number getNumberOfAttributes() const;
    /**
     * Returns a pointer to an array specifying whether an attribute should be
     * fixed at birth of an ageable or not.
     * @return a pointer to a single indexed array
     */
    const bool *getAttributesIsFixedArray() const; 
    /**
     * Returns a pointer to an array specifying whether an attribute that should
     * be fixed at birth of an ageable is being fixed by bin or overall.
     * @return a pointer to a single indexed array
     */
    const bool *getAttributesIsFixedPerBinArray() const;
    /**
     * Returns a pointer to an array that contains the distributions for each
     * Attribute
     * @return a pointer to a single indexed array, containing pointers to the
     * Distribution objects.
     */
    const Distribution * const *getAttributesArray() const; 
    /**
     * Returns the distribution that corresponds to the Attribute given by the
     * 'attr' argument.
     * @param attr the Attribute whose Distribution is requested
     * @return a pointer to a Distribution object
     */
    const Distribution *getAttribute(Attribute attr) const;
    /**
     * Returns the name of an Attribute
     * @param attr the Attribute whose name is requested
     * @return a C++ string
     */
    std::string getAttributeName(Attribute attr) const;
            
private:
    /** The name of this collection */
    std::string name;
    /** An array with the names of the creators managed by this collection (con-
    and destructed automatically) */
    std::string creatorname[MAXTYPES];
    /** An array with the pointers to the creators managed by this collection
    (the array is con- and destructed automatically, but the creators are
    deleted by the preDelete() method) */
    Creator *creators[MAXTYPES];
    /** the number of types managed by this collection */
    int typenum;
    /** an array that contains the number of bins managed by each type (con- and
    destructed automatically) */
    int binnum[MAXTYPES];
    /** a double indexed array that maps a type/bin combination to the
    linearised number; @see CreatorCollection::linearise() */
    Bin map[MAXTYPES][MAXBINS];
    /** the total number of bins managed by this collection */
    int len;
    /** an optional Distribution used by getRandomCreator() */
    Distribution *dist;
    /** An array with the names of the attributes managed by this collection
    (con- and destructed automatically) */
    std::string attrname[MAXNEWATTRIBUTES];
    /** An array with the distributions of the attributes managed by this
    collection (con- and destructed of the array is automatically, but the
    distributions are deleted by preDelete()) */
    Distribution *attr[MAXNEWATTRIBUTES];
    /** An array with the information if an attribute should be fixed 
    at birth (con- and destructed of the array is automatically) */
    bool attrisfixed[MAXNEWATTRIBUTES];
    /** An array with the information if an attribute should be fixed 
    at birth by bin independent of the bin (con- and destructed of the array is
    automatically) */
    bool attrisfixedperbin[MAXNEWATTRIBUTES];
    /** the number of attributes managed by this collection */
    int attrnum;
    
    
};


#endif

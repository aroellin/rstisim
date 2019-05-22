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
#ifndef PERSON_H
#define PERSON_H

#include <string>

#include "typedefs.h"
#include "object.h"


/**
 * This is the main class for a Person type with basetype "GENERIC"
 */
class PersonCreator : public Creator {
friend class Person;
public:
    /**
     * Constructor<br>
     * The Constructor reads in the configuration file, essentially lifetime
     * distribution, shift at birth (ie the minimal age to be inserted in the
     * population), the rate of seeking general testing
     * @param collection (see Creator::Creator())
     * @param name (see Creator::Creator())
     * @param cfg  (see Creator::Creator())
     */
    PersonCreator(
        CreatorCollection *collection, std::string name, ROBJ cfg
    );
    /**
     * Destructor<br>
     * Deletes the infections (current and old ones) attached to this host
     */
    ~PersonCreator();
    
    /**
     * This is the main interface to create a new Person: the Person
     * automatically registers in the Population object
     * @param why see CauseCreation
     * @param mother an optional pointer to the mother
     * @param father an optional pointer to the father
     * @return a pointer to a newly created Person object 
     */
    virtual Person* createPerson(
        CauseCreation why, PersonFemale *mother=0, PersonMale *father=0
    );
    /**
     * Returns some human readable information about this ageable
     * @return a C++ string
     */
    virtual std::string str() const;
    /**
     * Returns the basetype of this Creator; this corresponds to the 'basetype'
     * element in the configuration file. Need
     * @return A C++ string
     */
    virtual std::string getBasetype() const { return("GENERIC"); };

private:
    /** the lifespan attribute */
    Attribute lifespan;
    /** the minimal age at which people are introduced */
    Attribute birthshift;
    /** the distribution that is used when populating the model */
    Attribute initran;
    /** The age at immigration */
    Attribute immigrage;
    /** The distribution to sample the next treatment */
    Attribute visitgp;
    /** A factor to influence the time of next treatment */
    Attribute visitgpfactor;
    /** A factor to influence the time of next treatment */
    Attribute visitgpfactor2;
    /** A probability to influence the time of next treatment */
    Attribute visitgpprobability;
    /** A factor to influence the time of next treatment */
    Attribute visitgpprobabilityfactor;
    /** A factor to influence the time of next treatment */
    Attribute visitgpprobabilityfactor2;
    /** The distribution to sample what type of visit */
    Attribute gpvisittype;

    /** A uniform distribition used if initran is not defined (con- and
    destructed by this object) */
    Distribution *uniform;
    
    /** A uniform distribution for the gpvisit procedure */
    Distribution *gpvisitsuniform;
    /** A flag to tell if 'initran' was defined or the uniform Distribution
    should be used */
    bool initranpresent;
};

/**
 * This is the class for a Person type with basetype "MALE"; in addition to
Person a male can have a number of children; 
 */
class PersonCreatorMale : public PersonCreator {
friend class PersonMale;
public:
    /**
     * Constructor
     * @param collection (see Creator)
     * @param name (see Creator)
     * @param cfg (see Creator)
     */
    PersonCreatorMale(
        CreatorCollection *collection, std::string name, ROBJ cfg
    );
    /**
     * Destructor
     */
    ~PersonCreatorMale();
    
    /**
     * Creates a Person of basetype 'MALE'
     * @see Person::createPerson()
     */
    virtual Person* createPerson(
        CauseCreation why, PersonFemale *mother=0, PersonMale *father=0
    );
    
    /**
     * Returns the basetype of this Creator; this corresponds to the 'basetype'
     * element in the configuration file. Need
     * @return A C++ string
     */
    virtual std::string getBasetype() const { return("MALE"); };
    
};

/**
 * This is the class for a Person type with basetype "FEMALE"; in addition to
 * Person, a female can get pregnant and have children
 */
class PersonCreatorFemale : public PersonCreator {
friend class PersonFemale;
public:
    /**
     * Constructor<br>
     * Reads the pregnancy parameters
     * @param collection (see Creator)
     * @param name (see Creator)
     * @param cfg (see Creator)
     */
    PersonCreatorFemale(
        CreatorCollection *collection, std::string name, ROBJ cfg
    );
    /**
     * Destructor
     */
    ~PersonCreatorFemale();
    
    /**
     * Creates a Person of basetype 'FEMALE'
     * @see Person::createPerson()
     */
    virtual Person* createPerson(
        CauseCreation why, PersonFemale *mother=0, PersonMale *father=0
    );
    
    /**
     * Returns the basetype of this Creator; this corresponds to the 'basetype'
     * element in the configuration file. Need
     * @return A C++ string
     */
    virtual std::string getBasetype() const { return("FEMALE"); };
    
protected:
    /** Attribute that gives the probability of getting pregnant per
    unprotected sexual contact in a partnership */
    Attribute probpregnancypercontact;
    /** Attribute that gives a factor to the probabilty of getting pregnant*/
    Attribute probpregnancypercontactfactor;
    /** Attribute that gives the duration of a regular pregnancy */
    Attribute pregnancyduration;
    /** Attribute that gives the general rate of getting pregnant if female is
    in atleast one partnership; this is used independently of the percontact
    probabilities */
    Attribute pregnancygeneralrate; 
    /** Attribute that gives a factor to the general rate of getting
    pregnat*/
    Attribute pregnancygeneralfactor;
    /** A uniform distributio that is being used to test if a female gets
    pregnant during a unprotected sexual contact */
    Distribution *uniform;
};

// declare the enumPeople() function from the entrypoints.cpp file
extern "C" { void enumPeople(); }

class Person : public Ageable
{
friend void enumPeople();
public:
    /** An ID that is unique among all Person objects */
    Counter puid; 

    /**
     * 
     * @param pc (see Ageable)
     * @param why the reason that this Person was created; has effect on the
     * lifetime distribution
     * @param mother optional; a pointer to the mother object
     * @param father optional; a pointer to the father object
     */
    Person(
        PersonCreator *pc, CauseCreation why, 
        PersonFemale *mother = 0, PersonMale *father = 0
    );
    /**
     * Destructor
     */
    ~Person();
        
    /**
     * Returns the corresponding PopID object of this Person
     * @return 
     */
    PopID getPopID() const;
    /**
     * Sets the corresponding PopID object of this Person; this should not be
     * used as the Person gets a PopID; it is only used by
     * Population::slotRegisterPerson to set the PopID as soon as possible
     * because it is used by other functions as well, which would crash if the
     * PopID is not set properly; it's possible that one could remove this
     * mechanism as I have replaced the use of PopID as a function argument
     * in favour of a Pointer to the Person
     * @param popid the new PopID 
     */
    void setPopID(PopID popid);
        
    /**
     * Returns the last Partnership with the Person given in the argument,
     * including current partnerships; the search is based on the puid
     * @param person a pointer to a Person object, the potential partner 
     * @return a pointer to the Partnership object if such one was found,
     * otherwise returns 0;
     */
    const Partnership * getLastPartnershipWith(const Person *person) const;
    
    /**
     * Returns the number of what is requested; Person supports in addition the
     * NumbeOf types NumberOfTreatments, NumberOfPartnersCurrent,
     * NumberOfPartnersTotal, NumberOfPartnersWithin, NumberOfContacts,
     * NumberOfContactsUnprotected, NumberOfInfectionsCurrent,
     * NumberOfInfectionsWithin, NumberOfInfectionsTotal, NumberOfLink;
     * @param what (see NumberOf)
     * @return a Number object
     * @see Ageable::getNumber()
     */
    Number getNumber(NumberOf what) const;
        
    /**
     * Returns a list of the current infections (including the ones in the
     * immune state 'cleared' and 'treated')
     * @return a pointer to an InfectionList object
     */
    const InfectionList *getInfections() const;
    /**
     * Returns a list of the old infections (ie. not including the ones in the
     * immune state 'cleared' and 'treated', those belong to the current
     * infections)
     * @return a pointer to an InfectionList object
     */
    const InfectionList *getInfectionsOld() const;
        
    /**
     * Returns some human readable information about this ageable
     * @return a C++ string
     */
    virtual std::string str() const;
    
    const PartnershipList *getPartnerships() const;
    
    const PartnershipList *getPartnershipsOld() const;
    
    const SingleNotificationList *getSingleNotifications() const;

    const SingleGPVisitList *getSingleGPVisits() const;
    
    void slotVisitGPNotified(Type gpvisittype, Time wait, Notification notif);
    
    /**
     * Sets the link number of the partnernotification
     * @param linknumber the new linknumber
     */
    virtual void setLinkNumber(Number linknumber);
    
    /**
     * Query the infection status
     * @return 'true' if person has atleast one infection that is not in
     * either of the immune states 'treated' or 'cleared', 'false' otherwise
     */
    bool isInfected() const;
    /**
     * Query the infection status of specific infection type
     * @param infectiontype the Type of the Infection
     * @return 'true' if person is infected with infection of Type
     * 'infectiontype' and that infection is not in one of the immune states
     * 'cleared' or 'treated'; 'false' otherwise
     */
    bool isInfected(Type infectiontype) const;
    /**
     * Query whether person was ever infected in general
     * @return 'true' if person was infected at least once, 'false' otherwise
     */
    bool wasEverInfected() const;
    /**
     * Query whether person was ever infected by a specific Infection type
     * @param infectiontype the Type of the Infection
     * @return 'true' if person was infected at least once by Infection
     * 'infectiontype', 'false' otherwise
     */
    bool wasEverInfected(Type infectiontype) const;
    /**
     * Get an Infection of a Person
     * @param infectiontype the Type of Infection
     * @return a pointer to the infection if such an infection is present (also
     * if this infection is in either of the immune states 'cleared' or
     * 'treated'); 0 otherwise
     */
    Infection *getInfection(Type infectiontype) const;
        
    /**
     * This function is called (usually by a EventDeath) if the person is
     * supposed to die; the method deregiseters from partnership formation and
     * informs all the infections that the host just died; any pending events
     * are removed
     * @see Ageable::slotDeath()
     */
    virtual void slotDeath();
    /**
     * This method also updates the partnership formation and resamples the
     * EventHaveSex for each partnership that this person is involved in
     * @param from (see Ageable::slotBinChange())
     * @param to (see Ageable::slotBinChange())
     * @see Ageable::slotBinChange()
     */
    virtual void slotBinChange(Bin from, Bin to);
    /**
     * This function is usually called by a partnership for each partner when
     * there is a sexual contact between them
     * @param ps a pointer to the calling Partnerhsip object
     * @param unprotected a flag to indicate whether the contact is protected
     * unprotected
     */
    virtual void slotNotifyHaveContact(Partnership *ps, bool unprotected);
    
    /**
     * This function is usually called by an infection when in changes its bin
     * @param infection a pointer to the calling Infection object
     * @param from the bin the infection is currently in
     * @param to the bin into which the infection is changing
     */
    virtual void slotInfectionChangedState(
        const Infection *infection, Bin from, Bin to
    );
    
    // Other slots
    /**
     * This function is usually called by a Partnership object when it is
     * created, to inform both partners about the new partnership
     * @param ps a poiner to a Partnership object
     */
    virtual void slotRegisterPartnership(Partnership *ps);
    /**
     * This function is usually called by a Partnership object when the
     * partnership is broken up
     * @param ps a poiner to a Partnership object
     */
    virtual void slotDeregisterPartnership(Partnership *ps);
    /**
     * This object is called if an infection was able to infect a person (be it
     * during initialization or through transmition during a sexial contact);
     * howver, the person might already be infected by this type of Infection,
     * in which case nothing happens; if the person is not infected by this
     * Infection type, this method uses the Infection::clone() method to create
     * a new Infection object
     * @param infection the infection that should be cloned
     * @param ps optional; if the infection happens during a sexual contact,
     * this is a pointer to the Partnership object; otherwise 0
     * @see Infection::clone()
     */
    virtual void slotInfect(
        const Infection *infection, const Partnership *ps=0
    );
    /**
     * If a a infection is successfully created, the Person::slotInfect()
     * method also registers the infection through this method. 
     * @param infection the infection that should be registered with this Person
     */
    virtual void slotRegisterInfection(Infection *infection);
    /**
     * If a infection dies, it deregisters from the host thorugh this method;
     * the infection is moved to the list of old infections but not removed from
     * the memory; if the infection is not in the list of current infections,
     * an error is thrown
     * @param infection a pointer to the Infection object that should be
     * deregistered;    
     */
    virtual void slotDeregisterInfection(Infection *infection);
    
    /**
     * This method is usually called by an EventVisitGP event. It makes
     * the person test for infections and, if tested positive, install a
     * EventTreat event; if the person is treated in general, false positive
     * results are possible, so the EventTreat event are installed too and if
     * no infection is present at the time of actual treatment, the counter
     * 'countertreatmentvain' is increased
     */
    virtual void slotVisitGP(
        Type gpvisittype, CauseVisitGP cvgp, Notification notif = global::NOTNOTIFIED
    );
    
    /**
     * If GP tests patient positive, this function is called
     */
    virtual void slotTestedPositive(Type infectiontype);
    
    /**
     * Treat against a specific infection. General treatment throws an error in
     * the current implementation, so the Treatment has to be specific with an
     * infection type given. If a treated against an infection the person is
     * not infected with, 'countertreatmentvain' is increased, otherwise
     * nothing happens
     * @param treatment the type of treatment (general or specific); only
     * specfic allowed for the time being.
     * @param infectiontype the infection type against which to treat; although
     * optional, must be given or the time being. 
     */
     virtual void slotTreat(Type infectiontype, Time wait = 0);
 
    /** 
     * This function checks whether the person was already notified under
     * the notification chain given by "notif"
     */
    virtual bool isNotifiedAlready(Notification notif);
    
    /** 
     * This function is called from the notifier module whenever this
     * person starts a notification batch
     */ 
     virtual void slotNotificationStarts(Notification notif);    
   
    /** 
     * This function is called whenever the person who currently is 
     * notifying, is about to notify a partner. The function is called before
     * the partner is notified
     */
     virtual void slotNotifyingPartner(
        Person *partner, Partnership *ps, 
        Notifier *notifier, Notification notif);
     
      
    /**
     * This method removes references to the given Partnership from this Person.
     * Such reference can be in the list of old partnerships or in an
     * infection. Current partnerships are not checkes because they are not
     * supposed to be deleted from the memory if still active
     * @param ps the pointer to the parthership to be removed
     * @see Population::internalRemoveOldPartnerships()
     */
    virtual void internalRemovePartnership(Partnership *ps);
    /**
     * This method removes references to the given Infection from this Person.
     * Such reference can be in the list of infections and old infections,
     * either as the infection itself or as a parent reference 
     * Current infections are not checked because they are not
     * supposed to be deleted from the memory if still active.
     * @param infection the pointer to the infection to be removed
     */
    virtual void internalRemoveInfection(Infection *infection);

    
    
protected:
    /** Stores the PopID obtained thorugh registering and deregistering */
    PopID popid; 
    /** The reason this Person was created */
    CauseCreation causeCreated;
   
    /** The number of current partners */
    Number partnerscurrent;
    /** The number of partners in the past and now */
    Number partnerstotal;
    /** The list of current partnerships */
    PartnershipList partnerships;
    /** The list of past partnerships */
    PartnershipList partnershipsold;
    
    /** A pointer to the father or 0 */
    PersonMale *father;
    /** A pointer to the mother or 0 */
    PersonFemale *mother;
    
    /** The number of sexual contacts up to now */
    Number contacts;

    /** The number of unprotected sexual contacts up to now */
    Number contactsunprot;
    
    /** The number of  GP visits */
    Number gpvisits;
    
    /** The number of positive tests at the last gpvisit */
    Number positivetests;
    
    /** The total number of times this person has been notified */ 
    Number notifications;

    /** The number of partners this person has already notified during
        the current notification batch */
    Number alreadynotified; 
            
    /** Link number in a chain of notifications; 0 means the index case, 1 the
    partners of the index case, 2 the partners of the partners of the index
    case, etc.; this number is set every time a partnernotification event
    happens, so it is the link number of the last notification event */
    Number linknumber;
    
    /** The numebr of current infection; this does not include those infections
    in the list 'infections' that are in one of the immune states 'cleared' or
    'treated'; so it can be that infectionscurrent != infections.size() */
    Number infectionscurrent;

    /** The total number of infections, past and present */
    Number infectionstotal;

    /** The list of current infections; including the ones that are one of the
    immune states 'cleared' or 'treated' */
    InfectionList infections;

    /** The list of past infections */
    InfectionList infectionsold;
    
    /** The total number of treatments */
    Number treatments;
    
    /** Process to manage the specific treaments */
    Process *procSpecTreatment;
    
    /** Process to manage the general treatments */
    Process procVisitGP;
    
    /** A list with previous notifications so that a person is not notified 
        itself from its partners */
    NotificationList notiflist;
    
    /** A list with previous single notifications with detailed informatio */
    SingleNotificationList singlenotiflist;
    
    /** A list with the detailed information about the GPVisits */
    SingleGPVisitList singlegpvisitlist;
    
    /**
     * This method throws the next EventVisitGP event with
     * CauseVisitGP::CauseSymptomsGeneral.
     * @param conditional optional; conditional sampling with respect to now or
     * the last event time 
     */
    virtual void throwEventVisitGP(bool conditional=false);
};

class PersonMale : public Person {
public:
    /**
     * Constructor
     * @param pc (see Person::Person())
     * @param why (see Person::Person())
     * @param mother (see Person::Person())
     * @param father (see Person::Person())
     * @see Person::Person()
     */
    PersonMale(
        PersonCreatorMale *pc, CauseCreation why,
        PersonFemale *mother = 0, PersonMale *father = 0
    );
    /**
     * Destructor
     */
    ~PersonMale();
    
    /**
     * Extends the 'NumberOf' feature by "NumberOfChildren"
     * @param what (see Ageable::getNumber());
     * @return the number requested
     */
    Number getNumber(NumberOf what) const;
    
    /**
     * Called by the mother if mother gives birth to a child
     * @param mother the calling mother 
     */
    virtual void slotBabyIsBorn(Person *mother);
private:
    /** The number of childern that this male if father to */
    Number numchildren;
};

class PersonFemale : public Person {
public:
    /**
     * Constructor
     * @param pc (see Person::Person())
     * @param why (see Person::Person())
     * @param mother (see Person::Person())
     * @param father (see Person::Person())
     */
    PersonFemale(
        PersonCreatorFemale *pc, CauseCreation why, 
        PersonFemale *mother = 0, PersonMale *father = 0
    );
    /**
     * Constructor
     */
    ~PersonFemale();
    
    /**
     * Extends the 'NumberOf' feature by 'NumberOfChildren'. 'IsPregnant',
     * 'NumberOfPregnancies' and 'NumberOfAbortions'
     * @param what (see Ageable::getNumber());
     * @return the number requested
     */
    Number getNumber(NumberOf what) const;
    /**
     * Returns the state of pregnancy
     * @return 'true' is female is pregnant, 'false' otherwise
     */
    bool isPregnant() const;
        
    /**
     * This method samples the event that a female gets pregnant from this
     * contact  
     * @see Person::slotNotifyHaveContact()
     */
    virtual void slotNotifyHaveContact(Partnership *ps, bool unprotected);
    /**
     * This method is called by EventBirth if a mother was specified
     * @param father the father if specified, otherwise 0
     */
    virtual void slotBabyIsBorn(PersonMale *father = 0);
    /**
     * This method is called by EventAbortion in case of an abortion
     * @param why the reason for the abortion
     */
    virtual void slotAbortion(CauseAbortion why);
    /**
     * This method is called by EventGetPregnant; it is also used by the
     * mechanism that checks eack sexual contact.
     * @param father a father if available
     */
    virtual void slotGetPregnant(PersonMale *father = 0);
    /**
     * Updates the EventGetPregnant Process
     * @param from (see Ageable::slotBinChange())
     * @param to  (see Ageable::slotBinChange())
     */
    virtual void slotBinChange(Bin from, Bin to);
    /**
     * Updates the EventGetPregnant Process
     * @param ps (see Person::slotRegisterPartnership())
     */
    virtual void slotRegisterPartnership(Partnership *ps);
    /**
     * Updates the EventGetPregnant Process
     * @param ps (see Person::slotRegisterPartnership())
     */
    virtual void slotDeregisterPartnership(Partnership *ps);
    /** Installes the next EventGetPregnant event */
    virtual void throwEventGetPregnant(bool conditional = false);
    
    /**
     * Returns some human readable information about this ageable
     * @return a C++ string
     */
    virtual std::string str() const;
protected:
    /** Process to manage the EventGetPregnant events */
    Process procGetPregnant;
    /** A flag that indicates pregnancy */
    bool pregnant;
    /** The total number of pregnancies that this female had and has */
    Number numpregnancies;
    /** The number of born children */
    Number numchildren;
    /** The number of abortions */
    Number numabortions;
};


#endif

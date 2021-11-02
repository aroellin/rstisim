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
#ifndef EVENT_H
#define EVENT_H

#include <string>

#include "constants.h"
#include "typedefs.h"

namespace global { extern Time abstime; extern Notification NOTNOTIFIED;}

/**
 * This class can be used to manage a sequence of events, such as EventHaveSex
 * with in a partnership
 */
class Process {
public:
    
    /**
     * Constructor
     */
    Process();
    /**
     * Destructor<br>
     * Should an event still be in the queue, this event is deactivated (and
     * later removed from the memory by the 'Scheduler' class
     */
    ~Process();
    /**
     * Replaces the current event by a new one without setting the time of the
     * last execution; this function is used when a process is updated
     * @param event the new event
     */
    void update(EventID event);
    /**
     * Replaces the current event by a new one, setting the time of execution;
     * this method should be used when an event in a process occured and the
     * next event should be installed 
     * @param event the new event
     */
    void replace(EventID event);
    /**
     * Returns the last executeion time
     * @return the time of execution of the last event; if no event was
     * installed, the time when the process was created is returned
     */
    Time lastTime() const;
    /**
     * Sets the last execution time independent of the effective last events;
     * produced an error if the time is in the future (>global::abstime)
     * @param time sets the last execution time
     */
    void setLastTime(Time time = global::abstime);
    /**
     * Returns the next execution time
     * @return the time of execution of the next event; if no event is
     * scheduled, MAXDOUBLE is returned
     */
    Time nextTime() const;
    /** 
     * Checks if an event is scheduled or not.
     * @return 'false' if an event is scheduler, 'true' if no event is scheduled
     */
    bool empty() const;
    /**
     * Removes the next event if present
     */
    void clear();
    
protected:
    /**
     * The next event
     */
    EventID nextevent;
    /**
     * The time of last execution
     */
    Time lastexecution;
};

/**
 * This is the base class for all events. Although its not abstract, it should
 * not be used directly. If you inherit from this class, you need to define the
 * 'execute' and 'str' methods. 
 */
class Event
{
public:
    /**
     * Constructor
     * @param time the time when the event should be executed 
     */
    Event(Time time);
    /**
     * Destructor<br> Note that it is virtual.
     */
    virtual ~Event();
    /**
     * A unique event id based on global::countereuid
     */
    Counter euid;
    /**
     * The time when this event was created
     */
    Time timecreated;
    /**
     * The class id
     * @see constants.h
     */
    int idclass;
    /**
     * The subclass id
     * @see constants.h
     */
    int idsubclass;
    /**
     * The time when the event should be executed
     */
    Time time;
    /**
     * If false, the event will not be executed 
     */
    bool active;
    /**
     * 'True', if the 'execute' method was called, 'false' otherwise
     */
    bool executed;
    /**
     * Executes the event; this method should be overwritten by child classes
     * but still called before anything else is done; it essentially updates the
     * global::abstime variable and sets the 'executed' flag to true
     */
    virtual void execute();
    /**
     * Returns a description of the event; should be overwritten by child
     * classes; see examples of how to do use this method
     * @return a string with a description of the event
     */
    virtual std::string str() const;
    /**
     * Prints the string that is returned by the str method using the R output
     * interface 'Rprintf'
     */
    virtual void print() const;
};

/**
 * This is the event for the death of an Ageable (person, partnership,
 * infection). When executed it calls the 'slotDeath' method of the ageable.
 * For person this is the actual death, for partnership when it ends. For
 * and infections this is the timepoint when the infection is removed from the
 * host (after being in one of the immune states 'cleared' or 'treated', so it's
 * not when the host defeats the infection and becomes immune
 */
class EventDeath : public Event {
public:
    /**
     * Constructor
     * @param time (see Event::time)
     * @param target the ageable that should die at the specific time
     */
    EventDeath(Time time, Ageable *target);
    /**
     * Destructor
     */
    ~EventDeath();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /**
     * A pointer to the ageable that will reveive the death event
     */
    Ageable *target;
};

/** 
 * This is the event for the birth of a new person; the execute() method
 * first calls the slotBabyIsBorn() method of the mother if available and
 * then calls slotBirth of the Population class which actually creates
 * the new Person; the father is informed by the mother
 */
class EventBirth : public Event {
public:
    /**
     * Constructor
     * @param time (see Event::time)
     * @param why the cause of the birth
     * @param mother a pointer to the mother if available
     * @param father a pointer to the father if available
     */
    EventBirth(
        Time time, CauseCreation why, 
        PersonFemale *mother = 0, PersonMale *father = 0
    );
    /**
     * Destructor
     */
    ~EventBirth();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** The cause of creatino */
    CauseCreation why;
    /** The mother, or 0 if not available */
    PersonFemale *mother;
    /** The father, or 0 if not available */
    PersonMale *father;
};

/**
 * This is the Event that a female becomes pregnant and used by the
 * 'generalrate' feature, but not by the 'percontact' feature; for the latter
 * the female gets pregnant instantaniously. The execute() method calls the
 * PersonFemale::slotGetPregnant() method of the mother
 */ 
class EventGetPregnant : public Event {
public:
    /**
     * Constructor
     * @param time (see Event::time)
     * @param mother a pointer to the mother
     * @param father if available, a pointer to the father
     */
    EventGetPregnant(Time time, PersonFemale *mother, PersonMale *father = 0);
    /**
     * Destructor
     */
    ~EventGetPregnant();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** A pointer to the mother of the child */
    PersonFemale *mother;
    /** A pointer to the father of the child, or 0 if not available */
    PersonMale *father;
};

/** 
 * This is the Event of an abortion and is the alternative of EventBirth; the
 * execute() method calls the PersonFemale::slotAbortion() method
 */
class EventAbortion : public Event {
public:
    /**
     * Constructor 
     * @param time (see Event::time)
     * @param why the cause of abortion
     * @param mother if available, a pointer to the mother
     * @param father if available, a pointer to the father
     */
    EventAbortion(
        Time time, CauseAbortion why, 
        PersonFemale *mother, PersonMale *father = 0
    );
    /**
     * Destructor
     */
    ~EventAbortion();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** The cause of abortion */
    CauseAbortion why;
    /** A pointer to the mother, or 0 if not available */
    PersonFemale *mother;
    /** A pointer to the father, or 0 if not available */
    PersonMale *father;
};

/** 
 * This is the Event of an immigration; the execute() method calls the
 * Population::slotImmigration() method.
 */
class EventImmigration : public Event {
public:
    /**
     * Constructor
     * @param time (see Event::time)
     */
    EventImmigration(Time time);
    /**
     * Destructor
     */
    ~EventImmigration();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
};

/** 
 * This is the event of an ageable (person, partnership, infection) changing
 * its bin; the execute() method calls the Ageable::slotBinChange method
 * (or the corresponding overwritten method, respectively)
 */
class EventBinChange : public Event {
public:
    /**
     * Constructor
     * @param time (see Event::time) 
     * @param target a pointer the ageable
     * @param from the bin in which the ageable is in; this is for later
     * control only
     * @param to the bin into which the ageable should change
     */
    EventBinChange(Time time, Ageable *target,Bin from, Bin to);
    /**
     * Destructor
     */
    ~EventBinChange();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** A pointer to the ageable changing the bin */
    Ageable *target;
    /** The bin the Ageable currently is in */
    Bin from;
    /** The bin the Ageable is going into */
    Bin to;
};

/** 
 * This is the Event to initiate a Partnership initiated by an individual and
 * is used by PSFormerIndivSearch. The execute() method calls the
 * PSFormerIndivSearch::slotPSInitiate() method
 */ 
class EventPSInitiate : public Event {
public:
    /**
     * Constructor
     * @param time (see Event::time) 
     * @param psf a pointer to the PSFormerInfivSearch object
     * @param person a pointer to the Person that will initiate a Partnership
     */
    EventPSInitiate(
        Time time, PSFormerIndivSearch *psf, Person *person
    );
    /**
     * Destructor
     */
    ~EventPSInitiate();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** A pointer to the PSFormerInfivSearch object */
    PSFormerIndivSearch *psf;
    /** a pointer to the Person that will initiate a Partnership */
    Person *person;      
};

/** 
 * This is the Event when two persons have sexual contact during a partnership;
 * the execute() method calls the Partnership::slotHaveSex() method
 */
class EventHaveSex : public Event {
public: 
    /**
     * Constructor
     * @param time (see Event::time) 
     * @param ps a pointer to the Partnership in which the sexual contact will
     * take place
     * @param unprotected a flag whether the contact is unprotected or not
     */
    EventHaveSex(Time time, Partnership *ps, bool unprotected);
    /**
     * Destructor
     */
    ~EventHaveSex();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** A pointer to the Partnership in which the sexual contact will
     * take place */
    Partnership *ps;
    /** a flag to indicate whether the contact is unprotected or not */
    bool unprotected;
};

/**
 * This is the Event to remove old objects from the memory; the execute()
 * method calls one of the Population::internalRemoveOld[...] methods
 */ 
class EventRemoveOld : public Event {
public:
    /**
     * Constructor
     * @param time (see Event::time) 
     * @param what the category of objects that should be deleted (infections,
     * partnerships)
     */
    EventRemoveOld(Time time, EnumRemove what);
    /**
     * Destructor
     */
    ~EventRemoveOld();

    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;

private:
    /** the category of objects that should be deleted (infections,
     * partnerships) */ 
    EnumRemove what;
};

/**
 * This is the Event that a Person seeks treatment; the execute() method calls
 * the Person::slotVisitGP() method
 */
class EventVisitGP : public Event {
public: 
    /**
     * Constructor<br>
     * If the 'treatment' argument equals to TypeOfTreatment::TreamentSpecific,
     * the 'infection' argument must be given
     * @param time (see Event::time) 
     * @param person a pointer to the Person that visits the GP
     * @param gpvisittype
     * @param notif
     * that the Person seeks treatment
     */
    EventVisitGP(
        Time time, Person *person, 
        Type gpvisittype, Notification notif = global::NOTNOTIFIED, 
        enum CauseVisitGP cause = CauseSymptomsGeneral
    );
    /**
     * Destructor
     */
    ~EventVisitGP();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    Person *person;
    Type gpvisittype;
    Notification notif;
    enum CauseVisitGP cause;
};

/** 
 * This is the Event that an infection provokes that its host goes to the GP
 * and tests for one or all infections; the execute() method calls the
 * Infection::slotProvokeGPVisit method of the infection.
 */ 
class EventProvokeGPVisit : public Event {
public: 
    /**
     * Constructor<br>
     * @param time (see Event::time) 
     * @param infection a pointer to the infection that will provoke that
     * its host tests for an infection
     */
    EventProvokeGPVisit(Time time, Infection *infection);
    /**
     * Destructor
     */
    ~EventProvokeGPVisit();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** A pointer to the infection */
    Infection *infection;
    /** The cause of visit */
    // CauseVisitGP cause; // unused at the moment
};

/**
 * This is the Event that a Person actually is treated, usually a while after
 * seeking treatment; the execute() method calls the Person::slotTreat() method.
 */
class EventTreat : public Event {
public: 
    /**
     * Constructor<br>
     * If the 'treatment' argument equals to TypeOfTreatment::TreamentSpecific,
     * the 'infection' argument must be given
     * @param time (see Event::time) 
     * @param person a pointer to the Person that will be treated
     * @param treatment the kind of treatment (specific, general)
     * @param infectiontype if available, the infection type that caused the
treatment
     */
    EventTreat(
        Time time, Person *person, Type infectiontype
    );
    /**
     * Destructor
     */
    ~EventTreat();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** a pointer to the Person that will be treated */
    Person *person;
    /** the infectiontype to treat against */
    Type infectiontype;
};

/**
 * This is the Event that a specific person is infected, usually when
 * populating the model or when immigrating. The execute() method calls the 
 * InfectionCreator::slotInfectPerson() method. This Event is thrown by the
 * InfectionCreator::slotRandomlyInfectPerson() method with a certain
 * probabiliy. This Event class was introduced to avoid certain problems, when
 * a person is registering itself in the population, which happens when
 * Person::Person() is called, but at that time the object is not fully
 * initialised yet, so any infection is postponed to a latter timepoint
 * (actually to the same timepoint, but as a different Event in the event queue)
 */
class EventInfectPerson : public Event {
public:
    /**
     * Constructor
     * @param time (see Event::time) 
     * @param inftype a pointer to the InfectionCreator that will infect the
     * victim
     * @param person a pointer to the Person that will be the victim of the
     * infection
     */
    EventInfectPerson(Time time, InfectionCreator *inftype, Person *person);
    /**
     * Destructor
     */
    ~EventInfectPerson();
    
    /**
     * (see Event class)
     */
    void execute();
    /**
     * (see Event class)
     */
    std::string str() const;
    
private:
    /** a pointer to the InfectionCreator that will infect the victim */
    InfectionCreator *inftype;
    /** a pointer to the Person that will be the victim of the infection */
    Person *person;
};

#endif

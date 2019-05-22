\name{sti.infections}
\alias{sti.infections}

\title{Get a detailed list of infections from the simulation}

\description{
This command is the main interface to obtain information about the infections
in the population. After each call to \code{sti.run} or \code{sti.execute} it
is necessary to call this function again to obtain an updated version. 
}

\usage{
sti.infections(old.infections=FALSE, relevel=TRUE, current.people=TRUE)
}

\arguments{
\item{old.infections}{logical; if \code{TRUE}, a list with the past infections
of the population is returned, otherwise (default) the still active infections
and infections in the \code{cleared} or \code{treated} state are returned; }
\item{relevel}{logical; if \code{TRUE} (default), the \code{bin}-factor is
re-coded so that all bins with the same name represent the same factor level; if
\code{FALSE}, bins of different infection types having the same name will
represent different levels}
\item{current.people}{logical; if \code{FALSE} the infections with respect
to the people who left the population are returned; otherwise (default) the
infections of the current population are returned}
}

\details{
This function can return four different types of infection lists. Without any
arguments it returns the active infections (where the \code{cleared} and
\code{treated} states are also counted as active) with respect to people of the
active population. If \code{old.infections=TRUE} the past infections are
returned but still only of the current population. 

Once a person leaves (death or emigration) the population, the state of all its
infections is frozen, so that a dead person still can have active infections.
These are returned if \code{sti.infections} is called only
with \code{current.people=FALSE}. The forth combination
\code{old.infections=TRUE} and \code{current.people=FALSE} then returns all the
past infections of people that left the population.
}

\value{
Alist of class \code{data.frame} with the following elements:
\item{infuid}{integer; an ID that uniquely identifies the infection; this number
is unique within a single run of the simulation among all infections, active or
removed}
\item{strainid}{integer; the 'genetic code' of the infection, inherited from the
parent infection; this number is inherited when an infection is cloned, ie.
transmitted from host to host; when an infection enters the population without a
parent (initial infection at the beginning of the simulation or if a person
immigrates as an infected) it is assigned a new \code{strainid}}
\item{type}{factor; the type of the infection as defined in the 'diseases'
section of the configuration file}
\item{bin}{factor; the 'bin' in which the infection is at the moment; the levels
of the this factor depend on the 'relevel' argument}
\item{begin}{the time point of infection}
\item{endofinfection}{the time point when the infection enters one of the
immune states \code{cleared} or \code{treated}; if the infection is
active but not in the immune \code{cleared} or \code{treated} (ie some
state prior to these states) the time point when this happend is unknown and
hence this variable is set to NA}
\item{endofinfection}{the time point when the infection enters one
 of the immune states \code{cleared} or \code{treated}; if the
infection is active but not yet in one of these two states (ie some state prior
to these states) the end of infection time is yet unknown and hence set to NA}
\item{endofimmunity}{the time point when the infection is removed from the host;
THIS IS THE TIME WHEN THE IMMUNITY STATES \code{cleared} OR \code{treated}
ENDS AND THE HOST CAN BE INFECTED AGAIN BY THIS TYPE OF INFECTION, it is
*not* when the infection enters the immune states \code{cleared} or
\code{treated} (see previous value); if the infection is active but not in
the one of the immune states \code{cleared} or \code{treated} (ie some state
prior to these states) the time of removal is unknown and hence set to NA; once
it enters one of these states, the time of removal is simulated from the
\code{immunity}-distribution}
\item{parentinfuid}{integer; the \code{infuid} of the parent infection}
\item{puid}{integer; the ID that uniquely identifies the host the infection is
attached to; this number is unique within a single run of the simulation among
all the people, living or dead}
\item{adhocid}{integer; the adhoc ID of the host the infection is attached
to; this is a sequential number within all the living people; the youngest
person obtains the id 1 and the oldest (still living) person obtains the highest
id (which equals the total number of individuals in the living population); THIS
NUMBER WILL CHANGE IF THE SIMULATION IS CONTINUED! More precisely: each time a
person enters or leaves the population, this number will change. This number is
the row number of the data frame returned by \code{sti.people} and NA if the
person has left the population)}
\item{psuid}{integer; the ID that uniquely identifies the partnership; this
number is unique within a single run of the simulation among all the
partnerships, active or ended; it is set to NA if the infection was created
during initiating the model or by immigration}
}

\seealso{
\code{sti.people}, \code{sti.partnerships}, \code{sti.prevalence}
}

\examples{\dontrun{
# load the chlamydia-model from the package
sti.init("chlamydia.cfg")

# simulate; this may take a few minutes
sti.run(10*365)

# get all infections, past and current, of dead and living people
inf <- rbind(sti.infections(),sti.infections(old=TRUE),
    sti.infections(current=FALSE),sti.infections(old=TRUE,current=FALSE))

# the number of infections that already know the time of removal against those
# that dont't
table(is.na(inf$end))

# only use the ones with end time
withend <- !is.na(inf$endofinfection)
chlamydia <- inf$type=="chlamydia"

# get the sex of the host; we cannot work with the adhocid because some of the
# hosts are already dead
p <- rbind(sti.people(),sti.people(cur=FALSE))    # get all the people
rownames(p) <- p$puid                           # use puid to identify rows
inf$hostsex <- p[as.character(inf$puid),"type"] # get the gender using the puid

# plot Kaplan-Meier for the duration of infection of chlamydia, split by the sex
# of the host and whether cleared naturaly or treated
library(survival)
inf$binbysex = inf$hostsex:inf$bin
inf$binbysex = factor(levels(inf$binbysex)[unclass(inf$binbysex)])
sf <- survfit(Surv(endofinfection-begin) ~ binbysex, inf, 
        subset=withend & chlamydia) 
plot(sf, col=1:4,xlab="days",
    main="Kaplan-Meier curves for duration of infection")
legend(1000,0.9,substr(names(sf$strata),10,99),lty=1,col=1:nlevels(inf$binbysex)
)

# get some statistics of the duration of infection
duration <- inf$endofinfection[withend & chlamydia] -
            inf$begin[withend & chlamydia]
summary(duration)
hist(duration,breaks=100)


# remove model from meory
sti.clear()
}}

\name{sti.gpvisits}
\alias{sti.gpvisits}

\title{Get a detailed list of GP visits from the simulation}

\description{
This command is the main interface to obtain information about the GP visits
that happened in the population. After each call to \code{sti.run} or
\code{sti.execute} it is necessary to call this function again to obtain an
updated list. 
}

\usage{
sti.gpvisits(relevel=TRUE, all=TRUE, sortby=c("time", "none", "nuid"))
}

\arguments{
\item{relevel}{logical; if \code{TRUE} (default), the \code{bin}-factors of all
types are re-coded so that all bins with the same name represent the same factor
level; if \code{FALSE}, bins of different types having the same name will 
represent different levels;}
\item{all}{logical; if \code{TRUE} (default), a list with all the GP visits 
from the simulation is returned, otherwise only the GP visits of the 
current population is returned. }
\item{sortby}{sort the returned list according to either \code{time} (default), 
\code{nuid}, or \code{none}}
}

\details{
This function returns the GP visits that happened in the population. Two 
possible lists can be returned: either the list of all GP visits throughout 
the simulation or only the GP visits made by the people in the current 
population. 
}

\value{
A list of class \code{data.frame} with the following elements:
\item{gpvtype}{factor; the type of GP visit used for the visit}
\item{time}{the time point at which the GP visit took place}
\item{cause}{factor; reason for the visit; possible values are "general", 
"symptoms" (caused by an infection), "notified" (caused through notification)}
\item{puid}{integer; the unique ID for the person that visits the GP}
\item{ptype}{factor; the type of the person that visits the GP}
\item{pbin}{factor; the bin of the person that visits the GP AT THE TIME OF
VISIT, not at the current time}
\item{linknumber}{the linknumber of the visiting person; index case has 
linknumber 0, each person notified by the index case has linknumber 1, and so
on and so forth; people who got notified within a specific notification cascade
are not notified again (including the index case)}
\item{nuid}{integer; an ID that uniquely identifies the notification cascade; 
this ID is uniquely created for each index case; this is only different from NA
cause="notified"}
\item{dirtreated}{the number of infections directly treated without testing}
\item{tested}{the number of infections for which a test is done}
\item{posresults}{the number of positive test results}
\item{ntype}{factor; the type of the notifier module that is being called *after* 
the GP visit; if no notifier module is activated, this is NA; if more than one 
notifications modules are called, the last one being called is the one stored here}
}


\examples{\dontrun{
# load the chlamydia-model from the package
sti.init("ex08.cfg")

# simulate;
sti.run(10*365)

# get all infections, past and current, of dead and living people
nf <- sti.notifications()

# remove model from meory
sti.clear()
}}

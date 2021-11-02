\name{sti.notifications}
\alias{sti.notifications}

\title{Get a detailed list of notifications from the simulation}

\description{
This command is the main interface to obtain information about the notifications
that happened in the population. After each call to \code{sti.run} or
\code{sti.execute} it is necessary to call this function again to obtain an
updated version. 
}

\usage{
sti.notifications(relevel=TRUE, all=TRUE, sortby=c("nuid","none","time"))
}

\arguments{
\item{relevel}{logical; if \code{TRUE} (default), the \code{bin}-factors of all
types are re-coded so that all bins with the same name represent the same factor
level; if \code{FALSE}, bins of different types having the same name will 
represent different levels;}
\item{all}{logical; if \code{TRUE} (default), a list with all the notifications 
from the simulation is returned, otherwise only the notifications of the 
current population is returned. }
\item{sortby}{sort the returned list according to either \code{nuid} (default), 
\code{time}, or \code{none}}
}

\details{
This function returns the notifications that happened in the population. Two 
possible lists can be returned: either the list of all notifications throughout 
the simulation or only the notifications made by the people in the current 
population. Note that in the latter case, the notification cascades may not be 
complete. 
}


\value{
A list of class \code{data.frame} with the following elements:
\item{ntype}{factor; the type of the notifier module}
\item{nuid}{integer; an ID that uniquely identifies the notification cascade; 
this ID is uniquely created for each index case; note that not each possible
number will appear: if the index case does not notify anyone then this
\code{nuid} will not appear in the list}
\item{linknumber}{the linknumber of the notifying person; index case has 
linknumber 0, each person notified by the index case has linknumber 1, and so
on and so forth; people who got notified within a specific notification cascade
are not notified again (including the index case)}
\item{time}{the time point at which the notification took place}
\item{puid1}{integer; the unique ID for the person that notifies}
\item{ptype1}{factor; the type of the person that notifies}
\item{pbin1}{factor; the bin of the person that notifies AT THE TIME OF
NOTIFICATION, not at the current time}
\item{puid2}{integer; the unique ID for the person that is being notified}
\item{ptype2}{factor; the type of the person that is being notified}
\item{pbin2}{factor; the bin of the person that is being notified AT THE TIME OF
NOTIFICATION, not at the current time}
\item{psuid}{integer; the unique ID for the partnership through which the
notifications happens}
\item{pstype}{factor; the type of the partnership through which the
notifications happens}
\item{psbin}{factor; the bin of the partnership through which the
notifications happens AT THE TIME OF NOTIFICATION, not at the current time} 
}

\examples{\dontrun{
# load the chlamydia-model from the package
sti.init("ex08.cfg")

# simulate;
sti.run(10*365)

# get all infections, past and current, of dead and living people
gpv <- sti.gpvisits()

# remove model from meory
sti.clear()
}}

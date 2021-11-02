\name{sti.events}
\alias{sti.events}

\title{Return a list with the next events in the event queue}

\description{
This function returns a list of current events in the event queue.
}

\usage{
sti.events(number=1, include.text=FALSE, active.only=TRUE)
}

\arguments{
\item{number}{integer; the number of events that are returned, starting with the 
event that will be executed next}
\item{include.text}{character; returns for each events a detailed string with a
lot of information; be careful with this feature when requesting the list of all
events as it may require a lot of memory}
\item{active.only}{boolean; if \code{FALSE} also the events that are outdated
are included; defaults to \code{TRUE}}
}

\value{A list of class \code{data.frame} with the following elements:
\item{time}{the time when the event will be executed (if active)}
\item{type}{factor; the type of event}
\item{active}{boolean; \code{TRUE} if the event is still active, \code{FALSE} if
the event is outdated}
\item{text}{a detailed description of the event if \code{include.text=TRUE}, NA
otherwise; check the source code for details}
}

\seealso{
\code{sti.scheduler}
}

\examples{\dontrun{
# load the 'chlamydia'-model from the package
sti.init("chlamydia.cfg")

# get the first 100 events with detailed description
sti.events(100, include.text=TRUE)

# remove the model
sti.clear()
}}

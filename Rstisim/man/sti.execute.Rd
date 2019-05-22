\name{sti.execute}
\alias{sti.execute}

\title{Run a certain number of events of an initialised model}

\description{
This function executes a given number of events of the simulation. During this
time it is not possible to stop or interact with the simulation in any way.
}

\usage{
sti.execute(number=1, verbose=TRUE, clearstats=FALSE)
}

\arguments{
\item{number}{the number of (active) events that should be executed}
\item{verbose}{logical; if \code{TRUE} (default), each event is being printed
before
executed} 
\item{clearstats}{logical; if \code{TRUE}, the \code{relative statistics} (see
below)
are cleared at the end the simulation; the default is \code{FALSE}}
}

\details{
This function is similar to \code{sti.run} but executes a fixed number of
events irrespective of the amount of time that passes. It is useful for
debugging purposes if too many events happen even in a short period of time due
to a misspecification of the model. Therefore, the 'verbose' argument is
activated by default. 

Note that only 'active' events are counted; c.f. with \code{sti.scheduler}.
}

\value{A list with two elements, each following the structure as returned by
\code{sti.stats()}.
\item{relative}{the relative change of the global statistics; this is just the
difference of the global statistics between the end of the execute of this
functions and the last time the statistics were reseted (usualy the last time
\code{sti.run} was called)}
\item{total}{the values of the global statistics reltaive to the absolut
beginning of the simulation, ie. time 0}
}

\seealso{
\code{sti.run}, \code{sti.scheduler}
}

\examples{\dontrun{
# load the 'chlamydia'-model from the package
sti.init("chlamydia.cfg")

# execute the first 100 events
sti.execute(100)

# remove the model
sti.clear()
}}
\name{sti.partnerships}
\alias{sti.partnerships}

\title{Get a detailed list of partnerships from the simulation}

\description{
This command is the main interface to obtain information about the partnerships
in the population. After each call to \code{sti.run} or \code{sti.execute} it
is necessary to call this function again to obtain an updated version. 
}

\usage{
sti.partnerships(old.partnerships=FALSE, relevel=TRUE) 
}

\arguments{
\item{old.partnerships}{logical; if \code{TRUE}, a list with the past
partnerships returned (of the current population *and* people that died or
emigrated), otherwise (default) the still active partnerships are returned
(of the current population, as an active partnerships can only exists between
two persons in the current population)}
\item{relevel}{logical; if \code{TRUE} (default), the \code{bin}-factor is
re-coded so that all bins with the same name represent the same factor level; if
\code{FALSE}, bins of different partnership types having the same name will
represent different levels}
}

\details{
ToDo
}

\value{
A list of class \code{data.frame} with the following elements:
\item{psuid}{integer; an ID that uniquely identifies the partnership; this
number is unique within a single run of the simulation among all the
partnerships, active or ended}
\item{type}{factor; the type of the partnership as defined in the 'partnerships'
section of the configuration file}
\item{bin}{factor; the 'bin' in which the person is at the moment; the levels
of the this factor depend on the 'relevel' argument}
\item{begin}{the time of creation in days, relative to the absulute time 0}
\item{end}{the time of separation of the partnership, relative to the absolute
time 0}
\item{puid1}{integer; the unique ID of the first person of the partnership; see
\code{sti.people}}
\item{adhocid1}{integer; the adhoc ID of the first person of the partnership;
\code{sti.people}}
\item{puid2}{integer; the unique ID of the second person of the partnership;
\code{sti.people}}
\item{adhocid2}{integer; the adhoc ID of the second person of the partnership;
\code{sti.people}}
\item{formertype}{factor; the type of the partnership former}
\item{fitness}{the probability that was used to sample whether this partnership
should be formed or not; the lower this value, the less likely it was that this
partnership would form; }
\item{tries}{integer; the number of tries that were necessary to form this 
partnership}
\item{contacts}{integer; the number of sexual contacts in this partnership}
\item{contactsunprot}{integer; the number of unprotected sexual contacts in this
partnership}
}

\seealso{
\code{sti.people}, \code{sti.infections}, \code{factor}
}

\examples{\dontrun{
# load the chlamydia-model from the package
sti.init("chlamydia.cfg")

# simulate; this may take a few minutes
sti.run(10*365)




# remove model from memory
sti.clear()

}}

\name{sti.stats}
\alias{sti.stats}

\title{Returns the global statisitcs of the simulation}

\description{
This function returns a vector with some important global statistics of the
simulation, such as the current time, population size, number of executed
events etc. The numbers are always absolute, ie. relative to time zero.
Relative values, as returned by \code{sti.run} are calculated by storing the
global statistics at different time points.
}

\usage{
sti.stats()
}

\value{A named vector with the following elements:
\item{time}{the current time of the simulation in days; the simulation always
start at time point 0}
\item{population size}{the current population size}
\item{events}{the total number of events that have been successfully executed
(events that have been inserted but deleted before execution are not counted)}
\item{births}{the total number of people introduced in the population, either
through replacing dead people or through natural birth (depends on the
parameters in the 'model.population' section in the configuration file); it
only counts the people actually inserted into the population}
\item{deaths}{the total number of people that died}
\item{immigrations}{the total number of people that immigrated into the
population}
\item{emigrations}{the total number of people that left the population through
emigration}
\item{person bin changes}{the total number of bin changes of people in the
population}
\item{ps bin changes}{the total number of bin changes of partnerships in the
population}
\item{inf bin changes}{the total number of bin changes of infections in the
population}
\item{new partnerships}{the total number of partnerships that where created}
\item{ended partnerships}{the total number of partnerships that ended}
\item{contacts}{the total number of sexual contacts}
\item{unprotected contacts}{the total number of unprotected sexual contacts,
always less or equal to 'contacts'}
\item{pregnancies}{the total number of women that got pregnant}
\item{infections}{the total number of individuals that got infected not
including people that are infected at initialisation or immigration}
\item{clearances}{the total number of infividuals that naturally cleared an
infection; this is essentialy the number of infections that entered the state
'cleared', even if this state lasts for zero length of time}
\item{treatments}{the total number of individuals that were treated}
\item{treatmentsvain}{the number of individuals that were treated although the
infection was not present because of natural clearance between testing and
treating or because of a false positive test result)}
\item{followupvisits}{the number of people visiting the gp because they have 
been notified by their partners}
}

\seealso{
\code{sti.run}
}


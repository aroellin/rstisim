\name{sti.people}
\alias{sti.people}

\title{Get a detailed list of people from the simulation}

\description{
This command is the main interface to obtain information about the individuals
in the population. After each call to \code{sti.run} or \code{sti.execute} it
is necessary to call this function again to obtain an updated version. 
}

\usage{
sti.people(current.people=TRUE, relevel=TRUE,
    breaks=seq(floor(sti.agerange()[1]/5)*5,ceiling(min(sti.agerange()[2]/5,10000))*5,5),
    right=FALSE, include.lowest=TRUE)
}

\arguments{
\item{current.people}{logical; if \code{FALSE}, a list with the people who left
the population is given, otherwise (default) the current population is returned}
\item{relevel}{logical; if \code{TRUE} (default), the \code{bin}-factor is
re-coded so that all bins with the same name represent the same factor level; if
\code{FALSE}, bins of different types having the same name will represent
different levels}
\item{breaks, right, include.lowest}{these arguments control the generation of
the \code{agegroup}-factor (see \code{cut}-command); the default is groups
of 5-years}
}

\details{
ToDo
}

\value{
A list of class \code{data.frame} with the following elements:
\item{puid}{integer; an ID that uniquely identifies the person; this number is
unique within a single run of the simulation among all the people, living or
dead} 
\item{adhocid}{integer; the sequential number within the list of
living people; the youngest person obtains the id 1 and the oldest (still
living) person obtains the highest id (which equals the total number of
individuals in the living population); THIS NUMBER WILL CHANGE IF THE SIMULATION
IS CONTINUED! More precisely: each time a person enters or leaves the
population, this number will change. This number is the row number of
this data frame and NA if the person has left the population (only if
\code{old=TRUE})}
\item{type}{factor; the type of the person as defined in the 'population'
section of the configuration file}
\item{bin}{factor; the 'bin' in which the person is at the moment; the levels
of the this factor depend on the 'relevel' argument}
\item{birth}{the time point of birth in days; may be negative for people that
entered the population during initialization of the model or immigration}
\item{death}{the time point of death in days; lying in the future for people
from the current population}
\item{partnerscurrent}{integer; the number of current partners}
\item{partnerstotal}{integer; the total number of partners that this person has,
including the current ones}
\item{partnerswithin}{integer; the number of partners that this person had
within the last period of time (specified by the parameter
\code{simulation.withintimelag} in the configuration file, defaults to 1y),
including the current partners}
\item{contacts}{integer; the total number of contacts}
\item{contactsunprot}{integer; the total number of unprotected contacts}
\item{pregnant}{logical; \code{TRUE} if the person has basetype "FEMALE" and is
pregnant, \code{FALSE} if the person has basetype "FEMALE" and is not
pregnant, NA otherwise}
\item{pregnancies}{integer; the total number of pregnancies (including a
possible current one) if person type has basetype "FEMALE"; NA otherwise}
\item{abortions}{integer; the total number of abortions if person type has
basetype "FEMALE"; NA otherwise}
\item{children}{integer; if person type has basetype "MALE" or "FEMALE" the
number of children; NA otherwise; for male, this number is only increased if
the pregnancy occures during a specific contact, for general pregnancies the
fatherhood is not tracked}
\item{infectionscurrent}{integer; the number of current infections; this number
cannot be bigger than the number of infection types defined in the model}
\item{infectionstotal}{integer; the total number of infections (including the
current ones)}
\item{infectionswithin}{integer; the number of infections that this person had
within the last period of time (specified by the parameter
\code{simulation.withintimelag}), including the current infections}
\item{treatments}{integer; the number of treatments that the person has
undergone}
\item{gpvisits}{integer; the number the person went to the GP}
\item{notifications}{integer; the number of notifications the person has received2}
\item{age}{the age of the person in years; NA if \code{old=TRUE}}
\item{agegroup}{the age group of the person (controled by the arguments
\code{breaks}, \code{right} and \code{include.lowest}); if
\code{old=TRUE}, a factor with just one level \code{dead}}
}

\seealso{
\code{sti.partnerships}, \code{sti.infections}
}

\examples{\dontrun{
# load the 'chlamydia'-model from the package
sti.init("chlamydia.cfg")

# run the simulation; this may take a few minutes
sti.run(10*365)

# get the population and some subgroups
p <- sti.people()
females <- p$type=="female"
between <- p$age>20 & p$age<35

# get the percentage of pregnant females, split by bin
t = table(p$bin[females],p$pregnant[females])
print(t / rowSums(t))

# plot the number of partners within last year split by risk group and sex
# for people older than 20 and younger than 35
plot(sti.dist(type ~ partnerswithin | bin, p, subset=between),legend=TRUE)

plot(sti.dist(type ~ partnerswithin, p, subset=between),legend=TRUE)
plot(sti.dist(type ~ partnerswithin, p, subset=between),legend=TRUE)

# Delete model from memory
sti.clear()

}}

\name{sti.run}
\alias{sti.run}

\title{Run an initialised model for a specific amount of time}

\description{
This function runs the model for a specific length of time. During this time it
is not possible to stop or interact with the simulation in any way.
}

\usage{
sti.run(time, verbose=TRUE, clearstats=TRUE)
}

\arguments{
\item{time}{the amount of time in days the simulations will run}
\item{verbose}{logical; if \code{TRUE} (default), the CPU execution time is printed 
after the simulation is finished} 
\item{clearstats}{logical; if \code{TRUE} (default), the \code{relative
statistics} (see below) are cleared at the end the simulation}
}

\details{
This is the workhorse of the package. The function essentialy constists of a
loop that executes events in the event list until the stop time (i.e. start
time plus 'howmuch') is reached.

If you want to run the simulation for a specific amount of time, say 50y, and
you want to extract information about the model stats (e.g. prevalence) in
shorter time intervals then you need to write a loop that calls \code{sti.run}
just for a short amount of time, 1y say, and in between these function calls
extract and store the specific information in \code{R} variables. It is not
possible to interrupt the simulation while the C++-code is running. If you use a
loop with R, however, you can savely interupt the simulation with Ctrl-C as
such an interupt will stop the loop between different calls of \code{sti.run}.

During the development phase of a model, it is also important to start with
small time intervals such as 1 (that is 1 day), so that you get an impression of
how fast your model runs. Otherwise you might end up guessing after a few hours
of simulation whether everything is fine or, maybe due to a missspecification,
your the model is not doing what you would like it to do.
}

\value{A list with two elements, each following the structure as returned by
\code{sti.stats()}.
\item{relative}{the relative change of the global statistics (see
\code{sti.stats}; this is the difference of the global statistics between
the end of the execute of this functions and the last time the statistics were
reseted (usualy the last time \code{sti.run} was called)}
\item{total}{the values of the global statistics relative to the absolut
beginning of the simulation, ie. time 0}
}

\seealso{
\code{sti.init}, \code{sti.execute}, \code{sti.stats}.
}

\examples{\dontrun{
## SIR model
# load the SIR model from the package
sti.init("SIR.cfg")
popsize <- sti.stats()["population size"]

# let the model run in 1 day intervals and store the number of susceptibles,
# infected and recovered people
S <- numeric(0)
I <- numeric(0)
R <- numeric(0)

for (i in 1:200) {  # this may take a few minutes
    sti.run(1)
    p <- sti.people()
    S <- append(S, popsize - sum(p$infectionscurrent))
    I <- append(I,           sum(p$infectionscurrent))
    R <- append(R, popsize - S[i] - I[i])
}

# show the result
plot(S/popsize,t="l",ylab="percentage",xlab="time in days",ylim=0:1)
lines(I/popsize)
lines(R/popsize)

# remove the model from the memory
sti.clear()


# load the SIS model from the package
sti.init("SIS.cfg")

# let the model run in monthly intervals and store the number of
# infected people
prev <- numeric(0)

for (i in 1:200) {  # this may take a few minutes
    sti.run(365/12)
    prev <- append(prev, sti.prevalence(breaks=c(0,Inf))$generic)
}

# show the result
plot(prev)
abline(h=mean(prev))

# calcuate and plot the theoretical equilibrium
beta <- 2 * sti.set("model.partnershipformers.individual.seek.rate",ver=FALSE)
nu <- sti.set("model.infections.generic.bins.transitions.i2c.at.rate",ver=FALSE)
abline(h=(beta-nu)/beta,lty=2)

# remove the model from the memory
sti.clear()


## More complex model with prevalence per age
# load the 'chlamydia'-model from the package
sti.init("chlamydia.cfg")

prev <- numeric(0)

# run the model for 8000 days and store the prevalence every 20 days
for (i in 1:400) {
    sti.run(20)
    p <- sti.people(breaks=0:100)
    prevf <- sti.prevalence(p,subset=p$type=="female")$chlamydia[13:35]
    prevm <- sti.prevalence(p,subset=p$type=="male")$chlamydia[13:35]
    prev <- rbind(prev,c(prevf,prevm))
}


}}

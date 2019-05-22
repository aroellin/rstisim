\name{sti.scheduler}
\alias{sti.scheduler}

\title{Statistics about the scheduler}

\description{
Internally, events are stored in a binary heap structure, a special priority
queue, that allows for quick insertion of events according to execution time
and quick deletion of the event with the hightest priority (ie. the smallest
execution time). In the current implementation, if an arbitrary events should
be deleted, only a flag is set that marks this event as deleted and which is
therefore not executed when it is the next event in the queue. As the length of
the queue is fixed at the beginning (through the 'eventlistsizefactor'
parameter in the configuration file), it is necessary to control this queue to
avoid an overflow exception error. 

The number of events (inactive plus/or active) in the queue may also be used to 
check if the model is in equilibrium.
}

\usage{
sti.scheduler()
}

\value{A named vector with the following elements:
\item{maximal queue size}{the maximal number of events that the scheduler can
manage; an attept to insert more events will result in an error}
\item{events in queue}{the physical number of events that are in the queue at
the moment (active plus inactive)}
\item{active events in queue}{the number of active events in the queue}
}

\examples{\dontrun{
# load the 'chlamydia'-model from the package
sti.init("chlamydia.cfg")

# run the model and record the total number of events and the number of active
# events (this may take from some minutes up to half an hour or so)
total = numeric(0)
for (i in 1:50) {
    sti.run(365)
    total = rbind(total, sti.scheduler()[2:3])
# plot the output
# note that the simulation need about 25 years to stabilise which is roughly
# a full generation of "sexual active" people; that is, if people start sexual 
# activity at the age of 15 and stop at 40 (ie. stay with the same partner from
# then on) it needs 25 years until all the people from the initialisation of the
# model disapear from the active population
    plot(total[,1],ylim=c(min(total),max(total)),t="l")
    lines(total[,2])
}

# remove the model
sti.clear()

}}
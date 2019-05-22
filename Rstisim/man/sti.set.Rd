\name{sti.set}
\alias{sti.set}

\title{Print and/or set specific values in a loaded configuration}

\description{
Use this function to print and modify single values in a configuration that has
been loaded before by \code{sti.init}. You can set elements only if the model
has been removed already from the memory with \code{sti.clear}.
}

\usage{
sti.set(path,value=NULL,verbose=TRUE)
}

\arguments{
\item{path}{a quoted string that desribes the path of the element as given in
the configuration file, where the elements are separated by single dots}
\item{value}{if not \code{NULL}, the new value for the element given by
the \code{path} argument}
\item{verbose}{if \code{TRUE}, the value of the element and some attributes of
this elements are printed and NULL is returned; if \code{FALSE} the function
does no printing but returns the value of the element; if the element is set by
giving a non-null argument 'value' the new value is returned, not the old}
}

\seealso{
\code{sti.init}, \code{sti.clear}, \code{sti.config}.
}

\examples{\dontrun{
# load the SIS model from the package
sti.init("SIS.cfg")

# get immunity parameter (should be 0)
sti.set("model.infections.generic.immunity")
# set immunity to some non-zero value, so we obtain an SIRS model
sti.clear()
sti.set("model.infections.generic.immunity", 5)
sti.reinit()

# let the model run in monthly intervals and store the number of
# infected people
prev <- numeric(0)

for (i in 1:400) {  # this may take a few minutes
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

}}

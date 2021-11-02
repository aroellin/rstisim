\name{sti.config}
\alias{sti.config}

\title{Get the configuration list created from the configuration file}

\description{
This function is for debugging only. It returns the nested list as
created by the parser of the configuration file. Do not manipulate this list, as
the effect will be unpredictable.
}

\usage{
sti.config()
}

\value{ A nested list with the same hierarchical structure as the configuration
file. Each element has the following attributes (except for some automatically
created elements)
\item{sourceline}{the sourceline in the configuration file where this element
is defined}
\item{used}{logical; a flag that signals if this element was used during the
initialisation of the model}
\item{path}{the absolute path of this element within the list}
\item{randcore}{if this element defines a distribution use tha random number ge
nerator defined by this attribute}
}

\seealso{
\code{sti.init}
}


\examples{\dontrun{
# load the 'chlamydia'-model from the package
sti.init("chlamydia.cfg")

# get the configuration list
config <- sti.config()

# the survival probabilites for males
config$model$people$male$lifespan$survival

# remove the model
sti.clear()
}}

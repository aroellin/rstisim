\name{sti.reinit}
\alias{sti.reinit}

\title{Initialise model from a previously loaded configuration}

\description{
This functions initialise the model using an already loaded configuration. The
configuration needs to loaded previously by \code{sti.init} and the model
needs to removed from the memory by \code{sti.clear}. Then, the
configuration can be modified by the \code{sti.set} function and the model
initialised again by \code{sti.reinit}.
}

\usage{
sti.reinit(verbose=TRUE,executecode=TRUE)
}

\arguments{
\item{verbose}{logical; to suppress output about the initialisation set to FALSE}
\item{executecode}{logical; if \code{TRUE} (default), execute any code after the 
"#EXECUTE" statement in the configuration file; the code is executed after the 
model is initialised and in the environment returned by \code{globalenv()}}
}

\value{ 
A vector as returned by \code{sti.stats()}.
}

\seealso{
\code{sti.init}, \code{sti.clear}, \code{sti.set}
}


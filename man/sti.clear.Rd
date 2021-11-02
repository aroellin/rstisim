\name{sti.clear}
\alias{sti.clear}

\title{Deletes the current model from the memory}

\description{
This is the main function to remove an initialised model from the
memory. Only after calling this function a new model can be loaded. After
calling this function, the current state of the model is completely lost, except
for the configuration parameters. You can use \code{sti.set} to modify the
parameters and the \code{sti.reinit} to load the model again.
}

\usage{
sti.clear()
}


\seealso{
 \code{sti.init}, \code{sti.reinit}, \code{sti.set}.
}


\name{sti.ca.mixing}
\alias{sti.ca.mixing}

\title{Set the mixing coefficient}

\description{
This commands can be used to change the mixing coefficient in the risk class model from proportionate to fully assortative.
}

\usage{
sti.ca.mixing(epsilon = 0.5)
}

\arguments{
\item{epsilon}{mixing coefficient 0 (proportionate) and 1 (fully assortative) }
}

\details{
This function only works with the risk class model (risk.cfg) that is provided with the Rstisim package.
}

\seealso{
\code{sti.set}
}

\examples{\dontrun{
# Initialize the model
sti.init("risk.cfg")

# Set the mixing coefficient
sti.ca.mixing(epsilon = 0.5)

# Run a simulation for 10 years
sti.run(10*365)

# remove model from meory
sti.clear()
}}

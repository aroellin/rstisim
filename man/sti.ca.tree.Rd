\name{sti.ca.tree}
\alias{sti.ca.tree}

\title{Plot a transmission tree}

\description{
This commands plots the transmission tree of an infection through a sexual contact network.
}

\usage{
sti.ca.tree(gobackyears=10, largest.component=TRUE)
}

\arguments{
\item{gobackyears}{number of years in the past during which infections are recorded; }
\item{largest.component}{logical, if \code{TRUE} (default), only the largest component of all transmission trees is shown; if \code{FALSE}, all transmission trees are shown}
}

\details{
This function makes use of the package igraph to plot the transmission trees over a certain time period. Note that it will be important to define infectionsolderthan sufficiently long in the configuration file.
}

\seealso{
\code{sti.ca.infections}
}

\examples{\dontrun{
# load the chlamydia-model from the package
sti.init("risk.cfg")

# simulate; this may take a while
sti.run(100*365)

# Plot a transmission tree
sti.ca.tree()

# remove model from meory
sti.clear()
}}

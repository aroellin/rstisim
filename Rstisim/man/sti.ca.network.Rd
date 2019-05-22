\name{sti.ca.network}
\alias{sti.ca.network}

\title{Plot the sexual contact network}

\description{
This command plots the sexual contact network over a certain period of time.
}

\usage{
sti.ca.network(gobackyears=1, largest.component=FALSE, infections=FALSE)
}

\arguments{
\item{gobackyears}{the number of years in the past over which the network is constructed; }
\item{largest.component}{logical, if \code{TRUE}, only the largest component of the entire network is shown; if \code{FALSE} (default), the entire network is shown}
\item{infections}{logical, if \code{TRUE}, infected nodes (individuals) are indicated in red; if \code{FALSE} (default), the infection status is not shown}
}

\details{
This function makes use of the package igraph to plot the sexual contact network over a certain time period. The edges of the network consist of all new contacts that are made during the defined time period (gobackyears) until the current simulation time. The nodes of females and males are indicated in white and gray, respectively. Note that it will be important to define partnershipsolderthan sufficiently long in the configuration file.
}

\seealso{
\code{sti.partnerships}
}

\examples{\dontrun{
# load the chlamydia-model from the package
sti.init("risk.cfg")

# simulate; this may take a while
sti.run(100*365)

# Plot the sexual contact network
sti.ca.network()

# remove model from meory
sti.clear()
}}

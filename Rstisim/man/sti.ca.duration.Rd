\name{sti.ca.duration}
\alias{sti.ca.duration}

\title{Calculate the duration of partnerships and gaps/overlaps}

\description{
This command returns the duration the three most recent partnerships and the gaps/overlaps between them for all individuals.
}

\usage{
sti.ca.duration(gobackyears=100, from=0, to=100)
}

\arguments{
\item{gobackyears}{number of years in the past during which partnerships are reported; }
\item{from, to}{age range of the individuals}
}

\details{
This function performs an artificial survey of all selected individuals at cross-section to obtain the start and end date of their three most recent partnerships. This information is then used to calculate the duration of the partnerhsips and the gaps or overlaps between them. Note that it will be important to define partnershipsolderthan sufficiently long in the configuration file. The recency of partnerships is defined by the date they have ended. If individuals have more than one partnership at cross-section, the order of recency of those partnerships is randomly defined.
}

\value{
A list of class \code{data.frame} with the following elements:
\item{d1}{duration of the most recent partnership}
\item{d2}{duration of the second most recent partnership}
\item{d3}{duration of the third recent partnership}
\item{g1}{duration of the gap/overlap between the most recent and second most recent partnership}
\item{g2}{duration of the gap/overlap between the second most recent and third most recent partnership}
Overlaps between partnerships result in a negative gap duration. NA is returned if there are no partnerships to calculate the duration.
}

\seealso{
\code{sti.people}, \code{sti.partnerships}
}

\examples{\dontrun{
# load the chlamydia-model from the package
sti.init("chlamydia.cfg")

# simulate; this may take a while
sti.run(100*365)

# get the duration of partnerships and gaps/overlaps
data <- sti.ca.duration(gobackyears=100, from=16, to=44)

# histogram of the duration of gaps/overlaps between the most recent and second most recent partnership
hist(data$g1)

# remove model from meory
sti.clear()
}}

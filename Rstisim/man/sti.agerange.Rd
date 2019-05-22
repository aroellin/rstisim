\name{sti.agerange}
\alias{sti.agerange}

\title{The minimum and maximum value for the ages that can occur in the
population}

\description{
This function returns the minimum and maximum value for the age that people can
have. A non-zero minimal age is possible through the 'birthshift' feature which
makes it possible to insert people at a certain age. For sexually transmitted
diseases, for example, it is not necessary to simulate children until they are,
say, 12 years old. 
}

\usage{
sti.agerange(inyears=TRUE)
}

\arguments{
\item{inyears}{logical; if \code{TRUE} (default) the age range is divided by 365
and thus the returned values are in years; otherwise in days}
}

\value{
A vector of size two with the minimal and maximal age that can occur.
}
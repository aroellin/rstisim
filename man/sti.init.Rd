\name{sti.init}
\alias{sti.init}

\title{Initialise model from configuration file}

\description{
This is the main function to load a configuration file and initialise the model.
It reads the configuration file given by \code{filename} which should describe
an \code{Rstisim} model as described in \code{sti.config} and initialises the
model so that it is ready to run. If a configuration is already loaded, you can
also use \code{sti.reinit} instead.
}

\usage{
sti.init(filename, verbose=TRUE, executecode=TRUE)
}

\arguments{
\item{filename}{character string; the filename in quotation marks pointing to
the configuration file; if not found, the file is searched in the subdirectory
\code{Rstisim} in the standard R directories returned by the function
\code{.libPaths()}}
\item{verbose}{logical; to suppress output about the initialisation set to FALSE}
\item{executecode}{logical; if \code{TRUE} (default), execute any code after the 
"#EXECUTE" statement in the configuration file; the code is executed after the 
model is initialised and in the environment returned by \code{globalenv()}}
}

\details{
After the configuration file has been loaded and converted into a nested list
and after some preprocessing has been done, this list is then passed to the C++
code that installs the required objects into the memory. Detailed output is
provided at each stage. COMPARE THIS OUTPUT CAREFULLY IT WITH YOUR CONFIGURATION
FILE! If an error occures it should be possible to locate the error or at
least the part in the configuration file where the error occured. 

When calling this function, two main sources of error are possible: either a
syntax error in the configuration file which is thrown by the configuration file
parser (for example a missing semicolon), or an error produced by a conceptual
error (for example a negative lifetime distribution). The latter type of error
is thrown on the the C++-code level when the model is built up in the memory,
after the configuration file has been correctly read. If an error occures it may
be necessary to call \code{sti.clear()} first before trying to load the
corrected configuration file again.

If no error has occured, a check is performed to look for unused elements in
the configuration and, if such are found, a corresponding warning (``unused
elements in configuration file detected'') is given for each element. If this
happens, make sure you either know exactly what you are doing or otherwise check
the spelling of the elements as most probably you missspelled the element and
the model uses a default value instead.

The function \code{sti.init} may through a warning about some objects being kept
in memory. By default, all the objects (people, partnerships, infections) once
generated are being kept in the memory and this warning (``keeping all ... in
memory'') is hence thrown by default. However, some models will quickly fill the
whole memory and it will be necessary to delete old objects from time to time in
this case; see the documentation for the \code{simulation.remove} elements of
the configuration file.
}

\value{ 
A vector as returned by \code{sti.stats()}.
}

\seealso{
\code{sti.run}, \code{sti.execute}, \code{sti.clear}.
}


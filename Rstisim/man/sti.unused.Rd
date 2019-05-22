\name{sti.unused}
\alias{sti.unused}

\title{Print the unused elements from the configuration file}

\description{
Every element in the configuration file is tagged by a 'used' flag. If an
element is accessed during model initialisation this tag is set from FALSE to
TRUE. The function 'sti.showunused' lists all the elements where this flag is
still set to FALSE and can therefore be used to detect typos in the
configuration file. If the initialisation procedure of the model does not find
a specific element it often assigns a default value instead. Use the function
'sti.showunused' to detect this kind of errors (it is automatically called by
\code{sti.init})
}

\usage{
sti.unused()
}

\seealso{
\code{sti.init}
}


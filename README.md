# RSTISim
Agent-based STI simulator for R

Rstisim (R Sexually Transmitted Infection Simulator) is an individual-based model simulation software written in C++ and is fully integrated into the statistical software package R (www.r-project.org). Its main purpose is to simulate the dynamics of sexually transmitted infections (STI), which require the formation of complex dynamic partnership networks.

The software is event-based and continuous time with an event-queue at its heart. The different (simplified) conceptual layers that Rstisim currently supports are:

Agents (birth/death/immigration),
Partnerships (formation/dissolution, any number of concurrent partnerships possible),
Sexual contacts,
Infections (transmission during sexual contact, multiple infections possible),
Intervention (e.g. screening of asymptomatics, testing and treatment of symptomatics),
Partner notification / contact tracing (current/previous partners).
Models are defined through configuration files, which are read and interpreted by the R part of the package. This initialises the C++ code, which then does the actual simulation. The user can retrieve various data frames, describing current individual states of the agents, partnerships, infections, etc. The basic data frames are detailed enough to reconstruct more complex statistics, such as detailed partnership networks and partner notification cascades. All the data frames are stored in native R formats so one can make full use of all the built-in functionality and extensions of R, which simplifies and speeds up the analysis.

The code is stable for our current model implementations, but we consider it to be beta state. You can download it from here and use it at your own risk. A manual for the R interface and the syntax of the configuration file is included in the package. Otherwise trial-and-error is your friend.

The package is currently developed and maintained by Christian Althaus (University of Bern) and Adrian Roellin (National University of Singapore). If you have any question please contact either of us.

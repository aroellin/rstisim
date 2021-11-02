# Load the necessary libraries
library(Rstisim)
library(igraph)

# Initialize the model
sti.init("risk.cfg")

# Set the mixing coefficient
sti.ca.mixing(epsilon = 0.5)

# Run a simulation for 10 years
sti.run(10*365)

# Plot the sexual contact network of the past year
sti.ca.network(gobackyears = 1, largest.component = FALSE, infection = FALSE)

# Plot the largest transmission tree over the last 10 years
sti.ca.tree(gobackyears = 10, largest.component = TRUE)

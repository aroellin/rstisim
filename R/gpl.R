# Function to calculate the partnership duration and the length of gaps/overlaps
# Parameters:
# after :   only consider partnerships that begin after this time (default is 0 years)
# current:  time where people are "asked" about their partnerships (default is 100 years)

sti.gpl <- function(after=0*365, current=100*365)
{
	# the current people
	pe <- sti.people()
    # the old and current partnerships
    ps <- rbind(sti.partnerships(old=T),sti.partnerships())
    # select partnerships where begin > after
    ps <- ps[(ps$begin>after),]
    # selection parternships of people that according to the Natsal-2 survey
    puid <- pe$puid[(current-pe$birth)>(16*365) & (current-pe$birth)<(45*365) & pe$partnerstotal > 0]
    tab <- table(puid)
    puid <- as.integer(names(which(tab>0)))
    ps <- ps[(ps$puid1 %in% puid) | (ps$puid2 %in% puid),]

    data <- data.frame(puid=puid)
    data$d1 <-0 # this will store the duration of the most recent partnership
    data$d2 <-0 # this will store the duration of the 2nd most recent partnership
    data$d3 <-0 # this will store the duration of the 3rd most recent partnership
    data$g1 <-0 # this will store the length of the most recent gap/overlap
    data$g2 <-0 # this will store the length of the 2nd most recent gap/overlap

    # Loop over all the people
    for (i in 1:length(puid)) {
        # get the list of partnership for this person
        thisps <- ps[ps$puid1==puid[i] | ps$puid2==puid[i],]

        # make an array that stores the end (and begin) time of all partnerships
        # make a second array that stores the partnership identifier (psuid)
        end <- thisps$end
        begin <- thisps$begin
        id <- thisps$psuid
        
        # now we have to sort the most recent partnerships
        # get the permutation that starts with the partnership that ends latest as first (-> most recent)
        perm <- order(end,decreasing=TRUE)
        # permute the time points and psuid's of all arrays
        end <- end[perm]
        begin <- begin[perm]
        id <- id[perm]
        
        # the number of recorded parterships of a given person
		p<-length(id)
		
		# if the person just has had one partnership
		if(p==1) data$d1[i] <- min(current,end[1]) - begin[1]
		
		# if the person just has had two partnerships
		if(p==2) {
			# randomly shuffle the partnerships that are ongoing
			o <- 1
			if(end[2] > current) o <- 2
			perm <- order(end,decreasing=TRUE)
			perm[1:o] <- sample(o)							
			end <- end[perm]
        	begin <- begin[perm]
        	id <- id[perm]
        	# calculate the partnerships and gaps
			data$d1[i] <- min(current,end[1]) - begin[1]
			data$d2[i] <- min(current,end[2]) - begin[2]
			data$g1[i] <- max(begin[1],begin[2]) - min(current,end[2])
		}
		
		# if the person has had more than two partnerships
		if(p>2) {
			# randomly shuffle the partnerships that are ongoing
			o <- 1
			if(end[2] > current) o <- 2
			if(end[3] > current) o <- 3
			perm <- order(end,decreasing=TRUE)
			perm[1:o] <- sample(o)							
			end <- end[perm]
        	begin <- begin[perm]
        	id <- id[perm]
        	# calculate the partnerships and gaps
			data$d1[i] <- min(current,end[1]) - begin[1]
			data$d2[i] <- min(current,end[2]) - begin[2]
			data$d3[i] <- min(current,end[3]) - begin[3]
			data$g1[i] <- max(begin[1],begin[2]) - min(current,end[2])
			data$g2[i] <- max(begin[2],begin[3]) - min(current,end[3])
		}
   }		
   return(data)
}

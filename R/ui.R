.sti.env <- new.env()

.sti.checkifrunning <- function()
{
    if (!exists("isrunning",.sti.env)||!get("isrunning",.sti.env)) {
        stop("STI model has not been initialised")
    }
}

sti.clear <- function() 
{
    .sti.checkifrunning()
    .Call("rif_removeModel")
    assign("isrunning",FALSE, .sti.env)
}

sti.stats <- function()
{
    .sti.checkifrunning()

    stats <- .Call("rif_getStatistics")
    labels <- c(
    	"time", "population_size", "events", 
    	"births", "deaths", "immigrations", "emigrations", 
    	"person_bin_changes", "ps_bin_changes", "inf_bin_changes",
        "new_partnerships", "ended partnerships", 
        "contacts", "unprotected_contacts", "pregnancies", 
        "infections", "clearances", 
        "tests", "treatments", "treatmentsvain",
        "followupvisits")
    names(stats) <- labels
    return(stats)
}

sti.rtest <- function(n, atleast=NULL)
{
    .sti.checkifrunning()
    return(.Call("rif_testDistribution",as.integer(n),atleast));
}

sti.agerange <- function(inyears=TRUE)
{
    .sti.checkifrunning()
    ans <- .Call("rif_getAgeRange")
    if (inyears) {
        ans <- ans/365   
    }
    names(ans) = c("minimum","maximum")
    return(ans)
}

sti.run <- function(time, verbose=TRUE, clearstats=TRUE) 
{
    .sti.checkifrunning()
    if (time <= 0) {
        stop("argument must be positive amount of time");
    }

    start = Sys.time()

    .Call("rif_advanceBy",as.numeric(time))

    stats <- sti.stats();

    timediff = as.double(Sys.time()-start, units="secs")

    ans <- list(relative=stats-get("laststats",.sti.env),total=stats, simulationtime=timediff);
    if (clearstats) {
        assign("laststats",stats,.sti.env)
    }

    if (timediff > 20) {
        timediff = round(timediff)
    } else {
        timediff = round(timediff, 5)
    }
    if (as.logical(verbose)) { message("Time elapsed: ", timediff, " secs") }
    return(ans)
}

sti.execute <- function(number=1, verbose=TRUE, clearstats=FALSE)
{
    .sti.checkifrunning()
    if (number <= 0) {
        stop("argument must be positive number");
    }
    .Call("rif_executeN",as.integer(number), as.integer(verbose))

    stats <- sti.stats();

    ans <- list(relative=stats-get("laststats",.sti.env),total=stats);

    if(clearstats) {
        assign("laststats",stats,.sti.env)
    }

    return(ans)
}


.sti.unused <- function(l = NULL, ans = character(0))
{
    if (is.null(l))
        l <- get("config",.sti.env)   
    if (is.list(l) && length(l)>0) {
        nl <- names(l)
        for (i in 1:length(l)) {
            if (!(nl[i] %in% c("uniformdistribution","constantdistribution","eval","evaluate","randcore"))) {
#                 message("exploring ", nl[i])                
                ans <- .sti.unused(l[[i]], ans)   
            }
        }
    } else {
        if (!is.null(attr(l,"used")) && !attr(l,"used")) {
            path <- attr(l,"path")
            sourceline <- attr(l,"sourceline")
            if (!is.null(path) && !is.null(sourceline)) {
                ans <- rbind(ans, c(path, sourceline))
            }
        } else if (is.null(attr(l,"used"))) {
            warning("element does not have 'used' attribute: ", l);
        }

    }
    return(ans)
}

sti.unused <- function()
{
   .sti.checkifrunning()

    ans <- .sti.unused()
    if (length(ans)>0) {
        ans <- data.frame(path=ans[,1], sourceline=as.integer(ans[,2]),
            stringsAsFactors=FALSE)
        return(ans)
    } else {
        return(NULL)
    }
}


.sti.init <- function(verbose=TRUE, executecode=TRUE)
{
    assign("isrunning", TRUE, .sti.env)

    .Call("rif_initModel",get("config",.sti.env), !verbose);
    .Call("rif_populate")
    uu <- sti.unused()
    if (!is.null(uu)) {
        warning("unused elements in configuration file detected")
        message("WARNING: unused elements in configuration file detected")
        print(uu)
    }
    assign("laststats",sti.stats(),.sti.env);

    if (executecode) {
    	code = get("code",.sti.env)
        if (verbose & (code!="#NOCODE")) message("Executing configuration script")
    	code = gsub("\r","",code)
	    code = parse(text=code)
        eval(code, envir=globalenv())
    }
}

sti.init <- function(filename, verbose=TRUE, executecode=TRUE)
{
    if (exists("isrunning",.sti.env) && get("isrunning",.sti.env))  {
        stop(
        "STI model already initialised; use sti.clear to remove current model")
    }

    filename <- c(filename,paste(.libPaths(),filename,sep="/Rstisim/"))
    ans <- ""
    anslist <- character(0)
    for (fn in filename) {
        f <- file(fn)           
        warn = options(warn=-1)
        ans <- try(open(f,"rb"), silent=TRUE)
        options(warn)
        anslist <- append(anslist,ans[1])
        if (is.null(ans)) break;
        close(f)
    }

    if (!is.null(ans)) {
        stop(anslist[1]);
    }

    assign("config", .sti.configloader(f), .sti.env)

    close(f)
    
    f = file(fn)
    open(f,"rb"); code = readChar(f,1000000); close(f)
    pos = regexpr("#EXECUTE",code)
    if(pos != -1) {
        code = substr(code,pos,1000000)
        assign("code", code, .sti.env)
    } else {
        assign("code", "#NOCODE", .sti.env)
    }

    .sti.init(verbose, executecode)

    return(get("laststats",.sti.env))
}

sti.reinit <- function(verbose=TRUE, executecode=TRUE)
{
    if (exists("isrunning",.sti.env) && get("isrunning",.sti.env))  {
        stop(
        "STI model already initialised; use sti.clear to remove current model")
    }

    if (exists("config", envir=.sti.env) && 
        inherits(get("config",.sti.env),"sti.configuration")) {
        .sti.init(verbose,executecode);
    } else {
        stop("model has not been initialized from file before")
    }
    return(get("laststats",.sti.env))
}

sti.config <- function()
{
    .sti.checkifrunning()
    return(get("config",.sti.env))
}

.sti.printelement <- function(value)
{
    if(typeof(value)=="character") {
    	message("value\t\t\"",value,"\"")
    } else {	
    	message("value:\t\t",as.numeric(value))
    }
    message("type:\t\t",class(value))
    a <- attr(value, "sourceline")
    if (!is.null(a)) {
        message("sourceline:\t",a)
    }
    a <- attr(value,"path")
    if (!is.null(a)) {
        message("path:\t\t",a)
    }
    a <- attr(value,"randcore")
    if (!is.null(a)) {
        message("randcore:\t",a)
    }
    a <- attr(value,"used")
    if (!is.null(a)) {
        message("used:\t\t",a)
    }
}

sti.set <- function(path, value = NULL, verbose=TRUE)
{
    if(!exists("config",.sti.env)) {
        stop("no coniguration loaded")
    }

    path <- chartr(".","$",path)

    v <- eval(parse(text=paste("config",path,sep="$")),.sti.env) 

    if (is.null(v)) {
        stop("element not found in configuration")
    }
    if (is.list(v)) {
    	if(is.null(value)) {
	    return(names(v))
	} else {
	    stop("lists cannot be changed only single values")
	}
    }
    if(!(typeof(v) %in% c("integer","double","character"))) {
        stop("cannot manipulate elements of type '",typeof(v),"'")
    }

    if (!is.null(value)) {
        if (exists("isrunning",.sti.env) && get("isrunning",.sti.env))  {
        	stop(
        "STI model already initialised; use sti.clear to remove current model")
    	}

        if(!(typeof(v) %in% c("integer","double","character"))) {
            stop("'value' argument must be of type 'double', 'integer' or 'character'")
        }
        if (typeof(v) == "integer") {
            if (value != floor(as.numeric(value))) {
            	warning(
            	"type of element is integer, but new value contains fractional part")
            }
            value <- as.integer(value)
        } else if (typeof(v) == "double") {
            value <- as.numeric(value)
        } else {
            value <- as.character(value)
        }
        attributes(value) = attributes(v)
        assign("newvalue",value,.sti.env)
        eval(parse(text=paste(c("config$", path, "<-newvalue"),collapse="")),.sti.env)
    }
    if (verbose) {
        message("element\t\t'config$",path,"'")
        v <- eval(parse(text=paste("config",path,sep="$")),.sti.env) 
        .sti.printelement(v)
    } else {
     return(v)
    }
}


sti.people <- function(
    current.people=TRUE,
    relevel=TRUE,
		breaks=seq(floor(sti.agerange()[1]/5)*5,
							ceiling(min(sti.agerange()[2]/5,10000))*5,5),
    right=FALSE,
    include.lowest=TRUE)
{
    .sti.checkifrunning()

    if (length(breaks)>1000) {
        breaks=c(0,Inf)
    }

    d = .Call("rif_getPeople", as.integer(!current.people))

    for (i in 1:length(d)) {
        if (!is.null(attr(d[[i]],"levels"))) {
            if (length(d[[1]])>0) {
                d[[i]] <- d[[i]] + as.integer(1)
                d[[i]][d[[i]]==0] = NA
            }
            class(d[[i]]) <- "factor"
        }
    }

    names(d) <- c("puid","adhocid","type","bin","birth","death",
        "partnerscurrent", "partnerstotal", "partnerswithin", "contacts", "contactsunprot",
        "pregnant", "pregnancies", "abortions", "children", 
        "infectionscurrent", "infectionstotal", "infectionswithin", "treatments", "gpvisits", "notifications")
    d <- as.data.frame(d)
    if (relevel && length(d[[1]])>0) {
        for (i in 1:length(d)) {
            if (is.factor(d[[i]]))
                d[[i]] <- factor(levels(d[[i]])[unclass(d[[i]])],levels=unique(levels(d[[i]])))
        }
    }

    if (current.people) {
        d$age <- (sti.stats()["time"]-d$birth)/365
        d$agegroup <- cut(d$age,breaks=breaks,right=right,include.lowest=include.lowest)    
        rownames(d) <- d$adhocid
    } else {
        if (nrow(d)>0) {
          d$age <- NA
          d$agegroup <- factor("dead")
          d$partnerscurrent <- NA
          d$partnerswithin <- NA        
          d$infectionscurrent <- NA
          d$infectionswithin <- NA
        }
    }
    # d$adhocid <- NULL
    return(d)
}

sti.partnerships <- function(old.partnerships=FALSE, relevel=TRUE) 
{
    .sti.checkifrunning()

    d <- .Call("rif_getPartnerships", as.integer(old.partnerships))

    for (i in 1:length(d)) {
        if (!is.null(attr(d[[i]],"levels"))) {
            if (length(d[[i]])>0) {
                d[[i]] = d[[i]] + as.integer(1)
                d[[i]][d[[i]]==0] <- NA
            }
            class(d[[i]]) <- "factor"
        }
    }
    names(d) <- c(
        "psuid", "type", "bin", "begin", "end", 
        "puid1", "adhocid1", "puid2", "adhocid2", 
        "formertype", "fitness", "tries", 
        "contacts", "contactsunprot")
    d <- as.data.frame(d)
    if (relevel && length(d[[1]])>0) {
        for (i in 1:length(d)) {
            if (is.factor(d[[i]]))
                d[[i]] = factor(levels(d[[i]])[unclass(d[[i]])],levels=unique(levels(d[[i]])))
        }
    }

    return(d)
}

sti.infections <- function(
    old.infections=FALSE, relevel=TRUE, current.people=TRUE) 
{
    .sti.checkifrunning()

    d <- .Call("rif_getInfections", 
        as.integer(old.infections), as.integer(!current.people))

    
    for (i in 1:length(d)) {
        if (!is.null(attr(d[[i]],"levels"))) {
            if (length(d[[i]])) {
                d[[i]] = d[[i]] + as.integer(1)
                d[[i]][d[[i]]==0] = NA
            }
            class(d[[i]]) <- "factor"
        }
    }
    names(d) <- c(
        "infuid", "strainid", "type", "bin", 
        "begin", "endofinfection", "endofimmunity", 
        "parentinfuid", "puid", "adhocid", "psuid"
        )
    d <- as.data.frame(d)
    if (relevel && length(d[[1]])>0) {
        for (i in 1:length(d)) {
            if (is.factor(d[[i]]))
                d[[i]] <- factor(levels(d[[i]])[unclass(d[[i]])],levels=unique(levels(d[[i]])))
        }
    }
     
    return(d)
}

.sti.list2df <- function(d,col.names) 
{
    for (i in 1:length(d)) {
        if (!is.null(attr(d[[i]],"levels"))) {
            if (length(d[[i]])) {
                d[[i]] = d[[i]] + as.integer(1)
                d[[i]][d[[i]]==0] <- NA
            }
            class(d[[i]]) <- "factor"
        }
    }
    names(d) <- col.names
    return(as.data.frame(d))
}

.sti.rbind <- function(d1,d2) {
    d <- list()
    
    for (i in 1:length(d1)) {
        d[[i]] = c(d1[[i]],d2[[i]])
        attributes(d[[i]]) = attributes(d1[[i]])
    }
    return(d)
}


sti.notifications <- function(
    relevel=TRUE, all=TRUE, sortby=c("nuid","none","time")) 
{
    .sti.checkifrunning()

    d <- .Call("rif_getNotifications", as.integer(FALSE))

    if (all) {
        d2 <- .Call("rif_getNotifications", as.integer(TRUE))
        
        d <- .sti.rbind(d,d2)        
    }

    d <- .sti.list2df(d, c("ntype", "nuid", "linknumber", "time", 
                            "puid1", "ptype1", "pbin1", 
                            "puid2", "ptype2", "pbin2",
                            "psuid", "pstype", "psbin"))
    
    if (relevel && length(d[[1]])>0) {
        for (i in 1:length(d)) {
            if (is.factor(d[[i]]))
                d[[i]] <- factor(levels(d[[i]])[unclass(d[[i]])],levels=unique(levels(d[[i]])))
        }
    }
     
    sortby <- match.arg(sortby)
    switch(sortby,
        none = {},
        time = { d = d[sort(d$time,index=TRUE)$ix,] },
        nuid = { d = d[sort(d$linknumber,index=TRUE)$ix,];
                d = d[sort(d$nuid,index=TRUE)$ix,]
         }
     )
    return(d)
}



sti.gpvisits <- function(
    relevel=TRUE, all=TRUE, sortby=c("time","none","nuid")) 
{
    .sti.checkifrunning()

    d <- .Call("rif_getGPVisits", as.integer(FALSE))

    if (all) {
        d2 <- .Call("rif_getGPVisits", as.integer(TRUE))
        
        d = .sti.rbind(d,d2)        
    }

    d <- .sti.list2df(d, c("gpvtype", "time", "cause", 
                           "puid", "ptype", "pbin",
                            "linknumber", "nuid", 
                           "dirtreated", "tested", "posresults","ntype"
                           ))
    
    if (relevel && length(d[[1]])>0) {
        for (i in 1:length(d)) {
            if (is.factor(d[[i]]))
                d[[i]] <- factor(levels(d[[i]])[unclass(d[[i]])],levels=unique(levels(d[[i]])))
        }
    }

    # d$nuid[d$nuid==0] <- NA # not used anymore as every gpvisits creates a new nuid
     
    sortby <- match.arg(sortby)
    switch(sortby,
        time = { d = d[sort(d$time,index=TRUE)$ix,] },
        none = {},
        nuid = { d = d[sort(d$linknumber,index=TRUE)$ix,];
                d = d[sort(d$nuid,index=TRUE)$ix,]}
     )
    return(d)
}




sti.network <- function(agerange=c(15,20), includepast=0, ...)
{
    require(network)
    if (!exists("isrunning",.sti.env)||!get("isrunning",.sti.env)) {
        stop("STI model has not been initialised")
    }
    
    people <- sti.people(...)
    people$intnum <- 1:nrow(people)
    
    for (i in 1:ncol(people)) {
      people[[i]] <- c(unclass(people[[i]]))
    }
    
    w <- which(people$age>=agerange[1] & people$age<=agerange[2] &
                people$partnerstotal>0)
    
    people$sort <- NA
    people$sort[w] <- 1:length(w);

    peoplenum <- as.numeric(length(w))

    n <- network::network.initialize(peoplenum);

    edgeset = rbind(sti.partnerships(),sti.partnerships(old.partnerships=TRUE))
    
    edgeset <- edgeset[is.element(edgeset$adhocid1,people$adhocid[w]) &
                       is.element(edgeset$adhocid2,people$adhocid[w]),]

    edgeset <- edgeset[edgeset$end-sti.stats()["time"]+includepast>0,]
    
    n[cbind(people$sort[edgeset$adhocid1],people$sort[edgeset$adhocid2])] <- 1
  
    for (i in 3:ncol(edgeset)) {    
        network::set.edge.attribute(n, names(edgeset)[i], c(unclass(edgeset[[i]])))
    }

    people$sort <- NULL
    people$intnum <- NULL
    n <- network::network(n, vertex.attr=people[w,])

    return(n)
}

sti.mixing <- function(x, data=NULL, subset=TRUE, partnerships=sti.partnerships(), symmetric=TRUE)
{
    x = latticeParseFormula(x, data=data, subset=subset)

    if (is.null(x$right)) {
        stop("missing right hand side of formula")
    }

    if (is.null(x$left)) {
        x$left=gl(1,length(x$right))
    }

    if (!is.null(x$condition)) {
        stop("conditioning not supported")
    }

    if (!is.factor(x$left)) {
        # warning("'",x$left.name,"' is not a factor")
        x$left <- factor(as.integer(x$left))
        levels(x$left) <- paste(x$left.name,levels(x$left),sep="=")
    }

    ans = list()

    isnumby = FALSE
    if (!is.factor(x$right)) {
        warning("'",x$right.name,"' is not a factor")
        x$right.num = floor(x$right)
        x$right <- factor(x$right.num)
        isnumby = TRUE
    }

    for (from in 1:nlevels(x$left)) 
    for (to   in 1:nlevels(x$left)) {

        ps <- partnerships

        if (symmetric) {
            p1 <- c(ps$adhocid1,ps$adhocid2)
            p2 <- c(ps$adhocid2,ps$adhocid1)
        } else {
            p1 <- ps$adhocid1
            p2 <- ps$adhocid2
        }

        sat1 = x$left[p1]==levels(x$left)[from]
        sat2 = x$left[p2]==levels(x$left)[to]
        sat = sat1 & sat2

        tab = table(x$right[p1[sat]],x$right[p2[sat]],dnn=c(levels(x$left)[from],levels(x$left)[to]))

        attr(tab,"left.name") = x$left.name
        attr(tab,"left.levels") = levels(x$left)[c(from,to)]
        attr(tab,"right.name") = x$right.name
        if (isnumby) {
            attr(tab,"right.levels") = sort(unique(x$right.num))
        } else {
            attr(tab,"right.levels") = 1:nlevels(x$right)
        }
        ans[[paste(levels(x$left)[from],levels(x$left)[to],sep=":")]] = tab
    }

    class(ans) = c("sti.mix", class(ans))
    return(ans)
}


plot.sti.mix <- function(mix,which=2,...)
{
    if (!inherits(mix, "sti.mix"))
        stop("argument is not of class 'sti.mix'");

    mix = mix[[which]]

    image(attr(mix,"right.levels"),attr(mix,"right.levels"),as.matrix(mix),
        xlab=attr(mix,"left.levels")[1], ylab=attr(mix,"left.levels")[2],
        main=paste(c("mixing pattern of",attr(mix,"right.name"),"by", attr(mix,"left.name")),collapse=" "),
        col=gray(seq(1,0,l=255))
    ) 
}


sti.dist <- function(x, data=NULL, subset=TRUE, relative=FALSE)
{
    require(lattice)
    x <- lattice::latticeParseFormula(x, data=data, subset=subset)
    if (is.null(x$right)) {
        stop("missing right hand side of formula")
    }

    if (is.null(x$left)) {
        x$left <- gl(1,length(x$right))
        x$left.name <- ""
    }

    if (is.null(x$condition)) {
        x$condition <- list(condition=gl(1,length(x$right)))
    }

    if (!is.factor(x$left)) {
        # warning("'",x$left.name,"' is not a factor")
        x$left <- factor(as.integer(x$left))
        levels(x$left) <- paste(x$left.name,levels(x$left),sep="=")
    }
    if (!is.factor(x$right)) {
        # warning("'",x$right.name,"' is not a factor")
        x$right <- factor(floor(x$right))
    }
    condnames = names(x$condition)
    for (i in 1:length(condnames)) {
        if (!is.factor(x$condition[[i]])) {
            warning("'",condnames[i],"' is not a factor")
            x$condition[[i]] <- factor(as.integer(x$condition[[i]]))
            levels(x$condition[[i]]) <- paste(condnames[i],levels(x$condition[[i]]),sep="=")
        }  
    }

    ans = list()

    for (i in 1:length(condnames)) {
        dist <- table(x$left, x$right, x$condition[[i]])

        if(relative) {
            for(j in 1:nlevels(x$condition[[i]])) {
                cs = colSums(dist[,,j])
                cs[cs==0] = 1
                dist[,,j] = t(t(dist[,,j]) / cs)
            }
        } 

        attr(dist, "dimtitles") = c(x$left.name, x$right.name, condnames[i])
        ans[[condnames[i]]] = dist
    }

    class(ans) <- c("sti.dist",class(ans))
    return(ans)
}

plot.sti.dist <- function(dist, which=1, legend=FALSE, ...)
{
    if (!inherits(dist,"sti.dist"))
        stop("argument is not of class 'sti.dist'");

    dist = dist[[which]]
    dimtitles = attr(dist,"dimtitles")

    old <- par(mfrow=c(dim(dist)[3],1))       

    for (j in 1:dim(dist)[3]) {
        if (legend) {
            barplot(dist[,,j],
                main=paste(c(dimtitles[1],"distribution of",dimnames(dist)[[3]][j]),collapse=" "),
                legend.text=dimnames(dist)[[1]],
                xlab=dimtitles[2], ylab="number of individuals", ...
            )
        } else {
            barplot(dist[,,j],
                main=paste(c(dimtitles[1],"distribution of",dimnames(dist)[[3]][j]),collapse=" "),
                xlab=dimtitles[2], ylab="number of individuals", ...
            )
        }
    }
    par(old)
}

sti.prevalence <- function(
    people = NULL, 
    infections = sti.infections(), 
    subset=TRUE, 
    bybin=FALSE, 
    exclude=c("latent","cleared","treated"), ...)
{
    if (is.null(people)) {
        people = sti.people(...)   
    }
    ans = list()
    agedist = table(people$agegroup[subset])

    for (disease in levels(infections$type)) {
        if (bybin) {
          ans[[disease]] = matrix(NA,nlevels(people$bin)+1,length(agedist))
          overallidx = nlevels(people$bin)+1;
          colnames(ans[[disease]]) = names(agedist)
          rownames(ans[[disease]]) = c(levels(people$bin),"(overall)")
          for (bin in levels(people$bin)) {
              w = which(people$bin==bin & subset)
              agedistbin = table(people$agegroup[w])
              w = intersect(w, infections$adhocid[infections$type==disease & !(infections$bin %in% exclude)])
              ans[[disease]][bin,] = table(people$agegroup[w])/agedistbin
              ans[[disease]][bin,agedistbin==0] = NA
          }
          w = which(rep(TRUE,nrow(people)) & subset)
          w = intersect(w, infections$adhocid[infections$type==disease & !(infections$bin %in% exclude)])
          ans[[disease]][overallidx,] = table(people$agegroup[w])/agedist
          ans[[disease]][overallidx,agedist==0] = NA
        } else {
          w = which(rep(TRUE,nrow(people)) & subset)
          w = intersect(w, infections$adhocid[infections$type==disease & !(infections$bin %in% exclude)])
          ans[[disease]] = table(people$agegroup[w])/agedist
          ans[[disease]][agedist==0] = NA
        }
    }
    return(ans)
}


sti.infectiontree <- function()
{
    require(network)
    inf <- rbind(sti.infections(),
                 sti.infections(old.infections=TRUE),
                 sti.infections(current.people=FALSE))
                 
    inf <- inf[!is.na(inf$parentinfuid),]

    adhocidlist <- unique(c(inf$infuid,inf$parentinfuid))

    adhocid <- 1:length(adhocidlist)
    names(adhocid) <- adhocidlist

    inf$infuid <- as.character(inf$infuid)
    inf$parentinfuid <- as.character(inf$parentinfuid)

    n <- network::network.initialize(length(adhocid))

    edgelist <- na.omit(cbind(adhocid[inf$parentinfuid],adhocid[inf$infuid]))
    attr(edgelist,"na.action") <- NULL
    n[edgelist] <- 1
    return(n)
}


sti.scheduler <- function()
{
    .sti.checkifrunning()
    ans <- .Call("rif_getSchedulerSizes");
    names(ans) = c("maximal queue size","events in queue","active events in queue")
    return(ans)
}

sti.events <- function(number=1, include.text=FALSE, active.only=TRUE)
{
    .sti.checkifrunning()
    d <- .Call("rif_getEventQueue", 
        as.integer(number), as.integer(include.text), as.integer(active.only))

    for (i in 1:length(d)) {
        if (!is.null(attr(d[[i]],"levels"))) {
            if (length(d[[1]])>0) {
                # d[[i]] <- d[[i]] + as.integer(1)
                d[[i]][d[[i]]==0] = NA
            }
            class(d[[i]]) <- "factor"
        }
    }
    names(d) <- c("time","type","active", "text")
    d <- as.data.frame(d, stringsAsFactors=FALSE)
    return(d)
}

.sti.randcores <- function(l, ans=character(0)) { 
    path <- attr(l,"path")
    randcore <- attr(l,"randcore")
    if (!is.null(path) && !is.null(randcore)) {
        ans <- rbind(ans, c(path,randcore))
    }
    if (is.list(l) && length(l)>0) {
        for (i in 1:length(l)) 
            ans <- .sti.randcores(l[[i]], ans)
    }
    return(ans)
}

sti.randcores <- function() {
    .sti.checkifrunning()
    ans <- .sti.randcores(sti.config())
    ans <- data.frame(path=ans[,1],randcore=as.integer(ans[,2]),
        stringsAsFactors=FALSE)
    return(ans)    
}

sti.seed <- function()
{
    .sti.checkifrunning()
    return(.Call("rif_getSeed"));
}


# Function to calculate the partnership duration and the length of gaps/overlaps
# Parameters:
# after :   only consider partnerships that begin after this time (default is 0 years)
# current:  time where people are "asked" about their partnerships (default is 100 years)

sti.gpl <- function(after=0*365, current=100*365)
{
	# the current people
	pe <- sti.people()
    # the old and current partnerships
    ps <- rbind(sti.partnerships(old.partnerships=T),sti.partnerships())
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



sti.ca.duration <- function(gobackyears = 100, from = 0, to = 100) {
	# the current time
    timenow <- sti.stats()["time"]	
	# the current people
	pe <- sti.people()
    # the old and current partnerships
    ps <- rbind(sti.partnerships(old.partnerships=T),sti.partnerships())
    # select partnerships during gobackyears
    ps <- ps[(ps$begin>(timenow - gobackyears*365)) & (ps$begin<timenow),]
    # selection parternships of people according to age
    puid <- pe$puid[(timenow-pe$birth)>(from*365) & (timenow-pe$birth)<((to+1)*365) & pe$partnerstotal > 0]
    tab <- table(puid)
    puid <- as.integer(names(which(tab>0)))
    ps <- ps[(ps$puid1 %in% puid) | (ps$puid2 %in% puid),]

    data <- data.frame(puid=puid)
    data$d1 <- NA # this will store the duration of the most recent partnership
    data$d2 <- NA # this will store the duration of the 2nd most recent partnership
    data$d3 <- NA # this will store the duration of the 3rd most recent partnership
    data$g1 <- NA # this will store the length of the most recent gap/overlap
    data$g2 <- NA # this will store the length of the 2nd most recent gap/overlap

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
		if(p==1) data$d1[i] <- min(timenow,end[1]) - begin[1]
		
		# if the person just has had two partnerships
		if(p==2) {
			# randomly shuffle the partnerships that are ongoing
			o <- 1
			if(end[2] > timenow) o <- 2
			perm <- order(end,decreasing=TRUE)
			perm[1:o] <- sample(o)							
			end <- end[perm]
        	begin <- begin[perm]
        	id <- id[perm]
        	# calculate the partnerships and gaps
			data$d1[i] <- min(timenow,end[1]) - begin[1]
			data$d2[i] <- min(timenow,end[2]) - begin[2]
			data$g1[i] <- max(begin[1],begin[2]) - min(timenow,end[2])
		}
		
		# if the person has had more than two partnerships
		if(p>2) {
			# randomly shuffle the partnerships that are ongoing
			o <- 1
			if(end[2] > timenow) o <- 2
			if(end[3] > timenow) o <- 3
			perm <- order(end,decreasing=TRUE)
			perm[1:o] <- sample(o)							
			end <- end[perm]
        	begin <- begin[perm]
        	id <- id[perm]
        	# calculate the partnerships and gaps
			data$d1[i] <- min(timenow,end[1]) - begin[1]
			data$d2[i] <- min(timenow,end[2]) - begin[2]
			data$d3[i] <- min(timenow,end[3]) - begin[3]
			data$g1[i] <- max(begin[1],begin[2]) - min(timenow,end[2])
			data$g2[i] <- max(begin[2],begin[3]) - min(timenow,end[3])
		}
   }		
   return(data)
}

sti.ca.tree <- function(gobackyears = 10, largest.component = TRUE) {
	# requires the igraph package
	require(igraph)
	# define the vertices and edges of the graph
	timenow <- sti.stats()["time"]
	inf <- rbind(sti.infections(),sti.infections(old.infections=TRUE),sti.infections(current.people=FALSE))
	inf <- inf[!is.na(inf$parentinfuid),]
	inf <- inf[inf$begin >= (timenow - gobackyears*365),]
	edges <- cbind(inf$parentinfuid,inf$infuid)
	# create the graph
	g <- igraph::graph.data.frame(edges)
	# plot the graph
	if(largest.component) {
		graphs <- igraph::decompose.graph(g)
		largest <- graphs[[which.max(sapply(graphs, igraph::vcount))]]
		w <- which(degree(largest,mode="in") == 0)
		plot(largest, layout=igraph::layout.reingold.tilford(largest, root = w), vertex.label=NA, vertex.size=2+0.5*degree(largest,mode="out"), edge.arrow.size=0.2, edge.color="black",vertex.color="red")
	} else plot(g, layout=igraph::layout.fruchterman.reingold, vertex.label=NA, vertex.size=2+0.5*degree(g,mode="out"), edge.arrow.size=0.2, edge.color="black",vertex.color="red")
}

.sti.ca.set.color <- function(vertices, infections) {
		col <- NULL
		people <- rbind(sti.people(),sti.people(current.people=FALSE))
		for(i in vertices) {
			if(is.na(people$infectionscurrent[people$puid == i]) & infections) {
				if(people$type[people$puid == i] == "female") col <- c(col,"white")
				else col <- c(col,"gray")
			} else {
				if(people$infectionscurrent[people$puid == i] == 1 & infections) col <- c(col,"red")
				else { 
					if(people$type[people$puid == i] == "female") col <- c(col,"white")
					else col <- c(col,"gray")
				}
			}
		}
		col
	}

sti.ca.network <- function(gobackyears = 1, largest.component = FALSE, infections = FALSE) {
	# requires the  package
	require(igraph)
	# function to define color of nodes
	# define edges and create the graph
	timenow <- sti.stats()["time"]
	partnerships <- rbind(sti.partnerships(),sti.partnerships(old.partnerships=TRUE))
	partnerships <- partnerships[partnerships$end >= (timenow - gobackyears*365) & partnerships$begin < timenow,]
	edges <- cbind(partnerships$puid1,partnerships$puid2)
	vertices <- unique(c(partnerships$puid1,partnerships$puid2))
	g <- igraph::graph.data.frame(edges,directed=FALSE)
	col <- .sti.ca.set.color(vertices,infections)
	# plot the graph
	if(largest.component) {
		graphs <- igraph::decompose.graph(g)
		largest <- graphs[[which.max(sapply(graphs, igraph::vcount))]]
		col <- .sti.ca.set.color(as.numeric(igraph::V(largest)$name),infections)
		plot(largest, layout=igraph::layout.fruchterman.reingold, vertex.label=NA, vertex.size=1.5+0.5*igraph::degree(largest), edge.arrow.size=0.2, edge.color="black",vertex.color=col)
	} else plot(g, layout=igraph::layout.fruchterman.reingold, vertex.label=NA, vertex.size=1.5+0.5*igraph::degree(g), edge.arrow.size=0.2, edge.color="black",vertex.color=col)
}

sti.ca.mixing <- function(epsilon = 0.5) {
	# This function only works for the risk class model (risk.cfg)
	sti.clear()
	# Proportion in low and high risk class
	N <- c(sti.set("model.people.female.bins.distribution",verbose=FALSE)[1],sti.set("model.people.female.bins.distribution",verbose=FALSE)[2])
	# Partner change rates
	c <- c(sti.set("model.partnershipformers.individual.seek.female.low.rate",verbose=FALSE)[1],sti.set("model.partnershipformers.individual.seek.female.high.rate",verbose=FALSE)[1])
	cN <- sum(c*N)
	NCLASS <- 2
	# Mixing matrix for Rstisim
	rho <- matrix(nrow=NCLASS,ncol=NCLASS)
	for(i in 1:NCLASS) {
		for(j in 1:NCLASS) {
			if(i == j) rho[i,j] <- (epsilon + (1.-epsilon)*c[j]*N[j]/cN)/N[j]
			else rho[i,j] <- (1.-epsilon)*c[j]*N[j]/cN/N[j]
		}
	}
	# Normalization
	rho <- rho/max(rho)
	# Setting the values in the model
	sti.set("model.partnershipformers.individual.mixingfactors.fem2malelow.value.low",rho[1,1],verbose=FALSE)
	sti.set("model.partnershipformers.individual.mixingfactors.fem2malelow.value.high",rho[2,1],verbose=FALSE)
	sti.set("model.partnershipformers.individual.mixingfactors.fem2malehigh.value.low",rho[1,2],verbose=FALSE)
	sti.set("model.partnershipformers.individual.mixingfactors.fem2malehigh.value.high",rho[2,2],verbose=FALSE)
	sti.reinit(verbose=FALSE)
}

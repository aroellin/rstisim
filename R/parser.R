
# This code is pretty messy...

.sti.configloader <- function(f) { # f is the filehandler

filestring <- readChar(f, 1000000)

#crop everything beyond the first #EXECUTE statement
pos = regexpr("#EXECUTE",filestring)
if (pos != -1) {
    filestring = substr(filestring, 1, pos-1)
}

#remove comments
filestring <- gsub("(?m)#.*$","",filestring,perl=TRUE)
filestring <- gsub("(?m)//.*$","",filestring,perl=TRUE)
#filestring <- gsub("(?U)/\\*(.*\n?)*\\*/","",filestring,perl=TRUE)
# scan direct y's
filestring <- gsub("([^[:alpha:]])([[:digit:]]*[\\.]?[[:digit:]]+)s","\\1(\\2*1.15740740740741e-05)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]+)\\.s","\\1(\\2*1.15740740740741e-05)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]*[\\.]?[[:digit:]]+)m","\\1(\\2*0.000694444444444444)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]+)\\.m","\\1(\\2*0.000694444444444444)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]*[\\.]?[[:digit:]]+)h","\\1(\\2*0.0416666666666667)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]+)\\.h","\\1(\\2*0.0416666666666667)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]*[\\.]?[[:digit:]]+)d","\\1(\\2*1)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]+)\\.d","\\1(\\2*1)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]*[\\.]?[[:digit:]]+)y","\\1(\\2*365)",filestring)
filestring <- gsub("([^[:alpha:]])([[:digit:]]+)\\.y","\\1(\\2*365)",filestring)
processedstring <- filestring
#remove leading whitespaces
filestring <- gsub("\n[[:blank:]]*","\n",filestring)


filestring <- strsplit(filestring,"")[[1]]
filestringint <- .Call("rif_charToInt",filestring)
sourceline <- 1+cumsum(filestring == "\n")

filestring[filestringint < 32] <- " "
filestringint[filestringint < 32] <- 32

thisenv <- new.env()
assign("stop",2^32-1,thisenv)
assign("never",2^32-1,thisenv)

pos <- 1

getNextChar <- function() {
	pos <<- pos + 1
}

gotoNextNonSpace <- function() {
	while (!is.na(filestringint[pos]) && filestringint[pos] == 32) pos <<- pos+1
}

NAMECHARS <- c(
    .Call("rif_intToChar",48:57),
    .Call("rif_intToChar",65:90),
    .Call("rif_intToChar",97:122),
    "*","_","$")
LETTERCHARS <- c(
    .Call("rif_intToChar",65:90),
    .Call("rif_intToChar",97:122),".")

NUMBERCHARS <- c(.Call("rif_intToChar",48:57),".","s", "m", "h", "d", "y")

getName <- function() {
	gotoNextNonSpace();
    name <- ""
    while (!is.element(filestring[pos],c(" ","=",":",";"))) {
        if (is.element(filestring[pos],NAMECHARS)) {
#		if ((filestring[pos]int >=48 && filestring[pos]int <=57)
#			|| (filestring[pos]int >=65 && filestring[pos]int <=90)
#			|| (filestring[pos]int >=97 && filestring[pos]int <=122)
#			|| is.element(filestring[pos],c("_","$"))) {
            name <- paste(name,filestring[pos],sep="")
        } else {
            stop("config line ",sourceline[pos],": invalid name")
        }
        getNextChar()
    }
	gotoNextNonSpace();
	if (is.element(filestring[pos],c("=",":"))) {
		getNextChar();
	} else if (filestring[pos]==";") {
        # don't do anything
    } else {
    	stop("config line ",sourceline[pos],
    ": unexpected symbol'",filestring[pos],"'")
	}
	return(name);
}

getString <- function() {
	if (filestring[pos] != "\"") 
		stop("config line ",sourceline[pos],": this is not a string");
	value <- "";
	repeat {
		getNextChar()
		if (filestring[pos] == "\"") break;
		if(filestring[pos] == "\\" && filestring[pos+1] == "\"") {
			value <- paste(value, "\"", sep="");
			getNextChar();
		} else {
			value <- paste(value, filestring[pos], sep="");
		}
	}
	getNextChar();
	return(value);
}

getNumber <- function(nonchar=c()) {
    value <- ""
    repeat {
            if (is.element(filestring[pos],c(";","}",nonchar))) break;
            value <- paste(value, filestring[pos], sep="")
            getNextChar();
    }
    n <- nchar(value)
    while (substr(value,n,n)==" ") {
        value <- substr(value, 1, n-1);
        n <- n-1;        
    }
    if (substr(value,n,n) == "y" 
        && n>1
        && !is.element(substr(value,n-1,n-1),LETTERCHARS) ) {
		value <- substr(value, 1, n-1)
        value <- paste(value, "*365",sep="")
    } else if (substr(value,n,n) == "s" 
        && n>1
        && !is.element(substr(value,n-1,n-1),LETTERCHARS) ) {
                value <- substr(value, 1, n-1)
        value <- paste(value, "*1.15740740740741e-05",sep="")
    } else if (substr(value,n,n) == "m" 
        && n>1
        && !is.element(substr(value,n-1,n-1),LETTERCHARS) ) {
                value <- substr(value, 1, n-1)
        value <- paste(value, "*0.000694444444444444",sep="")
    } else if (substr(value,n,n) == "h" 
        && n>1
        && !is.element(substr(value,n-1,n-1),LETTERCHARS) ) {
                value <- substr(value, 1, n-1)
        value <- paste(value, "*0.0416666666666667",sep="")
    } else if (substr(value,n,n) == "d" 
        && n>1
        && !is.element(substr(value,n-1,n-1),LETTERCHARS) ) {
                value <- substr(value, 1, n-1)
        value <- paste(value, "*1",sep="")
    } 
    if (sum(!is.element(strsplit(value,split="")[[1]],
        c(.Call("rif_intToChar",48:57),"(",")","*")))==0) {
            value <- paste(c("as.integer(",value,")"),collapse="")
    }
    return(eval(parse(text=value),envir=thisenv))
}

getToken <- function(nonchar=c()) {
	if (filestring[pos] == "\"") {
		return(getString());
	} else {
		return(getNumber(nonchar));
	}
}

getArray <- function() {
    if (filestring[pos] != "[") 
        stop("config line ",sourceline[pos],": this is not an array");
    getNextChar()
    value <- ""
    repeat {
        if (filestring[pos] == "]") break;
        value <- paste(value,filestring[pos],sep="");
        getNextChar();
    }
    getNextChar()
    gotoNextNonSpace()
    if (filestring[pos] == "y") {
        value <- paste(c("c(",value, ")*365"),collapse="")
        getNextChar();
    } else if (filestring[pos] == "s") {
        value <- paste(c("c(",value, ")*1.15740740740741e-05"),collapse="")
        getNextChar();
    } else if (filestring[pos] == "m") {
        value <- paste(c("c(",value, ")*0.000694444444444444"),collapse="")
        getNextChar();
    } else if (filestring[pos] == "h") {
        value <- paste(c("c(",value, ")*0.0416666666666667"),collapse="")
        getNextChar();
    } else if (filestring[pos] == "d") {
        value <- paste(c("c(",value, ")"),collapse="")
        getNextChar();
    } else if (filestring[pos] %in% c("*","/","-","+")) {
        op <- filestring[pos]
        getNextChar()
        val <- getNumber()
        value <- paste(c("c(",value, ")",op,as.character(val)),collapse="")
    } else {
        value <- paste(c("c(",value, ")"),collapse="")
    }

    return(eval(parse(text=value),envir=thisenv))
}

# getArray <- function() {
# 	if (filestring[pos] != "[") 
# 		stop("Error, this is not a string");
# 	getNextChar()
# 	value <- c()
# 	i = 1
# 	repeat {
# 		gotoNextNonSpace();
# 		if (filestring[pos] == "]") break;
# 		value[i] = getToken(nonchar=c(",","]"))
# 		i = i + 1
# 		if (filestring[pos] == ",") {
# 			getNextChar()
# 		}
# 	}
# 	getNextChar()
# 	return(value)
# }

setCoreAttr <- function(l, randcore = as.integer(0))
{
    if (is.list(l) && length(l)>0) {
        thisrandcore <- randcore
        if (is.element("randcore",names(l))) {
            thisrandcore <- as.integer(l$randcore);
        }
        attr(l,"randcore") <- thisrandcore;
        for(i in 1:length(l)) {
            l[[i]] <- setCoreAttr(l[[i]],thisrandcore)
        }
    } else {
        attr(l,"randcore") <- randcore;
    }
    return(l)
}

startlist <- function(level = 0, path="") {
    l <- list()
	gotoNextNonSpace();
	if (level > 0 && filestring[pos] != "{") {
		stop("config line ",sourceline[pos],": missing '{'"); 
	}
	if (filestring[pos] == "{") getNextChar()
    repeat {
        gotoNextNonSpace();
        if (is.na(filestring[pos]) && level != 0) {
            stop("unexpected end of file: unbalanced '{...}' brackets") 
        }
        if ((level==0 && is.na(filestring[pos])) || filestring[pos] == "}")
            break;
		name <- getName();
		sl <- sourceline[pos]
        result <- numeric(0);
		gotoNextNonSpace();
        if (filestring[pos] == ";") {
            result <- TRUE;
        } else if (filestring[pos] == "[") {
            result <- getArray()
        } else if (filestring[pos] == "{") {
            result <- 
                startlist(level=level+1, paste(c(path,".",name),collapse=""));
        } else {
			result <- getToken();
        }
        l[[name]] <- result;
        attr(l[[name]],"sourceline") <- as.integer(sl)
        attr(l[[name]],"used") <- FALSE
        attr(l[[name]],"path") <- paste(c(path,".",name),collapse="");
		gotoNextNonSpace();
		if (filestring[pos] != ";") {
			stop("config line ",sourceline[pos],": missing semicolon");
		}
		getNextChar();
#         message(name);
    }
    getNextChar()
    return(l)
}


expandNumbers <- function(l) 
{
  if (!is.list(l)) return(l);
  if (length(l) == 0) return(l);

  for (i in 1:length(l)) {
    l[[i]] <- expandNumbers(l[[i]]);
  }
  ans <- l
  nums <- as.numeric(names(l))
  if (!identical(nums,round(nums))) return(l)
  if (sum(!is.na(nums)) == 0) return(l)

  lastobj <- NULL
  for(i in min(nums,na.rm=TRUE):max(nums,na.rm=TRUE)) {
    if (is.element(i,nums)) 
      lastobj <- l[[as.character(i)]]
    if (!is.na(lastobj)) 
      ans[[as.character(i)]] <- lastobj
    if (is.na(ans[[as.character(i)]])) 
      ans[[as.character(i)]] <- NULL  
  }
  if (is.null(ans$depends)) {
    ans$depends <- "currentpartners";
  }
  attr(ans$depends,"path") <- paste(attr(l,"path"),"depends",sep=".")
  attr(ans$depends,"used") <- FALSE
  ans$maximum <- as.integer(max(nums,na.rm=TRUE));
  attr(ans$maximum,"path") <- paste(attr(l,"path"),"maximum",sep=".")
  attr(ans$maximum,"used") <- FALSE
  ans$minimum <- as.integer(min(nums,na.rm=TRUE));
  attr(ans$minimum,"path") <- paste(attr(l,"path"),"minimum",sep=".")
  attr(ans$minimum,"used") <- FALSE
  return(ans)
}

expandSpecials <- function(l) 
{
    if (is.list(l)) {
        if (length(l) > 0) {
					namelist <- names(l);
					for (i in 1:length(l)) {
            if (namelist[i] == "bybin" && l$bybin) {
                l$depends <- "bin"
                attr(l$depends,"path") <- paste(attr(l,"path"),"depends",sep=".")
                attr(l$depends,"used") <- FALSE
                attr(l$depends,"sourceline") <- attr(l$bybin,"sourceline")
                l$bybin <- NULL                        
            } else if (namelist[i] == "bytype" && l$bytype) {
                l$depends <- "type"
                attr(l$depends,"path") <- paste(attr(l,"path"),"depends",sep=".")
                attr(l$depends,"used") <- FALSE
                attr(l$depends,"sourceline") <- attr(l$bytype,"sourceline")
                l$bytype <- NULL
            } else if (namelist[i] == "ispregnant" && l$ispregnant) {
                l$depends <- "ispregnant"
                attr(l$depends,"path") <- paste(attr(l,"path"),"depends",sep=".")
                attr(l$depends,"used") <- FALSE
                attr(l$depends,"sourceline") <- attr(l$ispregnant,"sourceline")
                l$ispregnant <- NULL
            } else {
                l[[namelist[i]]] <- expandSpecials(l[[namelist[i]]]);     
            }
					}
				}
        if (is.null(l[["uniformdistribution"]])) {
          l$uniformdistribution <- list(type="uniform");
          attr(l$uniformdistribution,"path") <- paste(attr(l,"path"),"uniformdistribution",sep=".")
          attr(l$uniformdistribution,"used") <- FALSE
          attr(l$uniformdistribution,"sourceline") <- attr(l,"sourceline");
          attr(l$uniformdistribution$type,"path") <- paste(attr(l,"path"),"uniformdistribution.type",sep=".")
          attr(l$uniformdistribution$type,"used") <- FALSE
          attr(l$uniformdistribution$type,"sourceline") <- attr(l,"sourceline");
        } 
        if (is.null(l[["constantdistribution"]])) {
          l$constantdistribution <- 0
          attr(l$constantdistribution,"used") <- FALSE
          attr(l$constantdistribution,"path") <- paste(attr(l,"path"),"constantdistribution",sep=".")
          attr(l$constantdistribution,"sourceline") <- attr(l,"sourceline");  
        }
    }
    return(l)
}


l <- startlist()

l <- expandSpecials(l);

warn <- options(warn=-1)
l <- expandNumbers(l)
options(warn)

l <- setCoreAttr(l)

class(l) <- "sti.configuration"

return(l)

}


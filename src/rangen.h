/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***********************************************************************
 
  uniform unsigned int random generator rand_dia.

  Persi Diaconis's (Harvard Univ.) personal favorite is a multiplicative
  "lagged Fibonacci" generator that starts with 55 odd numbers and than 
  uses the formular
  
    x(n) = x(n-24) * x(n-55)  mod (2**32)

  to generate a sequence of random numbers. 
  (Diaconis, "The Unreasonable Effectiveness of Number Theory", AAAS'94 talk)
  This "lagged Fibonacci" generators was considered by Marsaglia.
  He defines F(R,S,OP) as the recurence: x(n) = x(n-R) OP x(n-S).
  Parameters he considers: F(17,5,*), F(31,13,*), F(55,24,*), F(127,30,*).

  To analyze F(R,S,*) with R>S use the transformation (Knuth TAOCP2 3.2.2
Ex31): 
    x(n) = (-1)**s(n) * 3**z(n).
  Then we have the recurences for z and s
    z(n) = z(n-R) + z(n-S)  mod (2**32)
    s(n) = s(n-R) + s(n-S)  mod 2
  We see that we should avoid setting all x(n) be +1 or -1 modulo 8.
  The maximal period for n bit words is (2**R - 1) * 2**(n-3) for n>=3.
  For a test you can use F(3,1,*) and n=4 (the last hex-digit).
  The lowest hex-digit of b0 has period 56 if calculated with 2 implicid bits.

  -= one implicid bit version =-
  As we know that all values are odd, lets calculate with an implicid last bit
    x(n) = x(n-R) * x(n-S)  mod (2**33)
  on 32-bit machines instead. Let x(n) = 2 y(n) + 1 than we have
    
    y(n) = 2 * y(n-R) * y(n-S) + y(n-R) + y(n-S) mod (2**32)

  -= two implicid bits version =-
  If we calculate with 3**z values only, the bit pattern ending with
  the three bits 0X1. Therefore we can drop two bits and calculate
  with two implicid bits.
  
  Ref: - Barry A. Cipra,
     AAAS'94: Random Numbers, Art and Math,
     SIAM News, Aug./Sept. 1994, p 24 and 18.

       - G. Marsaglia,
     A Current View of Random Number Generators,
     Computer Science and Statistics, 1985, 3-10

       - D. E. Knuth,
     The Art of Computer Programming, Vol2: Semi-numerical Algorithms,
     3rd. Edition, Addison-Wesley, Reading, 1997 (not 2nd Edition)
     Sec. 3.2:   Generating uniform random variables
     Sec. 3.2.2: Other methods
       equation (7') and exercise 3.2.2 Ex 31

  Implementation: Torsten Sillke, 1995 (one implicid bit version)
  Implementation: Torsten Sillke, 1998 (two implicid bits version)
  email: Torsten.Sillke@uni-bielefeld.de
 
***********************************************************************/

/**
 * The maximal number of parallel random number generators supported
 */ 
#define RANCORES 100

namespace ran {

/**
 * Initialise the random number generators  using the current time.
 */
void sran();
/**
 * Initialise the random number generators using a given seed. Note that only
 * the random number generator with core 0 will have exactly this seed, for the
 * others a different seed is calculated from this one, so that each random
 * number generator yields a different sequence of random numbers.
 * @param init the seed
 */
void sran(unsigned int init);

/**
 * Return an unsigned integer between 0 and 'max' (not including 'max')
 * @param randcore the core to be used (0 to RANCORES-1)
 * @param max the maximal integer
 * @return one random number
 */
unsigned int ran(unsigned int randcore, unsigned int max);
/**
 * Return an unsigned integer between 0 and 2^32-1 (including this value)
 * @param randcore the core to be used (0 to RANCORES-1)
 * @return one random number
 */
unsigned int rand_dia(unsigned int randcore);

/**
 * Return a random real number between 'min' and 'max'
 * @param randcore the core to be used (0 to RANCORES-1)
 * @param min the minimal value
 * @param max the maximal value
 * @return one random number
 */
double dran(unsigned int randcore, double min, double max);
/**
 * Return a random real number between 0 and 1
 * @param randcore the core to be used (0 to RANCORES-1)
 * @return one random number
 */
double dran(unsigned int randcore);

}

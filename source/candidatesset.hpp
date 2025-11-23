//
// C++ Interface: CandidatesSet
//
// Description: Base class to manage a Candidate Set.
//              This class is designed to switch between high-performance and low-memory variants of implementations.
//
//
// Author: Robert Bredereck <RBredereck@web.de>, (C) 2008, 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CANDIDATESSET_H
#define CANDIDATESSET_H

#include <stdio.h>
#include <stdlib.h>

// project internal
#include <votetyps.hpp>

const Candidate CandidateAsSetElement[32] = {1, 2, 4, 8 , 16 , 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648};

class CandidatesSet
{
public:
  static void insert ( const Candidate &c, Candidate &set );
  static void insert ( const Candidate &c, CandidateSet &set );
  static void erase ( const Candidate &c, Candidate &set );
  static void erase ( const Candidate &c, CandidateSet &set );

  static bool has ( const Candidate &c, Candidate &set );
  static bool has ( const Candidate &c, CandidateSet &set );

  static Candidate* first ( Candidate &set );
  static Candidate* next ( Candidate *current, Candidate &set );
  static Candidate* last ( Candidate &set );

  static int size ( Candidate &set );
  static int size ( CandidateSet &set );

  static CandidateSet asCandidateSet ( Candidate &set );
  static CandidateSet asCandidateSet ( CandidateSet &set );
private:
};

/*class CandidatesSetIterator
{
  public:
    operator *(CandidatesSetIterator iter);
    operator ==(CandidatesSetIterator iter1, CandidatesSetIterator iter2);
  private:
    Candidate set;
    Candidate current;
};*/

#endif

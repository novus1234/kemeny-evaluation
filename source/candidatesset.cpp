// C++ Implementation: CandidatesSet_performace
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
#include <stdio.h>
#include <stdlib.h>

// project internal
#include "votetyps.hpp"
#include "candidatesset.hpp"

static void insert ( const Candidate &c, Candidate &set )
{
  set = set or c;
}

static void erase ( const Candidate &c, Candidate &set )
{
  if ( CandidatesSet::has ( c, set ) )
  {
    set = set xor c;
  }
}

static bool has ( const Candidate &c, Candidate &set )
{
  return ( ( set & c ) == c );
}

static int size ( Candidate &set )
{
  return log2 ( set );
}

static CandidateSet asCandidateSet ( Candidate &set )
{
  CandidateSet returnSet;
  for ( int i = 1; i <= 32; i++ )
  {
    if ( CandidatesSet::has ( i, set ) )
      returnSet.insert ( i );
  }
  return returnSet;
}

static void insert ( const Candidate &c, CandidateSet &set )
{
  set.insert ( c );
}

static void erase ( const Candidate &c, CandidateSet &set )
{
  set.erase ( c );
}

static bool has ( const Candidate &c, CandidateSet &set )
{
  return ( set.find ( c ) != set.end() );
}

static int size ( CandidateSet &set )
{
  return set.size();
}

static CandidateSet asCandidateSet ( CandidateSet &set )
{
  return set;
}

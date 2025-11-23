/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file for data structures.
 *
 *   Kconsens is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Kconsens is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Kconsens.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

// project internal
#include "votetyps.hpp"

using namespace boost;
using namespace std;

std::size_t hash_value ( Vote const& v )
{
  boost::hash<string> hasher;
  return hasher ( v.asString );
};

bool operator== ( Vote const& a, Vote const& b )
{
  return ( a.asString == b.asString );
}

std::size_t hash_value ( CandidatePair const& p )
{
  boost::hash<Candidate> hasher;
  return hasher ( p.lower ) xor hasher ( p.upper );
};

bool operator== ( CandidatePair const& a, CandidatePair const& b )
{
  return ( a.lower == b.lower ) && ( a.upper == b.upper );
}

std::size_t hash_value ( CandidateTriple const& t )
{
  boost::hash<Candidate> hasher;
  return hasher ( t.lower ) xor hasher ( t.middle ) xor hasher ( t.upper );
};

bool operator== ( CandidateTriple const& a, CandidateTriple const& b )
{
  return ( a.lower == b.lower ) && ( a.middle == b.middle ) && ( a.upper == b.upper );
}

CandidatePair ascSortedCandidatePair ( CandidatePair &p )
{
  CandidatePair n;
  n.lower = min ( p.lower, p.upper );
  n.upper = max ( p.lower, p.upper );
  return n;
}

CandidatePair ascSortedCandidatePair ( Candidate x, Candidate y )
{
  CandidatePair p;
  p.lower = min ( x, y );
  p.upper = max ( x, y );
  return p;
}

CandidatePair dscSortedCandidatePair ( CandidatePair &p )
{
  CandidatePair n;
  n.lower = max ( p.lower, p.upper );
  n.upper = min ( p.lower, p.upper );
  return n;
}

CandidatePair dscSortedCandidatePair ( Candidate x, Candidate y )
{
  CandidatePair p;
  p.lower = max ( x, y );
  p.upper = min ( x, y );
  return p;
}

CandidatePair flipCandidatePair ( CandidatePair &p )
{
  CandidatePair newP;
  newP.lower = p.upper;
  newP.upper = p.lower;
  return newP;
}

CandidateTriple ascSortedCandidateTriple ( Candidate a, Candidate b, Candidate c )
{
  CandidateTriple t;
  t.lower = min ( a, min ( b, c ) );
  t.upper = max ( a, max ( b, c ) );
  if ( ( ( t.lower == a ) || ( t.upper == a ) ) && ( ( t.lower == b ) || ( t.upper == b ) ) )
    t.middle = c;
  if ( ( ( t.lower == a ) || ( t.upper == a ) ) && ( ( t.lower == c ) || ( t.upper == c ) ) )
    t.middle = b;
  if ( ( ( t.lower == c ) || ( t.upper == c ) ) && ( ( t.lower == b ) || ( t.upper == b ) ) )
    t.middle = a;
  return t;
}

CandidateTriple dscSortedCandidateTriple ( Candidate a, Candidate b, Candidate c )
{
  CandidateTriple t;
  t.lower = max ( a, max ( b, c ) );
  t.upper = min ( a, min ( b, c ) );
  if ( ( ( t.lower == a ) || ( t.upper == a ) ) && ( ( t.lower == b ) || ( t.upper == b ) ) )
    t.middle = c;
  if ( ( ( t.lower == a ) || ( t.upper == a ) ) && ( ( t.lower == c ) || ( t.upper == c ) ) )
    t.middle = b;
  if ( ( ( t.lower == c ) || ( t.upper == c ) ) && ( ( t.lower == b ) || ( t.upper == b ) ) )
    t.middle = a;
  return t;
}

std::size_t hash_value ( CandidateSetS const& cs )
{
  boost::hash<Candidate> hasher;
  size_t hashval = 0;
  for ( CandidateSetIterator c = cs.set.begin(); c != cs.set.end(); c++ )
  {
    hashval = hasher ( *c ) xor hashval;
  }
  return hashval;
};

bool operator== ( CandidateSetS const& a, CandidateSetS const& b )
{
  return ( a.set == b.set );
}

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
#include <iostream>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

// project internal
#include "kemtyps.hpp"
#include "votetyps.hpp"

using namespace boost;
using namespace std;

std::size_t hash_value ( const CandidateSetSt& s )
{
  boost::hash<Candidate> hasher;
  std::size_t hashres = 0;
  for ( CandidateSetIterator i = s.candidates.begin(); i != s.candidates.end(); i++ )
  {
    hashres = hashres xor hasher ( *i );
  }
  return hashres;
  //boost::hash_range ( s.candidates.begin(), s.candidates.end() );
};

bool operator== ( const CandidateSetSt& a, const CandidateSetSt& b )
{
  return ( a.candidates == b.candidates );
}

// transformations
void appendCandidateVector2CandidateList ( const CandidateVector &v, CandidateList &l )
{
  for ( int i = 0; i < v.size(); i++ )
  {
    l.push_back ( v.at ( i ) );
  }
}

void printL ( CandidateCandidateRelationsMap &L )
{
  for ( CandidateCandidateRelationsMapIterator l = L.begin(); l != L.end(); l++ )
  {
    for ( CandidateSetIterator c = ( *l ).second.lowers.begin(); c != ( *l ).second.lowers.end(); c++ )
    {
      cout << ( *c ) << ",";
    }
    cout << "[" << ( *l ).first << "]";
    for ( CandidateSetIterator c = ( *l ).second.uppers.begin(); c != ( *l ).second.uppers.end(); c++ )
    {
      cout << "," << ( *c );
    }
    cout << endl;
  }
  cout << endl;
}

void printSet ( CandidateSetSt &s )
{
  CandidateSetIterator iterator = s.candidates.begin();
  while ( iterator != s.candidates.end() )
  {
    if ( iterator == s.candidates.begin() )
    {
      cout << ( *iterator );
    }
    else
    {
      cout << "," << ( *iterator );
    }
    iterator++;
  }
  cout << endl;
}

/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of permutationmanagement
 *   Description: This class manages a permutation list.
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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// project internal
#include "errors.hpp"
#include "tools.hpp"
#include "permutationmanagement.hpp"

using namespace std;
using namespace boost;

permutationmanagement::permutationmanagement ( CandidateVector &element_numbers )
{
  this->m = element_numbers.size();
  this->elements = element_numbers;
  this->setFirst();
}

permutationmanagement::~permutationmanagement()
{
}

bool permutationmanagement::notlastpos()
{
  bool latestpos = true;
  //cout << "lll" << m << endl;
  for ( int i = 0; i < m; i++ )
  {
    int lpos = ( m - i - 1 );
    int apos = relpos.at ( i );
    //cout << i << ":" << apos << "<" << lpos << "?\n";
    if ( apos < lpos )
    {
      latestpos = false;
    }
  }
  return !latestpos;
}

bool permutationmanagement::isLast()
{
  if ( ! ( this->markLast ) )
  {
    markLast = !notlastpos();
  }
  return markLast;
}

void permutationmanagement::nextpos()
{
  int m = relpos.size();
  bool uebertrag = true;
  for ( int i = m - 1; i >= 0; i-- )
  {
    if ( uebertrag )
    {
      relpos[i+1] = 0;
      relpos[i] = relpos[i] + 1;
      uebertrag = ( relpos[i] > ( m - i - 1 ) );
    }
  }
}

bool permutationmanagement::setNext()
{
  if ( ! ( this->markLast ) )
  {
    this->nextpos();
  }
  else
    this->lastAgain = true;
  this->isLast();
  return this->lastAgain;
}

void permutationmanagement::setFirst()
{
  this->markLast = false;
  this->lastAgain = false;
  this->relpos.clear();
  for ( int i = 0; i < m; i++ )
  {
    this->relpos.push_back ( 0 );
    this->positions.push_back ( i );
  }
}

CandidateVector permutationmanagement::getAsVector()
{
  CandidateVector permAsVector;
  updateVector ( permAsVector );
  return permAsVector;
}

void permutationmanagement::updateVector ( CandidateVector &permAsVector )
{
  vector<int> permPos = positions;
  int perm[m];
  for ( int i = 1; i < ( this->m + 1 ); i++ )
  {
    int posOFi = permPos.at ( relpos[i-1] );
    //cout << posOFi << "/" << this->m << "\n";
    perm[posOFi] = i;
    //cout << relpos[i-1] << "/" << permPos.size() << "\n";
    permPos.erase ( permPos.begin() + relpos[i-1] );
  }
  permAsVector.clear();
  for ( int i = 0; i < m; i++ )
  {
    permAsVector.push_back ( this->elements[perm[i] - 1] );
  }
}

/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance
 *   Description: A partially abstract class to compute kemeny score and consensus.
 *                Here common data structures and functions are defined for all child classes.
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

// project internal
#include "tools.hpp"
#include "kconsens_instance.hpp"

using namespace std;
using namespace boost;

kconsens_instance::kconsens_instance ( stringstream &outdebug_ ) :
    outdebug ( outdebug_ )
{
  this->isSynchronized = false;
}

kconsens_instance::~kconsens_instance()
{
}

void kconsens_instance::saveConsensusToFile ( string filename )
{
  ofstream dest ( filename.data() );
  saveConsensusToStream ( dest );
  dest << endl;
  dest.close();
}

void kconsens_instance::saveConsensusToStream ( ostream &dest )
{
  for ( CandidateListIterator si = this->consens.begin(); si != this->consens.end();si++ )
  {
    if ( si != this->consens.begin() )
    {
      dest << ">";
    }
    dest << *si;
  }
}

void kconsens_instance::saveScoreToFile ( string filename )
{
  ofstream dest ( filename.data() );
  dest << this->consensScore << endl;
  dest.close();
}

long int kconsens_instance::getConsensusScore()
{
  return this->consensScore;
}

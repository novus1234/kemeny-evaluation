/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ implementation file of kconsens_instance_approx_pickpermrand
 *   Description: Class that realizes an approximazation algorithm for the Kemeny Score problem known as "pick a perm".
 *                It is a very simple algorithms designed as template.
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
#include <limits.h>

// project internal
#include "tools.hpp"
#include "randomtools.hpp"
#include "kemtyps.hpp"
#include "kconsens_instance_approx_pickpermrand.hpp"

kconsens_instance_approx_pickpermrand::kconsens_instance_approx_pickpermrand ( stringstream &outdebug ) : kconsens_instance ( outdebug )
{
  srand ( ( unsigned ) time ( 0 ) );
  this->isSynchronized = false;
}

kconsens_instance_approx_pickpermrand::~kconsens_instance_approx_pickpermrand()
{
  //this->~kconsens_instance();
}

void kconsens_instance_approx_pickpermrand::solve ( stringstream &outverbose, stringstream &outtable )
{
  base_generator_type generator ( 42u );
  generator.seed ( static_cast<unsigned int> ( time ( 0 ) ) xor rand() );
  if ( !this->isSubScoreAnalyzed )
    this->analyzeSubScore();

  unsigned int vid = getRandomUint ( 0, this->votes.size() - 1, generator );

  this->consens = this->votes.at ( vid ).asList;
  unsigned long k = this->kscore ( this->consens );

  this->consensScore = k;
  outverbose << "The vote " << vid << " was selected:\n";
  printVote ( this->consens, outverbose );
}

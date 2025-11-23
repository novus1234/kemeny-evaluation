/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance
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

#ifndef KCONSENS_INSTANCE_H
#define KCONSENS_INSTANCE_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <boost/thread.hpp>

// project internal
#include "votesmanagement.hpp"
#include "votetyps.hpp"
#include "kemtyps.hpp"

/**
 A partially abstract class to compute kemeny score and consensus.
 Here common data structures and functions are defined for all child classes.
 @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance : public votesmanagement
{
public:
  // const & dest
  kconsens_instance ( stringstream &outdebug_ );
  ~kconsens_instance();

  // important methods
  virtual void solve ( stringstream &outverbose, stringstream &outtable ) = 0;
  void saveConsensusToFile ( string filename );
  void saveConsensusToStream ( ostream &dest );
  void saveScoreToFile ( string filename );

  long getConsensusScore();

  string dirtySetSequence;
protected:
  /// optimal consensus list (as result of computation)
  CandidateList consens;

  /// score of the this->consens (as result of computation)
  unsigned long consensScore;
private:
  /// debug / logging output stream
  stringstream &outdebug;
};

#endif

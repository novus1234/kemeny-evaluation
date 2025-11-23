/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance_glpk
 *   Description: Class that transforms the problem instanze to a linear program in "GNU MathProg Language".
 *                The linear program will be solved by glpk.
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

#ifndef KCONSENS_INSTANCE_GLKP_H
#define KCONSENS_INSTANCE_GLPK_H

// project internal
#include "kconsens_instance.hpp"
#include "kconsens_instance_candidate_relations_based.hpp"

/**
 Class that transforms the problem instanze to a linear program in "GNU MathProg Language".*
 The linear program will be solved by glpk.
 @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance_glpk : public kconsens_instance_candidate_relations_based
{
public:
  // const & dest
  kconsens_instance_glpk ( stringstream &outdebug, int timelimit_, bool timelimit_flag_ );
  ~kconsens_instance_glpk();

  void solve ( stringstream &outverbose, stringstream &outtable );
protected:
  int timelimit;
  bool timelimit_flag;
private:
  long getGlpsolKscore ( string glpsolOutputFilename );
  int readCandidatePairRelations ( string cplexOutputFilename, CandidateCandidateRelationsMap &L );
};


#endif

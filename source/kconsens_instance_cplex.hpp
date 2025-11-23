/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance_cplex
 *   Description: Class that transforms the problem instanze into a linear program in cplex LP format.
 *                The linear program will be solved by cplex.
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

#ifndef KCONSENS_INSTANCE_CPLEX_H
#define KCONSENS_INSTANCE_CPLEX_H

// project internal
#include "kconsens_instance.hpp"
#include "kconsens_instance_candidate_relations_based.hpp"

/**
 * Class that transforms the problem instanze into a linear program in cplex LP format.
 * The linear program will be solved by cplex.
 * @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance_cplex : public kconsens_instance_candidate_relations_based
{
public:
  // const & dest
  kconsens_instance_cplex ( stringstream &outdebug, int timelimit_, bool timelimit_flag_ );
  ~kconsens_instance_cplex();

  void solve ( stringstream &outverbose, stringstream &outtable );

protected:
  int timelimit;
  bool timelimit_flag;

private:
  long getCplexKscore ( string cplexOutputFilename );
  int readCandidatePairRelations ( string cplexOutputFilename, CandidateCandidateRelationsMap &L );
};


#endif

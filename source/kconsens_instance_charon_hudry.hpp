/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2011 Robert Bredereck <RBredereck@web.de>
 *
 *   Methods in this class:
 *   (C) Irene Charon and Olivier Hudry
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance_charon_hudry
 *   Description: Class that transforms the problem instance into a a weighted
 *                tournament and solves the median order problem.
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

#ifndef KCONSENS_INSTANCE_CharonHudry_H
#define KCONSENS_INSTANCE_CharonHudry_H

// project internal
#include "kconsens_instance.hpp"

/**
 Class that transforms the problem instance into a a weighted
 tournament and solves the median order problem.
 @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance_charon_hudry : public kconsens_instance
{
public:
  // const & dest
  kconsens_instance_charon_hudry ( stringstream &outdebug, int timelimit_, bool timelimit_flag_ );
  ~kconsens_instance_charon_hudry();

  void solve ( stringstream &outverbose, stringstream &outtable );

protected:
  int timelimit;
  bool timelimit_flag;

private:
  long get_charon_hudry_Kscore ( string cplexOutputFilename );
  int read_charon_hudry_Consensus ( string cplexOutputFilename, CandidateList &clist );
};


#endif

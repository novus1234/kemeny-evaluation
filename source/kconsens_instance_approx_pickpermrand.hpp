/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file of kconsens_instance_approx_pickpermrand
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

#ifndef KCONSENS_INSTANCE_APPROX_PICKPERMRAND_H
#define KCONSENS_INSTANCE_APPROX_PICKPERMRAND_H

// project internal
#include "kconsens_instance.hpp"
#include "randomtools.hpp"

/**
 Very simple randomized algorithm class "pick a perm".
 @author Robert Bredereck <RBredereck@web.de>
*/
class kconsens_instance_approx_pickpermrand : public kconsens_instance
{
public:
  // const & dest
  kconsens_instance_approx_pickpermrand ( stringstream &outdebug );
  ~kconsens_instance_approx_pickpermrand();

  void solve ( stringstream &outverbose, stringstream &outtable );
protected:
  //
};


#endif

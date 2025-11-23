/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *
 *   Content: C++ header file jobtyps
 *   Description: This file contains structures constants relevant for the jobmanagement.
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

#ifndef JOBTYPS_H
#define JOBTYPS_H

#include <string>

using namespace std;

const string sepText = "\t";
const string sepLatex = " & ";

const string eolText = "\n";
const string eolLatex = "\\\\\n";

class SolveParams
{
public:
    explicit SolveParams ( int const& modus_, string const& instanceFilename_, int coreID_, long &timelimit_, bool &timelimit_flag_, bool verbose_flag_, stringstream &outverbose_, stringstream &outtable_, stringstream &outdebug_, double &cputime_, string &consensus_, long &score_, string &statusprefix_ ) :
            modus ( modus_ ),
            instancefilename ( instanceFilename_ ),
            coreID ( coreID_ ),
            timelimit ( timelimit_ ),
            timelimit_flag ( timelimit_flag_ ),
            verbose_flag ( verbose_flag_ ),
            outverbose ( outverbose_ ),
            outtable ( outtable_ ),
            outdebug ( outdebug_ ),
            cputime ( cputime_ ),
            consensus ( consensus_ ),
            score ( score_ ),
            statusprefix ( statusprefix_ )
    {}
    int const& modus;
    string const& instancefilename;
    int coreID;
    long &timelimit;
    bool &timelimit_flag;
    bool &verbose_flag;
    stringstream &outverbose;
    stringstream &outtable;
    stringstream &outdebug;
    double &cputime;
    string &consensus;
    long &score;
    string &statusprefix;
};

#endif

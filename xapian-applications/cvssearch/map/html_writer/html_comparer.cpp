/************************************************************
 *
 *  html_comparer.cpp implementation.
 *
 *  (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 *
 ************************************************************/

#include "html_comparer.h"
#include <strstream>
#include "code_to_html.h"

extern string scvs_update;

html_comparer::html_comparer(const vector<unsigned int> & input1,
                             const vector<unsigned int> & input2, 
                             const string & filename, 
                             const string & pathname,
                             const cvs_revision & revision0, 
                             const cvs_revision & revision1, 
                             const cvs_revision & revision2, const diff & diff)
    : html_writer(string("comparing")),
      _input1(input1),
      _input2(input2),
      _revision0(revision0),
      _revision1(revision1),
      _revision2(revision2),
      _filename(filename),
      _pathname(pathname),
      _diff(diff)
{
    p0 = p1 = p2 = 0;
    ostrstream ost0;
    ost0 << scvs_update << "-r" << revision0
         << " " << filename << " 2>/dev/null" << ends;
    
    ostrstream ost1;
    ost1 << scvs_update << "-r" << revision1 
         << " " << filename << " 2>/dev/null" << ends;
    
    ostrstream ost2;
    ost2 << scvs_update << "-r" << revision2 
         << " " << filename << " 2>/dev/null" << ends;

    p0 = new process(ost0.str());
    p1 = new process(ost1.str());
    p2 = new process(ost2.str());

    _diff.unalign_top();
}

html_comparer::~html_comparer()
{
    _diff.align_top();
    delete p0;
    delete p1;
    delete p2;
}

void
html_comparer::get_class_type (string & select0, unsigned int index0, bool & do0,
                               string & select1, unsigned int index1, bool & do1,
                               string & select2, unsigned int index2, bool & do2,
                               unsigned int & diff_index) const
{
    select0 = "";
    select1 = "";
    select2 = "";

    if (!do0) {
        select0 = " class=\"n\"";
    }

    if (!do1) {
        select1 = " class=\"n\"";
    }

    if (!do2) {
        select2 = " class=\"n\"";
    }
    

    if (diff_index >= _diff.size()) {
        // ----------------------------------------
        // no more differences, all select class are 
        // default to normal
        // ----------------------------------------
        return;
    } 

    if (index1 == _diff[diff_index].source().end() && 
        index2 == _diff[diff_index].dest().end() &&
        diff_index + 1< _diff.size()) 
    {
        ++diff_index;
    }


    if (0) {
    } else if (!do0 && do1 && do2) {
        // ----------------------------------------
        // only show lines in early & later version
        // need to figure out which ones to show
        // and which ones not to show.
        // ----------------------------------------
        switch (_diff[diff_index].type())
        {
        case e_delete:
            assert(_diff[diff_index].dest().begin() == _diff[diff_index].dest().end());
            if (_diff[diff_index].dest().begin() <= index2 &&
                _diff[diff_index].source().end() > index1) 
            {
                do2 = false;
            }
            break;
        case e_add:
            assert(_diff[diff_index].source().begin() == _diff[diff_index].source().end());
            if (_diff[diff_index].source().begin() <= index1 &
                _diff[diff_index].dest().end() > index2) 
            {
                do1 = false;
            }
            break;
        case e_change:
            break;
        case e_none:
            break;
        }
    }

    if (do1 && do2) 
    {
        if (_diff[diff_index].source().begin() <= index1 && 
            _diff[diff_index].source().end()   >  index1) 
        {
            select1 = " class=\"c\"";        
        }
        if (_diff[diff_index].dest().begin() <= index2 && 
            _diff[diff_index].dest().end()   >  index2) 
        {
            select2 = " class=\"c\"";
        }
    }
    
    if (!do1 && do2) {
        if (_diff[diff_index].dest().begin() <= index2 && 
            _diff[diff_index].dest().end()   >  index2) 
        {
            select2 = " class=\"d\"";
            select1 = " class=\"n\"";
        }
    }
    
    if (!do2 && do1) {
        if (_diff[diff_index].source().begin() <= index1 && 
            _diff[diff_index].source().end()   >  index1) 
        {
            select2 = " class=\"n\"";        
            select1 = " class=\"a\"";
        }
    }

    cerr << do0 << " " << index0 << select0 << endl;
    cerr << do1 << " " << index1 << select1 << endl;
    cerr << do2 << " " << index2 << select2 << endl;
    cerr << _diff[diff_index] << endl;
    cerr << endl;
    
    
}

void 
html_comparer::write_line(ostream & os, 
                       string & select0, unsigned int & index0, bool do0,
                       string & select1, unsigned int & index1, bool do1,
                       string & select2, unsigned int & index2, bool do2,
                       unsigned int & diff_index) const
{
    string line0, line1, line2;
    istream * pis0, * pis1, * pis2;
    pis0 = p0->process_output();
    pis1 = p1->process_output();
    pis2 = p2->process_output();    

    get_class_type (select0, index0, do0,
                    select1, index1, do1,
                    select2, index2, do2,
                    diff_index);

    if (do0) getline(*pis0, line0);
    if (do1) getline(*pis1, line1);
    if (do2) getline(*pis2, line2);

    if (*pis0 || *pis1 || *pis2) 
    {
        unsigned int size = 40;
        code_to_html converter0(line0, size);
        code_to_html converter1(line1, size);
        code_to_html converter2(line2, size);
        os << "<TR>";
        os << "<TD" << select2 << "> "; if (do2) os << index2;     os << "</TD>";
        os << "<TD" << select2 << "> "; if (do2) os << converter2; os << "</TD>";
        os << "<TD" << select1 << "> "; if (do1) os << index1;     os << "</TD>";
        os << "<TD" << select1 << "> "; if (do1) os << converter1; os << "</TD>";
        os << "<TD" << select0 << "> "; if (do0) os << index0;     os << "</TD>";
        os << "<TD" << select0 << "> "; if (do0) os << converter0; os << "</TD>";
        os << "</TR>" << endl;
    }
    if (do0) ++index0;
    if (do1) ++index1;
    if (do2) ++index2;
}

ostream &
html_comparer::write(ostream & os) const
{
    string select0 = " class=\"s\"";
    string select1 = " class=\"s\"";
    string select2 = " class=\"s\"";
    
    os << "<H3 align=center>aligned diff for " << _pathname 
       << " between "
       << " version " << _revision2
       << " and"
       << " version " << _revision1
       << "</H3>" << endl
       << " Here we show where the differences occurred between "
       << " version " << _revision2
       << " and"
       << " " << _revision1
       << ", and the propagation of the affected lines to version " << _revision0
       << endl;

    os << "<TABLE align=center border=0 cellspacing=0 cellpadding=1>" << endl;
    os << "<TR>";
    os << "<TD" << select2 << "> </TD>";
    os << "<TD" << select2 << "align=center> <B>version " << _revision2 << "</B></TD>";
    os << "<TD" << select1 << "> </TD>";
    os << "<TD" << select1 << "align=center> <B>version " << _revision1 << "</B></TD>";
    os << "<TD" << select0 << "> </TD>";
    os << "<TD" << select0 << "align=center> <B>version " << _revision0 << "</B></TD>";
    os << "</TR>" << endl;

    unsigned int index0 = 1, index1 = 1, index2 = 1;
    string line0, line1, line2;

    istream *pis0, *pis1, *pis2;
    unsigned int diff_index = 0;

    if (p0 && (pis0 = p0->process_output()) &&
        p1 && (pis1 = p1->process_output()) &&
        p2 && (pis2 = p2->process_output()))
    {
        assert(_input1.size() == _input2.size());
        
        while (index0 < _input1.size())
        {
            // ----------------------------------------
            // currently we want to print line #index0
            // of the latest version.
            // 
            // it is possible that index2 is smaller than
            // or equal to _input2[index0] because there
            // are more things in the early version to be printed.
            //
            // it is possible that index1 is smaller than
            // or equal to _input1[index0] because there
            // are more things in the later version to be printed.
            // 
            // it is not possible that index2 is larger than
            // (_input2[index0] > 0), this means we are ready to 
            // print some line in the early version, but at the same,
            // we want to print a line in the final version that
            // matches an earlier line in the earlier version.
            //
            // it is not possible that index1 is larger than
            // (_input1[index0] > 0), this means we are ready to 
            // print some line in the later version, but at the same,
            // we want to print a line in the final version that
            // matches an earlier line in the later version.
            // ----------------------------------------
            assert(_input2[index0] == 0 || _input2[index0] >= index2);
            assert(_input1[index0] == 0 || _input1[index0] >= index1);
            // ----------------------------------------
            // _input2[index0] gives the line # of later version
            // that matches line index0 of the latest version.
            // 
            // if _input2[index0] == 0 that means
            // not used.
            // ----------------------------------------

            if (0) {
            } else if (_input1[index0] == index1) {
                if (0) {
                } else if (_input2[index0] == index2) {
                    // ----------------------------------------
                    // should be the most common case.
                    // 
                    // index0 == line # of final version
                    // index1 == line # of later version
                    // index2 == line # of earlier version
                    // 
                    // umm.. all the lines are need to be produced.
                    // ----------------------------------------
                    write_line(os,
                               select0, index0, true,
                               select1, index1, true,
                               select2, index2, true,
                               diff_index);
                } else if (_input2[index0] > index2) {
                    // ----------------------------------------
                    // do not print a line in the final version
                    // because we still need to print some line
                    // in the early version.
                    // since the line in the later version
                    // matches the line in the final version,
                    // don't print that either.
                    // ----------------------------------------
                    write_line(os,
                               select0, index0, false,
                               select1, index1, false,
                               select2, index2, true,
                               diff_index);
                } else {
                    assert (_input2[index0] == 0);
                    // ----------------------------------------
                    // this means a line in the final version
                    // still exists in the later version,
                    // but is deleted in the earlier version.
                    // do not print anything from the earlier 
                    // version.
                    // ----------------------------------------
                    write_line(os,
                               select0, index0, true,
                               select1, index1, true,
                               select2, index2, false,
                               diff_index);
                }
            } else if (_input1[index0] == 0) {
                // ----------------------------------------
                // a line is deleted in the later version.
                // it must be also deleted in the earlier version.
                // leave both earlier and later version
                // blank and only print the line in final version.
                // ----------------------------------------
                assert(_input2[index0] == 0);
                write_line(os,
                           select0, index0, true,
                           select1, index1, false,
                           select2, index2, false,
                           diff_index);
            } else if (_input1[index0] > index1) {
                if (0) {
                } else if (_input2[index0] == index2) {
                    // ----------------------------------------
                    // a line in the earlier version matches
                    // a line in the final version, but
                    // there are still lines in the later version
                    // to be printed.
                    // ----------------------------------------
                    write_line(os,
                               select0, index0, false,
                               select1, index1, true,
                               select2, index2, false,
                               diff_index);
                } else if (_input2[index0] >  index2) {
                    // ----------------------------------------
                    // most difficult case, cause 
                    // a line in the earlier and later version
                    // are deleted in the final version.
                    // ----------------------------------------
                    write_line(os,
                               select0, index0, false,
                               select1, index1, true,
                               select2, index2, true,
                               diff_index);
                } else if (_input2[index0] == 0) {
                    write_line(os,
                               select0, index0, false,
                               select1, index1, true,
                               select2, index2, false,
                               diff_index);
                }
            }

//             if (0) {
//             } else if (_input1[index0] == index1 && _input2[index0] == index2) {
//                 // ----------------------------------------
//                 // index0 == line # of latest version
//                 // index1 == line # of later version
//                 // index2 == line # of earlier version
//                 // 
//                 // umm.. all the lines are need to be produced.
//                 // ----------------------------------------
//                 write_line(os,
//                            select0, index0, true,
//                            select1, index1, true,
//                            select2, index2, true,
//                            diff_index);
//             } else if (_input1[index0] == index1 && _input2[index0] >  index2) {
//                 // ----------------------------------------
//                 // later version == last version, but something in earlier version
//                 // is deleted in the later version, so only show earlier version.
//                 // ----------------------------------------
//                 write_line(os,
//                            select0, index0, false,
//                            select1, index1, false,
//                            select2, index2, true, 
//                            diff_index);
//             } else if (_input1[index0] == index1 && _input2[index0] <  index2) {
//                 // ----------------------------------------
//                 // something is inserted in the later version
//                 // and propagated to the final version
//                 // ----------------------------------------
//                 assert(_input2[index0] == 0);
//                 write_line(os,
//                            select0, index0, true,
//                            select1, index1, true,
//                            select2, index2, false,
//                            diff_index);
//             } else if (_input1[index0] >  index1 && _input2[index0] == index2) {
//                 assert(0);
//                 write_line(os,
//                            select0, index0, false,
//                            select1, index1, true,
//                            select2, index2, false,
//                            diff_index);
//             } else if (_input1[index0] >  index1 && _input2[index0] >  index2) {
//                 // ----------------------------------------
//                 // some thing to show in both later and early 
//                 // version, but not in final version.
//                 // ----------------------------------------
//                 write_line(os,
//                            select0, index0, false,
//                            select1, index1, true,
//                            select2, index2, true,
//                            diff_index);
//             } else if (_input1[index0] >  index1 && _input2[index0] <  index2) {
//                 // ----------------------------------------
//                 // something is inserted in the later version
//                 // and propagated to the final version
//                 // ----------------------------------------
//                 write_line(os,
//                            select0, index0, false,
//                            select1, index1, true,
//                            select2, index2, false,
//                            diff_index);
//             } else if (_input1[index0] <  index1 && _input2[index0] == index2) {
//                 // ----------------------------------------
//                 // something is inserted in the later version
//                 // and propagated to the final version
//                 // ----------------------------------------
//                 write_line(os,
//                            select0, index0, true,
//                            select1, index1, false,
//                            select2, index2, true,
//                            diff_index);
//             } else if (_input1[index0] <  index1 && _input2[index0] >  index2) {
//                 // ----------------------------------------
//                 // something is inserted in the later version
//                 // and propagated to the final version
//                 // ----------------------------------------
//                 write_line(os,
//                            select0, index0, false,
//                            select1, index1, false,
//                            select2, index2, true,
//                            diff_index);
//             } else if (_input1[index0] <  index1 && _input2[index0] <  index2) {
//                 // ----------------------------------------
//                 // something is inserted in the later version
//                 // and propagated to the final version
//                 // ----------------------------------------
//                 write_line(os,
//                            select0, index0, true,
//                            select1, index1, false,
//                            select2, index2, false,
//                            diff_index);
//             }
        }
        while (*pis1 || *pis2) 
        {
            bool do1 = false, do2 = false;
            if (*pis1) {
                do1 = true;
            } 
            if (*pis2) {
                do2 = true;
            }
            write_line(os,
                       select0, index0, false,
                       select1, index1, do1,
                       select2, index2, do2,
                       diff_index);
        }
    }
    os << "</TABLE>" << endl;
    os << "<HR width=100%>" << endl;
    os << "<TABLE border=0>" << endl;
    os << "<TR><TD colspan=2>Legend:</TD></TR>" << endl;
    os << "<TR><TD class=\"n\"> </TD><TD align=center class=\"a\"> added in v."
       << _revision1 << " and propagated to v." << _revision0 << "</TD></TR>" << endl;
    os << "<TR><TD colspan=2 align=center class=\"c\"> changed lines from v." << _revision2 << " to v." 
       << _revision1 << " and propagated to v." << _revision0 << "</TD></TR>" << endl;
    os << "<TR><TD align=center class=\"d\"> removed in v." << _revision1 << "</TD><TD class=\"n\"> </TD></TR>" << endl;
    os << "</TABLE>" << endl;

    return os;
}

ostream &
html_comparer::style(ostream & os) const
{
    os << "<STYLE TYPE-\"type/css\">" << endl;
    os << "BODY  {background-color:#EEEEEE;}" << endl;
    os << "TABLE {background-color:#FFFFFF;}" << endl;
    os << "TD    {font-family:fixed;white-space:pre; nowrap:1; overflow:hidden}" << endl;
    os << ".s {background-color:yellow;}" << endl;
    os << ".a {background-color:#CCCCFF;}" << endl;
    os << ".c {background-color:#99FF99;}" << endl;
    os << ".d {background-color:#FF9999;}" << endl;
    os << ".n {background-color:#CCCCCC;}" << endl;
    os << "</STYLE>" << endl;
    return os;
}

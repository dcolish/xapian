#ifndef __LINES_CMT_H__
#define __LINES_CMT_H__

#include "lines.h"

class lines_cmt : public lines
{
private:
    ifstream *in_comments;
    string prev_file;

    int current_offset;

    vector<string> files;
    vector<string> offsets;
protected:
    void load_offset_file(  const string& file_offset, vector<string>& files, vector<string>& offsets );
    void readVector( const string& line, const string& field, vector<string>& field_vector );
public:
    lines_cmt( const string& p,           // path (e.g., "cvsdata/root0/src/")
               const string& sroot,       // ?????????
               const string& pkg,         // package 
               const string& file_db,     // name of cmt file
               const string& file_offset, // name of offset file 
               const string& mes);
    
    ~lines_cmt() ;
    bool readNextLine();
};

#endif

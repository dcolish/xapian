#include <string>
#include <map>

#include <fstream>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <htmlparse.h>

#include <om/om.h>

// FIXME: these 2 copied from om/indexer/index_utils.cc
void lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while(i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

// Keep only the characters in keep
// FIXME - make this accept character ranges in "keep"
void select_characters(om_termname &term, const string & keep)
{
    string chars;
    if(keep.size() == 0) {
	chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    } else {
	chars = keep;
    }
    string::size_type pos;
    while((pos = term.find_first_not_of(chars)) != string::npos)
    {
	string::size_type endpos = term.find_first_of(chars, pos);
	term.erase(pos, endpos - pos);
    }
}


static OmWritableDatabase *db;

class MyHtmlParser : public HtmlParser {
    public:
    	string dump;
	void process_text(const string &text);
//	void opening_tag(const string &tag, const map<string,string> &p);
//	void closing_tag(const string &tag);
};

void
MyHtmlParser::process_text(const string &text)
{
    // some tags are meaningful mid-word so this is simplistic at best...
    dump += text + " ";
}

static string root = "/home/httpd/html/open.muscat.com";

static void
index_file(const string &file)
{
    std::ifstream in(file.c_str());
    if (!in) {
	cout << "Can't open \"" << file << "\" - skipping\n";
	return;
    }

    cout << "Indexing \"" << file << "\"\n";

    string text;   
    while (!in.eof()) {
	string line;
	getline(in, line);
	text += line;
    }
    in.close();

    MyHtmlParser p;
    OmStem stemmer("english");    

    p.parse_html(text);

    string dump = p.dump;

    // replace newlines with spaces
    size_t i = 0;    
    while ((i = dump.find("\n", i)) != string::npos) dump[i] = ' ';

    // Make the document
    OmDocumentContents newdocument;

    // Put the data in the document
    newdocument.data = string("url=file:") + file + "\n"
	+ string("sample=") + dump;

    size_t j;
    j = 0;
    int pos = 1;
    while ((i = dump.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				   "abcdefghijklmnopqrstuvwxyz", j))
	    != string::npos) {
	
	j = dump.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				   "abcdefghijklmnopqrstuvwxyz"
				   "0123456789", i);
	om_termname term = dump.substr(i, j - i);
	lowercase_term(term);
        term = stemmer.stem_word(term);
	newdocument.add_posting(term, pos++);
	i = j + 1;
    }

    // Add the document to the database
    db->add_document(newdocument);
}

static void
index_directory(const string &dir)
{
    DIR *d;
    struct dirent *ent;
    d = opendir(dir.c_str());
    if (d == NULL) {
	cout << "Can't open \"" << dir << "\" - skipping\n";
	return;
    }
    while ((ent = readdir(d)) != NULL) {
	struct stat statbuf;
	// ".", "..", and other special files
	if (ent->d_name[0] == '.') continue;
	string file = dir + '/' + ent->d_name;
	if (stat(file.c_str(), &statbuf) == -1) {
	    cout << "Can't stat \"" << file << "\" - skipping\n";
	    continue;
	}
	if (S_ISDIR(statbuf.st_mode)) {
	    index_directory(file);
	    continue;
	}
	if (S_ISREG(statbuf.st_mode)) {
	    if (file.substr(file.size() - 5) == ".html")
		index_file(file);
	    continue;
	}
	cout << "Not a regular file \"" << file << "\" - skipping\n";
    }
    closedir(d);
}

int main() {
    vector<string> parameters;
    parameters.push_back("/usr/om/data/default");
    db = new OmWritableDatabase("sleepycat", parameters);
    index_directory(root);
    delete db;
    return 0;   
}

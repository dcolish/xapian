#include <string>

class Stem {
    private:
    public:
        virtual string stem_word(const string &) = 0;
        virtual ~Stem() = 0;
};

class StemEn {
    private:
    public:
        string stem_word(const string &);
};

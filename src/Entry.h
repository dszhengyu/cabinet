#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <fstream>
using std::string;
using std::ofstream;
using std::ifstream;

class Entry
{   
public:
    Entry();
    explicit Entry(string content);
    Entry(long index, long term, string content);

    const long getIndex() const {return this->index;}
    void setIndex(long index) {this->index = index;}

    const long getTerm() const {return this->term;}
    void setTerm(long term) {this->term = term;}

    const string &getContent() const {return this->content;}
    void setContent(const string &content) {this->content = content;}

    const static char delimiter = '\r';
private:
    long index;
    long term;
    string content;
};

ifstream &operator>>(ifstream &in, Entry &entry);
ofstream &operator<<(ofstream &out, const Entry &entry);

#endif

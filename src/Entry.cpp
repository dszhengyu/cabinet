#include "Entry.h"
#include "Const.h"
#include "Log.h"
using std::getline;

Entry::Entry():
    Entry(0, 0, string())
{
}

Entry::Entry(string content):
    Entry(0, 0, content)
{
}

Entry::Entry(long index, long term, string content):
    index(index),
    term(term),
    content(content)
{
}

ifstream &operator>>(ifstream &in, Entry &entry) {    
    string indexStr;
    string termStr;
    string contentStr;
    getline(in, indexStr, Entry::delimiter);
    if (!in.good()) {
        return in;
    }
    getline(in, termStr, Entry::delimiter);
    if (!in.good()) {
        logWarning("pf file is broken");
        return in;
    }
    getline(in, contentStr, Entry::delimiter);
    if (!in.good()) {
        logWarning("pf file is broken");
        return in;
    }
    try {
        entry.setIndex(std::stoi(indexStr));
        entry.setTerm(std::stoi(termStr));
    } catch (std::exception &e) {
        logFatal("read entry from ifstream fail, receive exception, what[%s]", e.what());
        return in;
    }
    entry.setContent(contentStr);
    return in;
}

ofstream &operator<<(ofstream &out, const Entry &entry) {
    out << entry.getIndex() << Entry::delimiter;
    out << entry.getTerm() << Entry::delimiter;
    out << entry.getContent() << Entry::delimiter;
    out.flush();
    return out;
}

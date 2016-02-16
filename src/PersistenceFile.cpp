#include "PersistenceFile.h"
#include "Const.h"
using std::getline;

PersistenceFile::PersistenceFile():
    curPFEntry(),
    pFName(PersistenceFile::serverPFName),
    pFIn(),
    pFOut(pFName, ofstream::app)
{
}

int PersistenceFile::appendToPF(Client *client) {
    const string &curCommandBuf = client->getCurCommandBuf();
    pFOut << curCommandBuf << PersistenceFile::delimiter;
    pFOut.flush();
    return CABINET_OK;
}

int PersistenceFile::initReadPF() {
    this->pFIn.open(pFName);
    if (this->pFIn.good()) {
        return CABINET_OK;
    }
    return CABINET_ERR;
}

int PersistenceFile::endReadPF() {
    this->pFIn.close();
    return CABINET_OK;
}

int PersistenceFile::readNextPFEntry() {
    if (!this->pFIn.good()) {
        return CABINET_ERR;
    }
    getline(this->pFIn, this->curPFEntry, PersistenceFile::delimiter);
    if (this->curPFEntry.length() == 0) {
        return CABINET_ERR;
    }
    return CABINET_OK;
}

PersistenceFile::~PersistenceFile() {
    this->pFIn.close();
    this->pFOut.close();
}

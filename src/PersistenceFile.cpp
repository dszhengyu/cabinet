#include "PersistenceFile.h"
#include "Const.h"

PersistenceFile::PersistenceFile():
    curPFEntry(),
    pFName(PersistenceFile::serverPFName),
    pFIn(),
    pFOut(pFName, ofstream::app)
{
}

int PersistenceFile::appendToPF(const Entry &entry) {
    this->pFOut << entry;
    return CABINET_OK;
}

int PersistenceFile::initReadPF() {
    logDebug("init read pf");
    this->pFIn.open(pFName);
    if (this->pFIn.good()) {
        return CABINET_OK;
    }
    return CABINET_ERR;
}

int PersistenceFile::endReadPF() {
    logDebug("end read pf");
    this->pFIn.close();
    return CABINET_OK;
}

int PersistenceFile::readNextPFEntry() {
    logDebug("read next pf entry");
    if (!this->pFIn.good()) {
        return CABINET_ERR;
    }
    this->pFIn >> this->curPFEntry;
    if (!this->pFIn.good()) {
        return CABINET_ERR;
    }
    return CABINET_OK;
}

PersistenceFile::~PersistenceFile() {
    this->pFIn.close();
    this->pFOut.close();
}

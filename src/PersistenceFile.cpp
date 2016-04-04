#include "PersistenceFile.h"
#include "Const.h"
#include <cstdio>

PersistenceFile::PersistenceFile(const string &pfName, const string &tmpPFName):
    pFName(pfName),
    tmpPFName(tmpPFName),
    pFOut(pFName, ofstream::app),
    pFIn(pFName)
{
}

int PersistenceFile::appendToPF(const Entry &entry) {
    logDebug("append entry to pf[%s]", this->pFName.c_str());
    this->pFOut << entry;
    return CABINET_OK;
}

int PersistenceFile::findEntry(const long index, Entry &entry) {
    logDebug("find entry in pf[%s], index[%ld]", this->pFName.c_str(), index);
    this->resetFileStream();
    while (this->getNextPFEntry(entry) != CABINET_ERR) {
        if (entry.getIndex() == index) {
            return CABINET_OK;
        }
    }
    return CABINET_ERR;
}

int PersistenceFile::findLastEntry(Entry &entry) {
    logDebug("find last entry in pf[%s]", this->pFName.c_str());
    if (this->resetFileStream() == CABINET_ERR) {
        logNotice("empty pf[%s]", this->pFName.c_str());
        return CABINET_ERR;
    }
    while (this->getNextPFEntry(entry) != CABINET_ERR) {

    }
    return CABINET_OK;
}

/*
 *brief: 1. open tmp file
 *      2. get entry, put into tmp file
 *      3. mv tmp file to pf file
 *      4. reopen in and out file object
 */
int PersistenceFile::deleteEntryAfter(long index) {
    logDebug("delete entry and after in pf[%s], index[%ld]", this->pFName.c_str(), index);
    if (this->resetFileStream() == CABINET_ERR) {
        logWarning("delete entry after index, but pf[%s] is empty", this->pFName.c_str());
        return CABINET_OK;
    }

    ofstream tmpStream(this->tmpPFName);
    Entry tmpEntry;
    while (this->getNextPFEntry(tmpEntry) != CABINET_ERR) {
        if (tmpEntry.getIndex() == index) {
            break;
        }
        tmpStream << tmpEntry;
    }
    if (tmpEntry.getIndex() != index) {
        logWarning("delete entry in pf[%s] after index but index not found. index[%ld]", this->pFName.c_str(), index);
    }

    remove(this->pFName.c_str());
    rename(this->tmpPFName.c_str(), this->pFName.c_str());

    this->pFOut.open(this->pFName, ofstream::app);
    this->pFIn.open(this->pFName);
    return CABINET_OK;
}

int PersistenceFile::getNextPFEntry(Entry &entry) {
    logDebug("get next pf[%s] entry", this->pFName.c_str());
    this->pFIn >> entry;
    if (!this->pFIn.good()) {
        return CABINET_ERR;
    }
    return CABINET_OK;
}

int PersistenceFile::resetFileStream() {
    logDebug("reset file stream");
    //this->pFIn.seekg(0, this->pFIn.beg);
    this->pFIn.close();
    this->pFIn.open(this->pFName);
    if (!this->pFIn.good()) {
        if (this->pFIn.fail()) {
            logDebug("reset file stream error[fail]");
        }
        if (this->pFIn.eof()) {
            logDebug("reset file stream error[eof]");
        }
        if (this->pFIn.bad()) {
            logDebug("reset file stream error[bad]");
        }
        return CABINET_ERR;
    }
    return CABINET_OK;
}

PersistenceFile::~PersistenceFile() {
    this->pFIn.close();
    this->pFOut.close();
}

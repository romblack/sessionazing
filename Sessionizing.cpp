/* Sessionizing.cpp */
#include "Sessionizing.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::endl;

// init the Sessionizing singleton
Sessionizing* Sessionizing::s_sessionizing = nullptr;

// singleton get instance
Sessionizing* Sessionizing::GetInstance() {
    if(s_sessionizing == nullptr){
        s_sessionizing = new Sessionizing();
    }
    return s_sessionizing;
}

Sessionizing::Sessionizing()
{
}

Sessionizing::~Sessionizing() {
    // free all memory allocations
    for(auto const& visitor : m_visitorsDatabase) {
        delete visitor.second;
    }
    for(auto const& site : m_sitesDatabase) {
        delete site.second;
    }
}

STATUS Sessionizing::init() {
    STATUS retVal = OK;
    
    retVal = handleAllEntries();
    // error-code handling
    retVal = finalizeInit();
    // error-code handling
    return retVal;
}


void Sessionizing::handleInputCommand(string const & inputCommand) {
    if(inputCommand == INIT_COMMAND) {
        cout << "Running init" << endl;
        Sessionizing::GetInstance()->init();
    }
    else {
        readCsvFile(inputCommand);
    }
    return;
}

void Sessionizing::readCsvFile(string const & inputCommand) {
    std::ifstream f;
    f.open(inputCommand);
    if (! f.is_open()) {    /* validate file open for reading */
            std::cerr << "error: file open failed - " << inputCommand << endl;
            return;
    }
    
    string line, val;                  /* string for line & value */
    vector<vector<uint32_t> > array;   /* vector of vector<int>  */

    while(std::getline(f, line)) {        /* read each line */
        vector<uint32_t> v;                 /* row vector v */
        std::stringstream s (line);         /* stringstream line */
        while (getline (s, val, ','))       /* get each value (',' delimited) */
            v.push_back(filterDigits(val));  /* add to row vector */
        array.push_back(v);                /* add row vector to array */
    }
    m_inputEntries.push_back(array);

    // add this input file to priority_queue
    m_currentLines.push_back(0);
    uint32_t currentLine = m_currentLines.size() -1;
    insertToPrioQueue(currentLine);

    return;
}

uint32_t Sessionizing::filterDigits(string const & input) {  
    auto pos = input.find_first_of("0123456789");
    uint32_t output = 0;

    try {
        if (string::npos != pos) {
            output = std::stoul(input.substr(pos));
        }
    }
    catch (...) {
        cout << "failed converting " << input << endl;
    }

    return output;
}
// priority_queue optimizes logic efficiency
void Sessionizing::insertToPrioQueue(uint32_t const column) {
    // check if input vector did not reach its end
    if(m_currentLines[column] < m_inputEntries[column].size()) {
        // adding the next element's timestamp to queue
        uint32_t timestamp = m_inputEntries[column][m_currentLines[column]][TIMESTAMP];
        m_pQueue.push(std::make_pair(timestamp, column));
    }
    return;
}

void Sessionizing::handleEntry(uint32_t const column) {
    // verifying entry is legal
    if(m_currentLines[column] < m_inputEntries[column].size()) {
        // handling the entry
        ParsedEntryRow parsedData;
        parsedData.visitor = m_inputEntries[column][m_currentLines[column]][VISITOR];
        parsedData.site = m_inputEntries[column][m_currentLines[column]][SITE];
        parsedData.timestamp = m_inputEntries[column][m_currentLines[column]][TIMESTAMP];
        
        STATUS retVal = updateVisitorActivity(parsedData);
    }
    return;
}

STATUS Sessionizing::handleAllEntries() {
    while(m_pQueue.empty() == false) {
        uint32_t pos = m_pQueue.top().second;   // get min value in queue
        m_pQueue.pop();              // remove from queue
        handleEntry(pos);            // handle the corresponding entry data
        m_currentLines[pos] += 1;    // increment colums position
        insertToPrioQueue(pos);      // push next entry to queue
    }
}

void Sessionizing::handleInputQuery(string const & inputCommand) {
    string inputValue = "";
    if(inputCommand == NUM_SESSIONS_QUERY) {
        cout << "which site?" << endl;
        cin >> inputValue;
        Sessionizing::GetInstance()->getNumOfSessions(filterDigits(inputValue));
    }
    else if(inputCommand == MEDIAN_SESSIONS_QUERY) {
        cout << "which site?" << endl;
        cin >> inputValue;
        Sessionizing::GetInstance()->getSessionsMedian(filterDigits(inputValue));
    }
    else if(inputCommand == UNIQUE_SITES_QUERY) {
        cout << "which visitor?" << endl;
        cin >> inputValue;
        Sessionizing::GetInstance()->getUniqueSites(filterDigits(inputValue));
    }
    else{
        cout << "unrecognized query" << endl;
    }
    return;
}
// calls site instance for getNumOfSessions
void Sessionizing::getNumOfSessions(uint32_t const siteUrl) {
    if(m_sitesDatabase.count(siteUrl) > 0) {
        cout << m_sitesDatabase[siteUrl]->getNumOfSessions() << " session in this site" << endl;
    }
    else {
        cout << "unrecognized site" << endl;
    }
}
// calls site instance for getSessionsMedian
void Sessionizing::getSessionsMedian(uint32_t const siteUrl) {
    if(m_sitesDatabase.count(siteUrl) > 0) {
        double median = m_sitesDatabase[siteUrl]->getSessionsMedian();
        cout << median << " seconds median length" << endl;
    }
    else {
        cout << "unrecognized site" << endl;
    }
}
// calls visitor instance for getUniqueSites
void Sessionizing::getUniqueSites(uint32_t const visitor) {
    if(m_visitorsDatabase.count(visitor) > 0) {
        cout << m_visitorsDatabase[visitor]->getNumUniqueSites() << " unique sites" << endl;
    }
    else {
        cout << "unrecognized visitor" << endl;
    }
}
// sends entry data to visitor instance
STATUS Sessionizing::updateVisitorActivity(ParsedEntryRow const parsedData) {
    uint32_t visitorId = parsedData.visitor;
    VisitorData* visitorData;
    // check if the visitor exists in DB
    if(m_visitorsDatabase.count(visitorId) > 0) {
        visitorData = m_visitorsDatabase[visitorId];
    }
    else {     // adding new visitor to DB
        try {
            visitorData = new VisitorData;
        }
        catch (...) {
            cout << "failed creating new visitor" << endl;
            return ERROR;
        }
        m_visitorsDatabase[visitorId] = visitorData;
    }

    STATUS retVal = visitorData->updateSession(parsedData);
    
    return retVal;
}
// sends session data to site instance
STATUS Sessionizing::updateSessionStats(uint32_t const siteUrl, uint32_t const sessionLength) {
    STATUS retVal = OK;
    // check if this site exists in DB
    if(m_sitesDatabase.count(siteUrl) > 0) {
        retVal = m_sitesDatabase[siteUrl]->updateSessionStats(sessionLength);
        // error-code handling
    }
    else { // add this site to DB
        SiteData* siteData = nullptr;
        try {
            siteData = new SiteData;
        }
        catch (...) {
            cout << "failed creating new Site" << endl;
            return ERROR;
        }
        m_sitesDatabase[siteUrl] = siteData;
        retVal = m_sitesDatabase[siteUrl]->updateSessionStats(sessionLength);
    }
    
    return retVal;
}

STATUS Sessionizing::finalizeInit() {
    STATUS retVal = OK;
    // fianlize all sessions for every visitor
    for(auto const& visitor : m_visitorsDatabase) {
        retVal = visitor.second->finalizeAllSessions();
        // error-code handling
    }
    // finalize count and calculate median for all sites
    for(auto const& site : m_sitesDatabase) {
        retVal = site.second->calculateSessionMedian();
        // error-code handling
    }
    return retVal;
}

/* VisitorData class functions */
VisitorData::VisitorData() : uniqueSitesCounter(0)
{
}

VisitorData::~VisitorData() {
    // free all memory allocations
    for(auto const& site : sitesData) {
        delete site.second;
    }
}

STATUS VisitorData::updateSession(ParsedEntryRow const parsedData) {
    uint32_t siteUrl = parsedData.site;
    STATUS retVal = OK;
    // check if this site exists in this visitor's DB
    if(sitesData.count(siteUrl) > 0) {
        Timestamps* timestamps = sitesData[siteUrl];
        // check if a new session begins
        if(timestamps->lastLogin + SESSION_TIMEOUT < parsedData.timestamp) {
            uint32_t sessionLength = timestamps->lastLogin - timestamps->sessionOpen;
            retVal = sessionEnded(parsedData.site, sessionLength); // update session stats
            // set values of new session
            timestamps->sessionOpen = parsedData.timestamp;
            timestamps->lastLogin = parsedData.timestamp;
        }
        else {  // same session continues
            timestamps->lastLogin = parsedData.timestamp;
        }
    }
    else {      // adding this site to visitor's DB
        Timestamps* timestamps = nullptr;
        try {
            timestamps = new Timestamps;
        }
        catch (...) {
            cout << "failed creating new Timestamps" << endl;
            return ERROR;
        }
        uniqueSitesCounter++;   // increase unique sites counter
        timestamps->sessionOpen = parsedData.timestamp;
        timestamps->lastLogin = parsedData.timestamp;
        sitesData[siteUrl] = timestamps;
    }
    
    return retVal;
}
// sends session data to site via sessioning singleton
STATUS VisitorData::sessionEnded(uint32_t const siteUrl, uint32_t const sessionLength) {
    return Sessionizing::GetInstance()->updateSessionStats(siteUrl, sessionLength);
}

STATUS VisitorData::finalizeAllSessions() {
    STATUS retVal = OK;
    // iterate through and close all sessions and update stats
    for(auto const& siteData : sitesData) {
        Timestamps* timestamps = siteData.second;
        uint32_t sessionLength = timestamps->lastLogin - timestamps->sessionOpen;
        retVal = sessionEnded(siteData.first, sessionLength); 
        // handling of error-code in retVal
    }
    return retVal;
}

/* SiteData class functions */
SiteData::SiteData() : m_sessionsMedian(0), m_sessionsCounter(0)
{
}

SiteData::~SiteData()
{
}

STATUS SiteData::updateSessionStats(uint32_t const sessionLength) {
    m_sessionsCounter++;    // increase sessions counter
    m_sessionsTime.push_back(sessionLength);    // log session length
    
    return OK;
}

STATUS SiteData::calculateSessionMedian() {
    // basic median calculation
    std::sort(m_sessionsTime.begin(), m_sessionsTime.end());
    m_sessionsMedian = ( m_sessionsTime[m_sessionsTime.size() /2]
                     + m_sessionsTime[(m_sessionsTime.size() -1)/2]);
    m_sessionsMedian /= 2.0;
    
    return OK;
}


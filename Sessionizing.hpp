/* Sessionizing.hpp */

#include "structs.hpp"
#include <iostream>
#include <vector>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>

using std::string;
using std::vector;

class VisitorData;
class SiteData;

const string INIT_COMMAND = "init";
const string NUM_SESSIONS_QUERY = "num_sessions";
const string MEDIAN_SESSIONS_QUERY = "median_session_length";
const string UNIQUE_SITES_QUERY = "num_unique_visited_sites";
// main singleton class
class Sessionizing {
public:
    static Sessionizing *GetInstance();
    STATUS updateSessionStats(uint32_t const siteUrl, uint32_t const sessionLength);
    void readCsvFile(string const & inputCommand);
    STATUS init();
    void handleInputCommand(string const & inputCommand);
    void handleInputQuery(string const & inputCommand);

private:
    Sessionizing();
    virtual ~Sessionizing();
    STATUS updateVisitorActivity(ParsedEntryRow const parsedData);
    STATUS handleAllEntries();
    STATUS finalizeInit(); 
    void getNumOfSessions(uint32_t const siteUrl);
    void getSessionsMedian(uint32_t const siteUrl);
    void getUniqueSites(uint32_t const visitor);
    uint32_t filterDigits(string const & input);
    void insertToPrioQueue(uint32_t const column);
    void handleEntry(uint32_t const column);
    // members
    static Sessionizing* s_sessionizing;
    std::unordered_map<uint32_t,VisitorData*> m_visitorsDatabase;
    std::unordered_map<uint32_t,SiteData*> m_sitesDatabase;
    vector< vector< vector< uint32_t > > > m_inputEntries;
    vector<uint32_t> m_currentLines;
    std::priority_queue<myPair, vector<myPair >, std::greater<myPair > > m_pQueue;
};

// holds data and functions, created for each visitor seperately
class VisitorData {
public:
    VisitorData();
    virtual ~VisitorData();
    uint32_t getNumUniqueSites() { return uniqueSitesCounter; };
    STATUS updateSession(ParsedEntryRow const parsedData);
    STATUS finalizeAllSessions();
private:
    STATUS sessionEnded(uint32_t const siteUrl, uint32_t const sessionLength);
    std::unordered_map<uint32_t,Timestamps*> sitesData;
    uint32_t uniqueSitesCounter;
};

// holds data and functions, created for each site seperately
class SiteData {
public:
    SiteData();
    virtual ~SiteData();
    uint32_t getNumOfSessions() { return m_sessionsCounter; };
    double getSessionsMedian() { return m_sessionsMedian; };
    STATUS updateSessionStats(uint32_t const sessionLength);
    STATUS calculateSessionMedian();
private:
    vector<uint32_t> m_sessionsTime;
    double m_sessionsMedian;
    uint32_t m_sessionsCounter;
};



/* structs.hpp */
#include <iostream>

enum STATUS {
	OK = 0,
	ERROR = 1
};

enum COLUMN {
	VISITOR = 0,
	SITE = 1,
	PAGE = 2,
	TIMESTAMP = 3
};

const uint32_t SESSION_TIMEOUT = 1800; // 30 minutes in seconds

typedef std::pair<uint32_t, uint32_t> myPair;

typedef struct {
    	uint32_t visitor;
    	uint32_t site;
    	uint32_t timestamp;
} ParsedEntryRow;

typedef struct {
    	uint32_t sessionOpen;
    	uint32_t lastLogin;
} Timestamps;


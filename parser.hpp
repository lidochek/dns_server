
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace parser {

    constexpr size_t QUESTION_OFFSET = 12;


    enum class KindQuery {
        QUERY = 0,
        IQUERY = 1,
        STATUS = 2
    };
    enum class QueryType {
        UNKNOWN     = 0,
        A           = 1,
        NS          = 2,
        MD          = 3,
        MF          = 4,
        CNAME       = 5,
        SOA         = 6,
        MB          = 7,
        MG          = 8,
        MR          = 9,
        NULL_TYPE   = 10,
        WKS         = 11,
        PTR         = 12,
        HINFO       = 13,
        MINFO       = 14,
        MX          = 15,
        TXT         = 16,
    };
    enum class QueryClass {
        UNKNOWN     = 0,
        IN          = 1,
        CH          = 2,
        HS          = 3,
        NONE        = 254,
        ANY         = 255,
    };
    
    struct Header {
        uint64_t id;
        bool qr;
        KindQuery opcode;
        bool authoritative_answer;
        bool trun_cation;
        bool recursion_desired;
        bool recursion_available;
        uint64_t response_code;
        uint64_t question_count;
        uint64_t answer_count;
        uint64_t name_service_count;
        uint64_t additional_count;
    };
    
    struct Question {
        std::vector<std::string> domain;
        QueryType q_type;
        QueryClass q_class;
    };

    struct Resource {
        std::string name;
        QueryType r_type;
        QueryClass r_class;
        uint64_t ttl;
        uint64_t r_length;
        std::string r_data;
    };
    
    Header ParseHeader(unsigned char* buffer);
    std::pair<std::vector<Question>, size_t> ParseQuestion(unsigned char* buffer, uint64_t count);
    std::pair<std::vector<Resource>, size_t> ParseResource(unsigned char* buffer, uint64_t count);
    
    std::ostream& operator<< (std::ostream& os, const Header& header);
    std::ostream& operator<< (std::ostream& os, const Question& header);
    std::ostream& operator<< (std::ostream& os, const Resource& header);
}

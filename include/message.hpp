#ifndef MESSAGE
#define MESSAGE

#include <iostream>

#include "../json/json11.hpp"

class message {
   public:
    static const char SPACE = 0x20;
    static const char NUL = 0x0;
    static const char CR = 0xd;
    static const char LF = 0xa;

    message(std::string str, bool debug = false);
    void parse();
    std::string asJson();
    void print_all();

    std::string msg,
        prefix, server, nick, user, host,
        command, trailing,
        crlf;
    std::vector<std::string> middle;

   private:
    bool debug;
    std::string unparsed_msg;
};
#endif
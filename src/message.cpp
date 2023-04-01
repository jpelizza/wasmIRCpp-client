#include "../include/message.hpp"

message::message(std::string str, bool debug) {
    unparsed_msg = str;
    this->debug = debug;
    parse();
}

void message::print_all() {
    if (!debug) return;
    std::cout << "\\\\\\\\\\\\\\\\\\\\\\" << std::endl;
    std::cout << "\\ unparsed_message: " << unparsed_msg << std::endl;
    std::cout << "\\ prefix:           " << prefix << std::endl;
    std::cout << "\\ server:           " << server << std::endl;
    std::cout << "\\ nick:             " << nick << std::endl;
    std::cout << "\\ user:             " << user << std::endl;
    std::cout << "\\ host:             " << host << std::endl;
    std::cout << "\\ command:          " << command << std::endl;
    unsigned int auxCounter = 0;
    for (auto it = middle.begin(); it != middle.end(); it++) {
        std::cout << "\\ middle[" << auxCounter++ << "]: " << (*it) << std::endl;
    }
    std::cout << "\\ trailing:         " << trailing << std::endl;
    std::cout << "\\\\\\\\\\\\\\\\\\\\\\" << std::endl;
}

// [ OPTIONAL ]
//  <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
//  <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
//  <command>  ::= <letter> { <letter> } | <number> <number> <number>
//  <SPACE>    ::= ' ' { ' ' }
//  <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
//
//  <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
//                 or NUL or CR or LF, the first of which may not be ':'>
//  <trailing> ::= <Any, possibly *empty*, sequence of octets not including
//                   NUL or CR or LF>
//
//  <crlf>     ::= CR LF
//

void message::parse() {
    msg = unparsed_msg;
    // optional [':' <prefix> <SPACE> ]
    if (msg[0] == ':') {
        prefix = msg.substr(1, msg.find_first_of(SPACE) - 1);
        server = prefix;
        if (prefix.find('!') != std::string::npos) {
            nick = prefix.substr(0, prefix.find_first_of('!'));
        }
        if (prefix.find('!') != std::string::npos) {
            size_t nickSize = prefix.substr(0, prefix.find_first_of('!')).size() + 1;
            size_t domainSize = prefix.substr(prefix.find_first_of('@') + 1).size() + 1;
            user = prefix.substr(prefix.find_first_of('!') + 1, prefix.size() - nickSize - domainSize);
        }
        if (prefix.find('@') != std::string::npos) {
            host = prefix.substr(prefix.find_first_of('@') + 1);
        }
        // get remaining // consume
        msg = msg.substr(msg.find_first_of(SPACE) + 1);
        // consume spaces
        while (msg[0] == SPACE) msg = msg.substr(1);
    }
    // <command> <params> <crlf>
    // obligatory <command>
    if (std::isalpha(msg[0])) {
        //<letter> { <letter> }
        while (msg[0] != SPACE) {
            command += msg[0];
            msg = msg.substr(1);
        }
    } else {
        //<number> <number> <number>
        command = msg.substr(0, 3);
        msg = msg.substr(3);
    }
    // obligatory <params>
    // <SPACE> [ ':' <trailing> | <middle> <params> ]
    // consume spaces
    // start params
    int aux = 0;
    while (msg.size() > 0) {
        while (msg[0] == SPACE) msg = msg.substr(1);
        //':' <trailing>
        if (msg[0] == ':') {
            trailing = msg.substr(1);
            break;
        }
        //<middle> <params>
        else {
            middle.push_back(msg.substr(0, msg.find_first_of(SPACE)));
            msg = msg.substr(msg.find_first_of(SPACE));
        }
    }
    if (debug) print_all();
}

std::string message::asJson() {
    json11::Json::array mids;

    for (auto it = middle.begin(); it != middle.end(); it++) {
        mids.push_back((*it));
    }

    json11::Json retJson = json11::Json::object{
        {"server", server},
        {"nick", nick},
        {"user", user},
        {"host", host},
        {"command", command},
        {"middle", mids},
        {"trailing", trailing}};
    return retJson.dump();
}

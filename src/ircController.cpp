#include "../include/ircController.hpp"

ircController *ircC;
int ircController::websocket;

/**
 * @brief Construct a new irc Controller::irc Controller object
 * @note exported
 * @param debug
 */
ircController::ircController(bool debug) {
    ircC = this;
    this->debug = debug;
};

/**
 * @brief Return websocket value, if value is 0 then there is no connection
 * @note exported
 * @return int
 */
int ircController::getWebsocketConnection() {
    return websocket;
}

/**
 * @brief returns connected channels as array
 * @note exported
 * @return std::vector<std::string>
 */
std::vector<std::string> ircController::getChannels() {
    return channels;
}

/**
 * @brief set debug flag, if on then messages will be printed on console
 * @note exported
 * @param _debug
 */
void ircController::setDebug(bool _debug) {
    debug = _debug;
}

/**
 * @brief categorizes messages received.
 * Auto replies on PING
 * if categorize is false then all messages are loaded into messages
 * else separetes based on command, if digit then infoMessages else messages
 *
 * @param msg
 */
void ircController::categorizeMsg(std::string msg) {
    message m(msg, debug);
    // if command is ping, sends pong back
    if (m.command == "PING") pong(m.trailing);

    /* If categorize flag is up, all messages will be save on messages list*/
    // if (!categorize) messages.push_back(m);

    // else will filter messages into different lists
    if (std::isdigit(m.command[0])) {
        infoMessages.push_back(m);
    } else if (m.command == "PRIVMSG") {
        messages.push_back(m);
    } else {
        std::cout << "Uncaught categorization of message" << std::endl;
    }
}

/**
 * @brief sends raw message to server
 * @note exported
 * @param msg
 */
void ircController::sendMessage(std::string msg) {
    if (debug) std::cout << "[DEBUG][sendMessage]: " << msg << std::endl;
    emscripten_websocket_send_utf8_text(websocket, msg.c_str());
}

/**
 * @brief Complete function that register user and nick on server
 * @note exported
 * @note inspircd does not use hostname and servername
 * @note only the nick has to be unique
 * @param username USER
 * @param hostname USER
 * @param servername USER
 * @param realname USER
 * @param nick NICK
 */
void ircController::registerUser(std::string username, std::string hostname, std::string servername, std::string realname, std::string nick) {
    std::string msg = "USER ";
    msg += username + " " + hostname + " " + servername + " " + realname;
    sendMessage(msg);
    msg = "NICK " + nick;
    sendMessage(msg);
}

/**
 * @brief return next message on messages vector as json
 *
 * @return std::string
 */
std::string ircController::getNextMessage() {
    std::string retMsg = "";
    if (messages.size()) {
        retMsg = messages.front().asJson();
        messages.erase(messages.begin());
    }
    return retMsg;
}

/**
 * @brief return next message on info messages vector as json
 *
 * @return std::string
 */
std::string ircController::getNextInfoMessage() {
    std::string retMsg = "";
    if (infoMessages.size()) {
        retMsg = infoMessages.front().asJson();
        infoMessages.erase(infoMessages.end());
    }
    return retMsg;
}

/**
 * @brief Requests the contact details for the administrator of the specified server.
 * If <server> is not specified then it defaults to the local server.
 *
 * @param server server to request list of admins for
 */
void ircController::admin(std::string server) {
    server = (server != "") ? "ADMIN " + server : "ADMIN";
    sendMessage(server);
}

/**
 * @brief If message is specified marks yourself as being away with the specified message.
 *
 * @param away_msg message to set reason for being away
 * @example away("Washing my hair");
 */
void ircController::away(std::string away_msg) {
    away_msg = (away_msg != "") ? "AWAY :" + away_msg : "AWAY";
    sendMessage(away_msg);
}

/**
 * @brief Lists all commands that exist on the local server.
 *
 */
void ircController::commands() {
    sendMessage("COMMANDS");
}

/**
 * @brief If <duration> and <reason> are specified then exempts an ident@host mask from being affected by other (G, K, Z, etc) X-lines.
 * The <duration> may be specified as a number of seconds or as a duration in the format 1y2w3d4h5m6s. If the duration is zero then the E-line will be permanent.
 * Otherwise, if just <ident@host> is specified, removes an exemption on an ident@host mask.
 * This command is only usable by server operators with ELINE in one of their <class> blocks.
 *
 * @param userAThost <ident@host>[,<ident@host>]+
 * @param duration duration
 * @param reason reason
 * @return true
 * @return false if userAThost is empty or duration is given without a reason or vice versa
 */
bool ircController::eline(std::vector<std::string> userAThost, std::string duration, std::string reason) {
    if (!userAThost.size() || (duration != reason && (duration != "" || reason != ""))) return false;
    std::string msg = "ELINE ";
    for (auto it = userAThost.begin(); it != userAThost.end(); it++) {
        if (it != userAThost.begin()) msg += ",";
        msg += (*it);
    }
    if (duration != "")
        msg += " " + duration + " " + reason;

    sendMessage(msg);

    return true;
}

/**
 * @brief If <duration> and <reason> are specified then bans an ident@host mask from connecting to the network.
 * The <duration> may be specified as a number of seconds or as a duration in the format 1y2w3d4h5m6s. If the duration is zero then the G-line will be permanent.
 * Otherwise, if just <ident@host> is specified, removes a network-wide ban on an ident@host mask.
 * This command is only usable by server operators with GLINE in one of their <class> blocks.
 *
 * @param userAThost <ident@host>[,<ident@host>]+
 * @param duration duration
 * @param reason reason
 * @return true
 * @return false if userAThost is empty or duration is given without a reason or vice versa
 */
bool ircController::gline(std::vector<std::string> userAThost, std::string duration, std::string reason) {
    if (!userAThost.size() || (duration != reason && (duration != "" || reason != ""))) return false;
    std::string msg = "GLINE ";
    for (auto it = userAThost.begin(); it != userAThost.end(); it++) {
        if (it != userAThost.begin()) msg += ",";
        msg += (*it);
    }
    if (duration != "")
        msg += " " + duration + " " + reason;

    sendMessage(msg);

    return true;
}

/**
 * @brief Requests information on the developers and supporters who made the creation and continued development of this IRC server possible.
 *
 */
void ircController::info() {
    sendMessage("INFO");
}

/**
 * @brief If <nick> and <channel> are specified then sends an invite to <nick> inviting them to join <channel>.
 * If <duration> is specified then the invite will expire after the specified duration passes.
 * This duration can be given as a number of seconds or as a duration in the format 1y2w3d4h5m6s.
 * Otherwise, if no parameters are specified, then lists the invites which you have been sent that haven't been acted on yet.
 *
 * @param nick
 * @param chan
 * @param duration
 * @return true
 * @return false
 */
bool ircController::invite(std::string nick, std::string chan, std::string duration) {
    if (((nick.size() && !chan.size()) || (!nick.size() && chan.size())) || ((!nick.size() && duration.size()) || (!chan.size() && duration.size()))) return false;
    std::string msg = "INVITE";
    if (nick != "") {
        msg += " " + nick + " " + chan;
        if (duration != "")
            msg += " " + duration;
    }
    sendMessage(msg);
    return true;
}

/**
 * @brief Determines whether the specified nicknames are currently connected to the network.
 *
 * @param nick
 * @return true
 * @return false if nick vector is empty
 */
bool ircController::ison(std::vector<std::string> nick) {
    if (!nick.size()) return false;
    std::string msg = "ISON ";
    for (auto it = nick.begin(); it != nick.end(); it++) {
        if (it != nick.begin()) msg += " ";
        msg += (*it);
    }
    return true;
}

/**
 * @brief Joins channel array using keys if any
 * if channel is using key, then arrays must be the same size
 * @param chans channels to join
 * @param keys keys to use
 * @return true
 * @return false if keys size is not zero and chans and keys are different sizes
 */
bool ircController::join(std::vector<std::string> chans, std::vector<std::string> keys) {
    if (keys.size() != 0 && chans.size() != keys.size()) return false;

    std::string msg = "JOIN ";

    for (auto it = chans.begin(); it != chans.end(); it++) {
        if (it != chans.begin()) msg += ",";
        msg += (*it);
        channels.push_back(*it);
    }
    msg += " ";

    for (auto it = keys.begin(); it != keys.end(); it++) {
        if (it != keys.begin()) msg += ",";
        msg += (*it);
    }
    sendMessage(msg);
    return true;
}

/**
 * @brief Kicks one or more users from the specified channel.
 * You must be at least a channel half-operator, or channel operator if that channel mode is not enabled, and must be an equal or higher rank to the user you are kicking.
 *
 * @param chan channel to kick people from
 * @param nicks vector of nicknames to kick from channel
 * @param reason reason for kicking
 * @return true
 * @return false if no channel name is provided or nicks vector is empty
 */
bool ircController::kick(std::string chan, std::vector<std::string> nicks, std::string reason) {
    if (!chan.size() || !nicks.size()) return false;
    std::string msg = "KICK " + chan + " ";

    for (auto it = nicks.begin(); it != nicks.end(); it++) {
        if (it != nicks.begin()) msg += ",";
        msg += (*it);
    }

    if (reason != "") msg += " " + reason;

    return true;
}

/**
 * @brief Forcibly disconnects one or more specified users from the network with the specified reason.
 * This command is only usable by server operators with KILL in one of their <class> blocks.
 * @param nicks vector of nicknames to kick from server
 * @param reason reason for kicking
 * @return true
 * @return false if nicks vector is empty
 */
bool ircController::kill(std::vector<std::string> nicks, std::string reason) {
    if (!nicks.size()) return false;
    std::string msg = "KILL ";

    for (auto it = nicks.begin(); it != nicks.end(); it++) {
        if (it != nicks.begin()) msg += ",";
        msg += (*it);
    }

    if (reason != "") msg += " " + reason;

    return true;
}

/**
 * @brief If <duration> and <reason> are specified then bans an ident@host mask from connecting to the local server.
 * The <duration> may be specified as a number of seconds or as a duration in the format 1y2w3d4h5m6s. If the duration is zero then the K-line will be permanent.
 * Otherwise, if just <ident@host> is specified, removes a local server ban on an ident@host mask.
 * This command is only usable by server operators with KLINE in one of their <class> blocks.
 *
 * @param userAThost <ident@host>[,<ident@host>]+
 * @param duration duration
 * @param reason reason
 * @return true
 * @return false if userAThost is empty or duration is given without a reason or vice versa
 */
bool ircController::kline(std::vector<std::string> userAThost, std::string duration, std::string reason) {
    if (!userAThost.size() || (duration != reason && (duration != "" || reason != ""))) return false;
    std::string msg = "GLINE ";
    for (auto it = userAThost.begin(); it != userAThost.end(); it++) {
        if (it != userAThost.begin()) msg += ",";
        msg += (*it);
    }
    if (duration != "")
        msg += " " + duration + " " + reason;

    sendMessage(msg);

    return true;
}

/// LIST [ (>|<)<count> | C(>|<)<minutes> | T(>|<)<minutes> | [!]<pattern>]+
/**
 * @brief Lists all channels visible to the requesting user which match the specified criteria. If no criteria is specified then all visible channels are listed.
 *
 * @return true
 * @return false
 */
bool ircController::list(std::string patterns) {
    if (!patterns.size()) return false;

    std::string msg = "LIST ";
    for (auto it = patterns.begin(); it != patterns.end(); it++) {
        if (it != patterns.begin()) msg += " ";
        msg += (*it);
    }
    sendMessage(msg);
    return true;
}

/**
 * @brief Loads the specified module on the local server.
 * This command is only usable by server operators with LOADMODULE in one of their <class> blocks.
 *
 * @param module
 * @return true
 * @return false
 */
bool ircController::loadmodule(std::string module) {
    if (!module.size()) return false;
    sendMessage("LOADMODULE " + module);
    return true;
}

/**
 * @brief Requests information about the current and total number of servers, server operators, and users.
 *
 */
void ircController::lusers() {
    sendMessage("LUSERS");
}

/**
 * @brief Changes the modes which are set on a channel or a user.
 * For a list of modes see the channel modes and user modes pages.
 *
 * @param target channel or user
 * @param modes
 * @param params
 * @return true
 * @return false
 */
/// MODE <channel>|<user> <modes> [<parameters>]+
bool ircController::mode(std::string target, std::string modes, std::vector<std::string> params) {
    if (!target.size() || !modes.size()) return false;

    std::string msg = "MODE " + target + " " + modes + " ";
    for (auto it = params.begin(); it != params.end(); it++) {
        if (it != params.begin()) msg += " ";
        msg += (*it);
    }
    sendMessage(msg);
    return true;
}

/**
 * @brief Lists all modules which are loaded on the local server.
 */
void ircController::modules() {
    sendMessage("MODULES");
}

/**
 * @brief If <server> is specified then requests the message of the day for the specified server.
 * Otherwise, requests the message of the day for the local server.
 *
 * @param server
 */
void ircController::motd(std::string server) {
    std::string msg = (!server.size()) ? "MOTD" : "MOTD " + server;
    sendMessage(msg);
}

/**
 * @brief asks server for list of nicknames on given channel array
 *
 * @param chans
 */
void ircController::names(std::vector<std::string> chans) {
    std::string msg = "NAMES ";

    for (auto it = chans.begin(); it != chans.end(); it++) {
        if (it != chans.begin()) msg += ",";
        msg += (*it);
    }
    sendMessage(msg);
}

/**
 * @brief Changes your nickname to <nick>.
 * You may also change your nick to your UUID by specifying a nickname of "0".
 *
 * @param nickname
 * @return true
 * @return false
 */
bool ircController::nick(std::string nickname) {
    if (!nickname.size()) return false;
    std::string msg = "NICK " + nickname;
    sendMessage(msg);
    return true;
}

/**
 * @brief Sends a notice to the targets specified in <target>.
 * These targets can be a channel, a user, or a server mask (requires the users/mass-message server operator privilege).
 *
 * @param targets
 * @param message
 * @return true
 * @return false
 */
bool ircController::notice(std::vector<std::string> targets, std::string message) {
    if (!targets.size() || !message.size()) return false;
    std::string msg = "NOTICE ";
    for (auto it = targets.begin(); it != targets.end(); it++) {
        if (it != targets.begin()) msg += ",";
        msg += (*it);
    }
    msg += " " + message;
    sendMessage(msg);
    return true;
}

/**
 * @brief Logs into a server operator account with the specified name and password.
 *
 * @param name
 * @param pass
 * @return true
 * @return false
 */
bool ircController::oper(std::string name, std::string pass) {
    if (!name.size() || !pass.size()) return false;
    std::string msg = "OPER " + name + " " + pass;
    sendMessage(msg);
    return true;
}

/**
 * @brief Specifies the password used to log in to the local server.
 *
 * @param pass
 * @return true
 * @return false
 */
bool ircController::pass(std::string pass) {
    if (!pass.size()) return false;
    sendMessage("PASS " + pass);
    return true;
}

/**
 * @brief parts channels array
 *
 * @param chans channels to part
 * @param reason reason for parting
 * @return true
 * @return false if channel array size is zero
 */
bool ircController::part(std::vector<std::string> chans, std::string reason) {
    if (!channels.size()) return false;
    std::string msg = "PART ";

    for (auto it = chans.begin(); it != chans.end(); it++) {
        if (it != chans.begin()) msg += ",";
        msg += (*it);
        unsigned int pos = 0;
        for (auto jt = chans.begin(); jt != chans.end(); jt++) {
            if (*it == *jt)
                channels.erase(channels.begin() + pos);
            pos++;
        }
    }
    if (reason != "")
        msg += " :" + reason;

    sendMessage(msg);
    return true;
}

/**
 * @brief Pings <server> with the specified <cookie>. If <server> is not specified then it defaults to the local server.
 *
 * @param cookie
 * @param server
 * @return true
 * @return false
 */
bool ircController::ping(std::string cookie, std::string server) {
    if (!cookie.size()) return false;
    std::string msg = (!server.size()) ? "PING " + cookie : "PING " + cookie + " " + server;
    sendMessage(msg);
    return true;
}

/**
 * @brief PONG to keep connection alive
 *
 * @param daemon server
 */
void ircController::pong(std::string daemon) {
    std::string msg = "PONG " + daemon;
    sendMessage(msg);
}

/**
 * @brief sends message to all connected channels
 *
 * @param text
 */
void ircController::privmsg(std::string text) {
    std::string msg = "PRIVMSG ";
    for (auto it = channels.begin(); it != channels.end(); it++) {
        if (it != channels.begin()) msg += ",";
        msg += (*it);
    }
    msg += " :" + text;
    sendMessage(msg);
}

/**
 * @brief If <duration> and <reason> are specified then prevents a nickname from being used. The <duration> may be specified as a number of seconds or as a duration in the format 1y2w3d4h5m6s.
 * If the duration is zero then the Q-line will be permanent.
 * Otherwise, if just <nick> is specified, removes a reservation on a nickname.
 * This command is only usable by server operators with QLINE in one of their <class> blocks.
 *
 * @param nicks <nick>[,<nick>]+
 * @param duration duration
 * @param reason reason
 * @return true
 * @return false if nicks is empty or duration is given without a reason or vice versa
 */
bool ircController::qline(std::vector<std::string> nicks, std::string duration, std::string reason) {
    if (!nicks.size() || (duration != reason && (duration != "" || reason != ""))) return false;
    std::string msg = "QLINE ";
    for (auto it = nicks.begin(); it != nicks.end(); it++) {
        if (it != nicks.begin()) msg += ",";
        msg += (*it);
    }
    if (duration != "")
        msg += " " + duration + " " + reason;

    sendMessage(msg);

    return true;
}

/**
 * @brief disconnects from server with given message as reason
 *
 * @param msg
 */
void ircController::quit(std::string msg) {
    msg = (msg != "") ? "QUIT :" + msg : "QUIT";
    sendMessage(msg);
}

/**
 * @brief Reloads the server configuration.
 * If <type> is specified then a specific module is rehashed on the local server.
 * If <server> is specified then the specified server's configuration is reloaded.
 * Otherwise, if no parameters are specified, the local server's configuration is reloaded.
 * This command is only usable by server operators with REHASH in one of their <class> blocks.
 *
 * @param serverORtype
 * @return true
 * @return false
 */
bool ircController::rehash(std::string serverORtype) {
    if (!serverORtype.size()) return false;
    sendMessage(serverORtype);
    return true;
}

/**
 * @brief Reloads the specified module on the local server.
 * This command is only usable by server operators with RELOADMODULE in one of their <class> blocks.
 *
 * @param module
 * @return true
 * @return false
 */
bool ircController::reloadmodule(std::string module) {
    if (!module.size()) return false;
    std::string msg = "RELOADMOUDLE " + module;
    sendMessage(msg);
    return true;
}

/**
 * @brief Restarts the local server. The <server> parameter MUST match the name of the local server.
 * This command is only usable by server operators with RESTART in one of their <class> blocks.
 *
 * @param server e.g. irc2.example.com
 * @return true
 * @return false
 */
bool ircController::restart(std::string server) {
    if (!server.size()) return false;
    std::string msg = "RESTART " + server;
    sendMessage(msg);
    return true;
}

/**
 * @brief List network services that are currently connected to the network and visible to you.
 * The optional glob-based nick and oper-type parameters match against the nickname of the network service and the oper type of the network service.
 *
 * @param nick
 * @param operType
 * @return true
 * @return false
 */
bool ircController::servlist(std::string nick, std::string operType) {
    if (!nick.size() && operType.size()) return false;
    std::string msg = "SERVLIST";
    if (nick != "") msg += " " + nick;
    if (operType != "") operType += " " + operType;
    return true;
}

/**
 * @brief Sends a message to the service specified in <target>. This target must be on a U-lined server.
 *
 * @param target
 * @param message
 * @return true
 * @return false
 */
bool ircController::squery(std::string target, std::string message) {
    if (!target.size() || !message.size()) return false;
    std::string msg = "SQUERY " + target + " " + message;
    sendMessage(msg);
    return true;
}

/**
 * @brief Requests the specified server statistics.
 * If <server> is specified then requests the server statistics for the specified server.
 * Otherwise, requests the server statistics for the local server.
 *
 * @param character single character
 * @param server [optional] server to request
 * @return true
 * @return false
 */
bool ircController::stats(std::string character, std::string server) {
    if (character.size() != 1) return false;
    std::string msg = (server.size() == 0) ? "STATS " + character : "STATS " + character + " " + server;
    sendMessage(msg);
    return true;
}

/**
 * @brief If <server> is specified then requests the time on the specified server.
 * Otherwise, requests the time on the local server.
 *
 * @param server
 * @return true
 * @return false
 */
bool ircController::time(std::string server) {
    std::string msg = (server.size() == 0) ? "TIME" : "TIME " + server;
    sendMessage(msg);
    return true;
}

/**
 * @brief If <newtopic> is specified then changes the topic for <channel> to the specified value.
 * Otherwise, requests the topic for the specified channel.
 *
 * @param channel
 * @param newTopic
 * @return true
 * @return false
 */
bool ircController::topic(std::string channel, std::string newTopic) {
    if (!channel.size()) return false;
    std::string msg = (newTopic.size() == 0) ? "TOPIC " + channel : "TOPIC " + channel + " " + newTopic;
    sendMessage(msg);
    return true;
}

/**
 * @brief Unloads the specified module on the local server.
 * This command is only usable by server operators with UNLOADMODULE in one of their <class> blocks.
 *
 * @param module
 * @return true
 * @return false
 */
bool ircController::unloadmodule(std::string module) {
    if (!module.size()) return false;
    std::string msg = "UNLOADMODULE " + module;
    sendMessage(msg);
    return true;
}

/**
 * @brief Specifies a username (ident) and real name (gecos) when connecting to the server.
 *
 * @param username
 * @param hostname
 * @param servername
 * @param realname
 * @return true
 * @return false
 */
bool ircController::user(std::string username, std::string hostname, std::string servername, std::string realname) {
    std::string msg = "USER " + username + " " + hostname + " " + servername + " " + realname;
    sendMessage(msg);
    return true;
}

/**
 * @brief If <server> is specified then requests the version of the specified server.
 * Otherwise, requests the version of the local serve
 *
 * @param nicks
 * @return true
 * @return false
 */
bool ircController::userhost(std::vector<std::string> nicks) {
    if (!nicks.size()) return true;
    std::string msg = "USERHOST ";
    for (auto it = nicks.begin(); it != nicks.end(); it++) {
        if (it != nicks.begin()) msg += " ";
        msg += (*it);
    }
    sendMessage(msg);
    return false;
}

/**
 * @brief Requests the hostname of the specified users.
 *
 * @param server
 * @return true
 * @return false
 */
bool version(std::string server) {
    std::string msg = (!server.size()) ? "VERSION" : "VERSION" + server;
    return true;
}

/**
 * @brief Sends a message to all users with user mode w (wallops) enabled:
 * This command is only usable by server operators with WALLOPS in one of their <class> blocks.
 *
 * @param message
 * @return true
 * @return false
 */
bool ircController::wallops(std::string message) {
    if (!message.size()) return false;
    std::string msg = "WALLOPS " + message;
    sendMessage(msg);
    return true;
}

/**
 * @brief Requests information about users who match the specified condition.
 * One or more of the following flags may be used:
 *
 * @param pattern_s
 * @param flags
 * @param fields
 * @param queryType
 * @param pattern_e
 * @return true
 * @return false
 */
bool ircController::who(std::string pattern_s, std::string flags, std::string fields, std::string queryType, std::string pattern_e) {
    if ((!pattern_s.size() || !pattern_e.size()) ||
        !fields.size() && queryType.size())
        return false;

    std::string msg = "WHO " + pattern_s;
    msg = (flags.size()) ? msg + " " + flags : msg;
    msg = (fields.size()) ? msg + "%" + fields : msg;
    msg = (queryType.size()) ? msg + "," + queryType : msg;
    msg += pattern_e;

    sendMessage(msg);
    return true;
}
/**
 * @briefRequests information about users who are currently connected with the specified nicks:
 * If the <server> parameter is specified then only one <nick> can be specified and remote information
 * will be fetched about the user if they are not on the local server.
 *
 * @param server
 * @param nicks
 * @return true
 * @return false
 */
bool ircController::whois(std::string server, std::vector<std::string> nicks) {
    if (!nicks.size()) return false;

    std::string msg = (server.size()) ? "WHOIS " + server : "WHO";
    for (auto it = nicks.begin(); it != nicks.end(); it++) {
        if (it != nicks.begin()) msg += ",";
        msg += (*it);
    }

    return true;
}

/**
 * @brief Requests information about a user who was previously connected with the specified nick.
 * If is given, only return the most recent entries.
 *
 * @param nick
 * @param count
 * @return true
 * @return false
 */
bool ircController::whowas(std::string nick, std::string count) {
    if (!nick.size()) return false;
    std::string msg = (count.size()) ? "WHOWAS " + nick + " " + count : "WHOWAS " + nick;
}

bool ircController::zline(std::vector<std::string> ipaddr, std::string duration, std::string reason) {
    if (!ipaddr.size() || (duration != reason && (duration != "" || reason != ""))) return false;
    std::string msg = "ZLINE ";
    for (auto it = ipaddr.begin(); it != ipaddr.end(); it++) {
        if (it != ipaddr.begin()) msg += ",";
        msg += (*it);
    }
    if (duration != "")
        msg += " " + duration + " " + reason;

    sendMessage(msg);

    return true;
}

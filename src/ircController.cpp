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
    if (m.command == "PING") pong("", m.trailing);

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
 * @brief PONG to keep connection alive
 *
 * @param daemon server
 */
bool ircController::pong(std::string cookie, std::string server) {
    if (cookie == "") return false;
    std::string msg = (server == "") ? "PONG " + cookie : "PONG " + cookie + " " + server;
    sendMessage(msg);
    return true;
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
 * @brief sends message to all connected channels
 *
 * @param text
 */
bool ircController::privmsg(std::string text) {
    std::string msg = "PRIVMSG ";
    for (auto it = channels.begin(); it != channels.end(); it++) {
        if (it != channels.begin()) msg += ",";
        msg += (*it);
    }
    msg += " :" + text;
    sendMessage(msg);
    return true;
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
bool ircController::admin(std::string server) {
    server = (server != "") ? "ADMIN " + server : "ADMIN";
    sendMessage(server);
    return true;
}

/**
 * @brief If message is specified marks yourself as being away with the specified message.
 *
 * @param away_msg message to set reason for being away
 * @example away("Washing my hair");
 */
bool ircController::away(std::string away_msg) {
    away_msg = (away_msg != "") ? "AWAY :" + away_msg : "AWAY";
    sendMessage(away_msg);
    return true;
}

/**
 * @brief Lists all commands that exist on the local server.
 *
 */
bool ircController::commands() {
    sendMessage("COMMANDS");
    return true;
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
bool ircController::info() {
    sendMessage("INFO");
    return true;
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
    if (((nick != "" && chan == "") || (nick == "" && chan != "")) || ((nick == "" && duration != "") || (chan == "" && duration != ""))) return false;
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
    if (chan == "" || !nicks.size()) return false;
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
bool ircController::loadModule(std::string module) {
    if (module == "") return false;
    sendMessage("LOADMODULE " + module);
    return true;
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
    if (target == "" || modes == "") return false;

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
bool ircController::modules() {
    sendMessage("MODULES");
    return true;
}

/**
 * @brief If <server> is specified then requests the message of the day for the specified server.
 * Otherwise, requests the message of the day for the local server.
 *
 * @param server
 */
bool ircController::motd(std::string server) {
    std::string msg = (server == "") ? "MOTD" : "MOTD " + server;
    sendMessage(msg);
    return true;
}

/**
 * @brief disconnects from server with given message as reason
 *
 * @param msg
 */
bool ircController::quit(std::string msg) {
    msg = (msg != "") ? "QUIT :" + msg : "QUIT";
    sendMessage(msg);
    return true;
}

/**
 * @brief Requests information about the current and total number of servers, server operators, and users.
 *
 */
bool ircController::lusers() {
    sendMessage("LUSERS");
    return true;
}

/**
 * @brief asks server for list of nicknames on given channel array
 *
 * @param chans
 */
bool ircController::names(std::vector<std::string> chans) {
    std::string msg = "NAMES ";

    for (auto it = chans.begin(); it != chans.end(); it++) {
        if (it != chans.begin()) msg += ",";
        msg += (*it);
    }
    sendMessage(msg);
    return true;
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
    if (nickname == "") return false;
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
    if (!targets.size() || message == "") return false;
    std::string msg = "NOTICE ";
    for (auto it = targets.begin(); it != targets.end(); it++) {
        if (it != targets.begin()) msg += ",";
        msg += (*it);
    }
    msg += " " + message;
    sendMessage(msg);
    return true;
}

bool ircController::oper(std::string name, std::string password) {
    if (name == "" || password == "") return false;
    std::string msg = "OPER " + name + " " + password;
    sendMessage(msg);
    return true;
}

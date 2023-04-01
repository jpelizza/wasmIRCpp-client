#ifndef IRC_CONTROLLER
#define IRC_CONTROLLER

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/websocket.h>

#include <iostream>
#include <list>
#include <map>

#include "message.hpp"

class ircController {
   private:
    std::vector<message> messages, infoMessages;
    std::vector<std::string> channels;
    bool debug, categorize;

   public:
    static int websocket;
    static std::string server;

   public:
    // exported
    //  constructor
    ircController(bool debug = true);
    // misc
    int getWebsocketConnection();
    // config
    void setDebug(bool _debug);
    // general
    void registerUser(std::string username, std::string hostname, std::string servername, std::string realname, std::string nick);
    std::string getNextMessage();
    std::string getNextInfoMessage();
    void sendMessage(std::string msg);
    std::vector<std::string> getChannels();

    // actual IRC commands
    bool away(std::string away_msg);
    bool admin(std::string server);
    bool die(std::string server);
    bool eline(std::vector<std::string> userAThost, std::string duration, std::string reason);
    bool gline(std::vector<std::string> userAThost, std::string duration, std::string reason);
    bool info();
    bool ison(std::vector<std::string> nick);
    bool join(std::vector<std::string> chans, std::vector<std::string> keys);
    bool invite(std::string nick, std::string chan, std::string duration);
    bool kick(std::string chan, std::vector<std::string> nicks, std::string reason);
    bool kill(std::vector<std::string> nicks, std::string reason);
    bool kline(std::vector<std::string> userAThost, std::string duration, std::string reason);
    bool commands();
    bool privmsg(std::string text);
    bool quit(std::string msg);
    bool part(std::vector<std::string> chans, std::string reason = "");
    bool list(std::string patterns);
    bool loadModule(std::string module);
    bool lusers();
    bool mode(std::string target, std::string modes, std::vector<std::string> params);
    bool modules();
    bool motd(std::string server);
    bool names(std::vector<std::string> chans);
    bool nick(std::string nickname);
    bool notice(std::vector<std::string> targets, std::string message);
    bool oper(std::string name, std::string password);
    // bool part(std::vector<std::string> chans, std::string reason);
    // bool pass(std::string password);
    // bool ping(std::string cookie, std::string server);
    bool pong(std::string cookie, std::string server);
    // bool qline(std::vector<std::string> nick, std::string duration, std::string reason);
    // bool quit(std::string message);
    // bool rehash(std::string srvType);
    // bool reloadmodule(std::string module);
    // bool restart(std::string server);
    // bool servlist(std::string nick, std::string operType);
    // bool squery(std::string target, std::string message);
    // bool stats(std::string character, std::string server);
    // bool time(std::string server);
    // bool topic(std::string chan, std::string newTopic);
    // bool unloadmodule(std::string module);
    // bool user(std::string username, std::string hostname, std::string servername, std::string realname);
    // bool userhost(std::vector<std::string> nicks);
    // bool version(std::string server);
    // bool wallops(std::string message);
    // bool who(std::string pattern_b, std::string flags, std::string fields, std::string queryType, std::string pattern_e);
    // bool whois(std::string server, std::vector<std::string> nicks);
    // bool whowas(std::string nick, std::string count);
    // bool zline(std::vector<std::string> ipaddr, std::string duration, std::string reason);

    // void commands();

    // not exported
    void categorizeMsg(std::string msg);
};

#endif  // ircController

// Translates JS arrays into std::vectors and vice versa
namespace emscripten {
namespace internal {

template <typename T, typename Allocator>
struct BindingType<std::vector<T, Allocator>> {
    using ValBinding = BindingType<val>;
    using WireType = ValBinding::WireType;

    static WireType toWireType(const std::vector<T, Allocator> &vec) {
        return ValBinding::toWireType(val::array(vec));
    }

    static std::vector<T, Allocator> fromWireType(WireType value) {
        return vecFromJSArray<T>(ValBinding::fromWireType(value));
    }
};

template <typename T>
struct TypeID<T,
              typename std::enable_if_t<std::is_same<
                  typename Canonicalized<T>::type,
                  std::vector<typename Canonicalized<T>::type::value_type,
                              typename Canonicalized<T>::type::allocator_type>>::value>> {
    static constexpr TYPEID get() { return TypeID<val>::get(); }
};

}  // namespace internal
}  // namespace emscripten
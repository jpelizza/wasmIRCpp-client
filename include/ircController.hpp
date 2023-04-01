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
    void away(std::string away_msg);
    void admin(std::string server);
    void die(std::string server);
    bool eline(std::vector<std::string> userAThost, std::string duration, std::string reason);
    bool gline(std::vector<std::string> userAThost, std::string duration, std::string reason);
    void info();
    bool ison(std::vector<std::string> nick);
    bool join(std::vector<std::string> chans, std::vector<std::string> keys);
    bool invite(std::string nick, std::string chan, std::string duration);
    bool kick(std::string chan, std::vector<std::string> nicks, std::string reason);
    bool kill(std::vector<std::string> nicks, std::string reason);
    bool kline(std::vector<std::string> userAThost, std::string duration, std::string reason);
    void commands();
    void privmsg(std::string text);
    void quit(std::string msg);
    bool part(std::vector<std::string> chans, std::string reason = "");
    bool list(std::string patterns);
    bool loadModule(std::string module);
    void lusers();
    bool mode(std::string target, std::string modes, std::vector<std::string> params);
    void modules();
    void motd(std::string server);
    void names(std::vector<std::string> chans);
    bool nick(std::string nickname);
    bool notice(std::vector<std::string> targets, std::string message);
    // void commands();

    // not exported
    void categorizeMsg(std::string msg);

   private:
    void pong(std::string server);
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
#include "../include/main.hpp"

extern ircController *ircC;
static EMSCRIPTEN_WEBSOCKET_T ws;

EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
    if (!ircController::websocket) {
        ircController::websocket = websocketEvent->socket;
    }
    return EM_TRUE;
}

EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) {
    ircController::websocket = 0;
    return EM_TRUE;
}

EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
    ircController::websocket = 0;

    return EM_TRUE;
}

EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) {
    if (ircC == nullptr) return EM_TRUE;
    if (websocketEvent->isText) {
        // For only ascii chars.
        ircC->categorizeMsg((char *)websocketEvent->data);
    } else {
        std::cout << "Unkown event" << std::endl;
    }
    return EM_TRUE;
}

void openWebSocket(std::string url, std::string port) {
    std::string str_resolver = (url + ":" + port);
    EmscriptenWebSocketCreateAttributes ws_attrs = {str_resolver.c_str(), NULL, EM_TRUE};
    ws = emscripten_websocket_new(&ws_attrs);

    emscripten_websocket_set_onopen_callback(ws, NULL, onopen);
    emscripten_websocket_set_onerror_callback(ws, NULL, onerror);
    emscripten_websocket_set_onmessage_callback(ws, NULL, onmessage);
    emscripten_websocket_set_onclose_callback(ws, NULL, onclose);
}

int main(int argc, char *argv[]) {
    if (!emscripten_websocket_is_supported()) {
        return 0;
    }
}
//
EMSCRIPTEN_BINDINGS(websocket) {
    emscripten::function("openWebSocket", &openWebSocket);
}

// Binding code
EMSCRIPTEN_BINDINGS(irc_controller) {
    emscripten::class_<ircController>("ircController")
        .constructor()
        .function("getChannels", &ircController::getChannels)
        .function("getNextMessage", &ircController::getNextMessage)
        .function("getWebsocketConnection", &ircController::getWebsocketConnection)
        .function("away", &ircController::away)
        .function("admin", &ircController::admin)
        .function("die", &ircController::die)
        .function("eline", &ircController::eline)
        .function("gline", &ircController::gline)
        .function("info", &ircController::info)
        .function("ison", &ircController::ison)
        .function("join", &ircController::join)
        .function("invite", &ircController::invite)
        .function("kick", &ircController::kick)
        .function("kill", &ircController::kill)
        .function("kline", &ircController::kline)
        .function("commands", &ircController::commands)
        .function("privmsg", &ircController::privmsg)
        .function("quit", &ircController::quit)
        .function("part", &ircController::part)
        .function("list", &ircController::list)
        .function("loadmodule", &ircController::loadmodule)
        .function("lusers", &ircController::lusers)
        .function("mode", &ircController::mode)
        .function("modules", &ircController::modules)
        .function("motd", &ircController::motd)
        .function("names", &ircController::names)
        .function("nick", &ircController::nick)
        .function("notice", &ircController::notice)
        .function("oper", &ircController::oper)
        .function("pass", &ircController::pass)
        .function("ping", &ircController::ping)
        .function("qline", &ircController::qline)
        .function("rehash", &ircController::rehash)
        .function("reloadmodule", &ircController::reloadmodule)
        .function("restart", &ircController::restart)
        .function("servlist", &ircController::servlist)
        .function("squery", &ircController::squery)
        .function("stats", &ircController::stats)
        .function("time", &ircController::time)
        .function("topic", &ircController::topic)
        .function("unloadmodule", &ircController::unloadmodule)
        .function("user", &ircController::user)
        .function("userhost", &ircController::userhost)
        .function("version", &ircController::version)
        .function("wallops", &ircController::wallops)
        .function("who", &ircController::who)
        .function("whois", &ircController::whois)
        .function("whowas", &ircController::whowas)
        .function("zline", &ircController::zline);
};

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
        .function("info", &ircController::info)
        .function("join", &ircController::join)
        .function("lusers", &ircController::lusers)
        .function("names", &ircController::names)
        .function("part", &ircController::part)
        .function("privmsg", &ircController::privmsg)
        .function("quit", &ircController::quit)
        .function("registerUser", &ircController::registerUser)
        .function("sendMessage", &ircController::sendMessage)
        .function("setDebug", &ircController::setDebug);
};

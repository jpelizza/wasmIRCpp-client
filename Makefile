all:
	emcc --bind -lembind -lwebsocket.js -s MODULARIZE -s PROXY_POSIX_SOCKETS=1 \
	-o ./wasm/ircppwasm.js ./src/*.cpp ./json/json11.cpp 
clean:
	rm ./wasm/*.wasm ./wasm/*.js
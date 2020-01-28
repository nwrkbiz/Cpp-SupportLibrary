Giri's C++ Support Library
==========================

Lightweight C++ library providing everything you need to quickly create awesome applications.

This library includes:
* Websocket server/client based on boost beast
* HTTP server/client based on boost beast
* Blob class to handle files
* Generic implementations of sommon design patterns and idioms (Singleton, Observer, Passkey)

## Documentation

The API is well documented and equipt with code snippets to quickly get started.

## Dependencies

This library depends on Boost (tested with version 67) and OpenSSL and requires an C++17 compliant compiler.

## License

"THE BEER-WARE LICENSE" (Revision 42):
<giri@nwrk.biz> extended and modified this file. As long as you retain this notice you
can do whatever you want with this stuff. If we meet some day, and you think
this stuff is worth it, you can buy me a beer in return Daniel Giritzer

## Compile

To compile your programs following parameters are needed.

g++ main.cpp -std=c++17 -lboost_system -lboost_iostreams -pthread -lssl -lcrypto -lstdc++fs
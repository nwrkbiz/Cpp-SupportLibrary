Giri's C++ Support Library
==========================

C++ library providing everything you need to quickly create awesome applications.

This library includes:
* Websocket server/client based on boost beast
* HTTP server/client based on boost beast
* Blob class to handle files
* Generic implementations of common design patterns and idioms (Singleton, Observer, Passkey)

## Documentation

The API is well documented and equipt with code snippets to quickly get started.
Check it out: https://nwrkbiz.github.io/Cpp-SupportLibrary/html/index.html

## Example Code Snippets

* [Websocket client](https://nwrkbiz.github.io/Cpp-SupportLibrary/html/classgiri_1_1WebSocketClient.html#details)
* [Websocket server](https://nwrkbiz.github.io/Cpp-SupportLibrary/html/classgiri_1_1WebSocketServer.html#details)
* [HTTP server](https://nwrkbiz.github.io/Cpp-SupportLibrary/html/classgiri_1_1HTTPServer.html#details)
* [HTTP client](https://nwrkbiz.github.io/Cpp-SupportLibrary/html/classgiri_1_1HTTPClient.html#details)
* [PassKey idiom](https://nwrkbiz.github.io/Cpp-SupportLibrary/html/classgiri_1_1Key.html#details)
* Observer Pattern: [Observer](https://nwrkbiz.github.io/Cpp-SupportLibrary/html/classgiri_1_1Observable.html#details), [Observable](https://nwrkbiz.github.io/Cpp-SupportLibrary/html/classgiri_1_1Observer.html#details)
* [Singleton Pattern](https://nwrkbiz.github.io/Cpp-SupportLibrary/html/classgiri_1_1Singleton.html#details)



For more please check the documentation :)

## Dependencies

This library depends on Boost (tested with version 67) and OpenSSL and requires an C++17 compliant compiler.

## License

"THE BEER-WARE LICENSE" (Revision 42):
<giri@nwrk.biz> wrote this file. As long as you retain this notice you
can do whatever you want with this stuff. If we meet some day, and you think
this stuff is worth it, you can buy me a beer in return Daniel Giritzer

## Compile

To compile your programs following parameters are needed.

g++ main.cpp -std=c++17 -lboost_system -lboost_iostreams -pthread -lssl -lcrypto -lstdc++fs

## About

2020, Daniel Giritzer

https://page.nwrk.biz/giri
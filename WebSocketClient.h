/**
 * @file WebSocketClient.h
 * @brief Simple Websocket Client implementation.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_WEBSOCKETCLIENT_H
#define SUPPORTLIB_WEBSOCKETCLIENT_H
#include "Observer.h"
#include "Exception.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace giri {
    using tcp = boost::asio::ip::tcp;
    namespace ssl = boost::asio::ssl;
    namespace websocket = boost::beast::websocket;

    /**
     *  @brief Exception to be thrown on websocket client errors.
     */
    class WebSocketClientException : public ExceptionBase
    {
    public:
        WebSocketClientException(const std::string &msg) : ExceptionBase(msg) {}; 
        using SPtr = std::shared_ptr<WebSocketClientException>;
        using UPtr = std::unique_ptr<WebSocketClientException>;
        using WPtr = std::weak_ptr<WebSocketClientException>;
    };

    /**
     *  @brief Simple Websocket Client. 
     * 
     *  Supporting synchronous and asynchronous read.
     * 
     *  Example Usage:
     *  --------------
     *  @code{.cpp}
     *  #include <WebSocketClient.h>
     *  #include <iostream>
     *  #include <string>
     * 
     *   using namespace giri;
     *  
     *  // observer to receive async answers.
     *  // use std::lock_guard + std::mutex if resources of this class are 
     *  // accessed by multiple threads
     *  class WSCObserver : public Observer<WebSocketClient>
     *  {
     *  protected:
     *      void update(WebSocketClient::SPtr ptr)
     *      {
     *          std::cout << "async receive: " << ptr->getMessage() << std::endl;
     *      }
     *  };
     *  
     *  int main() {
     *      // Create client and async observer
     *      WebSocketClient::SPtr wsc = std::make_shared<WebSocketClient>("echo.websocket.org", "80");
     *      WSCObserver::SPtr obs = std::make_shared<WSCObserver>();
     *  
     *      // -- asynchronous write and read --
     *      std::cout << "Message to send (async):" << std::endl;
     *      std::string msg;
     *      std::cin >> msg;
     *      wsc->run();
     *      wsc->subscribe(obs);
     *      wsc->send(msg); // obs will receive answer
     *  
     *      // -- synchronous write and read --
     *      std::cout << "Message to send (sync):" << std::endl;
     *      std::cin >> msg;
     *      wsc->send(msg); // Send message
     *      std::cout << "sync receive: " << wsc->receive() << std::endl; // blocking wait for answer
     *  
     *      return EXIT_SUCCESS;
     *  }
     *  @endcode
     */
    class WebSocketClient : public Observable<WebSocketClient>
    {
    public:
        /**
         * Constructor to create a websocket client.
         * @param host Websocket server host.
         * @param port Websocket server port.
         * @param ssl Enables or disables ssl (defaults to false).
         * @param numThreads Number of threads to be used (defaults to 1).
         * @param resource Resource where websocket server is bound to (defaults to "/").
         */
        WebSocketClient(const std::string& host, const std::string& port, bool ssl = false, const size_t numThreads = 1, const std::string& resource = "/") : 
            m_Host(host), 
            m_Port(port), 
            m_Resource(resource),
            m_SSL(ssl),
            m_Ioc(numThreads)
        {
            auto const results = m_Resolver.resolve(m_Host, m_Port);
            if(m_SSL)
            {
                boost::asio::connect(m_Wss.next_layer().next_layer(), results.begin(), results.end());
                m_Wss.next_layer().handshake(ssl::stream_base::client);
                m_Wss.handshake(host, m_Resource);
                m_Wss.text(true);
            }
            else 
            {
                boost::asio::connect(m_Ws.next_layer(), results.begin(), results.end());
                m_Ws.handshake(host, m_Resource);
                m_Ws.text(true);
            }
            m_Threads.reserve(numThreads);
            for(auto i = numThreads; i > 0; --i)
                m_Threads.emplace_back(std::thread([this]{ m_Ioc.run();}));
        }
        /**
         * Send a message to server. Blocks until message was sent.
         * @brief msg Message to send
         */
        void send(const std::string& msg){
            if(!m_Ec)
                if(m_SSL) 
                    {if(m_Wss.is_open()){m_Wss.write(boost::asio::buffer(msg));}}
                else
                    {if(m_Ws.is_open()){m_Ws.write(boost::asio::buffer(msg));}}
        }
        /**
         * @returns last received message.
         */
        std::string getMessage(){ 
            return m_Message;
        }
        /**
         * @returns Host this client is connected to.
         */
        std::string getHost(){ 
            return m_Host;
        }
        /**
         * @returns Port this client is connected to.
         */
        std::string getPort(){ 
            return m_Port;
        }
        /**
         * @returns true if ssl is enabled, false otherwise.
         */
        bool getSSL(){ 
            return m_SSL;
        }
        /**
         * @returns Error code if last request was not successful.
         */
        boost::system::error_code getError(){
            return m_Ec;
        }
        /**
         * Starts receiving messages asynchrolously. Automatically
         * notifies subscribed Observer objects on new messages.
         */
        void run(){
            if(m_SSL)
                {if(m_Wss.is_open()){m_Wss.async_read(m_Buffer, std::bind(&WebSocketClient::on_read, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));}}
            else
                {if(m_Ws.is_open()){m_Ws.async_read(m_Buffer, std::bind(&WebSocketClient::on_read, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));}}
        }
        /**
         * Receives message synchronously. Blocks until a message was received.
         * @returns received message.
         */
        std::string receive(){        
            if(!m_Ec)
                if(m_SSL) 
                    {if(m_Wss.is_open()){m_Wss.read(m_Buffer);}}
                else
                    {if(m_Ws.is_open()){m_Ws.read(m_Buffer);}}
            std::ostringstream ostr;
            ostr << boost::beast::buffers(m_Buffer.data());
            m_Message = ostr.str();
            return m_Message;
        }
        /**
         * Close websocket client.
         */
        void close() {
            if(!m_Ec)
                if(m_SSL) 
                    {if(m_Wss.is_open()){m_Wss.close(websocket::close_code::normal);}}
                else
                    {if(m_Ws.is_open()){m_Ws.close(websocket::close_code::normal);}}
        }
        using SPtr = std::shared_ptr<WebSocketClient>;
        using UPtr = std::unique_ptr<WebSocketClient>;
    private:
        void on_read(boost::system::error_code ec, std::size_t bytes_transferred) {
            m_Ec = ec;
            m_Message.clear();
            if(!ec){
                std::ostringstream ostr;
                ostr << boost::beast::buffers(m_Buffer.data());
                m_Message = ostr.str();
            }
            m_Buffer.consume(m_Buffer.size()); // clear buffer
            notify(); // notify all subscribed observers
            if(!ec)
                run(); // wait for new message
        }
        std::string m_Host;
        std::string m_Port;
        std::string m_Resource;
        std::string m_Message;
        bool m_SSL;
        boost::asio::io_context m_Ioc;
        tcp::resolver m_Resolver{m_Ioc};
        ssl::context m_Ctx{ssl::context::sslv23_client};
        websocket::stream<tcp::socket> m_Ws{m_Ioc};
        websocket::stream<ssl::stream<tcp::socket>> m_Wss{m_Ioc, m_Ctx};
        boost::beast::multi_buffer m_Buffer;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_WG = boost::asio::make_work_guard(m_Ioc);
        std::vector<std::thread> m_Threads;
        boost::system::error_code m_Ec;
    };
}
#endif //SUPPORTLIB_WEBSOCKETCLIENT_H
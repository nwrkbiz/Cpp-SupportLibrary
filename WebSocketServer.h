/**
 * @file WebSocketServer.h
 * @brief Simple Websocket Server implementation.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_WEBSOCKETSERVER_H
#define SUPPORTLIB_WEBSOCKETSERVER_H
#include "Observer.h"
#include "Exception.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <filesystem>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace giri {

    using tcp = boost::asio::ip::tcp;
    namespace websocket = boost::beast::websocket;
    namespace ssl = boost::asio::ssl;

    /**
     *  @brief Exception to be thrown on websocket server errors.
     */
    class WebSocketServerException : public ExceptionBase
    {
    public:
        WebSocketServerException(const std::string &msg) : ExceptionBase(msg) {}; 
        using SPtr = std::shared_ptr<WebSocketServerException>;
        using UPtr = std::unique_ptr<WebSocketServerException>;
        using WPtr = std::weak_ptr<WebSocketServerException>;
    };

    /**
     * @brief Class representing one session/connection
     *  
     * Created by the server when accepting a client connection.
     */
    class WebSocketSession : public Observable<WebSocketSession>
    {
    public:
        /**
         * WebSocketSession constructor
         * 
         * @param socket Socket to use.
         * @param ssl true if ssl should be enabled, false otherwise.
         * @param cert If ssl is true, path to certificate file in *.pem format.
         * @param key If ssl is true path to private key file in *pem format.
         */
        explicit WebSocketSession(tcp::socket socket, bool ssl, const std::filesystem::path& cert, const std::filesystem::path& key) :
            m_Socket(std::move(socket)),
            m_SSL(ssl)
        {
            if(m_SSL)
            { 
                m_Ctx.set_options(boost::asio::ssl::context::default_workarounds |
                                boost::asio::ssl::context::no_sslv2 |
                                boost::asio::ssl::context::no_sslv3 |
                                boost::asio::ssl::context::no_tlsv1 |
                                boost::asio::ssl::context::single_dh_use);
                m_Ctx.use_certificate_chain_file(cert);
                m_Ctx.use_private_key_file(key, boost::asio::ssl::context::file_format::pem);
                m_Wss = std::make_shared<websocket::stream<ssl::stream<tcp::socket&> >>(m_Socket, m_Ctx);
                m_Wss->next_layer().set_verify_mode(ssl::verify_none);
                m_Strand = std::make_shared<boost::asio::strand<boost::asio::io_context::executor_type> >(m_Wss->get_executor());
                m_Wss->text(true);
            }
            else
            {
                m_Ws = std::make_shared<websocket::stream<tcp::socket&>>(m_Socket);
                m_Strand = std::make_shared<boost::asio::strand<boost::asio::io_context::executor_type> >(m_Ws->get_executor());
                m_Ws->text(true);
            }
        }
        /**
         * Starts receiving messages asynchrolously. Automatically
         * notifies subscribed Observer objects on new messages.
         */
        void run() {
            if(m_SSL)
                m_Wss->next_layer().async_handshake(ssl::stream_base::server,boost::asio::bind_executor(*m_Strand,std::bind(&WebSocketSession::on_handshake,this->shared_from_this(),std::placeholders::_1)));
            else
                m_Ws->async_accept(boost::asio::bind_executor(*m_Strand, std::bind(&WebSocketSession::on_accept, this->shared_from_this(), std::placeholders::_1)));
        }
        /**
         * Send a message to the client, this session is associated with. 
         * Blocks until message was sent.
         * @brief msg Message to send
         */
        void send(const std::string& msg){
            if(!m_Ec)
                if(m_SSL) 
                    {if(m_Wss->is_open()){m_Wss->write(boost::asio::buffer(msg));}}
                else
                    {if(m_Ws->is_open()){m_Ws->write(boost::asio::buffer(msg));}}
        }
        /**
         * @returns last received message.
         */
        std::string getMessage() const { 
            return m_Message;
        }
        /**
         * @returns true if ssl is enabled, false otherwise.
         */
        bool getSSL() const { 
            return m_SSL;
        }
        /**
         * @returns Error code if last request was not successful.
         */
        boost::system::error_code getError() const {
            return m_Ec;
        }
        /**
         * @returns IP of connected client.
         */
        std::string getClientIP() const {
            return m_Socket.remote_endpoint().address().to_string();
        }
        /**
         * @returns Port of connected client.
         */
        std::string getClientPort() const {
            return std::to_string(m_Socket.remote_endpoint().port());
        }
        /**
         * Close websocket session.
         */
        void close() {
            if(!m_Ec)
                if(m_SSL) 
                    {if(m_Wss->is_open()){m_Wss->close(websocket::close_code::normal);}}
                else
                    {if(m_Ws->is_open()){m_Ws->close(websocket::close_code::normal);}}
        }
        using SPtr = std::shared_ptr<WebSocketSession>;
        using UPtr = std::unique_ptr<WebSocketSession>;
        using WPtr = std::weak_ptr<WebSocketSession>;
    private:
        void on_read(boost::system::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            m_Message.clear();
            m_Ec = ec;
            if(!ec)
            {
                std::ostringstream ostr;
                ostr << boost::beast::buffers(m_Buffer.data());
                m_Message = ostr.str();
            }
            m_Buffer.consume(m_Buffer.size()); // clear buffer
            notify(); // notify all subscribed observers
            if(!ec)
                do_read(); // wait for new message
        }
        void on_accept(boost::system::error_code ec) {
            if(ec)
                throw WebSocketServerException("Accept: " + ec.message());
            do_read();
        }
        void do_read() {
            if(m_SSL)
                {if(m_Wss->is_open()){m_Wss->async_read(m_Buffer, boost::asio::bind_executor(*m_Strand, std::bind(&WebSocketSession::on_read, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2)));}}
            else
                {if(m_Ws->is_open()){m_Ws->async_read(m_Buffer, boost::asio::bind_executor(*m_Strand, std::bind(&WebSocketSession::on_read, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2)));}}
        }
        void on_handshake(boost::system::error_code ec) {
            if(ec)
                throw WebSocketServerException("Handshake: " + ec.message());
            m_Wss->async_accept(boost::asio::bind_executor(*m_Strand, std::bind( &WebSocketSession::on_accept, this->shared_from_this(), std::placeholders::_1)));
        }
        tcp::socket m_Socket;
        bool m_SSL;
        std::shared_ptr< websocket::stream<tcp::socket&> > m_Ws;
        std::shared_ptr< websocket::stream<ssl::stream<tcp::socket&> > > m_Wss;
        std::shared_ptr< boost::asio::strand<boost::asio::io_context::executor_type> > m_Strand;
        boost::beast::multi_buffer m_Buffer;
        std::string m_Message;
        ssl::context m_Ctx{ssl::context::sslv23};
        boost::system::error_code m_Ec;
    };

    /**
     * @brief Class representing a WebSocket Server.
     * 
     *  Example Usage:
     *  --------------
     * 
     *  @code{.cpp}
     *  #include <WebSocketServer.h>
     *  #include <iostream>
     *  #include <string>
     *  
     *  using namespace giri;
     * 
     *  // observer to receive async answers.
     *  // use std::lock_guard + std::mutex if resources of this class are 
     *  // accessed by multiple threads
     *  class WebSocketServerObserver : 
     *      public Observer<WebSocketServer>, 
     *      public Observer<WebSocketSession>, 
     *      public std::enable_shared_from_this<WebSocketServerObserver>
     *  {
     *      public:
     *          void update(WebSocketServer::SPtr serv){
     *              std::cout << "Connected... " << serv->getSession()->getClientIP() << ":" << serv->getSession()->getClientPort() << std::endl;
     *  
     *              // subscribe to session to receive notifications on message
     *              serv->getSession()->subscribe(this->shared_from_this());
     *          }
     *          void update(WebSocketSession::SPtr sess){
     *              std::cout << "Echoing received message: " << sess->getMessage() << std::endl;
     *              sess->send(sess->getMessage()); // send back message
     *          }
     *      using SPtr = std::shared_ptr<WebSocketServerObserver>;
     *      using UPtr = std::unique_ptr<WebSocketServerObserver>;
     *      using WPtr = std::weak_ptr<WebSocketServerObserver>;
     *  };
     *  
     *  int main()
     *  {
     *      std::shared_ptr<WebSocketServer> sptr = std::make_shared<WebSocketServer>("0.0.0.0", "1204");
     *      WebSocketServerObserver::SPtr obs = std::make_shared<WebSocketServerObserver>();
     *      sptr->subscribe(obs);
     *      sptr->run();
     *      while(true){}; // block until ctrl + c is pressed.
     *      return EXIT_SUCCESS;
     *  }
     *  @endcode
     */
    class WebSocketServer : public Observable<WebSocketServer>
    {
    public:

        /**
         * Constructor for Websocket server.
         * 
         * @param address Adress to bind this server to (defaults to 0.0.0.0 for any).
         * @param port Port to listen on (defaults to 80).
         * @param ssl Enable or disable ssl encryption (defaults to false).
         * @param numThreads Number of worker threads (defaults to 1).
         * @param cert If ssl is true path to certificate *.pem file needs to be passed.
         * @param key If ssl is true path to private key *.pem file needs to be passed.
         */
        WebSocketServer(const std::string& address = "0.0.0.0", const std::string& port = "80", bool ssl = false, const size_t numThreads = 1, const std::filesystem::path& cert = "", const std::filesystem::path& key = "") : 
            m_Endpoint(boost::asio::ip::make_address(address), 
                       std::atoi(port.c_str())),
            m_SSL(ssl),
            m_Ioc(numThreads),
            m_NumThreads(numThreads),
            m_Acceptor(m_Ioc),
            m_Socket(m_Ioc),
            m_Cert(cert),
            m_Key(key)
        {
            boost::system::error_code ec;
            m_Acceptor.open(m_Endpoint.protocol(), ec);
            if(ec)
                throw WebSocketServerException("Open: " + ec.message());
            m_Acceptor.set_option(boost::asio::socket_base::reuse_address(true));
            if(ec)
                throw WebSocketServerException("Set Option: " + ec.message());
            m_Acceptor.bind(m_Endpoint, ec);
            if(ec)
                throw WebSocketServerException("Bind: " + ec.message());
            m_Acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
            if(ec)
                throw WebSocketServerException("Listen: " + ec.message());
        }
        /**
         * Starts receiving messages asynchrolously. Automatically
         * creates sessions and notifies observers when accepting
         * new connections.
         */
        void run() {
            if(!m_Acceptor.is_open()) return;
            do_accept();

            m_Threads.reserve(m_NumThreads);
            for(auto i = m_NumThreads; i > 0; --i)
                m_Threads.emplace_back([this]{ m_Ioc.run();});
        }
        /**
         * @returns last created session.
         */
        WebSocketSession::SPtr getSession() const { 
            return m_NewSession;
        }
        /**
         * @returns true if ssl is enabled, false otherwise.
         */
        bool getSSL() const { 
            return m_SSL;
        }
        /**
         * @returns Path to the used certificate *.pem file.
         */
        std::filesystem::path getCert() const {
            return m_Cert;
        }
        /**
         * @returns Path to the used certificate privatekey *.pem file.
         */
        std::filesystem::path getKey() const {
            return m_Key;
        }
        using SPtr = std::shared_ptr<WebSocketServer>;
        using UPtr = std::unique_ptr<WebSocketServer>;
        using WPtr = std::weak_ptr<WebSocketServer>;
    private:
        void do_accept() {
            m_Acceptor.async_accept(m_Socket, std::bind(&WebSocketServer::on_accept, this->shared_from_this(), std::placeholders::_1));
        }
        void on_accept(boost::system::error_code ec) {
            if(ec)
                throw WebSocketServerException("Accept: " + ec.message());
            m_NewSession = std::make_shared<WebSocketSession>(std::move(m_Socket), m_SSL, m_Cert, m_Key);
            m_NewSession->run();
            notify(); // notify all subscribed observers
            do_accept(); // Accept another connection
        }
        tcp::endpoint m_Endpoint;
        boost::asio::io_context m_Ioc;
        std::vector<std::thread> m_Threads;
        bool m_SSL;
        size_t m_NumThreads;
        tcp::acceptor m_Acceptor;
        tcp::socket m_Socket;
        WebSocketSession::SPtr m_NewSession;
        std::filesystem::path m_Cert;
        std::filesystem::path m_Key;
    };
}
#endif //SUPPORTLIB_WEBSOCKETSERVER_H
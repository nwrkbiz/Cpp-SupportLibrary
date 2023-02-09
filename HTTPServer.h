/**
 * @file HTTPServer.h
 * @brief Simple HTTP Server implementation.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_HTTPSERVER_H
#define SUPPORTLIB_HTTPSERVER_H
#include "Object.h"
#include "Observer.h"
#include "Exception.h"
#include "FileSystem.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <map>

namespace giri {
    using tcp = boost::asio::ip::tcp;
    namespace ssl = boost::asio::ssl;
    namespace http = boost::beast::http;

    /**
     *  @brief Exception to be thrown on http server errors.
     */
    class HTTPServerException : public ExceptionBase
    {
    public:
        HTTPServerException(const std::string &msg) : ExceptionBase(msg) {}; 
        using SPtr = std::shared_ptr<HTTPServerException>;
        using UPtr = std::unique_ptr<HTTPServerException>;
        using WPtr = std::weak_ptr<HTTPServerException>;
    };

    /**
     * @brief Class representing one session/connection
     *  
     * Created by the server when accepting a client connection.
     */
    class HTTPSession : public Observable<HTTPSession>
    {
    public:
        /**
         * HTTPSession constructor
         * 
         * @param socket Socket to use.
         * @param docRoot Folder to serve via http (defaults to "./")
         * @param mimeTypes Mapping containing additional mimetypes. (will be appended to a set of default mimetypes, overrides existing defaults)
         * @param indexFile Index file to use if no file was provided by request. (defaults to "index.html")
         * @param serverString Server string to be added to the http answers. (defaults to "giris_supportlib_http_server")
         * @param ssl true if ssl should be enabled, false otherwise.
         * @param cert If ssl is true, path to certificate file in *.pem format.
         * @param key If ssl is true path to private key file in *pem format.
         * @param ioc I/O context which should be used.
         */
        explicit HTTPSession(tcp::socket socket, const std::filesystem::path& docRoot, const std::map<std::string, std::string>& mimeTypes, const std::string& indexFile, const std::string& serverString, bool ssl, const std::filesystem::path& cert, const std::filesystem::path& key, boost::asio::io_context& ioc) :
            m_Socket(std::move(socket)),
            m_DocRoot(docRoot),
            m_MimeTypes(mimeTypes),
            m_IndexFile(indexFile),
            m_ServerString(serverString),
            m_SSL(ssl),
            m_Strand(boost::asio::make_strand(ioc))
        {
            if(m_SSL)
            { 
                m_Ctx.set_options(boost::asio::ssl::context::default_workarounds |
                                boost::asio::ssl::context::no_sslv2 |
                                boost::asio::ssl::context::no_sslv3 |
                                boost::asio::ssl::context::no_tlsv1 |
                                boost::asio::ssl::context::single_dh_use);
                m_Ctx.use_certificate_chain_file(cert.string());
                m_Ctx.use_private_key_file(key.string(), boost::asio::ssl::context::file_format::pem);
                m_Stream = std::make_shared< ssl::stream<tcp::socket&> >(m_Socket, m_Ctx);
                m_Stream->set_verify_mode(ssl::verify_none);
            }
        }
        /**
         * Starts receiving messages asynchrolously. Automatically
         * notifies subscribed Observer objects on new messages.
         */
        void run() {
            if(m_SSL)
                m_Stream->async_handshake(ssl::stream_base::server, boost::asio::bind_executor(m_Strand, std::bind(&HTTPSession::on_handshake, this->shared_from_this(), std::placeholders::_1)));
            else
                do_read();
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
         * @returns HTTP request sent by the client.
         */
        http::request<http::string_body> getRequest() const {
            return m_Request;
        }
        /**
         * @returns Returns default result which the server is about to send back.
         */
        http::response<http::vector_body<char>> getResult() const {
            return m_Result;
        }
        /**
         * @returns Folder which is served via http.
         */
        std::filesystem::path getDocRoot() const {
            return m_DocRoot;
        }
        /**
         * @returns Default index file to be used.
         */
        std::string getIndexFile() const {
            return m_IndexFile;
        }
        /**
         * @returns Server string to be added to the http answers.
         */
        std::string getServerString() const {
            return m_ServerString;
        }
        /**
         * @returns Mapping containing all supported mimetypes.
         */
        std::map<std::string, std::string> getMimeTypes() const {
            return m_MimeTypes;
        }
        /**
         * Set custom result to be sent back to the client.
         * @param res Result to be sent back.
         */
        void setResult(const http::response<http::vector_body<char>> &res) {
            m_Result = res;
        }
        /**
         * Set doc root path. Only affects new requests within this HTTPSession.
         * @param path Path to serve html files from.
         */
        void setDocRoot(const std::filesystem::path& path) {
            m_DocRoot = path;
        }
        /**
         * Sets default index file to be used. Only affects new requests within this HTTPSession.
         * @param indx default index file to be used.
         */
        void setIndexFile(const std::string& indx) {
            m_IndexFile = indx;
        }
        /**
         * Set server string to be added to the http answers. Only affects new requests within this HTTPSession.
         * @param servstr server string to be added to the http answers.
         */
        void setServerString(const std::string& servstr) {
            m_ServerString = servstr;
        }
        /**
         * Add additional mimetypes. Only affects new requests within this HTTPSession.
         * @param mimeTypes Additional mimetypes to add.
         */
        void addMimeTypes(const std::map<std::string, std::string>& mimeTypes) {
            m_MimeTypes.insert(mimeTypes.begin(), mimeTypes.end());
        }
        /**
         * Close http session.
         */
        void close() {
            if(!m_Ec)
                if(m_SSL) 
                    {if(m_Socket.is_open()){m_Stream->async_shutdown(boost::asio::bind_executor(m_Strand,std::bind(&HTTPSession::on_shutdown,this->shared_from_this(),std::placeholders::_1)));}}
                else
                    {if(m_Socket.is_open()){m_Socket.shutdown(tcp::socket::shutdown_send, m_Ec);}}
        }
        using SPtr = std::shared_ptr<HTTPSession>;
        using UPtr = std::unique_ptr<HTTPSession>;
        using WPtr = std::weak_ptr<HTTPSession>;
    private:
        void on_read(boost::system::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            m_Ec = ec;
            if(!ec)
            {
                if(m_Request.method() != http::verb::get && m_Request.method() != http::verb::head){
                    m_Result = {http::status::bad_request, m_Request.version()};
                    m_Result.set(http::field::server, m_ServerString);
                    m_Result.set(http::field::content_type, "text/html");
                    m_Result.keep_alive(m_Request.keep_alive());
                    std::string msg = "Unknown HTTP-method";
                    m_Result.body().assign(msg.begin(), msg.end());
                    m_Result.prepare_payload();
                }
                else if(m_Request.target().empty() || m_Request.target()[0] != '/' || m_Request.target().find("..") != boost::beast::string_view::npos) {
                    m_Result = {http::status::bad_request, m_Request.version()};
                    m_Result.set(http::field::server, m_ServerString);
                    m_Result.set(http::field::content_type, "text/html");
                    m_Result.keep_alive(m_Request.keep_alive());
                    std::string msg = "Illegal request-target";
                    m_Result.body().assign(msg.begin(), msg.end());
                    m_Result.prepare_payload();
                }
                else {
                    std::error_code fEc;
                    std::filesystem::path path = m_DocRoot;
                    path += std::string{m_Request.target()};
                    if(m_Request.target().back() == '/')
                        path.append(m_IndexFile);
                    else if((m_Request.target().back() != '/') && std::filesystem::is_directory(path, fEc)) {
                        path += "/";
                        path.append(m_IndexFile);
                    }
                    if(!std::filesystem::exists(path, fEc)){
                        m_Result = {http::status::not_found, m_Request.version()};
                        m_Result.set(http::field::server, m_ServerString);
                        m_Result.set(http::field::content_type, "text/html");
                        m_Result.keep_alive(m_Request.keep_alive());
                        std::string msg = "The resource '" + std::string{m_Request.target()} + "' was not found.";
                        m_Result.body().assign(msg.begin(), msg.end());
                        m_Result.prepare_payload();
                    }
                    else if(fEc)
                    {
                        m_Result = {http::status::internal_server_error, m_Request.version()};
                        m_Result.set(http::field::server, m_ServerString);
                        m_Result.set(http::field::content_type, "text/html");
                        m_Result.keep_alive(m_Request.keep_alive());
                        std::string msg = "An error occurred: '" + fEc.message() + "'";
                        m_Result.body().assign(msg.begin(), msg.end());
                        m_Result.prepare_payload(); 
                    }
                    else {
                        try{
                            std::vector<char> sfile = FileSystem::LoadFile(path);
                            if(m_Request.method() == http::verb::head)
                                m_Result = {http::status::ok, m_Request.version()};
                            else {
                                m_Result = {http::status::ok, m_Request.version()};
                                m_Result.body() = sfile;
                            }
                            m_MimeTypes.try_emplace(path.extension().string(), "application/text");
                            m_Result.set(http::field::server, m_ServerString);
                            m_Result.set(http::field::content_type, m_MimeTypes[path.extension().string()]);
                            m_Result.keep_alive(m_Request.keep_alive());
                            m_Result.prepare_payload();
                        }
                        catch(const ExceptionBase& e)
                        {
                            m_Result = {http::status::internal_server_error, m_Request.version()};
                            m_Result.set(http::field::server, m_ServerString);
                            m_Result.set(http::field::content_type, "text/html");
                            m_Result.keep_alive(m_Request.keep_alive());
                            std::string msg = "An error occurred: '" + e.getMessage() + "'";
                            m_Result.body().assign(msg.begin(), msg.end());
                            m_Result.prepare_payload();
                        }
                        catch(...)
                        {
                            m_Result = {http::status::internal_server_error, m_Request.version()};
                            m_Result.set(http::field::server, m_ServerString);
                            m_Result.set(http::field::content_type, "text/html");
                            m_Result.keep_alive(m_Request.keep_alive());
                            std::string msg = "An unknown error occurred.";
                            m_Result.body().assign(msg.begin(), msg.end());
                            m_Result.prepare_payload();
                        }
                    }
                }
            }
            else if(ec == http::error::end_of_stream)
                return close();
            else {
                m_Result = {http::status::internal_server_error, m_Request.version()};
                m_Result.set(http::field::server, m_ServerString);
                m_Result.set(http::field::content_type, "text/html");
                m_Result.keep_alive(m_Request.keep_alive());
                std::string msg = "An error occurred: '" + ec.message() + "'";
                m_Result.body().assign(msg.begin(), msg.end());
                m_Result.prepare_payload(); 
            }
            m_Buffer.consume(m_Buffer.size()); // clear buffer
            notify(); // notify all subscribed observers
            if(m_SSL)
                http::async_write(*m_Stream, m_Result, boost::asio::bind_executor(m_Strand, std::bind(&HTTPSession::on_write, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, m_Result.need_eof())));
            else
                http::async_write(m_Socket, m_Result, boost::asio::bind_executor(m_Strand, std::bind(&HTTPSession::on_write, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, m_Result.need_eof())));
        }
        void do_read() {
            m_Request = {};
            if(m_SSL)
                {if(m_Socket.is_open()){http::async_read(*m_Stream, m_Buffer, m_Request, boost::asio::bind_executor(m_Strand, std::bind(&HTTPSession::on_read, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2)));}}
            else
                {if(m_Socket.is_open()){http::async_read(m_Socket, m_Buffer, m_Request, boost::asio::bind_executor(m_Strand, std::bind(&HTTPSession::on_read, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2)));}}
        }
        void on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close) {
            boost::ignore_unused(bytes_transferred);
            if(ec == boost::beast::errc::broken_pipe)
                return this->close();
            else if(ec)
                throw HTTPServerException("Write: " + ec.message());
            if(close)
                return this->close();
            m_Result.clear();
            do_read();
        }
        void on_handshake(boost::system::error_code ec) {
            if(ec){
                m_Result = {http::status::internal_server_error, m_Request.version()};
                m_Result.set(http::field::server, m_ServerString);
                m_Result.set(http::field::content_type, "text/html");
                m_Result.keep_alive(m_Request.keep_alive());
                std::string msg = "An error occurred during SSL handshake: '" + ec.message() + "'";
                m_Result.body().assign(msg.begin(), msg.end());
                m_Result.prepare_payload();
                http::async_write(m_Socket, m_Result, boost::asio::bind_executor(m_Strand, std::bind(&HTTPSession::on_write, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, m_Result.need_eof())));
            }
            else{
                do_read();
            }
        }
        void on_shutdown(boost::system::error_code ec) {
            if(ec && ec != boost::beast::errc::broken_pipe)
                throw HTTPServerException("Shutdown: " + ec.message());
        }
        tcp::socket m_Socket;
        std::filesystem::path m_DocRoot;
        bool m_SSL;
        std::shared_ptr< ssl::stream<tcp::socket&> > m_Stream;
        boost::asio::strand<boost::asio::io_context::executor_type> m_Strand;
        boost::beast::flat_buffer m_Buffer;
        http::request<http::string_body> m_Request;
        ssl::context m_Ctx{ssl::context::sslv23};
        boost::system::error_code m_Ec;
        http::response<http::vector_body<char>> m_Result;
        std::string m_ServerString;
        std::string m_IndexFile;
        std::map<std::string, std::string> m_MimeTypes;
    };

    /**
     * @brief Class representing a HTTP Server.
     * 
     *  Example Usage:
     *  --------------
     * 
     *  @code{.cpp}
     *  #include <HTTPServer.h>
     *  #include <Blob.h>
     *  #include <iostream>
     *  #include <string>
     *  using namespace giri;
     *  // observer to receive async answers.
     *  // use std::lock_guard + std::mutex if resources of this class are 
     *  // accessed by multiple threads
     *  class HTTPServerObserver : 
     *      public Observer<HTTPServer>, 
     *      public Observer<HTTPSession>, 
     *      public std::enable_shared_from_this<HTTPServerObserver>
     *  {
     *      public:
     *          void update(HTTPServer::SPtr serv){
     *              std::cout << "Connected... " << serv->getSession()->getClientIP() << ":" << serv->getSession()->getClientPort() << std::endl;
     *              // subscribe to session to receive notifications on message
     *              serv->getSession()->subscribe(this->shared_from_this());
     *          }
     *          void update(HTTPSession::SPtr sess){
     *              std::cout << "Server session requested: " << sess->getRequest().target() << std::endl;
     *  
     *              // manipulate result here if needed, set custom result as shown here:
     *              // (Body data is stored as std::vector<char>, can be set using a Blob object)
     *              if(sess->getRequest().target() == "/sayhi") {
     *                  http::response<http::vector_body<char>> myCustomResult;
     *                  Blob data;
     *                  data.loadString("Hello World :)");
     *                  myCustomResult = {http::status::ok, sess->getRequest().version()};
     *                  myCustomResult.set(http::field::server, sess->getServerString());
     *                  myCustomResult.set(http::field::content_type, "text/html");
     *                  myCustomResult.keep_alive(sess->getRequest().keep_alive());
     *                  myCustomResult.body().assign(data.begin(), data.end());
     *                  myCustomResult.prepare_payload();
     *                  sess->setResult(myCustomResult);
     *              }
     *          }
     *      using SPtr = std::shared_ptr<HTTPServerObserver>;
     *      using UPtr = std::unique_ptr<HTTPServerObserver>;
     *      using WPtr = std::weak_ptr<HTTPServerObserver>;
     *  };
     *  int main()
     *  {
     *      HTTPServer::SPtr sptr = std::make_shared<HTTPServer>("0.0.0.0", "8808", "/home/giri", 1);
     *      HTTPServerObserver::SPtr obs = std::make_shared<HTTPServerObserver>();
     *      sptr->subscribe(obs);
     *      sptr->run();
     *      while(true){}; // block until ctrl + c is pressed.
     *      return EXIT_SUCCESS;
     *  }
     *  @endcode
     */
    class HTTPServer : public Observable<HTTPServer>
    {
    public:
        /**
         * Constructor for HTTP server.
         * 
         * @param address Adress to bind this server to (defaults to 0.0.0.0 for any).
         * @param port Port to listen on (defaults to 80).
         * @param docRoot Folder to serve via http (defaults to "./").
         * @param numThreads Number of worker threads (defaults to 0, see poll).
         * @param ssl Enable or disable ssl encryption (defaults to false).
         * @param cert If ssl is true path to certificate *.pem file needs to be passed.
         * @param key If ssl is true path to private key *.pem file needs to be passed.
         * @param mimeTypes Mapping containing additional mimetypes. (will be appended to a set of default mimetypes, overrides existing defaults)
         * @param indexFile Index file to use if no file was provided by request. (defaults to "index.html")
         * @param serverString Server string to be added to the http answers. (defaults to "giris_supportlib_http_server")
         */
        HTTPServer(const std::string& address = "0.0.0.0", const std::string& port = "80", const std::filesystem::path& docRoot = "./", const size_t numThreads = 0, bool ssl = false, const std::filesystem::path& cert = "", const std::filesystem::path& key = "", const std::map<std::string, std::string>& mimeTypes = {}, const std::string& indexFile = "index.html", const std::string& serverString = "giris_supportlib_http_server") : 
            m_Endpoint(boost::asio::ip::make_address(address), 
                       std::atoi(port.c_str())),
            m_DocRoot(docRoot),
            m_NumThreads(numThreads),
            m_SSL(ssl),
            m_Cert(cert),
            m_Key(key),
            m_MimeTypes(mimeTypes),
            m_IndexFile(indexFile),
            m_ServerString(serverString),
            m_Ioc(numThreads),
            m_Acceptor(m_Ioc),
            m_Socket(m_Ioc)
        {
            boost::system::error_code ec;
            m_Acceptor.open(m_Endpoint.protocol(), ec);
            if(ec)
                throw HTTPServerException("Open: " + ec.message());
            m_Acceptor.set_option(boost::asio::socket_base::reuse_address(true));
            if(ec)
                throw HTTPServerException("Set Option: " + ec.message());
            m_Acceptor.bind(m_Endpoint, ec);
            if(ec)
                throw HTTPServerException("Bind: " + ec.message());
            m_Acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
            if(ec)
                throw HTTPServerException("Listen: " + ec.message());
            m_MimeTypes.try_emplace(".htm","text/html");
            m_MimeTypes.try_emplace(".html", "text/html");
            m_MimeTypes.try_emplace(".php", "text/html");
            m_MimeTypes.try_emplace(".txt", "text/plain");
            m_MimeTypes.try_emplace(".css", "text/css");
            m_MimeTypes.try_emplace(".map", "text/map");
            m_MimeTypes.try_emplace(".js", "application/javascript");
            m_MimeTypes.try_emplace(".json","application/json");
            m_MimeTypes.try_emplace(".xml","application/xml");
            m_MimeTypes.try_emplace(".swf", "application/x-shockwave-flash");
            m_MimeTypes.try_emplace(".flv", "video/x-flv");
            m_MimeTypes.try_emplace(".png", "image/png");
            m_MimeTypes.try_emplace(".jpg", "image/jpeg");
            m_MimeTypes.try_emplace(".jpe", "image/jpeg");
            m_MimeTypes.try_emplace(".jpeg", "image/jpeg");
            m_MimeTypes.try_emplace(".bmp", "image/bmp");
            m_MimeTypes.try_emplace(".ico", "image/vnd.microsoft.icon");
            m_MimeTypes.try_emplace(".svg", "image/svg+xml");
            m_MimeTypes.try_emplace(".svgz", "image/svg+xml");
            m_MimeTypes.try_emplace(".woff", "text/plain");
            m_MimeTypes.try_emplace(".woff2", "text/plain");
            m_MimeTypes.try_emplace(".ttf", "text/plain");
            m_MimeTypes.try_emplace(".m3u8", "application/x-mpegURL");
            m_MimeTypes.try_emplace(".m3u", "audio/x-mpegurl");
            m_MimeTypes.try_emplace(".wav", "audio/x-wav");
            m_MimeTypes.try_emplace(".mp3", "audio/mpeg");
            m_MimeTypes.try_emplace(".m4a", "audio/mpeg");
            m_MimeTypes.try_emplace(".mpeg", "video/mpeg");
            m_MimeTypes.try_emplace(".mpg","video/mpeg");
            m_MimeTypes.try_emplace(".ts","video/MP2T");
            m_MimeTypes.try_emplace(".gif", "image/gif");
            m_MimeTypes.try_emplace(".tiff", "image/tiff");
            m_MimeTypes.try_emplace(".tif", "image/tiff");
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
         * When no execution threads are used you can call this functioun constatly to handle ready executors.
         */
        void poll() {
            m_Ioc.poll_one();
        }
        /**
         * @returns last created session.
         */
        HTTPSession::SPtr getSession() const { 
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
        /**
         * @returns Folder which is served via http.
         */
        std::filesystem::path getDocRoot() const {
            return m_DocRoot;
        }
        /**
         * @returns Default index file to be used.
         */
        std::string getIndexFile() const {
            return m_IndexFile;
        }
        /**
         * @returns Server string to be added to the http answers.
         */
        std::string getServerString() const {
            return m_ServerString;
        }
        /**
         * @returns Mapping containing all supported mimetypes.
         */
        std::map<std::string, std::string> getMimeTypes() const {
            return m_MimeTypes;
        }
        /**
         * Set doc root path. Only affects new HTTPSessions.
         * @param path Path to serve html files from.
         */
        void setDocRoot(const std::filesystem::path& path) {
            m_DocRoot = path;
        }
        /**
         * Sets default index file to be used. Only affects new HTTPSessions.
         * @param indx default index file to be used.
         */
        void setIndexFile(const std::string& indx) {
            m_IndexFile = indx;
        }
        /**
         * Set server string to be added to the http answers. Only affects new HTTPSessions.
         * @param servstr server string to be added to the http answers.
         */
        void setServerString(const std::string& servstr) {
            m_ServerString = servstr;
        }
        /**
         * Add additional mimetypes. Only affects new HTTPSessions.
         * @param mimeTypes Additional mimetypes to add.
         */
        void addMimeTypes(const std::map<std::string, std::string>& mimeTypes) {
            m_MimeTypes.insert(mimeTypes.begin(), mimeTypes.end());
        }
        using SPtr = std::shared_ptr<HTTPServer>;
        using UPtr = std::unique_ptr<HTTPServer>;
        using WPtr = std::weak_ptr<HTTPServer>;
    private:
        void do_accept() {
            m_Acceptor.async_accept(m_Socket, std::bind(&HTTPServer::on_accept, this->shared_from_this(), std::placeholders::_1));
        }
        void on_accept(boost::system::error_code ec) {
            if(ec)
                throw HTTPServerException("Accept: " + ec.message());
            m_NewSession = std::make_shared<HTTPSession>(std::move(m_Socket), m_DocRoot, m_MimeTypes, m_IndexFile, m_ServerString, m_SSL, m_Cert, m_Key, m_Ioc);
            m_NewSession->run();
            notify(); // notify all subscribed observers
            do_accept(); // Accept another connection
        }
        tcp::endpoint m_Endpoint;
        boost::asio::io_context m_Ioc;
        std::filesystem::path m_DocRoot;
        size_t m_NumThreads;
        bool m_SSL;
        std::filesystem::path m_Cert;
        std::filesystem::path m_Key;
        std::map<std::string, std::string> m_MimeTypes;
        std::string m_IndexFile;
        std::string m_ServerString;  
        std::vector<std::thread> m_Threads;
        tcp::acceptor m_Acceptor;
        tcp::socket m_Socket;
        HTTPSession::SPtr m_NewSession;    
    };
}
#endif //SUPPORTLIB_HTTPSERVER_H

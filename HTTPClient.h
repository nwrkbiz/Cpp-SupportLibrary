/**
 * @file HTTPClient.h
 * @brief Simple HTTP Client implementation.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_HTTPCLIENT_H
#define SUPPORTLIB_HTTPCLIENT_H
#include "Object.h"
#include "Blob.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>
#include <string>
namespace giri {
    namespace ssl = boost::asio::ssl;
    namespace http = boost::beast::http;

    /**
     * @brief Simple HTTP Client.
     *
     *  Example Usage:
     *  --------------
     *  @code{.cpp}
     *  #include <HTTPClient.h>
     *  #include <Blob.h>
     *  #include <iostream>
     *  #include <string>
     *  
     *  using namespace giri;
     *  
     *  int main()
     *  {
     *      HTTPClient::SPtr httpc = std::make_shared<HTTPClient>(true);
     *  
     *      Blob b;
     *      b = httpc->HTTPGet("www.google.at", "443");
     *      std::cout << b.toString() << std::endl;
     *  
     *      Blob postData;
     *      postData.loadString("&Hello=World");
     *      httpc->setSSL(false);
     *      b = httpc->HTTPPost("ptsv2.com", "80", "/t/cu0dm-1579047674/post", postData);
     *      std::cout << b.toString() << std::endl;
     *  
     *      return EXIT_SUCCESS;
     *  }
     *  @endcode
     */
    class HTTPClient : public Object<HTTPClient> {
        using tcp = boost::asio::ip::tcp; 
    public:
        enum class Version : size_t {
            v_10 = 10,
            v_11 = 11
        };
        /**
         * HTTPClient Constructor
         * 
         * @param ssl Configure wether to use ssl or not. (defaults to false)
         * @param version HTTP version to use (10 or 11). (defaults to 11)
         * @param userAgent Useragent string to be used for all requests. (defaults to "giris_supportlib_http_client")
         * @param contentType Content type to be used for all HTTPPost requests. (defaults to "application/x-www-form-urlencoded")
         */
        HTTPClient(bool ssl = false, 
                   HTTPClient::Version version = HTTPClient::Version::v_11,
                   const std::string& userAgent = "giris_supportlib_http_client",
                   const std::string& contentType = "application/x-www-form-urlencoded"
                   ) : 
                   m_SSL(ssl), 
                   m_Version(version),
                   m_UserAgent(userAgent),
                   m_ContentType(contentType){}
        
        /**
         * @param userAgent Useragent string to be used for all requests.
         */
        void setUserAgentString(const std::string & userAgent) {
            m_UserAgent = userAgent;
        }

        /**
         * @param contentType Content type to be used for all HTTPPost requests.
         */
        void setContentType(const std::string & contentType) {
            m_ContentType = contentType;
        }

        /**
         * @param v HTTP version to use (10 or 11)
         */
        void setVersion(const HTTPClient::Version v) {
            m_Version = v;
        }

        /**
         * @param ssl Configure wether to use ssl or not.
         */
        void setSSL(bool ssl){
            m_SSL = ssl;
        }

        /**
         * @returns Useragent string used for all requests.
         */
        std::string getUserAgentString() const {
            return m_UserAgent;
        }

        /**
         * @returns Content type used for HTTPPost requests.
         */
        std::string getContentType() const {
            return m_ContentType;
        }
        /**
         * @returns HTTP version used by this client (10 or 11).
         */
        HTTPClient::Version getVersion() const {
            return m_Version;
        }

        /**
         * @returns true if ssl is used, false otherwise.
         */
        bool getSSL() const {
            return m_SSL;
        }
        /**
         * @returns resulting error code of last request.
         */
        boost::system::error_code getError() const {
            return m_Ec;
        }

        /**
         * Performs a HTTPGet request.
         * 
         * @param host Hostname or IP of server.
         * @param port Port of server.
         * @param target Targeted resource. (defaults to "/")
         */
        std::vector<char> HTTPGet(const std::string & host, const std::string& port, const std::string &target = "/")
        {
            http::request<http::string_body> req{http::verb::get, target, (size_t)m_Version};
            req.set(http::field::host, host);
            req.set(http::field::user_agent, m_UserAgent);
            performRequest(host, port, req);
            std::ostringstream ostr;
            ostr << boost::beast::buffers_to_string(m_Res.body().data()); std::string s = ostr.str();
            return std::vector<char>(s.begin(), s.end());
        }

        /**
         * Performs a HTTPPost request.
         * 
         * @param host Hostname or IP of server.
         * @param port Port of server.
         * @param target Targeted resource.
         * @param data Data to send via post request.
         */
        std::vector<char> HTTPPost(const std::string & host, const std::string& port, const std::string &target, const std::vector<char>& data)
        {
            http::request<http::string_body> req{http::verb::post, target, (size_t)m_Version};
            req.set(http::field::host, host);
            req.set(http::field::user_agent, m_UserAgent);
            req.set(http::field::content_type, m_ContentType);
            req.body() = std::string(data.begin(), data.end());
            req.prepare_payload();
            performRequest(host, port, req);
            std::ostringstream ostr;
            ostr << boost::beast::buffers_to_string(m_Res.body().data()); std::string s = ostr.str();
            return std::vector<char>(s.begin(), s.end());
        }

        /**
         * Performs a HTTPPost request.
         * 
         * @param host Hostname or IP of server.
         * @param port Port of server.
         * @param target Targeted resource.
         * @param data Data to send via post request.
         */
        std::vector<char> HTTPPost(const std::string & host, const std::string& port, const std::string &target, const Blob& data)
        {
            std::vector<char> dat;
            dat.reserve(data.size());
            dat.assign(data.begin(), data.end());
            HTTPPost(host, port, target, dat);
        }

        /**
         * Performs a HTTPPut request.
         * 
         * @param host Hostname or IP of server.
         * @param port Port of server.
         * @param target Targeted resource.
         * @param data Data to send via put request.
         */
        std::vector<char> HTTPPut(const std::string & host, const std::string& port, const std::string &target, const std::vector<char>& data)
        {
            http::request<http::string_body> req{http::verb::put, target, (size_t)m_Version};
            req.set(http::field::host, host);
            req.set(http::field::user_agent, m_UserAgent);
            req.set(http::field::content_type, m_ContentType);
            req.body() = std::string(data.begin(), data.end());
            req.prepare_payload();
            performRequest(host, port, req);
            std::ostringstream ostr;
            ostr << boost::beast::buffers_to_string(m_Res.body().data()); std::string s = ostr.str();
            return std::vector<char>(s.begin(), s.end());
        }

        /**
         * Performs a HTTPPut request.
         * 
         * @param host Hostname or IP of server.
         * @param port Port of server.
         * @param target Targeted resource.
         * @param data Data to send via put request.
         */
        std::vector<char> HTTPPut(const std::string & host, const std::string& port, const std::string &target, const Blob& data)
        {
            std::vector<char> dat;
            dat.reserve(data.size());
            dat.assign(data.begin(), data.end());
            HTTPPut(host, port, target, dat);
        }

    private:
        void performRequest(const std::string& host, const std::string& port, const http::request<http::string_body>& req)
        {
            tcp::resolver resolver{m_Ioc};
            auto const results = resolver.resolve(host, port);
            if(m_SSL){
                ssl::stream<tcp::socket> stream{m_Ioc, m_Ctx};
                boost::asio::connect(stream.next_layer(), results.begin(), results.end());
                stream.handshake(ssl::stream_base::client);
                http::write(stream, req);
                http::read(stream, m_Buffer, m_Res);
                stream.shutdown(m_Ec);
            }
            else{
                tcp::socket stream{m_Ioc};
                boost::asio::connect(stream, results.begin(), results.end());
                http::write(stream, req);
                http::read(stream, m_Buffer, m_Res);
                stream.shutdown(tcp::socket::shutdown_both, m_Ec);
            }
        }
        bool m_SSL;
        HTTPClient::Version m_Version;
        std::string m_UserAgent;
        std::string m_ContentType;
        ssl::context m_Ctx{ssl::context::sslv23_client};
        boost::asio::io_context m_Ioc;
        boost::system::error_code m_Ec;
        boost::beast::flat_buffer m_Buffer;
        http::response<http::dynamic_body> m_Res;
    };
}
#endif //SUPPORTLIB_HTTPCLIENT_H
/**
 * @file Exception.h
 * @brief Base exception to inherit custom exceptions from.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_EXCEPTION_H
#define SUPPORTLIB_EXCEPTION_H
#include <exception>
#include <string>
#include "Object.h"
namespace giri {
    /**
     * @brief  Base exception to inherit custom exceptions from.
     * 
     *  Example Usage:
     *  --------------
     * 
     *  @code{.cpp}
     *  #include <Exception.h>
     *  #include <iostream>
     *  class MyException final : public giri::ExceptionBase
     *  {
     *  public:
     *    MyException(const std::string &msg) : ExceptionBase(msg) {}; 
     *    using SPtr = std::shared_ptr<MyException>;
     *    using UPtr = std::unique_ptr<MyException>;
     *    using WPtr = std::weak_ptr<MyException>;
     *  };
     * 
     *  // use exception
     *  int main(int argc, char *argv[]){
     *      try {
     *          throw MyException("My Error Message!");
     *      }
     *      catch (const MyException& e)
     *      {
     *          std::cerr << e.what() << "\n";
     *      }
     *      return EXIT_SUCCESS;
     *  }
     *  @endcode
     */
    class ExceptionBase : public Object<ExceptionBase>, public std::exception
    {
    public:
        /**
         * Constructor takes error message as argument.
         * @param msg Exception error message, defaults to empty.
         */
        ExceptionBase(const std::string& msg = ""): m_Message(msg){};
        const char * what () const throw () {
            return m_Message.c_str();
        }
        /**
         * Returns the error message.
         * @returns Exception error message.
         */
        std::string getMessage() const{
            return m_Message;
        }
        /**
         * Sets the error message.
         * @param message Exception error message, defaults to empty.
         */
        void setMessage(const std::string & message = ""){
            m_Message = message;
        }
    private:
        std::string m_Message;
    };
}
#endif // SUPPORTLIB_EXCEPTION_H
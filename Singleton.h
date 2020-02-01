/**
 * @file Singleton.h
 * @brief Singleton Pattern implementation.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_SINGLETON_H
#define SUPPORTLIB_SINGLETON_H
#include "Object.h"
namespace giri {
    /**
     *  @brief Singleton Template Class.
     * 
     *  Example Usage:
     *  --------------
     *  @code{.cpp}
     *  #include <Singleton.h>
     *  #include <iostream>
     *  
     *  using namespace giri;
     *  
     *  // Class Implementation
     *  class MyClass : public Singleton<MyClass> 
     *  {
     *  public:
     *      // Class implementation belongs here
     *      void identify(){
     *          std::cout << "Hi, i am " << this << std::endl;
     *      }
     *      using UPtr = std::unique_ptr<MyClass>;
     *      ~MyClass(){
     *          std::cout << "Destroyed!" << std::endl;
     *      }
     *  protected:
     *      // protect ctor, only Singleton may call it
     *      MyClass(){
     *          std::cout << "Created!" << std::endl;
     *      }
     *      friend class Singleton<MyClass>;
     *  };
     *  // use Singleton
     *  int main(int argc, char *argv[]){
     *      MyClass::getInstance()->identify(); // Created!
     *      MyClass::getInstance()->identify();
     *      MyClass::destroy(); // Destroyed!
     *      MyClass::getInstance()->identify(); // Created!
     *      return EXIT_SUCCESS;
     *  }
     *  @endcode
     */
    template <typename T> class Singleton : public Object<T> {
    public:

        /**
         * @brief Generates a static Variable which can only be instanced once. 
         * Parameters depend on the inheriting class.
         * @returns Pointer to the only existing instance.
         */
        template<typename... Args> static T*getInstance(Args... args){
            if(m_Instance==nullptr)
                m_Instance.reset(new T{ std::forward<Args>(args)... });
            return m_Instance.get();
        }
        /**
         * @brief Destroys the Object held by the Singleton.
         */
        static void destroy(){
            m_Instance = nullptr;
        }
        virtual ~Singleton() = default;
    protected:
        //No Object of Singleton can be created
        Singleton() = default;

    private:
        //Delete the Copyconstructor and the Assignmentoperator
        Singleton(Singleton const& s) = delete;
        Singleton& operator= (Singleton const& s) = delete;
        inline static std::unique_ptr<T> m_Instance;
    };
}
#endif //SUPPORTLIB_SINGLETON_H

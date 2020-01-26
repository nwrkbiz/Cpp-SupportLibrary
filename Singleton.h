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
     * 
     *  // Class Implementation
     *  class MyClass : public giri::Singleton<MyClass> 
     *  {
     *      // Class implementation belongs here
     *  protected:
     *	    MyClass(); // disallow creating MyClass objects
    *	    friend class giri::Singleton<MyClass>;
    *  };
    * 
    *  // use singleton
    *  int main(int argc, char *argv[]){
    *      MyClass::UPtr ptr = MyClass::GetInstance(); // returns pointer to the only instance
    *      return EXIT_SUCCESS;
    *  }
    *  @endcode
    */
    template <typename T> class Singleton : public Object<T> {
    public:

        /**
         * @brief Generates a static Variable which can only be instanced once. 
         * Parameters depend on the inheriting class.
         * @returns Shared pointer to the only existing instance.
         */
        template<typename... Args> static std::unique_ptr<T> GetInstance(Args... args){
            static std::unique_ptr<T> m_Instance;
            if(m_Instance==nullptr)
            mInstance.reset(new T{ std::forward<Args>(args)... });
            return m_Instance;
        }

        virtual ~Singleton() = default;
    protected:
        //No Object of Singleton can be created
        Singleton() = default;

    private:
        //Delete the Copyconstructor and the Assignmentoperator
        Singleton(Singleton const& s) = delete;
        Singleton& operator= (Singleton const& s) = delete;
    };

    /**
     * \brief Singleton Template Class (using bare pointer).
     * 
     *  Example Usage:
     *  --------------
     * 
     *  @code{.cpp}
     *  #include <Singleton.h>
     * 
     *  // Class Implementation
     *  class MyClass : public giri::SingletonBarePtr<MyClass> 
     *  {
     *      // Class implementation belongs here
     *  protected:
     *	    MyClass(); // disallow creating MyClass objects
    *	    friend class giri::SingletonBarePtr<MyClass>;
    *  };
    * 
    *  // use singleton
    *  int main(int argc, char *argv[]){
    *      MyClass* ptr = MyClass::GetInstance(); // returns pointer to the only instance
    *      return EXIT_SUCCESS;
    *  }
    *  @endcode
    */
    template <typename T> class SingletonBarePtr : public Object<T> {
    public:

        /**
         * @brief Generates a static Variable which can only be instanced once. 
         * Parameters depend on the inheriting class.
         * @returns Shared pointer to the only existing instance.
         */
        template<typename... Args> static T* GetInstance(Args... args){
            static T* m_Instance;
            if(m_Instance == nullptr)
            m_Instance = (new T{ std::forward<Args>(args)... });
            return m_Instance;
        }

        virtual ~SingletonBarePtr() = default;
    protected:
        //No Object of Singleton can be created
        SingletonBarePtr() = default;

    private:
        //Delete the Copyconstructor and the Assignmentoperator
        SingletonBarePtr(SingletonBarePtr const& s) = delete;
        SingletonBarePtr& operator= (SingletonBarePtr const& s) = delete;
    };
}
#endif //SUPPORTLIB_SINGLETON_H

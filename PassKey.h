/**
 * @file PassKey.h
 * @brief Passkey idiom implementation.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_PASSKEY_H
#define SUPPORTLIB_PASSKEY_H
#include "Object.h"
namespace giri {
    /**
     *  @brief Key class implementing passkey idiom.
     * 
     *  Can be used for instance to lock pure virtual
     *  function implementations to be only called from base
     *  class.
     * 
     *  Example Usage:
     *  --------------
     *  @code{.cpp}
     *  #include <PassKey.h>
     *  #include <iostream>
     *  class MyClass : public giri::Object<MyClass>
     *  {
     *  public:
     *   MyClass() {
     *      Locked(m_Key); // Can be called
     *   };
     * 
     *   // Can only be called from this class, because only
     *   // MyClass can create Key<MyClass> Objects. 
     *   void Locked(giri::Key<MyClass> l){
     *      std::cout<<"Secret, Hello World. \n";
     *   };
     * 
     *   virtual void SayHiAndDoWork() final {
     *      std::cout << "Hi!\n";
     *      DoWork(m_Key); // Can only be called within this class
     *   }
     * 
     *  protected:
     *    // inherit to implement
     *    virtual void DoWork(giri::Key<MyClass> l) = 0;
     *  private:
     *      giri::Key<MyClass> m_Key;
     *  };
     * 
     *  class MyClassImpl : public MyClass
     *  {
     *  public:
     *      MyClassImpl(){
     *          //Key<MyClass> key; // cannot be created
     *          //Locked(key); // cannot be called 
     *          //DoWork(key); // cannot be called aswell
     *          SayHiAndDoWork(); // can be called
     *      }
     *  protected:
     *      virtual void DoWork(giri::Key<MyClass> l) override
     *      {
     *          std::cout << "Doing hard work\n";
     *      }
     *  };
     * 
     *  int main()
     *  {
     *      MyClassImpl cl;
     *      cl.SayHiAndDoWork(); // only callable function
     *      return EXIT_SUCCESS;
     *  }
     *  @endcode
     */
    template <typename T>
    class Key : public Object< Key<T> > {
        friend T;
        Key() {};
        Key(const Key&) = default;
        Key& operator=(const Key&) = delete;
    };
}
#endif //SUPPORTLIB_PASSKEY_H
/**
 * @file Object.h
 * @brief Base class of all classes.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_BASECLASS_OBJECT_H
#define SUPPORTLIB_BASECLASS_OBJECT_H
#include <memory>

/**
 * @brief Namespace for giri's C++ support library.
 */
namespace giri {
  /**
   *  @brief Base Class of all classes.
   * 
   *  Example Usage:
   *  --------------
   * 
   *  @code{.cpp}
   *  #include <Object.h>
   *  class MyClass : public giri::Object<MyClass> 
   *  {
   *    // Class implementation belongs here
   *  };
   *  @endcode
   */
  template <typename T>
  class Object{
  public:
    using SPtr = std::shared_ptr<T>;
    using UPtr = std::unique_ptr<T>;
    using WPtr = std::weak_ptr<T>;
  protected:
    virtual ~Object() = default;
    Object() = default;
  };
}
#endif //SUPPORTLIB_BASECLASS_OBJECT_H

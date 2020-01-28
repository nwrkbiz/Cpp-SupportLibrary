/**
 * @file Utilities.h
 * @brief Contains useful functions, that do not fit anywhere else.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_UTILITIES_H
#define SUPPORTLIB_UTILITIES_H
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace giri {
  /**
   * @returns A globally unique identifier.
   */
  std::string generateUUID(){
    boost::uuids::random_generator gen;
    boost::uuids::uuid id = gen();
    return boost::lexical_cast<std::string>(id);
  }
}
#endif //SUPPORTLIB_UTILITIES_H

/**
 * @file FileSystem.h
 * @brief Contains FileSystem I/O functions.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_FILESYSTEM_H
#define SUPPORTLIB_FILESYSTEM_H

#include "Exception.h"
#include <vector>
#include <fstream>
#include <filesystem>

namespace giri {

    /**
     * @brief Namespace containing FileSystem I/O commands.
     */
    namespace FileSystem {

        /**
         * @brief Exception to be thrown on FileSystem errors.
         */
        class FileSystemException final : public ExceptionBase
        {
        public:
            FileSystemException(const std::string &msg) : ExceptionBase(msg) {}; 
            using SPtr = std::shared_ptr<FileSystemException>;
            using UPtr = std::unique_ptr<FileSystemException>;
            using WPtr = std::weak_ptr<FileSystemException>;
        };

        /**
         * Loads blob data from the harddisk. Throws FileSystemException
         * on error.
         * @param file Filepath to load data from.
         */
        inline std::vector<char> LoadFile(const std::filesystem::path& file) {
            if(!std::filesystem::exists(file))
                throw FileSystemException("File does not exist: " + file.string());

            std::ifstream in;
            in.open(file, std::ios::in | std::ios::binary);
            if (!in.good())
                throw FileSystemException("Could not open file: " + file.string());

            in.seekg(0, in.end); size_t len = in.tellg(); in.seekg(0, in.beg);
            std::vector<char> b(len);
            in.read((char*)&b[0], len);
            in.close();
            return b;
        }

        /**
         * Writes data to the harddisk. If given file does not
         * exist it will be created. If file does already exist it will
         * be overridden.
         * @param file Filepath to write data to.
         * @param data Data to write.
         * @returns true on success, false on failure.
         */
        inline bool WriteFile(const std::filesystem::path& file, const std::vector<char>& data)
        {
            std::ofstream out;
            out.open(file, std::ios::out | std::ios::binary);
            if (!out.good())
                return false;
            out.write((char*)&data[0], data.size() * sizeof(char));
            out.close();
            return true;
        }
    }
}
#endif //SUPPORTLIB_FILESYSTEM_H
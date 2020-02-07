/**
 * @file Blob.h
 * @brief Binary large object implementation using std::vector<char>.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef SUPPORTLIB_BLOB_H
#define SUPPORTLIB_BLOB_H
#include "Object.h"
#include "Base64.h"
#include "FileSystem.h"
#include <vector>
#include <string>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
namespace giri {
    /**
     * @brief Binary large object.
     */
    class Blob : private std::vector<char>, public Object<Blob>
    {
    public:
        using std::vector<char>::push_back;
        using std::vector<char>::pop_back;
        using std::vector<char>::emplace_back;
        using std::vector<char>::front;
        using std::vector<char>::back;
        using std::vector<char>::operator[];
        using std::vector<char>::operator=;
        using std::vector<char>::begin;
        using std::vector<char>::end;
        using std::vector<char>::cbegin;
        using std::vector<char>::cend;
        using std::vector<char>::crbegin;
        using std::vector<char>::crend;
        using std::vector<char>::assign;
        using std::vector<char>::size;
        using std::vector<char>::at;
        using std::vector<char>::clear;
        using std::vector<char>::empty;
        using std::vector<char>::erase;
        using std::vector<char>::insert;
        using std::vector<char>::resize;
        using std::vector<char>::reserve;
        using std::vector<char>::iterator;
        using std::vector<char>::const_iterator;
        using std::vector<char>::vector;
        
        /**
         * @returns The stored data as string.
         */
        std::string toString() const {
            return std::string(begin(), end());
        }

        /**
         * Stores a string within the blob object.
         * @param s String data to load into the blob.
         */
        void loadString(const std::string& s) {
            assign(s.begin(), s.end());
        }

        /**
         * Writes blob data to the harddisk. If given file does not
         * exist it will be created. If file does already exist it will
         * be overridden.
         * @param file Filepath to write data to.
         * @returns true on success, false on failure.
         */
        bool toFile(const std::filesystem::path& file){
            return FileSystem::WriteFile(file, *this);
        }

        /**
         * Loads blob data from the harddisk. Throws FileSystemException
         * on error.
         * @param file Filepath to load data from.
         */
        void loadFile(const std::filesystem::path& file) {
            *this = FileSystem::LoadFile(file);
        }

        /**
         * @returns The stored data encoded as Base64 string.
         */
        std::string toBase64() const {
            return Base64::Encode(*this);
        }

        /**
         * Load Data from a Base64 string.
         * @param data Base64 encoded data.
         */
        void loadBase64(const std::string& data) {
            *this = Base64::Decode(data);
        }

        /**
         * Compresses the blob data.
         */
        void compress(){
            boost::iostreams::filtering_istreambuf in;
            in.push(boost::iostreams::gzip_compressor(boost::iostreams::zlib::best_compression));
            in.push(boost::iostreams::array_source(data(), size()));
            assign(std::istreambuf_iterator<char>{&in}, {});
        }

        /**
         * Decompresses formerly compressed blob data.
         */
        void decompress(){
            boost::iostreams::filtering_istreambuf in;
            in.push(boost::iostreams::gzip_decompressor(boost::iostreams::zlib::best_compression));
            in.push(boost::iostreams::array_source(data(), size()));
            assign(std::istreambuf_iterator<char>{&in}, {});
        }
    };
}
#endif //SUPPORTLIB_BLOB_H
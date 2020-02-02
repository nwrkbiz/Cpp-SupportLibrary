/**
 * @file Base64.h
 * @brief Contains Base64 encoding/decoding algorithms.
 * @author Megumi Tomita (tomykaira)
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> extended this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 * 
 * Further informations:
 * ---------------------
 * 
 * Based on: https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594
 * 
 * This version includes changes and documentation by <giri@nwrk.biz>.
 * 
 * ### Original License:
 * 
 * The MIT License (MIT)
 * Copyright (c) 2016 tomykaira
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef SUPPORTLIB_BASE64_H
#define SUPPORTLIB_BASE64_H
#include <string>
#include <vector>

namespace giri {

  /**
   * @brief Namespace containing Base64 encoding/decoding commands.
   */
  namespace Base64 {

    /**
     * Encodes a vector containing binray data to a Base64
     * string.
     * @param binaryData Vector containing binray data.
     * @returns Data represented as Base64 string.
     */
    inline std::string Encode(const std::vector<char>& data) {
      static constexpr char sEncodingTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
      };

      size_t in_len = data.size();
      size_t out_len = 4 * ((in_len + 2) / 3);
      std::string ret(out_len, '\0');
      size_t i;
      char *p = const_cast<char*>(ret.c_str());

      for (i = 0; i < in_len - 2; i += 3) {
        *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
        *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xF0) >> 4)];
        *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) | ((int) (data[i + 2] & 0xC0) >> 6)];
        *p++ = sEncodingTable[data[i + 2] & 0x3F];
      }
      if (i < in_len) {
        *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
        if (i == (in_len - 1)) {
          *p++ = sEncodingTable[((data[i] & 0x3) << 4)];
          *p++ = '=';
        }
        else {
          *p++ = sEncodingTable[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xF0) >> 4)];
          *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
      }

      return ret;
    }
    /**
     * Decodes a Base64 string.
     * @param base64Data Data represented as Base64 string.
     * @returns vector containing binray data.
     */
    inline std::vector<char> Decode(const std::string& base64Data) {
      static constexpr unsigned char kDecodingTable[] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
      };

      size_t in_len = base64Data.size();
      if (in_len % 4 != 0) return {};

      size_t out_len = in_len / 4 * 3;
      if (base64Data[in_len - 1] == '=') out_len--;
      if (base64Data[in_len - 2] == '=') out_len--;

      std::vector<char> out(out_len);

      for (size_t i = 0, j = 0; i < in_len;) {
        uint32_t a = base64Data[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64Data[i++])];
        uint32_t b = base64Data[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64Data[i++])];
        uint32_t c = base64Data[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64Data[i++])];
        uint32_t d = base64Data[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(base64Data[i++])];
        uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);
        if (j < out_len) out[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < out_len) out[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < out_len) out[j++] = (triple >> 0 * 8) & 0xFF;
      }

      return out;
    }
  }
}

#endif // SUPPORTLIB_BASE64_H
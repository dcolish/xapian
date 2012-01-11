/** @file filetests.h
 * @brief Utility functions for testing files.
 */
/* Copyright (C) 2012 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef XAPIAN_INCLUDED_FILETESTS_H
#define XAPIAN_INCLUDED_FILETESTS_H

#include "safesysstat.h"
#include <string>

using namespace std;

inline bool file_exists(const char * path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

inline bool file_exists(const std::string & path) {
    return file_exists(path.c_str());
}

inline bool directory_exists(const char * path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

inline bool directory_exists(const std::string & path) {
    return directory_exists(path.c_str());
}

#endif // XAPIAN_INCLUDED_FILETESTS_H

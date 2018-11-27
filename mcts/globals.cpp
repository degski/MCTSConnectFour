
// MIT License
//
// Copyright (c) 2018 degski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <Windows.h>

#include <cstdlib>

/*

http://boost.2283326.n4.nabble.com/shared-ptr-Clang-with-Microsoft-CodeGen-in-VS-2015-Update-1-td4681900.html

Marcel Raad wrote:

> The same problem occurs with clang-cl, which defines _MSC_VER instead of
> __GNUC__. Could that be fixed too please?
>
> clang/c2 defines __GNUC__ but doesn't support GCC assembly, so
> sp_counted_base_gcc_x86.hpp also has to be excluded for __c2__.

With all those subtly different clangs, things are getting a bit ridiculous.

Does #define BOOST_SP_USE_STD_ATOMIC work for clang-cl and clang/c2?

*/

#include <filesystem>
#include <random>

namespace fs = std::filesystem;

#include "splitmix.hpp"

#include <SFML/Graphics.hpp>


sf::Clock g_clock;

using rng_t = splitmix64;

std::uint64_t g_seed = 0;


rng_t g_rng ( 1234567890u );
// rng_t g_rng;


[ [ nodiscard ] ] std::uint64_t next_seed ( ) noexcept {
    ++g_seed;
    g_rng.seed ( g_seed );
    return g_seed;
}

void seed ( const std::uint64_t seed_ ) noexcept {
    g_rng.seed ( seed_ );
}


[[ nodiscard ]] bool bernoulli ( ) noexcept {
    static std::bernoulli_distribution g_bernoulli_distribution;
    return g_bernoulli_distribution ( g_rng );
}


std::int32_t g_max = 0;


[[ nodiscard ]] fs::path appDataPath ( std::string && name_ ) {
    char *value;
    std::size_t len;
    _dupenv_s ( &value, & len, "USERPROFILE" );
    fs::path return_value ( std::string ( value ) + std::string ( "\\AppData\\Roaming\\" + name_ ) );
    fs::create_directory ( return_value ); // No error if directory exists.
    return return_value;
}

const fs::path app_data_path_ = appDataPath ( "ConnectFour" );

fs::path & g_app_data_path = const_cast<fs::path &> ( app_data_path_ );


[[ nodiscard ]] fs::path getExePath ( ) noexcept {
    TCHAR exename [ 1024 ];
    GetModuleFileName ( NULL, exename, 1024 );
    return fs::path ( exename ).parent_path ( );
}

const fs::path app_path_ = getExePath ( );

fs::path & g_app_path = const_cast<fs::path &> ( app_path_ );


[[ nodiscard ]] std::int32_t getNumberOfProcessors ( ) noexcept {
    char *value;
    std::size_t len;
    _dupenv_s ( & value, & len, "NUMBER_OF_PROCESSORS" );
    return std::atoi ( value );
}


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

#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <lz4stream.hpp>

#include <SFML/Graphics.hpp>

#include "splitmix.hpp"

#define NO_DEFAULT_CASE default: abort ( )


// C++ global constants have static linkage. This is different from C. If you try to use a global
// constant in C++ in multiple files you get an unresolved external error. The compiler optimizes
// global constants out, leaving no space reserved for the variable. One way to resolve this error
// is to include the const initializations in a header file and include that header in your CPP files
// when necessary, just as if it was function prototype. Another possibility is to make the variable
// non-constant and use a constant reference when assessing it.


namespace fs = std::filesystem;

extern fs::path & g_app_data_path; // This needs to be in header file.
extern fs::path & g_app_path;

extern sf::Clock g_clock;

using rng_t = splitmix64;

extern std::uint64_t g_seed;
extern rng_t g_rng;

[[ nodiscard ]] std::uint64_t next_seed ( ) noexcept;
void seed ( const std::uint64_t seed_ ) noexcept;

[[ nodiscard ]] bool bernoulli ( ) noexcept;

extern std::int32_t g_max;

[[ nodiscard ]] sf::Time now ( ) noexcept {
    return g_clock.getElapsedTime ( );
}

[[ nodiscard ]] sf::Time since ( const sf::Time start_ ) noexcept {
    return g_clock.getElapsedTime ( ) - start_;
}


void sleep ( const sf::Int32 milliseconds_ ) noexcept {
    std::this_thread::sleep_for ( std::chrono::milliseconds ( milliseconds_ ) );
}

std::int32_t getNumberOfProcessors ( ) noexcept;


template<typename T>
void saveToFile ( const T & t_, std::string && file_name_ ) noexcept {
    std::ofstream compressed_ostream ( g_app_data_path / ( file_name_ + std::string ( ".lz4cereal" ) ), std::ios::binary );
    LZ4OutputStream lz4_ostream ( compressed_ostream );
    cereal::BinaryOutputArchive archive ( lz4_ostream );
    archive ( t_ );
    lz4_ostream.flush ( );
    compressed_ostream.flush ( );
    lz4_ostream.close ( );
    compressed_ostream.close ( );
}

template<typename T>
void loadFromFile ( T & t_, std::string && file_name_ ) noexcept {
    std::ifstream compressed_istream ( g_app_data_path / ( file_name_ + std::string ( ".lz4cereal" ) ), std::ios::binary );
    LZ4InputStream lz4_istream ( compressed_istream );
    cereal::BinaryInputArchive archive ( lz4_istream );
    archive ( t_ );
    compressed_istream.close ( );
}

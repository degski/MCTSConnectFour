
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

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <vector>

using Int = std::int32_t;

struct std_tag { };

// Tagged vector class, ast-InLists and ast-OutLists are now different types.
template<typename T, typename A = std::allocator<T>, typename Tag = std_tag>
class tagged_vector final : private std::vector<T, A> {

    public:

    using category = Tag;
    using vector = std::vector<T, A>;
    using difference_type = typename vector::difference_type;
    using value_type = typename vector::value_type;
    using reference = typename vector::reference;
    using const_reference = typename vector::const_reference;
    using pointer = typename vector::pointer;
    using const_pointer = typename vector::const_pointer;
    using iterator = typename vector::iterator;
    using const_iterator = typename vector::const_iterator;

    using vector::vector;
    using vector::push_back;
    using vector::emplace_back;
    using vector::back;
    using vector::pop_back;
    using vector::resize;
    using vector::size;
    using vector::empty;
    using vector::clear;
    using vector::reserve;
    using vector::data;
    using vector::begin;
    using vector::end;
    using vector::cbegin;
    using vector::cend;
    using vector::operator [ ];
};

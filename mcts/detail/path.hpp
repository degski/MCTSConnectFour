
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

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <iostream>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/pector.hpp>

#include "types.hpp"
#include "transition.hpp"


template<typename Tree>
class Path {

    // A stack-like structure...

    using ArcID = typename Tree::ArcID;
    using NodeID = typename Tree::NodeID;

    using Transition = Transition<Tree>;
    using iterator = typename tagged_vector<Transition>::iterator;
    using const_iterator = typename tagged_vector<Transition>::const_iterator;
    using reference = typename tagged_vector<Transition>::reference;
    using const_reference = typename tagged_vector<Transition>::const_reference;

    struct path_tag { };

    tagged_vector<Transition, std::allocator<Transition>, path_tag> m_path;

    public:

    Path ( ) noexcept :
        m_path { } {
    }
    Path ( const Transition & l_ ) noexcept :
        m_path { 1u, l_ } {
    }
    Path ( const ArcID a_, const NodeID t_ ) noexcept :
        m_path { 1u, Transition { a_, t_ } } {
    }

    void reset ( const ArcID a_, const NodeID t_ ) noexcept {
        m_path.resize ( 1u );
        m_path [ 0u ].arc = a_;
        m_path [ 0u ].target = t_;
    }

    void emplace ( Transition && l_ ) noexcept {
        m_path.emplace_back ( std::move ( l_ ) );
    }
    void push ( const Transition & l_ ) noexcept {
        m_path.push_back ( l_ );
    }
    void push ( const ArcID a_, const NodeID t_ ) noexcept {
        m_path.emplace_back ( a_, t_ );
    }
    [[ maybe_unused ]] Transition pop ( ) noexcept {
        const Transition r { m_path.back ( ) };
        m_path.pop_back ( );
        return r;
    }

    [[ nodiscard ]] reference back ( ) noexcept {
        return m_path.back ( );
    }
    [[ nodiscard ]] const_reference back ( ) const noexcept {
        return m_path.back ( );
    }

    [[ nodiscard ]] iterator begin ( ) noexcept {
        return m_path.begin ( );
    }
    [[ nodiscard ]] const_iterator begin ( ) const noexcept {
        return m_path.begin ( );
    }
    [[ nodiscard ]] const_iterator cbegin ( ) const noexcept {
        return m_path.cbegin ( );
    }

    [[ nodiscard ]] iterator end ( ) noexcept {
        return m_path.end ( );
    }
    [[ nodiscard ]] const_iterator end ( ) const noexcept {
        return m_path.end ( );
    }
    [[ nodiscard ]] const_iterator cend ( ) const noexcept {
        return m_path.cend ( );
    }

    void clear ( ) noexcept {
        m_path.clear ( );
    }
    void resize ( const size_t s_ ) noexcept {
        m_path.resize ( s_ );
    }
    void reserve ( const size_t s_ ) noexcept {
        m_path.reserve ( s_ );
    }

    template<typename Stream>
    [[ maybe_unused ]] friend Stream & operator << ( Stream & out_, const Path & p_ ) noexcept {
        for ( const auto & l : p_ ) {
            out_ << l;
        }
        out_ << L'\n';
        return out_;
    }

    private:

    friend class cereal::access;

    template < class Archive >
    void serialize ( Archive & ar_ ) {
        ar_ ( m_path );
    }
};

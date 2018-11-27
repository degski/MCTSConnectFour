
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

#include <random>
#include <iostream>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

#include "types.hpp"
#include "globals.hpp"


template<typename T, index_t S>
class Moves {

    typedef T moves_t [ S ]; // Typedeffing a c-array...

    index_t m_size = 0;
    moves_t m_moves;

    static const index_t s_capacity, s_binary_size;

public:

    using value_type = T;

    void clear ( ) noexcept {
        m_size = 0; // No destructor called.
    }

    [[ nodiscard ]] index_t size ( ) const noexcept {
        return m_size;
    }

    [[ nodiscard ]] index_t capacity ( ) const noexcept {
        return s_capacity;
    }

    [[ nodiscard ]] bool empty ( ) const noexcept {
        return not ( m_size );
    }

    [[ nodiscard ]] value_type at ( const index_t i_ ) const noexcept {
        assert ( i_ >= 0 );
        assert ( i_ < size ( ) );
        return m_moves [ i_ ];
    }

    [[ nodiscard ]] value_type front ( ) const noexcept {
        return m_moves [ 0 ];
    }

    void push_back ( const value_type m_ ) noexcept {
        m_moves [ m_size++ ] = m_;
        assert ( m_size <= S );
    }

    void emplace_back ( value_type && m_ ) noexcept {
        m_moves [ m_size++ ] = std::move ( m_ );
        assert ( m_size <= S );
    }

    [[ nodiscard ]] value_type random ( ) const noexcept {
        return m_moves [ std::uniform_int_distribution < ptrdiff_t > ( 0, m_size - 1 ) ( g_rng ) ];
    }

    [[ nodiscard ]] bool find ( const value_type m_ ) const noexcept {
        for ( index_t i = 0; i < m_size; ++i ) {
            if ( m_moves [ i ] == m_ ) {
                return true;
            }
        }
        return false;
    }

    [[ nodiscard ]] value_type draw ( ) noexcept {
        if ( 1 == m_size ) {
            return m_moves [ --m_size ];
        }
        else {
            const index_t i = std::uniform_int_distribution < index_t > ( 0, m_size - 1 ) ( g_rng );
            const value_type v = m_moves [ i ];
            m_moves [ i ] = m_moves [ --m_size ];
            assert ( m_size >= 0 );
            return v;
        }
    }

    [[ nodiscard ]] Moves & operator = ( const Moves & rhs_ ) noexcept {
        memcpy ( this, & rhs_, sizeof ( Moves ) );
        return * this;
    }

    void remove ( const value_type m_ ) noexcept {
        if ( m_size < 2 and m_moves [ 0 ] == m_ ) {
            m_size = 0;
            return;
        }
        const index_t last = m_size - 1;
        for ( index_t i = 0; i < last; ++i ) {
            if ( m_moves [ i ] == m_ ) {
                m_moves [ i ] = m_moves [ last ];
                --m_size;
                return;
            }
        }
        if ( m_moves [ last ] == m_ ) {
            --m_size;
        }
    }

    template < class Archive >
    void serialize ( Archive & ar_ ) noexcept {
        ar_ ( m_size );
        ar_ ( cereal::binary_data ( & m_moves, m_size * sizeof ( T ) ) );
    }
};

template < typename T, index_t S >
const index_t Moves < T, S >::s_capacity = S;

template < typename T, index_t S >
const index_t Moves < T, S >::s_binary_size = sizeof ( Moves < T, S > );

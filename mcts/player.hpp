
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

#include <iostream>

#include <cereal/cereal.hpp>

#include "types.hpp"
#include "globals.hpp"


class Player {

	typedef std::int8_t underlying_t;

public:

	enum class Type : underlying_t { invalid = -2, agent = -1, vacant = 0, human = 1 }; // Keep numbering this way...

private:

	Type m_value = Type::vacant;

public:

	Player ( ) noexcept { }
	Player ( const Type p_ ) noexcept : m_value ( p_ ) { }

	Type opponent ( ) const noexcept {

		return ( Type ) -( ( underlying_t ) m_value );
	}

	void next ( ) noexcept {

		m_value = opponent ( );
	}

	Type get ( ) const noexcept {

		return m_value;
	}

	index_t as_index ( ) const noexcept {

		return ( index_t ) m_value;
	}

	index_t as_01index ( ) const noexcept {

		switch ( as_index ( ) ) {

			case ( index_t ) Type::agent: return ( index_t ) 0;
			case ( index_t ) Type::human: return ( index_t ) 1;

			NO_DEFAULT_CASE;
		}
	}

	static Type random ( ) noexcept {

		return bernoulli ( ) ? Type::agent : Type::human;
	}

	bool invalid ( ) const noexcept {

		return m_value == Type::invalid;
	}

	bool agent ( ) const noexcept {

		return m_value == Type::agent;
	}

	bool vacant ( ) const noexcept {

		return m_value == Type::vacant;
	}

	bool human ( ) const noexcept {

		return m_value == Type::human;
	}

	bool occupied ( ) const noexcept {

		return ( index_t ) m_value & ( index_t ) 1;
	}

	void print ( ) const noexcept {

		switch ( m_value ) {

			case Type::invalid:	std::puts ( "invalid" );	break;
			case Type::agent:	std::puts ( "agent" );		break;
			case Type::vacant:	std::puts ( "vacant" );		break;
			case Type::human:	std::puts ( "human" );		break;

			NO_DEFAULT_CASE;
		}
	}

	bool operator == ( const Player p_ ) const noexcept {

		return m_value == p_.m_value;
	}

	bool operator != ( const Player p_ ) const noexcept {

		return m_value != p_.m_value;
	}

private:

	friend class cereal::access;

	template < class Archive >
	void serialize ( Archive & ar_ ) { ar_ ( m_value ); }
};

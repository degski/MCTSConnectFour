
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

#ifndef GRAPH_LINK_PATH_HPP
#define GRAPH_LINK_PATH_HPP

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <cassert>

#include <vector>
#include <iostream>

// #include <cereal/cereal.hpp>
// #include <cereal/archives/binary.hpp>
// #include <cereal/types/vector.hpp>

#include "types.hpp"


template < typename Graph >
struct Link {

	typedef typename Graph::Arc   Arc;
	typedef typename Graph::Node Node;

	Arc arc; Node target;

	Link ( ) noexcept { }
	Link ( Link && l_ ) noexcept : arc ( std::move ( l_.arc ) ), target ( std::move ( l_.target ) ) { }
	Link ( const Link & l_ ) noexcept : arc ( l_.arc ), target ( l_.target ) { }
    Link ( const Arc a_, const Node t_ ) noexcept : arc ( a_ ), target ( t_ ) { }
	Link ( const Arc a_ ) noexcept : arc ( a_ ) { }
	Link ( const Node t_ ) noexcept : target ( t_ ) { }

	bool operator == ( const Link & rhs_ ) { return arc == rhs_.arc && target == rhs_.target; }
	bool operator != ( const Link & rhs_ ) { return arc != rhs_.arc || target != rhs_.target; }

	Link & operator = ( const Link & l_ ) noexcept {

		arc = l_.arc;
		target = l_.target;

		return * this;
	}

private:

	// friend class cereal::access;

	// template < class Archive >
	// void serialize ( Archive & ar_ ) { ar_ ( arc, target ); }
};


template < typename Graph >
class Path {

	// A stack-like structure...

	typedef typename Graph::Arc   Arc;
	typedef typename Graph::Node Node;

	typedef Link < Graph > Link;

	std::vector < Link > m_path;

public:

	Path ( ) noexcept : m_path ( ) { }
	Path ( const Link & l_ ) noexcept : m_path ( 1, l_ ) { }
	Path ( const Arc a_, const Node t_ ) noexcept : m_path ( 1, Link ( a_, t_ ) ) { }

	void reset ( const Arc a_, const Node t_ ) noexcept {

		m_path.resize ( 1 );
		m_path [ 0 ].arc = a_;
		m_path [ 0 ].target = t_;
	}

	void emplace ( Link && l_ ) noexcept { m_path.emplace_back ( l_ ); }
	void push ( const Link & l_ ) noexcept { m_path.emplace_back ( l_ ); }
	void push ( const Arc a_, const Node t_ ) noexcept { m_path.emplace_back ( a_, t_ ); }
	Link pop ( ) noexcept { const Link r = m_path.back ( ); m_path.pop_back ( ); return r; }

	auto back ( ) const noexcept{ return m_path.back ( ); }

	auto begin ( ) const noexcept { return m_path.begin ( ); }
	auto end ( ) const noexcept { return m_path.end ( ); }

	auto cbegin ( ) const noexcept { return m_path.cbegin ( ); }
	auto cend ( ) const noexcept { return m_path.cend ( ); }

	void clear ( ) noexcept { m_path.clear ( ); }
	void resize ( const std::size_t s_ ) noexcept { m_path.resize ( s_ ); }
	void reserve ( const std::size_t s_ ) noexcept { m_path.reserve ( s_ ); }

	void print ( ) const noexcept {

		for ( const auto & l : m_path ) {

			std::cout << "[" << l.arc << ", " << l.target << "]";
		}

		std::cout << "\n";
	}

private:

	// friend class cereal::access;

	// template < class Archive >
	// void serialize ( Archive & ar_ ) { ar_ ( m_path ); }
};

#endif // GRAPH_LINK_PATH_HPP
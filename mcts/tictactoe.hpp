
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

#include <cstdlib>

#include <boost/optional.hpp>

#include <multi_array.hpp>

#include "typedefs.hpp"
#include "player.hpp"
#include "moves.hpp"


struct Location { // 2

	typedef int8_t type;

	static const type invalid = -3;

	union {

		struct {

			type c, r;
		};

		uint16_t v;
	};

	Location ( ) noexcept : c ( invalid ), r ( invalid ) { }
	Location ( const index_t c_, const index_t r_ ) noexcept : c ( c_ ), r ( r_ ) { }
	Location ( const Location & l_ ) noexcept : v ( l_.v ) { }
	Location ( Location && l_ ) noexcept : v ( std::move ( l_.v ) ) { }

	Location operator - ( const Location & rhs_ ) const {

		return Location ( c - rhs_.c, r - rhs_.r );
	}

	Location & operator = ( const Location & rhs_ ) noexcept {

		v = rhs_.v;

		return *this;
	}

	void print ( ) const {

		std::cout << " loc [" << ( index_t ) c << ", " << ( index_t ) r << "]\n";
	}
};



struct Move { // 2

	static const Move none;
	static const Move root;
	static const Move invalid;

	union {

		struct {

			Location m_loc;
		};

		uint16_t v;
	};

	Move ( ) noexcept : v ( invalid.v ) { }
	Move ( Location && f_ ) noexcept : m_loc ( std::move ( f_ ) ) { }
	Move ( const Move & m_ ) noexcept : v ( m_.v ) { }
	Move ( Move && m_ ) noexcept : v ( std::move ( m_.v ) ) { }

	Move & operator = ( const Move & rhs_ ) noexcept {

		v = rhs_.v;

		return * this;
	}

	bool operator == ( const Move & rhs_ ) const noexcept {

		return v == rhs_.v;
	}

	bool operator != ( const Move & rhs_ ) const noexcept {

		return v != rhs_.v;
	}
};



template < size_t NumRows = 3, size_t NumCols = 3 >
class TicTacToe {

public:

	typedef Player Player;

	typedef ma::MatrixCM < Player, NumRows, NumCols > Board;

	typedef uint64_t ZobristHash;
	typedef ma::Cube < ZobristHash, 2, NumRows, NumCols > ZobristHashKeys; // 2 players, 0 based...

	typedef Move Move;
	typedef Moves < Move, NumCols > Moves;

private:

	Board m_board;
	Player m_player_just_moved = Player::random ( ), m_winner = Player::type::vacant; // Human starts...
	Move m_move;
	index_t m_no_moves = 0; // Records last move...

public:

	TicTacToe ( ) { }
	TicTacToe ( const TicTacToe & rhs_ ) noexcept {

		memcpy ( this, & rhs_, sizeof ( TicTacToe ) );
	}


	Player playerJustMoved ( ) const noexcept {

		return m_player_just_moved;
	}

	Player playerToMove ( ) const noexcept {

		return m_player_just_moved.opponent ( );
	}


	void winner ( ) noexcept {

		// We only need to check around the last piece played...

		const Player piece = m_board.at ( m_move.m_loc.r, m_move.m_loc.c );

		// Horizontal...

		index_t left_right = 1;

		for ( index_t col = m_move.m_loc.c - 1; col >= 0 && m_board.at ( m_move.m_loc.r, col ) == piece; --col ) ++left_right;
		for ( index_t col = m_move.m_loc.c + 1; col < NumCols && m_board.at ( m_move.m_loc.r, col ) == piece; ++col ) ++left_right;

		if ( left_right == 3 ) { m_winner = piece; return; }

		// Vertical...

		index_t up_down = 1;

		for ( index_t row = m_move.m_loc.r - 1; row >= 0 && m_board.at ( row, m_move.m_loc.c ) == piece; --row ) ++up_down;
		for ( index_t row = m_move.m_loc.r + 1; row < NumRows && m_board.at ( row, m_move.m_loc.c ) == piece; ++row ) ++up_down;

		if ( up_down == 3 ) { m_winner = piece; return; }

		// Diagonal nw to se...

		up_down = 1;

		for ( index_t row = m_move.m_loc.r - 1, col = m_move.m_loc.c - 1; row >= 0 && col >= 0 && m_board.at ( row, col ) == piece; --row, --col ) ++up_down;
		for ( index_t row = m_move.m_loc.r + 1, col = m_move.m_loc.c + 1; row < NumRows && col < NumCols && m_board.at ( row, col ) == piece; ++row, ++col ) ++up_down;

		if ( up_down == 3 ) { m_winner = piece; return; }

		// Diagonal sw to ne...

		up_down = 1;

		for ( index_t row = m_move.m_loc.r + 1, col = m_move.m_loc.c - 1; row < NumRows && col >= 0 && m_board.at ( row, col ) == piece; ++row, --col ) ++up_down;
		for ( index_t row = m_move.m_loc.r - 1, col = m_move.m_loc.c + 1; row >= 0 && col < NumCols && m_board.at ( row, col ) == piece; --row, ++col ) ++up_down;

		if ( up_down == 3 ) { m_winner = piece; return; }
	}


	void move ( const Move move_ ) noexcept {

		m_move = move_;

		m_player_just_moved.next ( );

		m_board.at ( m_move.m_loc.r, m_move.m_loc.c ) = m_player_just_moved;

		++m_no_moves;
	}

	inline void move_winner ( const Move move_ ) noexcept {

		move ( move_ );
		winner ( );
	}


	inline bool moves ( Moves * m_ ) const noexcept {

		if ( m_no_moves == NumRows * NumCols or m_winner.occupied ( ) ) {

			return false;
		}

		m_->clear ( );

		for ( index_t col = 0; col < NumCols; ++col ) {

			for ( index_t row = 0; row < NumRows; ++row ) {

				if ( m_board.at ( row, col ).vacant ( ) ) {

					m_->emplace_back ( Location ( row, col ) );
				}
			}
		}

		return true;
	}


	void simulate ( ) noexcept {

		Moves m;

		while ( moves ( & m ) ) {

			move_winner ( m.random ( ) );
		}
	}


	inline float result ( const Player player_just_moved_ ) const noexcept {

		// Determine result: last player of path is the player to move...

		return m_winner.vacant ( ) ? 0.5f : ( m_winner == player_just_moved_ ? 1.0f : 0.0f );
	}


	boost::optional < Player > ended ( ) const noexcept {

		return ( m_winner.vacant ( ) and ( m_no_moves < NumRows * NumCols ) ) ? boost::optional < Player > ( ) : boost::optional < Player > ( m_winner );
	}
};

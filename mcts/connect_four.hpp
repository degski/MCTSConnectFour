
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
#include <array>

#include <optional>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

#include "multi_array.hpp"

#include "globals.hpp"
#include "types.hpp"
#include "player.hpp"
#include "moves.hpp"


struct Move { // 1

	typedef int8_t type;

	static const Move none;
	static const Move root;
	static const Move invalid;

	type m_loc;

	Move ( ) noexcept : m_loc ( invalid.m_loc ) { }
	Move ( const Move & m_ ) noexcept : m_loc ( m_.m_loc ) { }
	Move ( const index_t m_ ) noexcept : m_loc ( m_ ) { }
	Move ( Move && m_ ) noexcept : m_loc ( std::move ( m_.m_loc ) ) { }

	Move & operator = ( const Move & rhs_ ) noexcept {
		m_loc = rhs_.m_loc;
		return *this;
	}

	bool operator == ( const Move & rhs_ ) const noexcept {
		return m_loc == rhs_.m_loc;
	}

	bool operator != ( const Move & rhs_ ) const noexcept {
		return m_loc != rhs_.m_loc;
	}

	void print ( ) const noexcept {
		std::cout << ( int ) m_loc;
	}

private:

	friend class cereal::access;

	template < class Archive >
	void serialize ( Archive & ar_ ) { ar_ ( m_loc ); }
};

const Move Move::none = -1;
const Move Move::root = -2;
const Move Move::invalid = -3;


template < std::size_t NumRows = 6, std::size_t NumCols = 7 >
class ConnectFour {

public:

	typedef Player Player;

	typedef ma::MatrixCM < Player, NumRows, NumCols > Board;

	typedef std::uint64_t ZobristHash;
	typedef ma::Cube < ZobristHash, 2, NumRows, NumCols > ZobristHashKeys; // 2 players, 0 based...

	typedef Move Move;
	typedef Moves < Move, NumCols > Moves;

private:

	// 16 + 42 + 1 + 1 + 1 + 1 + 2 = 64 bytes...

	ZobristHash m_zobrist_hash = m_zobrist_player_keys [ ( index_t ) Player::type::vacant ]; // Hash of the current m_board...
	Board m_board;
	uint8_t m_no_moves = 0; // Records last move...
	Player m_player_just_moved = Player::random ( ), m_winner = Player::type::invalid; // Human starts...
	Move m_move = Move::root;
	uint8_t _padding [ 2 ] = { 0, 0 };

public:

	static const ZobristHashKeys m_zobrist_keys;
	static const ZobristHash m_zobrist_player_key_values [ 3 ];
	static const ZobristHash * m_zobrist_player_keys;
	static constexpr index_t max_no_moves = NumCols;

	ConnectFour ( ) noexcept { }

	void initialize ( ) noexcept {

		m_zobrist_hash = m_zobrist_player_keys [ ( index_t ) Player::type::vacant ];
		m_no_moves = 0;
		m_player_just_moved = Player::random ( );
		m_winner = Player::type::invalid;
		m_move = Move::root;
		memset ( _padding, 0, sizeof ( _padding ) );
	}

	ConnectFour ( const ConnectFour & rhs_ ) noexcept {

		memcpy ( this, & rhs_, sizeof ( ConnectFour ) );
	}


	bool operator == ( const ConnectFour & rhs_ ) const noexcept {

		return memcmp ( this, & rhs_, sizeof ( ConnectFour ) ) == 0;
	}

	bool operator != ( const ConnectFour & rhs_ ) const noexcept {

		return memcmp ( this, & rhs_, sizeof ( ConnectFour ) ) != 0;
	}


	Player playerJustMoved ( ) const noexcept {

		return m_player_just_moved;
	}

	Player playerToMove ( ) const noexcept {

		return m_player_just_moved.opponent ( );
	}


	struct Coordinates {

		index_t row = NumRows, col;

		Coordinates ( const Move m_ ) noexcept : col ( ( index_t ) m_.m_loc ) { }
	};


	void winner ( Coordinates && coordinates_ ) noexcept {

		// We only need to check around the last piece played...

		const Player piece = m_board.at ( coordinates_.row, coordinates_.col );

		// Horizontal...

		index_t counter = 1;

		if ( coordinates_.col > 0 ) {

			for ( index_t col = coordinates_.col - 1; col >= 0 && m_board.at ( coordinates_.row, col ) == piece; --col ) ++counter;
		}

		if ( coordinates_.col < ( ( NumCols - 4 ) + counter ) ) {

			for ( index_t col = coordinates_.col + 1; col < NumCols && m_board.at ( coordinates_.row, col ) == piece; ++col ) ++counter;

			if ( counter > 3 ) { m_winner = piece; return; }
		}

		// Vertical...

		if ( coordinates_.row < ( NumRows - 3 ) ) {

			counter = 1;

			for ( index_t row = coordinates_.row + 1; row < NumRows && m_board.at ( row, coordinates_.col ) == piece; ++row ) ++counter;

			if ( counter > 3 ) { m_winner = piece; return; }
		}

		// Diagonal nw to se...

		counter = 1;

		if ( coordinates_.row > 0 and coordinates_.col > 0 ) {

			for ( index_t row = coordinates_.row - 1, col = coordinates_.col - 1; row >= 0 && col >= 0 && m_board.at ( row, col ) == piece; --row, --col ) ++counter;
		}

		if ( coordinates_.row < ( ( NumRows - 4 ) + counter ) and coordinates_.col < ( ( NumCols - 4 ) + counter ) ) {

			for ( index_t row = coordinates_.row + 1, col = coordinates_.col + 1; row < NumRows && col < NumCols && m_board.at ( row, col ) == piece; ++row, ++col ) ++counter;

			if ( counter > 3 ) { m_winner = piece; return; }
		}

		// Diagonal sw to ne...

		counter = 1;

		if ( coordinates_.row < ( NumRows - 1 ) and coordinates_.col > 0 ) {

			for ( index_t row = coordinates_.row + 1, col = coordinates_.col - 1; row < NumRows && col >= 0 && m_board.at ( row, col ) == piece; ++row, --col ) ++counter;
		}

		if ( coordinates_.row > ( 3 - counter ) and coordinates_.col < ( ( NumCols - 4 ) + counter ) ) {

			for ( index_t row = coordinates_.row - 1, col = coordinates_.col + 1; row >= 0 && col < NumCols && m_board.at ( row, col ) == piece; --row, ++col ) ++counter;

			if ( counter > 3 ) { m_winner = piece; return; }
		}

		// Draw?

		if ( NumRows * NumCols == m_no_moves ) {

			m_winner = Player::type::vacant;
		}
	}


	Move lastMove ( ) const noexcept {

		return m_move;
	}


	[ [ maybe_unused ] ] Coordinates move ( const Move move_ ) noexcept {

		m_move = move_;

		Coordinates c ( move_ );

		while ( m_board.at ( --c.row, c.col ).occupied ( ) );

		m_player_just_moved.next ( );

		m_board.at ( c.row, c.col ) = m_player_just_moved;

		++m_no_moves;

		return c;
	}


	[ [ maybe_unused ] ] Coordinates hash ( Coordinates && coordinates_ ) noexcept {

		m_zobrist_hash ^= m_zobrist_keys.at ( m_player_just_moved.as_01index ( ), coordinates_.row, coordinates_.col );

		return coordinates_;
	}


	void move_hash ( const Move move_ ) noexcept {

		hash ( move ( move_ ) );
	}

	void move_hash_winner ( const Move move_ ) noexcept {

		winner ( hash ( move ( move_ ) ) );
	}

	void move_winner ( const Move move_ ) noexcept {

		winner ( move ( move_ ) );
	}


	ZobristHash zobrist ( ) const noexcept {

		return m_zobrist_hash ^ m_zobrist_player_keys [ m_player_just_moved.as_index ( ) ]; // Order of hashes is not relevant, as long it's the same every time...
	}


	[ [ maybe_unused ] ] bool moves ( Moves * m_ ) const noexcept {

		m_->clear ( );

		if ( NumRows * NumCols == m_no_moves or m_winner != Player::type::invalid ) {

			return false;
		}

		for ( index_t col = 0; col < NumCols; ++col ) {

			if ( m_board.at ( 0, col ).vacant ( ) ) {

				m_->push_back ( col );
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


	float result ( const Player player_just_moved_ ) const noexcept {

		// Determine result: last player of path is the player to move...

		// return m_winner.vacant ( ) ? 0.5f : ( m_winner == player_just_moved_ ? 1.0f : 0.0f ); // Score...

		return m_winner.vacant ( ) ? 0.0f : ( m_winner == player_just_moved_ ? 1.0f : -1.0f ); // Win-Rate...
	}


	std::optional<Player> ended ( ) const noexcept {

		return m_winner == Player::type::invalid ? std::optional<Player> ( ) : std::optional<Player> ( m_winner );
	}


	void print ( ) const noexcept { // Some c-style printing... :-)

		static const char player_markers_print [ 3 ] { 'C', '.', 'H' };

		std::fputs ( " +", stdout );

		for ( index_t col = 0; col < NumCols - 1; ++col ) {

			std::fputs ( "--", stdout );
		}

		std::puts ( "-+" );

		std::fputs ( " |", stdout );

		for ( index_t col = 48; col < NumCols + 47; ++col ) {

			std::putchar ( col );
			std::putchar ( ' ' );
		}

		std::putchar ( NumCols + 47 );
		std::puts ( "|" );

		std::fputs ( " +", stdout );

		for ( index_t col = 0; col < NumCols - 1; ++col ) {

			std::fputs ( "--", stdout );
		}

		std::puts ( "-+" );

		for ( index_t row = 0; row < NumRows; ++row ) {

			std::fputs ( " |", stdout );

			index_t col = 0;

			for ( ; col < NumCols - 1; ++col ) {

				std::putchar ( ( player_markers_print + 1 ) [ m_board.at ( row, col ).as_index ( ) ] );
				std::putchar ( ' ' );
			}

			std::putchar ( ( player_markers_print + 1 ) [ m_board.at ( row, col ).as_index ( ) ] );
			std::puts ( "|" );
		}

		std::fputs ( " +", stdout );

		for ( index_t col = 0; col < NumCols - 1; ++col ) {

			std::fputs ( "--", stdout );
		}

		std::puts ( "-+" );
	}

	void save ( ) {

		std::ofstream os ( g_app_data_path / std::string ( "connect_four_state.cereal" ), std::ios::binary );

		cereal::BinaryOutputArchive archive ( os );

		archive ( * this );

		os.flush ( );
		os.close ( );
	}

	void load ( ) {

		std::ifstream is ( g_app_data_path / std::string ( "connect_four_state.cereal" ), std::ios::binary );

		cereal::BinaryInputArchive archive ( is );

		archive ( * this );

		is.close ( );
	}

private:

	friend class cereal::access;

	template < class Archive >
	void serialize ( Archive & ar_ ) { ar_ ( * this ); }

}; // __attribute__ ( ( packed ) );

template < std::size_t NumRows, std::size_t NumCols >
const typename ConnectFour < NumRows, NumCols >::ZobristHashKeys ConnectFour < NumRows, NumCols >::m_zobrist_keys {

	0xa1a656cb9731c5d5ull, 0xc3dce6ad6465ea7aull, 0x9e2556e2bbec18d3ull, 0x900670630f4f76afull,
	0xda8071005889fa3cull, 0xd1efb50aec8b61a9ull, 0x73203d10cf4db8b8ull, 0x6ab7fd70679d877full,
	0x3a56cdae74f9d816ull, 0xb3b48dc62bacaf9bull, 0x27760b12660e6c3bull, 0xd9ac7fb482854702ull,
	0xd35e698b064e4f93ull, 0x7b379503f68242bdull, 0xdad6afcb4409d282ull, 0xf04b592c8e1183feull,
	0x6dbb4f77e63f5267ull, 0x970b0ae4e9e7d347ull, 0xd19027f157c2845aull, 0x82a53746e2d25fa5ull,
	0xe2097dbb17c142f7ull, 0x5eba98d936a14c91ull, 0x963286f60ab69777ull, 0x96e9eb899e5e615bull,
	0xecd8957747d0bef8ull, 0x961b3fb52b112218ull, 0x44c776ac7af4cc2dull, 0xfa2708e399719ac4ull,
	0xe34b58c2f6acac45ull, 0x7f6d2cb0416a63caull, 0x287ecf88477a3e7dull, 0xe57d268150b95703ull,
	0xf9cc76357617493cull, 0xe956f77acaa2f112ull, 0x9a9441286a0a70e7ull, 0x5b5a62ba1d8dfd33ull,
	0xb3d1b947205bf8f4ull, 0x4aabdee7fb6aa20bull, 0xa810d257d77576afull, 0x6a1789922b7af41aull,
	0x315833a0f0b5ceebull, 0x481a32e97fbd47d8ull, 0x11e80a41d2022fdcull, 0xfab59400ba6c780cull,
	0xfce9f47e1dc3037dull, 0xf5f404421f6c78b2ull, 0x274ef7151bd8503eull, 0x1d5268cdadd43ad3ull,
	0x59ed9dc04b81a0c1ull, 0x3c10ea92d1a6d79dull, 0x595d9292d07ee51dull, 0x1a62a32bb174ee71ull,
	0x417fd9b9b0bc7a47ull, 0x3e266eca431347d6ull, 0x74a093aeceb1fd60ull, 0x7720a5e78ae8d571ull,
	0x9645ae72f6f57362ull, 0xcc7279ab05731ef7ull, 0xf5a0574bc2385c6full, 0xb254ccf017ebc43bull,
	0x34184cd5945aff3eull, 0x4c5ede78a68fd1a5ull, 0x49adf513d838ce5dull, 0x44940842e2c75c16ull,
	0x7aacd877d0831e19ull, 0x9d8d5e4f7c511acdull, 0xac2f78583e0e9692ull, 0x03e2da677110440cull,
	0x07d2a6b527f4ef05ull, 0x91a680f12222cf16ull, 0x08617f45641626d0ull, 0xb2df85147e2a11cbull,
	0x6bf333747f7f10a4ull, 0xc6f2a33e3a94b2c1ull, 0xf5358b1cb75e528full, 0x904af33725c150b5ull,
	0xd75d6d3f202f964bull, 0x8d58eeece3979331ull, 0xb58f905351a0d8f1ull, 0x38ad67581ffcbdfbull,
	0xcd5f48e9ac464398ull, 0xfcc2df3237564c0cull, 0x1ea8202ddf77efdeull, 0x000617fafba044adull
};

template < std::size_t NumRows, std::size_t NumCols >
const typename ConnectFour < NumRows, NumCols >::ZobristHash ConnectFour < NumRows, NumCols >::m_zobrist_player_key_values [ 3 ] {

	0x41fec34015a1bef2ull, 0x8b80677c9c144514ull, 0xf6242292160d5bb7ull
};

template < std::size_t NumRows, std::size_t NumCols >
const typename ConnectFour < NumRows, NumCols >::ZobristHash * ConnectFour < NumRows, NumCols >::m_zobrist_player_keys {

	m_zobrist_player_key_values + 1
};

/* Spare hash keys...

0xe028283c7b3c8bc3ull, 0x0fce58188743146dull, 0x5c0d56eb69eac805ull

*/

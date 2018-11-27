
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
#include <cstring>

#include <array>
#include <vector>
#include <iostream>
#include <numeric>
#include <utility>
#include <memory>
#include <random>
#include <functional>

#ifdef COMETA
#include <cometa.hpp>
template < typename Function >
using function = cometa::function < Function >;
#else
template < typename Function >
using function = std::function < Function >;
#endif

#include <algorithm>
#include <optional>

using namespace std::placeholders;  // for _1, _2, _3...

#include <boost/container/static_vector.hpp>

#include <spatial/idle_point_multimap.hpp>
#include <spatial/neighbor_iterator.hpp>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

#include "multi_array.hpp"
#include <integer_utils.hpp>
#include "autotimer.hpp"

#include "types.hpp"
#include "globals.hpp"
#include "player.hpp"
#include "moves.hpp"
#include "resourcedata.hpp"


// ------------------------------------ OSKA ---------------------------------------
//
// OSKA is a speedy game for 2 players which is deceptively simple, using the
// Draughts (checkers) principle of diagonal movement and capture. BUT � keep well in
// mind the quirk that gives OSKA its bite. The winner is the human whose remaining
// pieces first reach the far side � the less pieces you have left, the easier this
// will be. The skill lies in when, or if, to capture, and when to force your
// opponent to capture you.� You take a piece by jumping it, so this can only be
// done forwards and so long as the space the other side of it is empty. You do not
// HAVE to take as the rules state �when, or if, to capture�. The winner is the
// first one to have ALL of their REMAINING pieces to the other side, so if all of
// their pieces are taken except one and they get that one to the other side before
// the other human gets all four of their pieces across, they win.
//
// Strategy:
//
// Now while the game is simple (once you�ve clarified the rules) and fast, there�s a
// lot that could be said about the strategy. For starters you cannot afford to make
// mistakes, I know this can be said for almost all games but one mistake in OSKA and
// the other person most often enters a state where they can�t be beaten because all
// series of moves that follow result in their victory!
//
// The key to the game, I believe, lies in forcing the other person to take you. You
// can create a series of plays where they have no option but to take you because
// they have no other available moves. And once you start to lose pieces you have an
// easier job than they do as you have less pieces to get across the board.
//
// I could go on about strategy and start drawing diagrams and things of this nature
// (as that how �into� this game I have got, a game that less than a month ago I was
// unaware existed). But I will spare you all, however, in the future I may write a
// specific post on the dynamics and maths of the game, which everyone can feel free
// not to read.
//
// History and Interesting Facts:
//
// Unfortunately due to the obscurity of this game the history and fasts that are
// about to follow are mostly off the back of the box as there�s not much more
// information on the game out there. Which is a shame as it�s a good game.
//
//     The game was originally devised by Bryn Jones in the 1950s.
//     Bryn Jones was a miner so the game was originally played scratched into the
//        dust on the floor of a mine at Lancashire Coalfields to pass time at breaks.
//     In the early 1990s Bryn brought the game to Michael Woodward Creations.
//     Woodward Creations annotated the rules and refined it into the more
//        presentable format you find it in now.
//     It can now be found as part of The Inventors Collection which is a gathering
//        of games and puzzles from the worlds top game inventors.
//     The game is made of eco-friendly wood.
//     It can be considered as part of the Draughts family of games.


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

private:

	friend class cereal::access;

	template < class Archive >
	void serialize ( Archive & ar_ ) { ar_ ( v ); }
};


struct Move { // 4

	static const Move none;
	static const Move root;
	static const Move invalid;

	enum class type : int8_t { LeftCapture = -2, LeftMove, None, RightMove, RightCapture, Invalid, UnInitialised, NoMove, RootMove };

	union {
		struct {
			Location m_from, m_to;
		};
		std::uint32_t v;
	};

	Move ( ) noexcept : v ( invalid.v )  { }
	Move ( Location && f_ ) noexcept : m_from ( std::move ( f_ ) ) { }
	Move ( const Location & f_, const Location & t_ ) noexcept : m_from ( f_ ), m_to ( t_ ) { }
	Move ( Location && f_, Location && t_ ) noexcept : m_from ( std::move ( f_ ) ), m_to ( std::move ( t_ ) ) { }
	Move ( const Move & m_ ) noexcept : v ( m_.v ) { }
	Move ( Move && m_ ) noexcept : v ( std::move ( m_.v ) ) { }

	Move & operator = ( const Move & rhs_ ) noexcept {
		v = rhs_.v;
		return *this;
	}

	bool operator == ( const Move & rhs_ ) const noexcept {
		return v == rhs_.v;
	}

	bool operator != ( const Move & rhs_ ) const noexcept {
		return v != rhs_.v;
	}

	bool isMove ( ) const noexcept {

		return m_to.r - m_from.r == 1 and std::abs ( m_to.c - m_from.c ) == index_t ( 1 );
	}

	bool isCapture ( ) const noexcept {
		return m_to.r - m_from.r == 2 and std::abs ( m_to.c - m_from.c ) == index_t ( 2 );
	}

	Location captured ( ) const noexcept {
		return Location	( ( m_from.c + m_to.c ) / 2, ( m_from.r + m_to.r ) / 2 );
	}

private:

	friend class cereal::access;

	template < class Archive >
	void serialize ( Archive & ar_ ) { ar_ ( v ); }
};

const Move Move::none = Move ( Location ( -1, -1 ), Location ( -1, -1 ) );
const Move Move::root = Move ( Location ( -2, -2 ), Location ( -2, -2 ) );
const Move Move::invalid;


#define NO_COLS( S ) ( S )
#define NO_ROWS( S ) ( 2 * ( ( S ) - 2 ) + 1 )

#define OB_COLS( S ) ( NO_COLS ( ( S ) ) + ( ( S ) - 1 ) + 2 )

#define OB_ROWS( S ) ( NO_ROWS ( ( S ) ) + 2 )

#define OB_HOME_ROW( S ) ( OB_ROWS ( ( S ) ) - 2 )

#define NO_HEXAGONS( S ) ( ( ( S ) * ( ( S ) + 1 ) ) - 4 )


typedef sf::Vector2f Point;


struct Hexagon { // 8

private:

	static float m_hori, m_vert, m_2_vert, m_2_vert_hori, m_center_l2radius;

public:

	Point m_center, m_offset;

	static void print_params ( ) noexcept {

		std::cout << m_hori << "\n";
		std::cout << m_vert << "\n";
		std::cout << m_2_vert << "\n";
		std::cout << m_2_vert_hori << "\n";
	}

	void print ( ) const noexcept {

		std::cout << " cen [" << m_center.x << ", " << m_center.y << "]\n";
		std::cout << " off [" << m_offset.x << ", " << m_offset.y << "]\n";
	}

private:

	static std::normal_distribution < float > m_disx, m_disy;

public:

	Hexagon ( ) { }

	Hexagon ( Point && c_, Point && o_ = Point ( m_disx ( g_rng ), m_disy ( g_rng ) ) ) noexcept :

		m_center ( std::move ( c_ ) ),
		m_offset ( std::move ( o_ ) ) {
	}

	static void initialize ( const sf::Vector2f d_ ) noexcept { // To be called once for initialisation of static variables...

		m_hori = 0.5f * d_.x, m_vert = 0.25f * d_.y;
		m_2_vert = 2.0f * m_vert, m_2_vert_hori = 2.0f * m_vert * m_hori;

		m_center_l2radius = ( ( 2.0f / 3.0f ) * ( 2.0f / 3.0f ) ) * m_vert * m_vert;

		m_disx = std::normal_distribution < float > ( 0.0f, 0.25f * std::sqrt ( d_.x ) );
		m_disy = std::normal_distribution < float > ( 0.0f, 0.25f * std::sqrt ( d_.y ) );
	}

	bool contains ( const Point & p_ ) const noexcept {

		// http://www.playchilla.com/how-to-check-if-a-point-is-inside-a-hexagon

		// Transform the test point locally and to quadrant 2...

		const Point q2 ( std::fabs ( p_.x - m_center.x ), std::fabs ( p_.y - m_center.y ) );

		// Bounding test (since q2 is in quadrant 2 only 2 tests are needed)...

		if ( q2.x > m_hori or q2.y > m_2_vert ) {

			return false;
		}

		// Finally, the dot product can be reduced to this due to the hexagon symmetry...

		return m_2_vert_hori - m_vert * q2.x - m_hori * q2.y >= 0.0f;
	}

	bool isClose ( const Point & point_ ) const noexcept {

		return ( point_.x - m_center.x ) * ( point_.x - m_center.x ) + ( point_.y - m_center.y ) * ( point_.y - m_center.y ) < m_center_l2radius;
	}

	Point point ( ) const noexcept {

		return m_center + m_offset;
	}

	Point center ( ) const noexcept {

		return m_center;
	}

	Point & center ( ) noexcept {

		return m_center;
	}

	Point offset ( ) const noexcept {

		return m_offset;
	}

	void setOffset ( const Point & o_ ) noexcept {

		m_offset = o_;
	}

	void setRandomOffset ( ) noexcept {

		m_offset.x = m_disx ( g_rng ), m_offset.y = m_disy ( g_rng );
	}

	void setOffsetFromPoint ( const Point & p_ ) noexcept {

		m_offset = p_ - m_center;
	}

private:

	friend class cereal::access;

	template < class Archive >
	void serialize ( Archive & ar_ ) { ar_ ( m_center, m_offset ); }
};

float Hexagon::m_hori;
float Hexagon::m_vert;
float Hexagon::m_2_vert;
float Hexagon::m_2_vert_hori;
float Hexagon::m_center_l2radius;

std::normal_distribution<float> Hexagon::m_disx;
std::normal_distribution<float> Hexagon::m_disy;


typedef boost::container::static_vector < int8_t, 8 > StoneID;


template < index_t S >
class OskaStateTemplate {

public:

	static constexpr index_t max_no_moves = 2 * S;

	typedef Player Player;
	typedef ZobristHash ZobristHash;
	typedef Move Move;
	typedef Moves < Move, max_no_moves > Moves;

private:

	typedef ma::Vector < Hexagon, NO_HEXAGONS ( S ) > Hexagons;
	typedef ma::MatrixRM < index_t, OB_COLS ( S ), OB_ROWS ( S ) > LocationToID;
	typedef ma::Vector < Location, NO_HEXAGONS ( S ) > IDToLocation;
	typedef ma::MatrixRM < Player, OB_COLS ( S ), OB_ROWS ( S ) > Board;
	typedef ma::Cube < ZobristHash, 2, OB_COLS ( S ), OB_ROWS ( S ) > ZobristHashKeys;

	typedef std::array < float, 2 > PointArray;
	typedef spatial::idle_point_multimap < 2, PointArray, index_t > PointToID;

	static Hexagons m_hexagons;
	static PointToID m_point_to_id;				// Lookup table from Point to Hexagon-id...
	static LocationToID m_location_to_id;		// Lookup table from Location to Hexagon-id...
	static IDToLocation m_id_to_location;		// Lookup table from Hexagon-id to Location...

	ZobristHash m_zobrist_hash = m_zobrist_player_keys [ ( index_t ) Player::type::vacant ]; // Hash of the current m_board...

	Board m_agent_board, m_human_board;			// Board from agent's and human's perspective, respectively...
	StoneID m_agent_stone_id, m_human_stone_id; // Stones from agent's perspective...

	index_t m_no_home_agent = 0, m_no_home_human = 0;

	Player m_player_to_move = Player::random ( ), m_winner = Player::type::invalid;

	Move m_last_move = Move::root;

	bool is_once_initialized = false;

	static ZobristHashKeys m_zobrist_keys;
	static const ZobristHash m_zobrist_player_key_values [ 3 ];
	static const ZobristHash * m_zobrist_player_keys;

public:

	OskaStateTemplate ( ) noexcept { }

	OskaStateTemplate ( const OskaStateTemplate & s_ ) noexcept {

		std::memcpy ( this, & s_, sizeof ( OskaStateTemplate ) );
	}

	// Initialise board and indices...

	void once_initialize ( ) {

		ResourceData resource_data ( S );

		Hexagon::initialize ( resource_data.m_xara_hex_dim );

		// Set all fields of boards to Player::type::invalid...

		index_t r, c;

		for ( r = 0; r < OB_ROWS ( S ); ++r ) {

			for ( c = 0; c < OB_COLS ( S ); ++c ) {

				m_location_to_id.at ( c, r ) = -1;
				m_human_board.at ( c, r ) = m_agent_board.at ( c, r ) = Player::type::invalid;
			}
		}

		std::uniform_int_distribution < std::size_t > dist;

		// Top of the board...

		m_agent_stone_id.reserve ( S );

		index_t li = 1, ri = OB_COLS ( S ) - 1, id = 0;

		float y = 0.5f * resource_data.m_xara_hex_dim.y + resource_data.m_margin;

		for ( r = 1; r < OB_ROWS ( S ) / 2; ++r, ++li, --ri ) {

			for ( c = li; c < ri; c += 2 ) {

				m_hexagons.at ( id ) = std::move ( Hexagon ( Point ( c * 0.5f * resource_data.m_xara_hex_dim.x + resource_data.m_margin, y ) ) );

				m_point_to_id.insert ( std::make_pair ( toArray ( m_hexagons.at ( id ).center ( ) ), id ) );

				m_location_to_id.at ( c, r ) = id;
				m_id_to_location.at ( id ) = std::move ( Location ( c, r ) );

				m_human_board.at_r ( c, r ) = m_agent_board.at ( c, r ) = r == 1 ? Player::type::agent : Player::type::vacant;

				m_zobrist_keys.at ( 0, c, r ) = dist ( g_rng );
				m_zobrist_keys.at ( 1, c, r ) = dist ( g_rng );

				if ( r == 1 ) {

					const Player player = Player::type::agent;

					m_zobrist_hash ^= m_zobrist_keys.at ( player.as_01index ( ), c, r );
					m_agent_stone_id.emplace_back ( id );
				}

				++id;
			}

			y += 0.75f * resource_data.m_xara_hex_dim.y;
		}

		// Bottom of the board, r, li and ri "fall through" from top of board...

		m_human_stone_id.reserve ( S );

		for ( ; r < OB_ROWS ( S ) - 1; ++r, --li, ++ri ) {

			for ( c = li; c < ri; c += 2 ) {

				m_hexagons.at ( id ) = std::move ( Hexagon ( Point ( c * 0.5f * resource_data.m_xara_hex_dim.x + resource_data.m_margin, y ) ) );

				m_point_to_id.insert ( std::make_pair ( toArray ( m_hexagons.at ( id ).center ( ) ), id ) );

				m_location_to_id.at ( c, r ) = id;
				m_id_to_location.at ( id ) = std::move ( Location ( c, r ) );

				m_human_board.at_r ( c, r ) = m_agent_board.at ( c, r ) = r == OB_HOME_ROW ( S ) ? Player::type::human : Player::type::vacant;

				m_zobrist_keys.at ( 0, c, r ) = dist ( g_rng );
				m_zobrist_keys.at ( 1, c, r ) = dist ( g_rng );

				if ( r == OB_HOME_ROW ( S ) ) {

					const Player player = Player::type::human;

					m_zobrist_hash ^= m_zobrist_keys.at ( player.as_01index ( ), c, r );
					m_human_stone_id.emplace_back ( id );
				}

				++id;
			}

			y += 0.75f * resource_data.m_xara_hex_dim.y;
		}

		m_point_to_id.rebalance ( );
	}

	void initialize ( ) {

		if ( not ( is_once_initialized ) ) {

			is_once_initialized = true;

			return once_initialize ( );
		}

		index_t r, c;

		for ( r = 0; r < OB_ROWS ( S ); ++r ) {

			for ( c = 0; c < OB_COLS ( S ); ++c ) {

				m_human_board.at ( c, r ) = m_agent_board.at ( c, r ) = Player::type::invalid;
			}
		}

		// Top of the board...

		m_agent_stone_id.clear ( );

		index_t li = 1, ri = OB_COLS ( S ) - 1, id = 0;

		for ( r = 1; r < OB_ROWS ( S ) / 2; ++r, ++li, --ri ) {

			for ( c = li; c < ri; c += 2 ) {

				m_human_board.at_r ( c, r ) = m_agent_board.at ( c, r ) = r == 1 ? Player::type::agent : Player::type::vacant;

				if ( r == 1 ) {

					m_agent_stone_id.emplace_back ( id );
				}

				++id;
			}
		}

		// Bottom of the board, r, li and ri "fall through" from top of board...

		m_human_stone_id.clear ( );

		for ( ; r < OB_ROWS ( S ) - 1; ++r, --li, ++ri ) {

			for ( c = li; c < ri; c += 2 ) {

				m_human_board.at_r ( c, r ) = m_agent_board.at ( c, r ) = r == OB_HOME_ROW ( S ) ? Player::type::human : Player::type::vacant;

				if ( r == OB_HOME_ROW ( S ) ) {

					m_human_stone_id.emplace_back ( id );
				}

				++id;
			}
		}

		m_no_home_agent = 0;
		m_no_home_human = 0;

		m_player_to_move = Player::random ( );
		m_winner = Player::type::invalid;

		m_last_move = Move::root;
	}

	bool isValidID ( const int8_t id_ ) const noexcept {

		return id_ >= 0 and id_ < NO_HEXAGONS ( S );
	}

	PointArray toArray ( const sf::Vector2f & v_ ) const noexcept {

		return * reinterpret_cast < const PointArray * > ( & v_ );
	}

	sf::Vector2f fromArray ( const PointArray & p_ ) const noexcept {

		return * reinterpret_cast < const sf::Vector2f * > ( & p_ );
	}

	index_t pointToHexID ( const Point & target_ ) const noexcept {

		return spatial::neighbor_begin ( m_point_to_id, toArray ( target_ ) )->second;
	}

	Location pointToHexLocation ( const Point & p_ ) const noexcept {

		return m_id_to_location.at ( pointToHexID ( p_ ) );
	}

	index_t pointToHumanID ( const Point & p_ ) const noexcept {

		const index_t id = pointToHexID ( p_ );

		if ( std::find ( std::begin ( m_human_stone_id ), std::end ( m_human_stone_id ), id ) == std::end ( m_human_stone_id ) ) {

			return -1;
		}

		return id;
	}

	Hexagon & getHexRefFromID ( const index_t i_ ) noexcept {

		return m_hexagons.at ( i_ );
	}

	index_t getIdFromLocation ( const Location & l_ ) const noexcept {

		// Agents' view...

		return m_location_to_id.at ( l_.c, l_.r );
	}

	StoneID & getAgentStoneIDs ( ) noexcept {

		return m_agent_stone_id;
	}

	StoneID & getHumanStoneIDs ( ) noexcept {

		return m_human_stone_id;
	}

	bool haveStones ( const Player player_ ) const noexcept {

		return player_ == Player::type::agent ? m_agent_stone_id.size ( ) : m_human_stone_id.size ( );
	}

	bool notHaveStones ( const Player player_ ) const noexcept {

		return player_ == Player::type::agent ? m_agent_stone_id.empty ( ) : m_human_stone_id.empty ( );
	}

	bool haveRemainingHome ( const Player player_ ) const noexcept {

		return player_ == Player::type::agent ? ( m_no_home_agent and ( m_no_home_agent == m_agent_stone_id.size ( ) ) ) : ( m_no_home_human and ( m_no_home_human == m_human_stone_id.size ( ) ) );
	}

	ZobristHash zobrist ( ) const noexcept {

		return m_zobrist_hash ^ m_zobrist_player_keys [ m_player_to_move.as_index ( ) ]; // m_player_to_move is opposite player, doesn't matter for the ZH...
	}

	Player playerToMove ( ) const noexcept {

		return m_player_to_move;
	}

	Player playerJustMoved ( ) const noexcept {

		return m_player_to_move.opponent ( );
	}

private:

	typedef ma::MatrixRM<bool, NO_HEXAGONS ( S ), NO_HEXAGONS ( S )> MoveValidator;

	MoveValidator buildMoveValidator ( ) const noexcept {

		MoveValidator validator;

		for ( index_t r = 0; r < NO_HEXAGONS ( S ); ++r ) {

			for ( index_t c = 0; c < NO_HEXAGONS ( S ); ++c ) {

				if ( r == c ) {

					validator.at ( c, r ) = false; continue;
				}

				const Location location_difference ( m_id_to_location.at ( r ) - m_id_to_location.at ( c ) );

				if ( std::abs ( location_difference.r ) > 2 or std::abs ( location_difference.c ) > 2 or std::abs ( location_difference.r ) not_eq std::abs ( location_difference.c ) ) {

					validator.at ( c, r ) = false; continue;
				}

				validator.at ( c, r ) = true;
			}
		}

		return validator;
	}

	// Other (mirrored) location...

public:

	Location other ( const Location & l_ ) const noexcept {

		return Location ( ( OB_COLS ( S ) - 1 ), ( OB_ROWS ( S ) - 1 ) ) - l_;
	}

	Move const randomMove ( ) const noexcept {

		Move move;

		if ( m_player_to_move == Player::type::agent ) {

			StoneID ids ( m_agent_stone_id );

			while ( ids.size ( ) ) {

				const index_t i = std::uniform_int_distribution<index_t> ( 0, ( index_t ) ids.size ( ) - 1 ) ( g_rng );

				const Location loc = m_id_to_location.at ( ids [ i ] );

				if ( bernoulli ( ) ) {

					move = leftMove ( m_agent_board, loc.c, loc.r );

					if ( move not_eq Move::invalid ) {

						return move;
					}

					move = rightMove ( m_agent_board, loc.c, loc.r );

					if ( move not_eq Move::invalid ) {

						return move;
					}
				}

				else {

					move = rightMove ( m_agent_board, loc.c, loc.r );

					if ( move not_eq Move::invalid ) {

						return move;
					}

					move = leftMove ( m_agent_board, loc.c, loc.r );

					if ( move not_eq Move::invalid ) {

						return move;
					}
				}

				ids.erase ( std::begin ( ids ) + i );
			}
		}

		else {

			StoneID ids ( m_human_stone_id );

			while ( ids.size ( ) ) {

				const index_t i = std::uniform_int_distribution<index_t> ( 0, ( index_t ) ids.size ( ) - 1 ) ( g_rng );

				const Location loc = m_id_to_location.at_r ( ids [ i ] );

				if ( bernoulli ( ) ) {

					move = leftMove ( m_human_board, loc.c, loc.r );

					if ( move not_eq Move::invalid ) {

						return move;
					}

					move = rightMove ( m_human_board, loc.c, loc.r );

					if ( move not_eq Move::invalid ) {

						return move;
					}
				}

				else {

					move = rightMove ( m_human_board, loc.c, loc.r );

					if ( move not_eq Move::invalid ) {

						return move;
					}

					move = leftMove ( m_human_board, loc.c, loc.r );

					if ( move not_eq Move::invalid ) {

						return move;
					}
				}

				ids.erase ( std::begin ( ids ) + i );
			}
		}

		return move;
	}

	Move const agentMove ( ) const noexcept {

		return randomMove ( );
	}

	Move const humanMove ( const index_t f_, const index_t t_ ) const noexcept {

		// Given indices, Human's move, output move from human's perspective...

		static const MoveValidator validator ( buildMoveValidator ( ) );

		if ( f_ == t_ ) {

			return Move::none;
		}

		if ( f_ < 0 or f_ > ( NO_HEXAGONS ( S ) - 1 ) or t_ < 0 or t_ > ( NO_HEXAGONS ( S ) - 1 ) or not ( validator.at ( f_, t_ ) ) ) {

			return Move::invalid;
		}

		// Human's perspective from here on...

		const Move move ( other ( m_id_to_location.at ( f_ ) ), other ( m_id_to_location.at ( t_ ) ) );

		if ( m_human_board.at ( move.m_from.c, move.m_from.r ) not_eq Player::type::human
			 or m_human_board.at ( move.m_to.c, move.m_to.r ) not_eq Player::type::vacant
			 or ( move.m_to.r - move.m_from.r == 2
				  and m_human_board.at ( ( move.m_from.c + move.m_to.c ) / 2, ( move.m_from.r + move.m_to.r ) / 2 ) not_eq Player::type::agent ) ) {

			return Move::invalid;
		}

		return move;
	}

private:

	// Do Player's Move...

	void moveStoneHash ( const Player player_, const Move & move_ ) noexcept {

		if ( player_ == Player::type::agent ) {

			m_agent_board.at ( move_.m_from.c, move_.m_from.r ) = m_human_board.at_r ( move_.m_from.c, move_.m_from.r ) = Player::type::vacant;
			m_agent_board.at ( move_.m_to.c, move_.m_to.r ) = m_human_board.at_r ( move_.m_to.c, move_.m_to.r ) = Player::type::agent;
			( *std::find ( std::begin ( m_agent_stone_id ), std::end ( m_agent_stone_id ), m_location_to_id.at ( move_.m_from.c, move_.m_from.r ) ) ) = m_location_to_id.at ( move_.m_to.c, move_.m_to.r );
			m_zobrist_hash ^= m_zobrist_keys.at ( 0, move_.m_from.c, move_.m_from.r );
			m_zobrist_hash ^= m_zobrist_keys.at ( 0, move_.m_to.c, move_.m_to.r );
			m_no_home_agent += move_.m_to.r == OB_HOME_ROW ( S );
		}

		else {

			m_human_board.at ( move_.m_from.c, move_.m_from.r ) = m_agent_board.at_r ( move_.m_from.c, move_.m_from.r ) = Player::type::vacant;
			m_human_board.at ( move_.m_to.c, move_.m_to.r ) = m_agent_board.at_r ( move_.m_to.c, move_.m_to.r ) = Player::type::human;
			( *std::find ( std::begin ( m_human_stone_id ), std::end ( m_human_stone_id ), m_location_to_id.at_r ( move_.m_from.c, move_.m_from.r ) ) ) = m_location_to_id.at_r ( move_.m_to.c, move_.m_to.r );
			m_zobrist_hash ^= m_zobrist_keys.at ( 1, move_.m_from.c, move_.m_from.r );
			m_zobrist_hash ^= m_zobrist_keys.at ( 1, move_.m_to.c, move_.m_to.r );
			m_no_home_human += move_.m_to.r == OB_HOME_ROW ( S );
		}
	}

	void captureStoneHash ( const Player player_, const Move & move_ ) noexcept {

		const Location captured = move_.captured ( );

		if ( player_ == Player::type::agent ) {

			m_agent_board.at ( captured.c, captured.r ) = m_human_board.at_r ( captured.c, captured.r ) = Player::type::vacant;
			m_human_stone_id.erase ( std::remove ( std::begin ( m_human_stone_id ), std::end ( m_human_stone_id ), m_location_to_id.at ( captured.c, captured.r ) ), std::end ( m_human_stone_id ) );
			m_zobrist_hash ^= m_zobrist_keys.at ( 1, ( OB_COLS ( S ) - 1 ) - captured.c, ( OB_ROWS ( S ) - 1 ) - captured.r );
		}

		else {

			m_human_board.at ( captured.c, captured.r ) = m_agent_board.at_r ( captured.c, captured.r ) = Player::type::vacant;
			m_agent_stone_id.erase ( std::remove ( std::begin ( m_agent_stone_id ), std::end ( m_agent_stone_id ), m_location_to_id.at_r ( captured.c, captured.r ) ), std::end ( m_agent_stone_id ) );
			m_zobrist_hash ^= m_zobrist_keys.at ( 0, ( OB_COLS ( S ) - 1 ) - captured.c, ( OB_ROWS ( S ) - 1 ) - captured.r );
		}
	}


	void moveStone ( const Player player_, const Move & move_ ) noexcept {

		if ( player_ == Player::type::agent ) {

			m_agent_board.at ( move_.m_from.c, move_.m_from.r ) = m_human_board.at_r ( move_.m_from.c, move_.m_from.r ) = Player::type::vacant;
			m_agent_board.at ( move_.m_to.c, move_.m_to.r ) = m_human_board.at_r ( move_.m_to.c, move_.m_to.r ) = Player::type::agent;
			( *std::find ( std::begin ( m_agent_stone_id ), std::end ( m_agent_stone_id ), m_location_to_id.at ( move_.m_from.c, move_.m_from.r ) ) ) = m_location_to_id.at ( move_.m_to.c, move_.m_to.r );
			m_no_home_agent += move_.m_to.r == OB_HOME_ROW ( S );
		}

		else {

			m_human_board.at ( move_.m_from.c, move_.m_from.r ) = m_agent_board.at_r ( move_.m_from.c, move_.m_from.r ) = Player::type::vacant;
			m_human_board.at ( move_.m_to.c, move_.m_to.r ) = m_agent_board.at_r ( move_.m_to.c, move_.m_to.r ) = Player::type::human;
			( *std::find ( std::begin ( m_human_stone_id ), std::end ( m_human_stone_id ), m_location_to_id.at_r ( move_.m_from.c, move_.m_from.r ) ) ) = m_location_to_id.at_r ( move_.m_to.c, move_.m_to.r );
			m_no_home_human += move_.m_to.r == OB_HOME_ROW ( S );
		}
	}

	void captureStone ( const Player player_, const Move & move_ ) noexcept {

		const Location captured = move_.captured ( );

		if ( player_ == Player::type::agent ) {

			m_agent_board.at ( captured.c, captured.r ) = m_human_board.at_r ( captured.c, captured.r ) = Player::type::vacant;
			m_human_stone_id.erase ( std::remove ( std::begin ( m_human_stone_id ), std::end ( m_human_stone_id ), m_location_to_id.at ( captured.c, captured.r ) ), std::end ( m_human_stone_id ) );
		}

		else {

			m_human_board.at ( captured.c, captured.r ) = m_agent_board.at_r ( captured.c, captured.r ) = Player::type::vacant;
			m_agent_stone_id.erase ( std::remove ( std::begin ( m_agent_stone_id ), std::end ( m_agent_stone_id ), m_location_to_id.at_r ( captured.c, captured.r ) ), std::end ( m_agent_stone_id ) );
		}
	}


public:

	void move_hash ( const Move & move_ ) noexcept {

		m_last_move = move_;

		moveStoneHash ( m_player_to_move, move_ );

		if ( move_.isCapture ( ) ) {

			captureStoneHash ( m_player_to_move, move_ );
		}

		m_player_to_move.next ( );
	}

	void move_hash_winner ( const Move & move_ ) noexcept {

		m_last_move = move_;

		moveStoneHash ( m_player_to_move, move_ );

		if ( move_.isCapture ( ) ) {

			captureStoneHash ( m_player_to_move, move_ );
		}

		winner ( );

		m_player_to_move.next ( );
	}

	void move_winner ( const Move & move_ ) noexcept {

		m_last_move = move_;

		moveStone ( m_player_to_move, move_ );

		if ( move_.isCapture ( ) ) {

			captureStone ( m_player_to_move, move_ );
		}

		winner ( );

		m_player_to_move.next ( );
	}


	// Search forward (from Player's perspective), left and right,
	// returning a potential move, either move or capture...

	Move leftMove ( const Board & b_, index_t c_, index_t r_ ) const noexcept { // Location is in players's perspective...

		Move move ( Location ( c_, r_ ) );

		++c_; ++r_;

		const Player player = b_.at ( c_, r_ );

		if ( player == Player::type::vacant ) { // ExtendedMove...

			move.m_to = std::move ( Location ( c_, r_ ) );

			return move;
		}

		else if ( player == b_.at ( move.m_from.c, move.m_from.r ).opponent ( ) ) { // Capture...

			++c_; ++r_;

			if ( b_.at ( c_, r_ ) == Player::type::vacant ) {

				move.m_to = std::move ( Location ( c_, r_ ) );

				return move;
			}
		}

		return Move::invalid;
	}

	Move rightMove ( const Board & b_, index_t c_, index_t r_ ) const noexcept { // Location is in Player's perspective...

		Move move ( Location ( c_, r_ ) );

		--c_; ++r_;

		const Player player = b_.at ( c_, r_ );

		if ( player == Player::type::vacant ) { // ExtendedMove...

			move.m_to = std::move ( Location ( c_, r_ ) );

			return move;
		}

		else if ( player == b_.at ( move.m_from.c, move.m_from.r ).opponent ( ) ) { // Capture...

			--c_; ++r_;

			if ( b_.at ( c_, r_ ) == Player::type::vacant ) {

				move.m_to = std::move ( Location ( c_, r_ ) );

				return move;
			}
		}

		return Move::invalid;
	}

	bool moves ( Moves * moves_ ) const noexcept {

		// Mcts class takes has ownership...

		if ( m_winner.occupied ( ) ) {

			return false;
		}

		moves_->clear ( );

		Move move;

		if ( m_player_to_move == Player::type::agent ) {

			for ( const index_t s : m_agent_stone_id ) {

				const Location l = m_id_to_location.at ( s );

				move = leftMove ( m_agent_board, l.c, l.r );

				if ( move not_eq Move::invalid ) {

					moves_->push_back ( move );
				}

				move = rightMove ( m_agent_board, l.c, l.r );

				if ( move not_eq Move::invalid ) {

					moves_->push_back ( move );
				}
			}
		}

		else {

			for ( const index_t s : m_human_stone_id ) {

				const Location l = m_id_to_location.at_r ( s );

				move = leftMove ( m_human_board, l.c, l.r );

				if ( move not_eq Move::invalid ) {

					moves_->push_back ( move );
				}

				move = rightMove ( m_human_board, l.c, l.r );

				if ( move not_eq Move::invalid ) {

					moves_->push_back ( move );
				}
			}
		}

		return moves_->size ( );
	}


	void simulate ( ) noexcept {

		Moves m;

		while ( moves ( & m ) ) {

			move_winner ( m.random ( ) );
		}
	}


	bool hasMoves ( const Player player_ ) const noexcept {

		if ( player_ == Player::type::agent ) {

			for ( const index_t s : m_agent_stone_id ) {

				const Location l = m_id_to_location.at ( s );

				if ( leftMove ( m_agent_board, l.c, l.r ) not_eq Move::invalid ) {

					return true;
				}

				if ( rightMove ( m_agent_board, l.c, l.r ) not_eq Move::invalid ) {

					return true;
				}
			}
		}

		else {

			for ( const index_t s : m_human_stone_id ) {

				const Location l = m_id_to_location.at_r ( s );

				if ( leftMove ( m_human_board, l.c, l.r ) not_eq Move::invalid ) {

					return true;
				}

				if ( rightMove ( m_human_board, l.c, l.r ) not_eq Move::invalid ) {

					return true;
				}
			}
		}

		return false;
	}

	bool hasNoMoves ( const Player player_ ) const noexcept {

		return not ( hasMoves ( player_ ) );
	}

	Player playerMostHomeStones ( ) const noexcept {

		if ( m_no_home_agent > m_no_home_human ) return Player::type::agent;
		if ( m_no_home_agent < m_no_home_human ) return Player::type::human;

		return Player::type::vacant;
	}


	std::optional<Player> ended ( ) const noexcept {

		// Game ends, when:
		//
		// 1. m_player_that_moved moved all his stones in the end row;
		// 2. player_to_move cannot move (this includes: player_to_move has no stones left);

		return m_winner == Player::type::invalid ? std::optional<Player> ( ) : std::optional<Player> ( m_winner );
	}


	void winner ( ) noexcept { // Before the player swap, but after m_player_to_move made his move...

		if ( haveRemainingHome ( m_player_to_move ) ) {

			m_winner = haveRemainingHome ( m_player_to_move.opponent ( ) ) ? playerMostHomeStones ( ) : m_player_to_move;
		}

		else if ( notHaveStones ( m_player_to_move.opponent ( ) ) ) {

			m_winner = m_player_to_move;
		}

		else if ( hasNoMoves ( m_player_to_move.opponent ( ) ) ) {

			m_winner = m_player_to_move.opponent ( );
		}
	}



	float result ( const Player player_just_moved_ ) const noexcept {

		// Determine result: last player of path is the player to move...

		// return m_winner.vacant ( ) ? 0.5f : ( m_winner == player_just_moved_ ? 1.0f : 0.0f ); // Score...

		return m_winner.vacant ( ) ? 0.0f : ( m_winner == player_just_moved_ ? 1.0f : -1.0f );
	}


	bool terminal ( ) const noexcept {

		return m_winner.occupied ( ); // or m_no_moves == NumRows * NumCols;
	}

	bool nonterminal ( ) const noexcept {

		return m_winner.vacant ( ); // and m_no_moves < NumRows * NumCols;
	}


	Move lastMove ( ) const noexcept {

		return m_last_move;
	}

	void printRaw ( const Board & b_ ) const noexcept {

		// Print board given perspective...

		putchar ( '\n' );

		for ( index_t r = 0; r < OB_ROWS ( S ); ++r ) {

			putchar ( ' ' );

			for ( index_t c = 0; c < OB_COLS ( S ); ++c ) {

				switch ( b_.at ( c, r ).as_index ( ) ) {

					case ( index_t ) Player::type::invalid: putchar ( ' ' ); break;
					case ( index_t ) Player::type::agent  : putchar ( 'A' ); break;
					case ( index_t ) Player::type::vacant : putchar ( '*' ); break;
					case ( index_t ) Player::type::human  : putchar ( 'H' ); break;

					NO_DEFAULT_CASE;
				}

				if ( c < OB_COLS ( S ) ) {

					putchar ( ' ' );
				}
			}

			putchar ( '\n' );
		}

		putchar ( '\n' );

		std::cout << " Agent: has " << m_agent_stone_id.size ( ) << " stones (" << m_no_home_agent << " stone(s) home)\n";
		std::cout << " Human: has " << m_human_stone_id.size ( ) << " stones (" << m_no_home_human << " stone(s) home)\n\n";
	}

	void print ( ) const noexcept {

		printRaw ( m_agent_board );
	}

private:

	friend class cereal::access;

	template < class Archive >
	void serialize ( Archive & ar_ ) { ar_ ( * this ); }
};

template <index_t S>
typename OskaStateTemplate < S >::Hexagons OskaStateTemplate < S >::m_hexagons;
template <index_t S>
typename OskaStateTemplate < S >::PointToID OskaStateTemplate < S >::m_point_to_id;
template <index_t S>
typename OskaStateTemplate < S >::LocationToID OskaStateTemplate < S >::m_location_to_id;
template <index_t S>
typename OskaStateTemplate < S >::IDToLocation OskaStateTemplate < S >::m_id_to_location;
template <index_t S>
typename OskaStateTemplate < S >::ZobristHashKeys OskaStateTemplate < S >::m_zobrist_keys;
template <index_t S>
const typename OskaStateTemplate < S >::ZobristHash OskaStateTemplate < S >::m_zobrist_player_key_values [ 3 ] {
	0x41fec34015a1bef2ull, 0x8b80677c9c144514ull, 0xf6242292160d5bb7ull
};
template <index_t S>
const typename OskaStateTemplate < S >::ZobristHash * OskaStateTemplate < S >::m_zobrist_player_keys {
	m_zobrist_player_key_values + 1
};


#if 0

#define BIND( S ) case S: m_state_##S = new OskaStateTemplate < S > ( ); bind < S > ( m_state_##S ); break;
#define BIND_COPY_FROM( S, OS ) case S: m_state_##S = new OskaStateTemplate < S > ( * OS.m_state_##S ); bind < S > ( m_state_##S ); break;
#define BIND_MOVE_FROM( S, OS ) case S: std::swap ( m_no_stones, OS.m_no_stones ); std::swap ( m_state_##S, OS.m_state_##S ); bind < S > ( m_state_##S ); break;
#define DESTROY( S ) case S: delete m_state_##S; break;

namespace os {

	class OskaState {

		// Dispatch class...

	public:

		typedef Move Move;
		typedef Moves < Move, 16 > Moves;
		typedef Player Player;

		static constexpr index_t max_no_moves = 16;

		index_t m_no_stones = 0;

		OskaState ( ) noexcept { }

	private:

		OskaStateTemplate < 4 > * m_state_4 = nullptr;
		OskaStateTemplate < 5 > * m_state_5 = nullptr;
		OskaStateTemplate < 6 > * m_state_6 = nullptr;
		OskaStateTemplate < 7 > * m_state_7 = nullptr;
		OskaStateTemplate < 8 > * m_state_8 = nullptr;

		function < void ( ) > m_initialize;
		function < Location ( const Location & ) > m_other;
		function < bool ( const int8_t ) > m_is_valid_id;
		function < index_t ( const Point & ) > m_point_to_human_id;
		function < index_t ( const Point & ) > m_point_to_hex_id;
		function < Hexagon & ( const index_t ) > m_get_hex_ref_from_id;
		function < index_t ( const Location & ) > m_get_id_from_location;
		function < StoneID & ( ) > m_get_agent_stone_id;
		function < StoneID & ( ) > m_get_human_stone_id;
		function < Move const ( const index_t, const index_t ) > m_human_move;
		function < Move ( ) > m_last_move;
		function < ZobristHash ( ) > m_zobrist;
		function < Player ( ) > m_player_to_move;
		function < bool ( Moves * ) > m_moves;
		function < void ( const Move & ) > m_move_hash;
		function < void ( const Move & ) > m_move_hash_winner;
		function < void ( ) > m_simulate;
		function < float ( const Player ) > m_result;
		function < boost::optional<Player> ( ) > m_ended;
		function < void ( ) > m_print;

		template<index_t S>
		void bind ( OskaStateTemplate<S> * m_state_ ) {

			m_initialize = std::bind ( &OskaStateTemplate<S>::initialize, m_state_ );
			m_other = std::bind ( &OskaStateTemplate<S>::other, m_state_, _1 );
			m_is_valid_id = std::bind ( &OskaStateTemplate<S>::isValidID, m_state_, _1 );
			m_point_to_human_id = std::bind ( &OskaStateTemplate<S>::pointToHumanID, m_state_, _1 );
			m_point_to_hex_id = std::bind ( &OskaStateTemplate<S>::pointToHexID, m_state_, _1 );
			m_get_hex_ref_from_id = std::bind ( &OskaStateTemplate<S>::getHexRefFromID, m_state_, _1 );
			m_get_id_from_location = std::bind ( &OskaStateTemplate<S>::getIdFromLocation, m_state_, _1 );
			m_get_agent_stone_id = std::bind ( &OskaStateTemplate<S>::getAgentStoneIDs, m_state_ );
			m_get_human_stone_id = std::bind ( &OskaStateTemplate<S>::getHumanStoneIDs, m_state_ );
			m_human_move = std::bind ( &OskaStateTemplate<S>::humanMove, m_state_, _1, _2 );
			m_last_move = std::bind ( &OskaStateTemplate<S>::lastMove, m_state_ );
			m_zobrist = std::bind ( &OskaStateTemplate<S>::zobrist, m_state_ );
			m_player_to_move = std::bind ( &OskaStateTemplate<S>::playerToMove, m_state_ );
			m_moves = std::bind ( &OskaStateTemplate<S>::moves, m_state_, _1 );
			m_move_hash = std::bind ( &OskaStateTemplate<S>::move_hash, m_state_, _1 );
			m_move_hash_winner = std::bind ( &OskaStateTemplate<S>::move_hash_winner, m_state_, _1 );
			m_simulate = std::bind ( &OskaStateTemplate<S>::simulate, m_state_ );
			m_result = std::bind ( &OskaStateTemplate<S>::result, m_state_, _1 );
			m_ended = std::bind ( &OskaStateTemplate<S>::ended, m_state_ );
			m_print = std::bind ( &OskaStateTemplate<S>::print, m_state_ );
		}

		void destroy ( ) noexcept {

			switch ( m_no_stones ) {

				DESTROY ( 4 );
				DESTROY ( 5 );
				DESTROY ( 6 );
				DESTROY ( 7 );
				DESTROY ( 8 );

				NO_DEFAULT_CASE;
			}
		}

	public:

		OskaState ( const OskaState & s_ ) noexcept {

			m_no_stones = s_.m_no_stones;

			switch ( m_no_stones ) {

				BIND_COPY_FROM ( 4, s_ );
				BIND_COPY_FROM ( 5, s_ );
				BIND_COPY_FROM ( 6, s_ );
				BIND_COPY_FROM ( 7, s_ );
				BIND_COPY_FROM ( 8, s_ );

				NO_DEFAULT_CASE;
			}
		}

		OskaState ( OskaState && s_ ) noexcept {

			assert ( m_no_stones == s_.m_no_stones );

			switch ( m_no_stones ) {

				BIND_MOVE_FROM ( 4, s_ );
				BIND_MOVE_FROM ( 5, s_ );
				BIND_MOVE_FROM ( 6, s_ );
				BIND_MOVE_FROM ( 7, s_ );
				BIND_MOVE_FROM ( 8, s_ );

				NO_DEFAULT_CASE;
			}
		}

		~OskaState ( ) noexcept {

			destroy ( );
		}

		void initialize ( const index_t no_stones_ ) {

			destroy ( );

			m_no_stones = no_stones_;

			switch ( m_no_stones ) {

				BIND ( 4 );
				BIND ( 5 );
				BIND ( 6 );
				BIND ( 7 );
				BIND ( 8 );

				NO_DEFAULT_CASE;
			}

			m_initialize ( );
		}

		void initialize ( ) const noexcept { m_initialize ( ); }
		Location other ( const Location & l_ ) const noexcept { return m_other ( l_ ); }
		bool isValidID ( const int8_t id_ ) const noexcept { return m_is_valid_id ( id_ ); }
		index_t pointToHumanID ( const Point & p_ ) const noexcept { return m_point_to_human_id ( p_ ); }
		index_t pointToHexID ( const Point & p_ ) const noexcept { return m_point_to_hex_id ( p_ ); }
		Hexagon & getHexRefFromID ( const index_t i_ ) noexcept { return m_get_hex_ref_from_id ( i_ ); }
		index_t getIdFromLocation ( const Location & l_ ) const noexcept { return m_get_id_from_location ( l_ ); }
		StoneID & getAgentStoneIDs ( ) noexcept { return m_get_agent_stone_id ( ); }
		StoneID & getHumanStoneIDs ( ) noexcept { return m_get_human_stone_id ( ); }
		Move const humanMove ( const index_t f_, const index_t t_ ) const noexcept { return m_human_move ( f_, t_ ); }
		Move lastMove ( ) const noexcept { return m_last_move ( ); }
		ZobristHash zobrist ( ) const noexcept { return m_zobrist ( ); }
		Player playerToMove ( ) const noexcept { return m_player_to_move ( ); }
		Player playerJustMoved ( ) const noexcept { return m_player_to_move ( ).opponent ( ); }
		bool moves ( Moves * moves_ ) const noexcept { return m_moves ( moves_ ); }
		void move_hash ( const Move & move_ ) noexcept { return m_move_hash ( move_ ); }
		void move_hash_winner ( const Move & move_ ) noexcept { return m_move_hash_winner ( move_ ); }
		void simulate ( ) noexcept { m_simulate ( ); }
		float result ( const Player player_ ) const noexcept { return m_result ( player_ ); }
		boost::optional<Player> ended ( ) const noexcept { return m_ended ( ); }
		void print ( ) const noexcept { return m_print ( ); }
	};
}

#endif

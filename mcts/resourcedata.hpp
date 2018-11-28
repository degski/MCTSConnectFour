
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

#include <cstdint>

#include <SFML/Graphics.hpp>

#include "Typedefs.hpp"

#include "resource.h"


template < std::int32_t S >
struct ResourceDataTemplate {

};

template<>
struct ResourceDataTemplate<4> {

	const sf::Vector2f m_xara_hex_dim = sf::Vector2f ( 225.0f, 262.5f );

	const sf::Vector2f m_xara_dim_small  = sf::Vector2f ( 499.0f, 576.0f );
	const sf::Vector2f m_xara_dim_medium = sf::Vector2f ( 531.0f, 614.0f );
	const sf::Vector2f m_xara_dim_large  = sf::Vector2f ( 748.0f, 864.0f );

	const float m_xara_height = 1110.0f;

	const float m_margin = 30.0f;

	const std::int32_t m_bg_png_small  = BOARD_BACKGROUND_W4_SMALL;
	const std::int32_t m_bg_png_medium = BOARD_BACKGROUND_W4_MEDIUM;
	const std::int32_t m_bg_png_large  = BOARD_BACKGROUND_W4_LARGE;

	const std::int32_t m_as_png_small = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_small = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_medium = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_medium = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_large = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_large = HUMAN_STONE_W8_LARGE;
};

template<>
struct ResourceDataTemplate<5> {

	const sf::Vector2f m_xara_hex_dim = sf::Vector2f ( 180.0f, 210.0f );

	const sf::Vector2f m_xara_dim_small  = sf::Vector2f ( 456.0f, 576.0f );
	const sf::Vector2f m_xara_dim_medium = sf::Vector2f ( 486.0f, 614.0f );
	const sf::Vector2f m_xara_dim_large  = sf::Vector2f ( 683.0f, 864.0f );

	const float m_xara_height = 1215.0f;

	const float m_margin = 30.0f;

	const std::int32_t m_bg_png_small  = BOARD_BACKGROUND_W5_SMALL;
	const std::int32_t m_bg_png_medium = BOARD_BACKGROUND_W5_MEDIUM;
	const std::int32_t m_bg_png_large  = BOARD_BACKGROUND_W5_LARGE;

	const std::int32_t m_as_png_small = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_small = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_medium = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_medium = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_large = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_large = HUMAN_STONE_W8_LARGE;
};

template<>
struct ResourceDataTemplate<6> {

	const sf::Vector2f m_xara_hex_dim = sf::Vector2f ( 150.0f, 175.0f );

	const sf::Vector2f m_xara_dim_small  = sf::Vector2f ( 431.0f, 576.0f );
	const sf::Vector2f m_xara_dim_medium = sf::Vector2f ( 459.0f, 614.0f );
	const sf::Vector2f m_xara_dim_large  = sf::Vector2f ( 646.0f, 864.0f );

	const float m_xara_height = 1285.0f;

	const float m_margin = 30.0f;

	const std::int32_t m_bg_png_small  = BOARD_BACKGROUND_W6_SMALL;
	const std::int32_t m_bg_png_medium = BOARD_BACKGROUND_W6_MEDIUM;
	const std::int32_t m_bg_png_large  = BOARD_BACKGROUND_W6_LARGE;

	const std::int32_t m_as_png_small = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_small = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_medium = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_medium = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_large = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_large = HUMAN_STONE_W8_LARGE;
};

template<>
struct ResourceDataTemplate<7> {

	const sf::Vector2f m_xara_hex_dim = sf::Vector2f ( 128.6f, 150.0f );

	const sf::Vector2f m_xara_dim_small  = sf::Vector2f ( 415.0f, 576.0f );
	const sf::Vector2f m_xara_dim_medium = sf::Vector2f ( 442.0f, 614.0f );
	const sf::Vector2f m_xara_dim_large  = sf::Vector2f ( 622.0f, 864.0f );

	const float m_xara_height = 1335.0f;

	const float m_margin = 30.0f;

	const std::int32_t m_bg_png_small  = BOARD_BACKGROUND_W7_SMALL;
	const std::int32_t m_bg_png_medium = BOARD_BACKGROUND_W7_MEDIUM;
	const std::int32_t m_bg_png_large  = BOARD_BACKGROUND_W7_LARGE;

	const std::int32_t m_as_png_small = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_small = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_medium = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_medium = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_large = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_large = HUMAN_STONE_W8_LARGE;
};

template<>
struct ResourceDataTemplate<8> { // Not done yet

	const sf::Vector2f m_xara_hex_dim = sf::Vector2f ( 112.5f, 131.3f );

	const sf::Vector2f m_xara_dim_small  = sf::Vector2f ( 403.0f, 576.0f );
	const sf::Vector2f m_xara_dim_medium = sf::Vector2f ( 430.0f, 614.0f );
	const sf::Vector2f m_xara_dim_large  = sf::Vector2f ( 605.0f, 864.0f );

	const float m_xara_height = 1373.0f;

	const float m_margin = 30.0f;

	const std::int32_t m_bg_png_small  = BOARD_BACKGROUND_W8_SMALL;
	const std::int32_t m_bg_png_medium = BOARD_BACKGROUND_W8_MEDIUM;
	const std::int32_t m_bg_png_large  = BOARD_BACKGROUND_W8_LARGE;

	const std::int32_t m_as_png_small = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_small = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_medium = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_medium = HUMAN_STONE_W8_LARGE;
	const std::int32_t m_as_png_large = AGENT_STONE_W8_LARGE;
	const std::int32_t m_hs_png_large = HUMAN_STONE_W8_LARGE;
};


struct ResourceData {

	enum class Type : std::int32_t { XARA_DIM_4, XARA_DIM_5, XARA_DIM_6, XARA_DIM_7, XARA_DIM_8 };

	Type m_type;

private:

	ResourceDataTemplate<4> m_dim_4;
	ResourceDataTemplate<5> m_dim_5;
	ResourceDataTemplate<6> m_dim_6;
	ResourceDataTemplate<7> m_dim_7;
	ResourceDataTemplate<8> m_dim_8;

public:

	sf::Vector2f m_xara_hex_dim, m_xara_dim;

	float m_margin;

	std::int32_t m_bg_png, m_as_png, m_hs_png;

	ResourceData ( const std::int32_t no_stones_  ) :

		m_type ( static_cast< Type > ( no_stones_ - 4 ) ) {

		const Screen screen = getScreenType ( );

		switch ( m_type ) {

		case Type::XARA_DIM_4:

		{
			const float scaling = screen == Screen::SMALL ? m_dim_4.m_xara_dim_small.y / m_dim_4.m_xara_height : screen == Screen::LARGE ? m_dim_4.m_xara_dim_large.y / m_dim_4.m_xara_height : m_dim_4.m_xara_dim_medium.y / m_dim_4.m_xara_height;

			m_xara_hex_dim = m_dim_4.m_xara_hex_dim * scaling;
			m_xara_dim = screen == Screen::SMALL ? m_dim_4.m_xara_dim_small : screen == Screen::LARGE ? m_dim_4.m_xara_dim_large : m_dim_4.m_xara_dim_medium;
			m_margin = m_dim_4.m_margin * scaling;
			m_bg_png = screen == Screen::SMALL ? m_dim_4.m_bg_png_small : screen == Screen::LARGE ? m_dim_4.m_bg_png_large : m_dim_4.m_bg_png_medium;
			m_as_png = screen == Screen::SMALL ? m_dim_4.m_as_png_small : screen == Screen::LARGE ? m_dim_4.m_as_png_large : m_dim_4.m_as_png_medium;
			m_hs_png = screen == Screen::SMALL ? m_dim_4.m_hs_png_small : screen == Screen::LARGE ? m_dim_4.m_hs_png_large : m_dim_4.m_hs_png_medium;

			break;
		}

		case Type::XARA_DIM_5:

		{
			const float scaling = screen == Screen::SMALL ? m_dim_5.m_xara_dim_small.y / m_dim_5.m_xara_height : screen == Screen::LARGE ? m_dim_5.m_xara_dim_large.y / m_dim_5.m_xara_height : m_dim_5.m_xara_dim_medium.y / m_dim_5.m_xara_height;

			m_xara_hex_dim = m_dim_5.m_xara_hex_dim * scaling;
			m_xara_dim = screen == Screen::SMALL ? m_dim_5.m_xara_dim_small : screen == Screen::LARGE ? m_dim_5.m_xara_dim_large : m_dim_5.m_xara_dim_medium;
			m_margin = m_dim_5.m_margin * scaling;
			m_bg_png = screen == Screen::SMALL ? m_dim_5.m_bg_png_small : screen == Screen::LARGE ? m_dim_5.m_bg_png_large : m_dim_5.m_bg_png_medium;
			m_as_png = screen == Screen::SMALL ? m_dim_5.m_as_png_small : screen == Screen::LARGE ? m_dim_5.m_as_png_large : m_dim_5.m_as_png_medium;
			m_hs_png = screen == Screen::SMALL ? m_dim_5.m_hs_png_small : screen == Screen::LARGE ? m_dim_5.m_hs_png_large : m_dim_5.m_hs_png_medium;

			break;
		}

		case Type::XARA_DIM_6:

		{
			const float scaling = screen == Screen::SMALL ? m_dim_6.m_xara_dim_small.y / m_dim_6.m_xara_height : screen == Screen::LARGE ? m_dim_6.m_xara_dim_large.y / m_dim_6.m_xara_height : m_dim_6.m_xara_dim_medium.y / m_dim_6.m_xara_height;

			m_xara_hex_dim = m_dim_6.m_xara_hex_dim * scaling;
			m_xara_dim = screen == Screen::SMALL ? m_dim_6.m_xara_dim_small : screen == Screen::LARGE ? m_dim_6.m_xara_dim_large : m_dim_6.m_xara_dim_medium;
			m_margin = m_dim_6.m_margin * scaling;
			m_bg_png = screen == Screen::SMALL ? m_dim_6.m_bg_png_small : screen == Screen::LARGE ? m_dim_6.m_bg_png_large : m_dim_6.m_bg_png_medium;
			m_as_png = screen == Screen::SMALL ? m_dim_6.m_as_png_small : screen == Screen::LARGE ? m_dim_6.m_as_png_large : m_dim_6.m_as_png_medium;
			m_hs_png = screen == Screen::SMALL ? m_dim_6.m_hs_png_small : screen == Screen::LARGE ? m_dim_6.m_hs_png_large : m_dim_6.m_hs_png_medium;

			break;
		}

		case Type::XARA_DIM_7:

		{
			const float scaling = screen == Screen::SMALL ? m_dim_7.m_xara_dim_small.y / m_dim_7.m_xara_height : screen == Screen::LARGE ? m_dim_7.m_xara_dim_large.y / m_dim_7.m_xara_height : m_dim_7.m_xara_dim_medium.y / m_dim_7.m_xara_height;

			m_xara_hex_dim = m_dim_7.m_xara_hex_dim * scaling;
			m_xara_dim = screen == Screen::SMALL ? m_dim_7.m_xara_dim_small : screen == Screen::LARGE ? m_dim_7.m_xara_dim_large : m_dim_7.m_xara_dim_medium;
			m_margin = m_dim_7.m_margin * scaling;
			m_bg_png = screen == Screen::SMALL ? m_dim_7.m_bg_png_small : screen == Screen::LARGE ? m_dim_7.m_bg_png_large : m_dim_7.m_bg_png_medium;
			m_as_png = screen == Screen::SMALL ? m_dim_7.m_as_png_small : screen == Screen::LARGE ? m_dim_7.m_as_png_large : m_dim_7.m_as_png_medium;
			m_hs_png = screen == Screen::SMALL ? m_dim_7.m_hs_png_small : screen == Screen::LARGE ? m_dim_7.m_hs_png_large : m_dim_7.m_hs_png_medium;

			break;
		}

		case Type::XARA_DIM_8:

		{
			const float scaling = screen == Screen::SMALL ? m_dim_8.m_xara_dim_small.y / m_dim_8.m_xara_height : screen == Screen::LARGE ? m_dim_8.m_xara_dim_large.y / m_dim_8.m_xara_height : m_dim_8.m_xara_dim_medium.y / m_dim_8.m_xara_height;

			m_xara_hex_dim = m_dim_8.m_xara_hex_dim * scaling;
			m_xara_dim = screen == Screen::SMALL ? m_dim_8.m_xara_dim_small : screen == Screen::LARGE ? m_dim_8.m_xara_dim_large : m_dim_8.m_xara_dim_medium;
			m_margin = m_dim_8.m_margin * scaling;
			m_bg_png = screen == Screen::SMALL ? m_dim_8.m_bg_png_small : screen == Screen::LARGE ? m_dim_8.m_bg_png_large : m_dim_8.m_bg_png_medium;
			m_as_png = screen == Screen::SMALL ? m_dim_8.m_as_png_small : screen == Screen::LARGE ? m_dim_8.m_as_png_large : m_dim_8.m_as_png_medium;
			m_hs_png = screen == Screen::SMALL ? m_dim_8.m_hs_png_small : screen == Screen::LARGE ? m_dim_8.m_hs_png_large : m_dim_8.m_hs_png_medium;

			break;
		}

		} // Switch...
	}

	Screen getScreenType ( ) {

		// Small: 1280 x 720 -> height 576
		// Medium: 1024 x 768 or 1366 � 768 -> height 614
		// Large: 1920 x 1080 -> height 864

		const float height = static_cast< float > ( sf::VideoMode::getDesktopMode ( ).height );

		if ( height > 1079.0f ) {

			return Screen::LARGE;
		}

		if ( height > 767.0f ) {

			return Screen::MEDIUM;
		}

		return Screen::SMALL;
	}
};

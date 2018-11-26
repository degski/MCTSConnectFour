
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

#include <SFML/Graphics.hpp>

#include "oska.hpp"


const Move Move::none = Move ( Location ( -1, -1 ), Location ( -1, -1 ) );
const Move Move::root = Move ( Location ( -2, -2 ), Location ( -2, -2 ) );
const Move Move::invalid; // Initialized to Move ( Location ( -3, -3 ), Location ( -3, -3 ) );
/*
float Hexagon::m_hori;
float Hexagon::m_vert;
float Hexagon::m_2_vert;
float Hexagon::m_2_vert_hori;
float Hexagon::m_center_l2radius;

std::normal_distribution<float> Hexagon::m_disx;
std::normal_distribution<float> Hexagon::m_disy;
*/

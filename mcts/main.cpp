
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

#define MSC_CLANG 1

#include <cstdint>
#include <cstdlib>

#include <iostream>

#include "autotimer.hpp"

#include "types.hpp"
#include "globals.hpp"
#include "player.hpp"
#include "moves.hpp"

#define CF 0

#if CF
#include "connect_four.hpp"
#else
#include "oska.hpp"
#endif

#include "mcts.hpp"


int wmain ( ) {
#if CF
    typedef ConnectFour<> State;
#else
    typedef OskaStateTemplate<5> State;
#endif
    using Mcts = mcts::Mcts<State>;
    std::optional<Player> winner;
    std::uint32_t matches = 0u, agent_wins = 0u, human_wins = 0u;
    putchar ( '\n' );
    sf::Time elapsed, match_start;
    for ( index_t i = 0; i < 1000; ++i ) {
        {
            State state;
            state.initialize ( );
            Mcts * mcts_agent = new Mcts ( ), * mcts_human = new Mcts ( );
            match_start = now ( );
            do {
                state.move_hash_winner ( state.playerToMove ( ) == Player::Type::agent ? mcts_agent->compute ( state, 20'000 ) : mcts_human->compute ( state, 2'000 ) );
                Mcts::prune ( state.playerToMove ( ) == Player::Type::agent ? mcts_agent : mcts_human, state );
            } while ( not ( winner = state.ended ( ) ) );
#if 0
            state.print ( );
            if ( winner.get ( ) == Player::Type::agent ) {
                std::wcout << L" Winner: Agent\n";
            }
            else if ( winner.get ( ) == Player::Type::human ) {
                std::wcout << L" Winner: Human\n";
            }
            else {
                std::wcout << L" Draw\n";
            }
#endif
            // saveToFile ( * mcts_human, "human" );
            delete mcts_human;
            // saveToFile ( * mcts_agent, "agent" );
            delete mcts_agent;
        }
        elapsed += since ( match_start );
        ++matches;
        switch ( winner->as_index ( ) ) {
            case ( index_t ) Player::Type::agent: ++agent_wins; break;
            case ( index_t ) Player::Type::human: ++human_wins; break;
            NO_DEFAULT_CASE;
        }
        float a = ( 1000.0f * agent_wins ) / float ( agent_wins + human_wins );
        a = ( ( int ) a ) / 10.0f;
        float h = ( 1000.0f * human_wins ) / float ( agent_wins + human_wins );
        h = ( ( int ) h ) / 10.0f;
        printf ( "\r Match %i: Agent%6.1f%% - Human%6.1f%% (%.1f Sec./Match - %.1f Sec.)", matches, a, h, elapsed.asSeconds ( ) / ( float ) matches, elapsed.asSeconds ( ) );
    }

    return EXIT_SUCCESS;
}

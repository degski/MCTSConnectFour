
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

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <cassert>

#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include "types.hpp"
// #include "vector_allocator.hpp"
#include "pool_allocator.hpp"

#include "graph_link_path.hpp"


template<typename ArcData, typename NodeData>
class RootedDiGraphAdjVectors {

    public:

    struct ArcType;
    using Arc = ArcType*;

    struct NodeType;
    using Node = NodeType*;

    // using InList = pt::pector<Arc, vector_allocator<Arc>, uindex_t>;
    // using OutList = pt::pector<Arc, vector_allocator<Arc>, uindex_t>;

    using InList = std::vector<Arc>;
    using OutList = std::vector<Arc>;

    using in_iterator = typename InList::iterator;
    using const_in_iterator = typename InList::const_iterator;

    using out_iterator = typename OutList::iterator;
    using const_out_iterator = typename OutList::const_iterator;

    using Link = Link<RootedDiGraphAdjVectors>;
    using Path = Path<RootedDiGraphAdjVectors>;

    struct ArcType {

        friend class RootedDiGraphAdjVectors<ArcData, NodeData>;

        template<typename ... Args>
        ArcType ( const Node source_, const Node target_, Args && ... args_ ) : source ( source_ ), target ( target_ ), data ( std::forward<Args> ( args_ ) ... ) { }

        Node source, target;
        ArcData data;
    };

    struct NodeType {

        friend class RootedDiGraphAdjVectors<ArcData, NodeData>;

        template<typename ... Args>
        NodeType ( Args && ... args_ ) : data ( std::forward<Args> ( args_ ) ... ) { }

        private:

        InList in_arcs; OutList out_arcs;

        public:

        NodeData data;
    };

    template<typename ... Args>
    RootedDiGraphAdjVectors ( Args && ... args_ ) :

        nodes_size ( 0 ), arcs_size ( 0 ),
        nodes ( ), arcs ( ),
        root_node ( addNode ( std::forward<Args> ( args_ ) ... ) ),
        top_node ( root_node ) {
    }

    void setRoot ( const Node node_ ) noexcept {

        root_node = node_;
        invalid_arc->target = root_node;
    }

    template<typename ... Args>
    Node addNode ( Args && ... args_ ) noexcept {

        Node node = nodes.new_element ( std::forward<Args> ( args_ ) ... );
        ++nodes_size;
        return node;
    }

    template<typename ... Args>
    Arc addArc ( const Node source_, const Node target_, Args && ... args_ ) noexcept {

        Arc arc = arcs.new_element ( source_, target_, std::forward<Args> ( args_ ) ... );
        ++arcs_size;
        outArcs ( source_ ).push_back ( arc );
        inArcs ( target_ ).push_back ( arc );
        return arc;
    }

    void erase ( const Arc arc_ ) noexcept {

        --arcs_size;
        inArcs ( arc_->target ).erase ( arc_ );
        outArcs ( arc_->source ).erase ( arc_ );
        arcs.delete_element ( arc_ );
    }

    void erase ( const Node node_ ) noexcept {

        for ( auto arc : inArcs ( node_ ) ) {

            erase ( arc );
        }

        for ( auto arc : outArcs ( node_ ) ) {

            erase ( arc );
        }

        --nodes_size;
        nodes.delete_element ( node_ );
    }

    Link link ( const Arc arc_ ) const noexcept { return Link ( arc_, arc_->target ); }
    Link link ( const Node source_, const Node target_ ) const noexcept {

        for ( const Arc arc : inArcs ( target_ ) ) {

            if ( source_ == arc->source ) {

                return { arc, target_ };
            }
        }

        return { invalid_arc, target_ };
    }

    template<typename It>
    Link link ( const It & it_ ) const noexcept { return Link ( *it_, ( *it_ )->target ); }

    bool isLeaf ( const Node node_ ) const noexcept { return node_->out_arcs.empty ( ); }
    bool isInternal ( const Node node_ ) const noexcept { return node_->out_arcs.size ( ); }

    const uindex_t inArcNum ( const Node node_ ) const noexcept { return node_->in_arcs.size ( ); }
    const uindex_t outArcNum ( const Node node_ ) const noexcept { return node_->out_arcs.size ( ); }

    const bool hasInArc ( const Node node_ ) const noexcept { return node_->in_arcs.size ( ); }
    const bool hasOutArc ( const Node node_ ) const noexcept { return node_->out_arcs.size ( ); }

    in_iterator beginIn ( const Node node_ ) { return std::begin ( inArcs ( node_ ) ); }
    const const_in_iterator cbeginIn ( const Node node_ ) const { return std::cbegin ( inArcs ( node_ ) ); }
    in_iterator endIn ( const Node node_ ) { return std::end ( inArcs ( node_ ) ); }
    const const_in_iterator cendIn ( const Node node_ ) const { return std::cend ( inArcs ( node_ ) ); }

    out_iterator beginOut ( const Node node_ ) { return std::begin ( outArcs ( node_ ) ); }
    const const_out_iterator cbeginOut ( const Node node_ ) const { return std::cbegin ( outArcs ( node_ ) ); }
    out_iterator endOut ( const Node node_ ) { return std::end ( outArcs ( node_ ) ); }
    const const_out_iterator cendOut ( const Node node_ ) const { return std::cend ( outArcs ( node_ ) ); }

    NodeData& operator [ ] ( const Node node_ ) noexcept { return node_->data; }
    const NodeData& operator [ ] ( const Node node_ ) const noexcept { return node_->data; }
    ArcData& operator [ ] ( const Arc arc_ ) noexcept { return arc_->data; }
    const ArcData& operator [ ] ( const Arc arc_ ) const noexcept { return arc_->data; }

    uindex_t nodeNum ( ) const noexcept { return nodes_size; }
    uindex_t arcNum ( ) const noexcept { return arcs_size; }

    InList& inArcs ( const Node node_ ) noexcept { return node_->in_arcs; }
    const InList& inArcs ( const Node node_ ) const noexcept { return node_->in_arcs; }
    OutList& outArcs ( const Node node_ ) noexcept { return node_->out_arcs; }
    const OutList& outArcs ( const Node node_ ) const noexcept { return node_->out_arcs; }

    private:

    uindex_t nodes_size, arcs_size;

    pa::pool_allocator<NodeType> nodes;
    pa::pool_allocator<ArcType> arcs;

    public:

    Node root_node;
    const Node top_node;

    private:

    static const std::unique_ptr<NodeType> smart_invalid_node;
    static const std::unique_ptr<ArcType> smart_invalid_arc;

    public:

    static const Node invalid_node;
    static const Arc invalid_arc;
};

template<typename ArcData, typename NodeData>
const std::unique_ptr<typename RootedDiGraphAdjVectors<ArcData, NodeData>::NodeType> RootedDiGraphAdjVectors<ArcData, NodeData>::smart_invalid_node = std::make_unique<typename RootedDiGraphAdjVectors<ArcData, NodeData>::NodeType> ();

template<typename ArcData, typename NodeData>
const std::unique_ptr<typename RootedDiGraphAdjVectors<ArcData, NodeData>::ArcType> RootedDiGraphAdjVectors<ArcData, NodeData>::smart_invalid_arc = std::make_unique<typename RootedDiGraphAdjVectors<ArcData, NodeData>::ArcType> ( smart_invalid_node .get ( ), smart_invalid_node.get ( ) );

template<typename ArcData, typename NodeData>
const typename RootedDiGraphAdjVectors<ArcData, NodeData>::Node RootedDiGraphAdjVectors<ArcData, NodeData>::invalid_node = RootedDiGraphAdjVectors<ArcData, NodeData>::smart_invalid_node.get ( );

template<typename ArcData, typename NodeData>
const typename RootedDiGraphAdjVectors<ArcData, NodeData>::Arc RootedDiGraphAdjVectors<ArcData, NodeData>::invalid_arc = RootedDiGraphAdjVectors<ArcData, NodeData>::smart_invalid_arc.get ( );

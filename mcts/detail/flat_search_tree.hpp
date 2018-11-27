
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
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>

#include <boost/container/deque.hpp>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

#include "types.hpp"
#include "transition.hpp"
#include "path.hpp"


namespace fst {

template<typename ArcData, typename NodeData>
class SearchTree;

namespace detail {

#define ARCID_INVALID_VALUE ( 0 )

struct ArcID {

    Int value;

    static const ArcID invalid;

    constexpr explicit ArcID ( ) noexcept :
        value { ARCID_INVALID_VALUE } { }
    explicit ArcID ( Int && v_ ) noexcept :
        value { std::move ( v_ ) } { }
    explicit ArcID ( const Int & v_ ) noexcept :
        value { v_ } { }
    explicit ArcID ( const std::size_t v_ ) noexcept :
        value { static_cast<Int> ( v_ ) } { }

    [[ nodiscard ]] constexpr const Int operator ( ) ( ) const noexcept {
        return value;
    }

    [[ nodiscard ]] bool operator == ( const ArcID rhs_ ) const noexcept {
        return value == rhs_.value;
    }
    [[ nodiscard ]] bool operator != ( const ArcID rhs_ ) const noexcept {
        return value != rhs_.value;
    }

    template<typename Stream>
    [[ maybe_unused ]] friend Stream & operator << ( Stream & out_, const ArcID id_ ) noexcept {
        if ( ARCID_INVALID_VALUE == id_.value ) {
            out_ << L'*';
        }
        else {
            out_ << static_cast< std::uint64_t > ( id_.value );
        }
        return out_;
    }

    private:

    friend class cereal::access;

    template<class Archive>
    inline void serialize ( Archive & ar_ ) {
        ar_ ( value );
    }
};

const ArcID ArcID::invalid { };


#define NODEID_INVALID_VALUE ( 0 )

struct NodeID {

    Int value;

    static const NodeID invalid;

    constexpr explicit NodeID ( ) noexcept :
        value { NODEID_INVALID_VALUE } { }
    explicit NodeID ( Int && v_ ) noexcept :
        value { std::move ( v_ ) } { }
    explicit NodeID ( const Int & v_ ) noexcept :
        value { v_ } { }
    explicit NodeID ( const std::size_t v_ ) noexcept :
        value { static_cast<Int> ( v_ ) } { }

    [[ nodiscard ]] constexpr const Int operator ( ) ( ) const noexcept {
        return value;
    }

    [[ nodiscard ]] bool operator == ( const NodeID rhs_ ) const noexcept {
        return value == rhs_.value;
    }
    [[ nodiscard ]] bool operator != ( const NodeID rhs_ ) const noexcept {
        return value != rhs_.value;
    }

    template<typename Stream>
    [[ maybe_unused ]] friend Stream & operator << ( Stream & out_, const NodeID id_ ) noexcept {
        if ( NODEID_INVALID_VALUE == id_.value ) {
            out_ << L'*';
        }
        else {
            out_ << static_cast<std::uint64_t> ( id_.value );
        }
        return out_;
    }

    private:

    friend class cereal::access;

    template<class Archive>
    inline void serialize ( Archive & ar_ ) {
        ar_ ( value );
    }
};

const NodeID NodeID::invalid { };


template<typename DataType>
struct Arc {

    NodeID source, target;
    ArcID next_in, next_out;

    using type = ArcID;
    using data_type = DataType;

    constexpr Arc ( ) noexcept {
    }
    template<typename ... Args>
    Arc ( NodeID && s_, NodeID && t_, Args && ... args_ ) noexcept :
        source { std::move ( s_ ) },
        target { std::move ( t_ ) },
        data { std::forward<Args> ( args_ ) ... } {
    }
    template<typename ... Args>
    Arc ( const NodeID s_, const NodeID t_, Args && ... args_ ) noexcept :
        source { s_ },
        target { t_ },
        data { std::forward<Args> ( args_ ) ... } {
    }

    template<typename Stream>
    [[ maybe_unused ]] friend Stream & operator << ( Stream & out_, const Arc a_ ) noexcept {
        out_ << L'<' << a_.source << L' ' << a_.target << L' ' << a_.next_in << L' ' << a_.next_out << L'>';
        return out_;
    }

    protected:

    template<typename ArcData, typename NodeData>
    friend class fst::SearchTree;

    DataType data;

    private:

    friend class cereal::access;

    template<class Archive>
    void serialize ( Archive & ar_ ) {
        ar_ ( source, target, next_in, next_out, data );
    }
};

template<typename DataType>
struct Node {

    ArcID head_in, tail_in, head_out, tail_out;
    Int in_size = 0, out_size = 0;

    using type = NodeID;
    using data_type = DataType;

    constexpr Node ( ) noexcept {
    }
    template<typename ... Args>
    Node ( Args && ... args_ ) noexcept :
        data { std::forward<Args> ( args_ ) ... } {
    }

    template<typename Stream>
    [[ maybe_unused ]] friend Stream & operator << ( Stream & out_, const Node node_ ) noexcept {
        out_ << L'<' << node_.head_in << L' ' << node_.tail_in << L' ' << node_.head_out << L' ' << node_.tail_out << L' ' << node_.in_size << L' ' << node_.out_size << L'>';
        return out_;
    }

    protected:

    template<typename ArcData, typename NodeData>
    friend class fst::SearchTree;

    DataType data;

    private:

    friend class cereal::access;

    template<class Archive>
    void serialize ( Archive & ar_ ) {
        ar_ ( head_in, tail_in, head_out, tail_out, in_size, out_size, data );
    }
};

} // namespace detail.


template<typename ArcData, typename NodeData>
class SearchTree {

    public:

    using ArcID = detail::ArcID;
    using NodeID = detail::NodeID;
    using Arc = detail::Arc<ArcData>;
    using Arcs = tagged_vector<Arc>;
    using Node = detail::Node<NodeData>;
    using Nodes = tagged_vector<Node>;
    using Transition = Transition<SearchTree>;
    using OptionalTransition = OptionalTransition<SearchTree>;
    using Path = Path<SearchTree>;
    using Visited = std::vector<NodeID>; // New m_nodes by old_index.
    using Stack = std::vector<NodeID>;
    using Queue = boost::container::deque<NodeID>;

    template<typename ... Args>
    SearchTree ( Args && ... args_ ) :
        root_arc { 1 },
        root_node { 1 },
        m_arcs { { }, { NodeID::invalid, root_node } },
        m_nodes { { }, { std::forward<Args> ( args_ ) ... } } {
        m_nodes [ root_node.value ].head_in = m_nodes [ root_node.value ].tail_in = root_arc;
        m_nodes [ root_node.value ].in_size = 1, m_nodes [ root_node.value ].out_size = 0;
    }

    template<typename ... Args>
    [[ maybe_unused ]] ArcID addArc ( const NodeID source_, const NodeID target_, Args && ... args_ ) noexcept {
        const ArcID id { m_arcs.size ( ) };
        m_arcs.emplace_back ( source_, target_, std::forward<Args> ( args_ ) ... );
        if ( ArcID::invalid == m_nodes [ source_.value ].head_out ) {
            m_nodes [ source_.value ].tail_out = m_nodes [ source_.value ].head_out = id;
        }
        else {
            m_nodes [ source_.value ].tail_out = m_arcs [ m_nodes [ source_.value ].tail_out.value ].next_out = id;
        }
        ++m_nodes [ source_.value ].out_size;
        if ( ArcID::invalid == m_nodes [ target_.value ].head_in ) {
            m_nodes [ target_.value ].tail_in = m_nodes [ target_.value ].head_in = id;
        }
        else {
            m_nodes [ target_.value ].tail_in = m_arcs [ m_nodes [ target_.value ].tail_in.value ].next_in = id;
        }
        ++m_nodes [ target_.value ].in_size;
        return id;
    }

    template<typename ... Args>
    [[ maybe_unused ]] NodeID addNode ( Args && ... args_ ) noexcept {
        const NodeID id { m_nodes.size ( ) };
        m_nodes.emplace_back ( std::forward<Args> ( args_ ) ... );
        return id;
    }

    class node_iterator {

        template<typename ArcData, typename NodeData>
        friend class SearchTree;

        typename Nodes::pointer m_ptr, m_end;

        public:

        using difference_type = typename Nodes::difference_type;
        using value_type = typename Nodes::value_type;
        using reference = typename Nodes::reference;
        using pointer = typename Nodes::pointer;
        using const_reference = typename Nodes::const_reference;
        using const_pointer = typename Nodes::const_pointer;
        using iterator_category = std::forward_iterator_tag;

        node_iterator ( SearchTree & tree_ ) noexcept :
            m_ptr { tree_.m_nodes.data ( ) },
            m_end { m_ptr + tree_.m_nodes.size ( ) } {
        }

        [[ nodiscard ]] const bool is_valid ( ) const noexcept {
            return m_end != m_ptr;
        }

        [[ maybe_unused ]] node_iterator & operator ++ ( ) noexcept {
            ++m_ptr;
            return * this;
        }

        [[ nodiscard ]] reference operator * ( ) const noexcept {
            return *m_ptr;
        }

        [[ nodiscard ]] pointer operator -> ( ) const noexcept {
            return m_ptr;
        }
    };

    class const_node_iterator {

        template<typename ArcData, typename NodeData>
        friend class SearchTree;

        typename Nodes::pointer m_ptr, m_end;

        public:

        using difference_type = typename Nodes::difference_type;
        using value_type = typename Nodes::value_type;
        using reference = typename Nodes::reference;
        using pointer = typename Nodes::pointer;
        using const_reference = typename Nodes::const_reference;
        using const_pointer = typename Nodes::const_pointer;
        using iterator_category = std::forward_iterator_tag;

        const_node_iterator ( const SearchTree & tree_ ) noexcept :
            m_ptr { tree_.m_nodes.data ( ) },
            m_end { m_ptr + tree_.m_nodes.size ( ) } {
        }

        [[ nodiscard ]] const bool is_valid ( ) const noexcept {
            return m_end != m_ptr;
        }

        [[ maybe_unused ]] const_node_iterator & operator ++ ( ) noexcept {
            ++m_ptr;
            return * this;
        }

        [[ nodiscard ]] const_reference operator * ( ) const noexcept {
            return *m_ptr;
        }

        [[ nodiscard ]] const_pointer operator -> ( ) const noexcept {
            return m_ptr;
        }
    };

    class iterator {

        template<typename ArcData, typename NodeData>
        friend class SearchTree;

        typename Arcs::pointer m_ptr, m_end;

        public:

        using difference_type = typename Arcs::difference_type;
        using value_type = typename Arcs::value_type;
        using reference = typename Arcs::reference;
        using pointer = typename Arcs::pointer;
        using const_reference = typename Arcs::const_reference;
        using const_pointer = typename Arcs::const_pointer;
        using iterator_category = std::forward_iterator_tag;

        iterator ( SearchTree & tree_ ) noexcept :
            m_ptr { tree_.m_arcs.data ( ) },
            m_end { m_ptr + tree_.m_arcs.size ( ) } {
        }

        [[ nodiscard ]] const bool is_valid ( ) const noexcept {
            return m_end != m_ptr;
        }

        [[ maybe_unused ]] iterator & operator ++ ( ) noexcept {
            ++m_ptr;
            return * this;
        }

        [[ nodiscard ]] reference operator * ( ) const noexcept {
            return *m_ptr;
        }

        [[ nodiscard ]] pointer operator -> ( ) const noexcept {
            return m_ptr;
        }
    };

    class const_iterator {

        template<typename ArcData, typename NodeData>
        friend class SearchTree;

        typename Arcs::pointer m_ptr, m_end;

        public:

        using difference_type = typename Arcs::difference_type;
        using value_type = typename Arcs::value_type;
        using reference = typename Arcs::reference;
        using pointer = typename Arcs::pointer;
        using const_reference = typename Arcs::const_reference;
        using const_pointer = typename Arcs::const_pointer;
        using iterator_category = std::forward_iterator_tag;

        const_iterator ( const SearchTree & tree_ ) noexcept :
            m_ptr { tree_.m_arcs.data ( ) },
            m_end { m_ptr + tree_.m_arcs.size ( ) } {
        }

        [[ nodiscard ]] const bool is_valid ( ) const noexcept {
            return m_end != m_ptr;
        }

        [[ maybe_unused ]] const_iterator & operator ++ ( ) noexcept {
            ++m_ptr;
            return * this;
        }

        [[ nodiscard ]] const_reference operator * ( ) const noexcept {
            return *m_ptr;
        }

        [[ nodiscard ]] const_pointer operator -> ( ) const noexcept {
            return m_ptr;
        }
    };

    class in_iterator {

        template<typename ArcData, typename NodeData>
        friend class SearchTree;

        SearchTree & m_st;
        ArcID m_id;

        public:

        using difference_type = typename Arcs::difference_type;
        using value_type = typename Arcs::value_type;
        using reference = typename Arcs::reference;
        using pointer = typename Arcs::pointer;
        using const_reference = typename Arcs::const_reference;
        using const_pointer = typename Arcs::const_pointer;
        using iterator_category = std::forward_iterator_tag;

        in_iterator ( SearchTree & tree_, const NodeID node_ ) noexcept :
            m_st { tree_ },
            m_id { m_st.m_nodes [ node_.value ].head_in } {
        }

        [[ nodiscard ]] const bool is_valid ( ) const noexcept {
            return ArcID::invalid != m_id;
        }

        [[ maybe_unused ]] in_iterator & operator ++ ( ) noexcept {
            m_id = m_st.m_arcs [ m_id.value ].next_in;
            return * this;
        }

        [[ nodiscard ]] reference operator * ( ) const noexcept {
            return m_st.m_arcs [ m_id.value ];
        }

        [[ nodiscard ]] pointer operator -> ( ) const noexcept {
            return m_st.m_arcs.data ( ) + m_id.value;
        }

        [[ nodiscard ]] const ArcID id ( ) const noexcept {
            return m_id;
        }
    };

    class const_in_iterator {

        template<typename ArcData, typename NodeData>
        friend class SearchTree;

        const SearchTree & m_st;
        ArcID m_id;

        public:

        using difference_type = typename Arcs::difference_type;
        using value_type = typename Arcs::value_type;
        using reference = typename Arcs::reference;
        using pointer = typename Arcs::pointer;
        using const_reference = typename Arcs::const_reference;
        using const_pointer = typename Arcs::const_pointer;
        using iterator_category = std::forward_iterator_tag;

        const_in_iterator ( const SearchTree & tree_, const NodeID node_ ) noexcept :
            m_st { tree_ },
            m_id { m_st.m_nodes [ node_.value ].head_in } {
        }

        [[ nodiscard ]] const bool is_valid ( ) const noexcept {
            return ArcID::invalid != m_id;
        }

        [[ maybe_unused ]] const_in_iterator & operator ++ ( ) noexcept {
            m_id = m_st.m_arcs [ m_id.value ].next_in;
            return * this;
        }

        [[ nodiscard ]] const_reference operator * ( ) const noexcept {
            return m_st.m_arcs [ m_id.value ];
        }

        [[ nodiscard ]] const_pointer operator -> ( ) const noexcept {
            return m_st.m_arcs.data ( ) + m_id.value;
        }

        [[ nodiscard ]] const ArcID id ( ) const noexcept {
            return m_id;
        }
    };

    class out_iterator {

        template<typename ArcData, typename NodeData>
        friend class SearchTree;

        SearchTree & m_st;
        ArcID m_id;

        public:

        using difference_type = typename Arcs::difference_type;
        using value_type = typename Arcs::value_type;
        using reference = typename Arcs::reference;
        using pointer = typename Arcs::pointer;
        using const_reference = typename Arcs::const_reference;
        using const_pointer = typename Arcs::const_pointer;
        using iterator_category = std::forward_iterator_tag;

        out_iterator ( SearchTree & tree_, const NodeID node_ ) noexcept :
            m_st { tree_ },
            m_id { m_st.m_nodes [ node_.value ].head_out } {
        }

        [[ nodiscard ]] const bool is_valid ( ) const noexcept {
            return ArcID::invalid != m_id;
        }

        [[ maybe_unused ]] out_iterator & operator ++ ( ) noexcept {
            m_id = m_st.m_arcs [ m_id.value ].next_out;
            return * this;
        }

        [[ nodiscard ]] reference operator * ( ) const noexcept {
            return m_st.m_arcs [ m_id.value ];
        }

        [[ nodiscard ]] pointer operator -> ( ) const noexcept {
            return m_st.m_arcs.data ( ) + m_id.value;
        }

        [[ nodiscard ]] const ArcID id ( ) const noexcept {
            return m_id;
        }
    };

    class const_out_iterator {

        template<typename ArcData, typename NodeData>
        friend class SearchTree;

        const SearchTree & m_st;
        ArcID m_id;

        public:

        using difference_type = typename Arcs::difference_type;
        using value_type = typename Arcs::value_type;
        using reference = typename Arcs::reference;
        using pointer = typename Arcs::pointer;
        using const_reference = typename Arcs::const_reference;
        using const_pointer = typename Arcs::const_pointer;
        using iterator_category = std::forward_iterator_tag;

        const_out_iterator ( const SearchTree & tree_, const NodeID node_ ) noexcept :
            m_st { tree_ },
            m_id { m_st.m_nodes [ node_.value ].head_out } {
        }

        [[ nodiscard ]] const bool is_valid ( ) const noexcept {
            return ArcID::invalid != m_id;
        }

        [[ maybe_unused ]] const_out_iterator & operator ++ ( ) noexcept {
            m_id = m_st.m_arcs [ m_id.value ].next_out;
            return * this;
        }

        [[ nodiscard ]] const_reference operator * ( ) const noexcept {
            return m_st.m_arcs [ m_id.value ];
        }

        [[ nodiscard ]] const_pointer operator -> ( ) const noexcept {
            return m_st.m_arcs.data ( ) + m_id.value;
        }

        [[ nodiscard ]] const ArcID id ( ) const noexcept {
            return m_id;
        }
    };


    [[ nodiscard ]] Transition link ( const ArcID arc_ ) const noexcept {
        return { arc_, m_arcs [ arc_.value ].target };
    }
    [[ nodiscard ]] OptionalTransition link ( const NodeID source_, const NodeID target_ ) const noexcept {
        for ( const_in_iterator it = cbeginIn ( target_ ); it.is_valid ( ); ++it ) {
            if ( source_ == it->source ) {
                return { it.id ( ), target_ };
            }
        }
        return { };
    }
    template<typename It>
    [[ nodiscard ]] Transition link ( const It & it_ ) const noexcept {
        return { it_.id ( ), it_->target };
    }


    [[ nodiscard ]] const bool isLeaf ( const NodeID node_ ) const noexcept {
        return not ( m_nodes [ node_.value ].out_size );
    }
    [[ nodiscard ]] const bool isInternal ( const NodeID node_ ) const noexcept {
        return m_nodes [ node_.value ].out_size;
    }

    [[ nodiscard ]] const Int inArcNum ( const NodeID node_ ) const noexcept {
        return m_nodes [ node_.value ].in_size;
    }
    [[ nodiscard ]] const Int outArcNum ( const NodeID node_ ) const noexcept {
        return m_nodes [ node_.value ].out_size;
    }

    [[ nodiscard ]] const bool hasInArc ( const NodeID node_ ) const noexcept {
        return m_nodes [ node_.value ].in_size;
    }
    [[ nodiscard ]] const bool hasOutArc ( const NodeID node_ ) const noexcept {
        return m_nodes [ node_.value ].out_size;
    }


    [[ nodiscard ]] in_iterator beginIn ( const NodeID node_ ) noexcept {
        return in_iterator { * this, node_ };
    }
    [[ nodiscard ]] const_in_iterator beginIn ( const NodeID node_ ) const noexcept {
        return const_in_iterator { * this, node_ };
    }
    [[ nodiscard ]] const_in_iterator cbeginIn ( const NodeID node_ ) const noexcept {
        return const_in_iterator { * this, node_ };
    }

    [[ nodiscard ]] out_iterator beginOut ( const NodeID node_ ) noexcept {
        return out_iterator { * this, node_ };
    }
    [[ nodiscard ]] const_out_iterator beginOut ( const NodeID node_ ) const noexcept {
        return const_out_iterator { * this, node_ };
    }
    [[ nodiscard ]] const_out_iterator cbeginOut ( const NodeID node_ ) const noexcept {
        return const_out_iterator { * this, node_ };
    }


    [[ nodiscard ]] ArcData & data ( const ArcID arc_ ) noexcept {
        return m_arcs [ arc_.value ].data;
    }
    [[ nodiscard ]] const ArcData & data ( const ArcID arc_ ) const noexcept {
        return m_arcs [ arc_.value ].data;
    }
    [[ nodiscard ]] NodeData & data ( const NodeID node_ ) noexcept {
        return m_nodes [ node_.value ].data;
    }
    [[ nodiscard ]] const NodeData & data ( const NodeID node_ ) const noexcept {
        return m_nodes [ node_.value ].data;
    }


    [[ nodiscard ]] Arc & operator [ ] ( const ArcID arc_ ) noexcept {
        return m_arcs [ arc_.value ];
    }
    [[ nodiscard ]] const Arc & operator [ ] ( const ArcID arc_ ) const noexcept {
        return m_arcs [ arc_.value ];
    }
    [[ nodiscard ]] Node & operator [ ] ( const NodeID node_ ) noexcept {
        return m_nodes [ node_.value ];
    }
    [[ nodiscard ]] const Node & operator [ ] ( const NodeID node_ ) const noexcept {
        return m_nodes [ node_.value ];
    }


    [[ nodiscard ]] const Int arcNum ( ) const noexcept {
        return static_cast<Int> ( m_arcs.size ( ) ) - 2;
    }
    [[ nodiscard ]] const Int nodeNum ( ) const noexcept {
        return static_cast<Int> ( m_nodes.size ( ) ) - 1;
    }


    // Destructively construct a sub-tree out of the current tree [Depth First].
    [[ nodiscard ]] SearchTree makeSubTree ( const NodeID root_node_to_be_ ) {
        assert ( NodeID::invalid != root_node_to_be_ );
        assert ( root_node != root_node_to_be_ );
        SearchTree sub_tree { std::move ( m_nodes [ root_node_to_be_.value ].data ) };
        // The Visited-vector stores the new NodeID's indexed by old NodeID's,
        // old NodeID's not present in the new tree have a value of NodeID::invalid.
        static Visited visited;
        visited.clear ( );
        visited.resize ( m_nodes.size ( ), NodeID::invalid );
        visited [ root_node_to_be_.value ] = sub_tree.root_node;
        static Stack stack;
        stack.clear ( );
        stack.push_back ( root_node_to_be_ );
        while ( stack.size ( ) ) {
            const NodeID parent = stack.back ( ); stack.pop_back ( );
            for ( ArcID a = m_nodes [ parent.value ].head_out; ArcID::invalid != a; a = m_arcs [ a.value ].next_out ) {
                const NodeID child { m_arcs [ a.value ].target };
                if ( NodeID::invalid == visited [ child.value ] ) { // Not visited yet.
                    visited [ child.value ] = sub_tree.addNode ( std::move ( m_nodes [ child.value ].data ) );
                    stack.push_back ( child );
                }
                sub_tree.addArc ( visited [ parent.value ], visited [ child.value ], std::move ( m_arcs [ a.value ].data ) );
            }
        }
        return sub_tree;
    }


    void traverseBreadthFirst ( const NodeID root_node_to_be_ = NodeID { 1 } ) { // Default is to walk the whole tree.
        assert ( NodeID::invalid != root_node_to_be_ );
        // The Visited-vector stores the new NodeID's indexed by old NodeID's,
        // old NodeID's not present in the new tree have a value of NodeID::invalid.
        static std::vector<bool> visited;
        visited.clear ( );
        visited.resize ( m_nodes.size ( ), false );
        visited [ root_node_to_be_.value ] = true;
        static Queue queue;
        queue.clear ( );
        queue.push_back ( root_node_to_be_ );
        while ( queue.size ( ) ) {
            const NodeID parent = queue.front ( ); queue.pop_front ( );
            for ( ArcID a = m_nodes [ parent.value ].head_out; ArcID::invalid != a; a = m_arcs [ a.value ].next_out ) {
                const NodeID child { m_arcs [ a.value ].target };
                if ( not ( visited [ child.value ] ) ) { // Not visited yet.
                    visited [ child.value ] = true;
                    queue.push_back ( child );

                    // All nodes traversed once here.

                    std::wcout << parent << L" -> " << ( a.value - 1 ) << L" -> " << child << L'\n';
                }

                // All arcs traversed once here [nodes are traversed (possibly) several times].
            }
        }
    }

    void traverseDepthFirst ( const NodeID root_node_to_be_ = NodeID { 1 } ) { // Default is to walk the whole tree.
        assert ( NodeID::invalid != root_node_to_be_ );
        // The Visited-vector stores the new NodeID's indexed by old NodeID's,
        // old NodeID's not present in the new tree have a value of NodeID::invalid.
        static std::vector<bool> visited;
        visited.clear ( );
        visited.resize ( m_nodes.size ( ), false );
        visited [ root_node_to_be_.value ] = true;
        static Stack stack;
        stack.clear ( );
        stack.push_back ( root_node_to_be_ );
        while ( stack.size ( ) ) {
            const NodeID parent = stack.back ( ); stack.pop_back ( );
            for ( ArcID a = m_nodes [ parent.value ].head_out; ArcID::invalid != a; a = m_arcs [ a.value ].next_out ) {
                const NodeID child { m_arcs [ a.value ].target };
                if ( not ( visited [ child.value ] ) ) { // Not visited yet.
                    visited [ child.value ] = true;
                    stack.push_back ( child );
                    std::wcout << parent << L" -> " << ( a.value - 1 ) << L" -> " << child << L'\n';
                }
                // std::wcout << parent << L" -> " << ( a.value - 1 ) << L" -> " << child << L'\n';
            }
        }
    }


    // Topological sorting, using Kahn's alogorithm (does not traverse all arcs).
    [[ nodiscard ]] std::vector<NodeID> topologicalSort ( ) const noexcept {
        std::vector<NodeID> sorted;
        static std::vector<bool> removed_arcs;
        removed_arcs.clear ( );
        removed_arcs.resize ( m_arcs.size ( ), false );
        static Stack stack;
        stack.clear ( );
        stack.push_back ( root_node );
        while ( stack.size ( ) ) {
            sorted.push_back ( stack.back ( ) ); stack.pop_back ( );
            for ( ArcID out = m_nodes [ sorted.back ( ).value ].head_out; ArcID::invalid != out; out = m_arcs [ out.value ].next_out ) {
                removed_arcs [ out.value ] = true;
                bool has_no_in_arcs = true;
                for ( ArcID in = m_nodes [ m_arcs [ out.value ].target.value ].head_in; ArcID::invalid != in; in = m_arcs [ in.value ].next_in ) {
                    if ( not ( removed_arcs [ in.value ] ) ) {
                        has_no_in_arcs = false;
                        break;
                    }
                }
                if ( has_no_in_arcs ) {
                    stack.push_back ( m_arcs [ out.value ].target );
                }
            }
        }
        return sorted;
    }

    // Data members.

    ArcID root_arc;
    NodeID root_node;

    private:

    Arcs m_arcs;
    Nodes m_nodes;
};

}

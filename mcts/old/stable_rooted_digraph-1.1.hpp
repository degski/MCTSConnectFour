
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

#include <iostream>
#include <mutex> // For std::lock_guard < >...

#include <boost/serialization/strong_typedef.hpp>
#include <boost/container/deque.hpp>

#define MSC_CLANG 1
#include <tbb/concurrent_vector.h>
#undef MSC_CLANG

#include <srwlock.hpp>
#include <integer_utils.hpp>

#define VERSION_11

#pragma warning ( push )
#pragma warning ( disable: 4244 )

// Root: The top node in a tree.
// Parent The converse notion of child.
// Siblings Nodes with the same parent.
// Descendant a node reachable by repeated proceeding from parent to child.
// Ancestor a node reachable by repeated proceeding from child to parent.
// Leaf a node with no children.
// Internal node a node with at least one child.
// External node a node with no children.
// Degree number of sub trees of a node.
// Edge connection between one node to another.
// Path a sequence of m_nodes and edges connecting a node with a descendant.
// Level The level of a node is defined by 1 + the number of connections between
//	the node and the root.
// Height of tree The height of a tree is the number of edges on the longest
//	downward path between the root and a leaf.
// Height of node The height of a node is the number of edges on the longest
//	downward path between that node and a leaf.
// Depth The depth of a node is the number of edges from the node to the tree's
//	root node.
// Forest A forest is a set of n = 0 disjoint trees.

namespace rt { // Rooted Tree namespace...

	typedef int32_t index_t;

	template < typename T, size_t Size = jimi::nextPowerOfTwo ( sizeof ( T ) ) - sizeof ( T ) >
	struct PaddedType : public T {

		template < typename ... Args > PaddedType ( Args && ... args_ ) : T ( std::forward < Args > ( args_ ) ... ) { }
		template < typename ... Args > PaddedType ( const bool b_, Args && ... args_ ) : T ( b_, std::forward < Args > ( args_ ) ... ) { }

	private:

		char padding_ [ Size ];
	};

	template < typename T >
	struct PaddedType < T, 0 > : public T { // No padding...

		template < typename ... Args > PaddedType ( Args && ... args_ ) : T ( std::forward < Args > ( args_ ) ... ) { }
		template < typename ... Args > PaddedType ( const bool b_, Args && ... args_ ) : T ( b_, std::forward < Args > ( args_ ) ... ) { }
	};


	typedef SRWLock < false > Lock;
	typedef std::lock_guard < Lock > ScopedLock;
	typedef struct nulldata { } nulldata_t;


	template < typename T >
	struct LockAndDataType : public Lock, public T {

		template < typename ... Args > explicit LockAndDataType ( const bool must_lock_, Args && ... args_ ) noexcept : T ( std::forward < Args > ( args_ ) ... ) { if ( must_lock_ ) { lock ( ); } }
	};

	template<>
	struct LockAndDataType < nulldata_t > : public Lock {

		explicit LockAndDataType ( const bool must_lock_ ) noexcept { if ( must_lock_ ) { lock ( ); } }
	};


	template < typename Graph >
	struct Link;


	template < typename ArcDataType, typename NodeDataType >
	class Tree {

	public:

		typedef Link < Tree > Link;

		BOOST_STRONG_TYPEDEF ( index_t, Arc )
		BOOST_STRONG_TYPEDEF ( index_t, Node )


	private:

		template < typename  T >
		class UnpaddedArcType {

			friend class Tree < ArcDataType, NodeDataType >;

			Node source, target;
			Arc next_in, next_out;

		protected:

			T data;

		public:

			template < typename ... Args >
			UnpaddedArcType ( const bool b_, Args && ... args_ ) noexcept : next_in ( Tree::invalid_arc ), next_out ( Tree::invalid_arc ), data ( b_, std::forward < Args > ( args_ ) ... ) { }

			template < typename ... Args >
			UnpaddedArcType ( const bool b_, const Node s_, const Node t_, Args && ... args_ ) noexcept : source ( s_ ), target ( t_ ), next_in ( Tree::invalid_arc ), next_out ( Tree::invalid_arc ), data ( b_, std::forward < Args > ( args_ ) ... ) { }

			void setSourceTarget ( const Node s_, const Node t_ ) noexcept { source = s_; target = t_; } // Must be locked.
		};


		template < typename T >
		class UnpaddedNodeType {

			friend class Tree < ArcDataType, NodeDataType >;

			Arc head_in, tail_in, head_out, tail_out;

		protected:

			T data;

		public:

			template < typename ... Args >
			UnpaddedNodeType ( const bool b_, Args && ... args_ ) noexcept : head_in ( Tree::invalid_arc ), tail_in ( head_in ), head_out ( Tree::invalid_arc ), tail_out ( head_out ), data ( b_, args_ ... ) { }

			template < typename ... Args >
			UnpaddedNodeType ( const bool b_, const Arc a_, Args && ... args_ ) noexcept : head_in ( a_ ), tail_in ( head_in ), head_out ( Tree::invalid_arc ), tail_out ( head_out ), data ( b_, args_ ... ) { }
		};


		// The main data containers...

		typedef PaddedType < UnpaddedArcType < LockAndDataType < ArcDataType > > >  ArcType;
		typedef PaddedType < UnpaddedNodeType < LockAndDataType < NodeDataType > > > NodeType;

		tbb::concurrent_vector <  ArcType >  m_arcs;
		tbb::concurrent_vector < NodeType > m_nodes;


	public:

		enum class ItType : index_t { in = 1, out = 2 };
		enum class LockType : index_t { read = 3, write = 4 };

		Arc   root_arc;
		Node root_node;

		static const Arc   invalid_arc;
		static const Node invalid_node;


		// Construct...

		Tree ( const size_t a_size_ = 128, const size_t n_size_ = 128 ) : root_arc ( 0 ), root_node ( 0 ) {

			m_arcs.reserve ( a_size_ ); m_nodes.reserve ( n_size_ );

			m_nodes.emplace_back ( false, static_cast < Arc > ( m_arcs.emplace_back ( false, invalid_node, root_node ) - m_arcs.begin ( ) ) );
		}

		void setRoot ( const Node n_ ) {

			root_node = n_;

			m_arcs [ root_arc ].setSourceTarget ( invalid_node, root_node );
		}

		void resetRoot ( ) {

			root_node = 0;

			m_arcs [ root_arc ].setSourceTarget ( invalid_node, root_node );
		}


	private:

		// Maps...

		template < typename T >
		class NodeMap {

			std::vector < T > m_map;

		public:

			NodeMap ( Tree & g_, const T & v_ = T ( ) ) : m_map ( g_.nodeNum ( ), v_ ) { }

			auto begin ( ) -> decltype ( m_map.begin ( ) ) const { return m_map.begin ( ); }
			auto end ( ) -> decltype ( m_map.end ( ) ) const { return m_map.end ( ); }

			T const & operator [ ] ( const Node i_ ) const noexcept { return m_map [ i_ ]; }
			T & operator [ ] ( const Node i_ ) noexcept { return m_map [ i_ ]; }
		};

		template < typename T >
		class ArcMap {

			std::vector < T > m_map;

		public:

			ArcMap ( Tree & g_, const T & v_ = T ( ) ) : m_map ( g_.arcNum ( ), v_ ) { }

			auto begin ( ) -> decltype ( m_map.begin ( ) ) const { return m_map.begin ( ); }
			auto end ( ) -> decltype ( m_map.end ( ) ) const { return m_map.end ( ); }

			T const & operator [ ] ( const Arc i_ ) const noexcept { return m_map [ i_ ]; }
			T & operator [ ] ( const Arc i_ ) noexcept { return m_map [ i_ ]; }
		};


	public:

		// Adding...

		Link addArc ( const Node s_, const Node t_ ) { // Add arc between existing m_nodes...

			const Link l ( static_cast < Arc > ( m_arcs.emplace_back ( false, s_, t_ ) - m_arcs.begin ( ) ), t_ );

			{
				ScopedLock node_lock ( m_nodes [ t_ ].data );
				ScopedLock arc_lock ( m_arcs [ m_nodes [ t_ ].tail_in ].data );

				m_nodes [ t_ ].tail_in = m_arcs [ m_nodes [ t_ ].tail_in ].next_in = l.arc;
			}

			{
				ScopedLock node_lock ( m_nodes [ s_ ].data );

				if ( m_nodes [ s_ ].head_out == invalid_arc ) {

					m_nodes [ s_ ].head_out = m_nodes [ s_ ].tail_out = l.arc;
				}

				else {

					ScopedLock arc_lock ( m_arcs [ m_nodes [ s_ ].tail_out ].data );

					m_nodes [ s_ ].tail_out = m_arcs [ m_nodes [ s_ ].tail_out ].next_out = l.arc;
				}
			}

			return l;
		}

		Link addArcUnsafe ( const Node s_, const Node t_ ) { // Add arc between existing m_nodes...

			// Non-locking addArc...

			const Link l ( static_cast < Arc > ( m_arcs.emplace_back ( false, s_, t_ ) - m_arcs.begin ( ) ), t_ );

			m_nodes [ t_ ].tail_in = m_arcs [ m_nodes [ t_ ].tail_in ].next_in = l.arc;

			if ( m_nodes [ s_ ].head_out == invalid_arc ) {

				m_nodes [ s_ ].head_out = m_nodes [ s_ ].tail_out = l.arc;
			}

			else {

				m_nodes [ s_ ].tail_out = m_arcs [ m_nodes [ s_ ].tail_out ].next_out = l.arc;
			}

			return l;
		}


	private:

		template < typename T >
		std::string inv ( const T t_ ) const {

			if ( t_ == invalid_arc ) {

				return std::string ( "invalid_arc" );
			}

			else if ( t_ == invalid_node ) {

				return std::string ( "invalid_node" );
			}

			else {

				char buf [ 64 ] = { };

				_itoa_s ( static_cast < index_t > ( t_ ), buf, 64, 10 );

				return std::string ( buf );
			}
		}


	public:

		void printArc ( const Arc a_ ) {

			std::cout << "Arc      " << inv ( a_ ) << std::endl;

			std::cout << "Source   " << inv ( m_arcs [ a_ ].source ) << std::endl;
			std::cout << "Next out " << inv ( m_arcs [ a_ ].next_out ) << std::endl;

			std::cout << "Target   " << inv ( m_arcs [ a_ ].target ) << std::endl;
			std::cout << "Next in  " << inv ( m_arcs [ a_ ].next_in ) << std::endl;

			std::cout << std::endl;

		}

		void printNode ( const Node n_ ) {

			std::cout << "Node     " << inv ( n_ ) << std::endl;

			std::cout << "Head in  " << inv ( m_nodes [ n_ ].head_in ) << std::endl;
			std::cout << "Tail in  " << inv ( m_nodes [ n_ ].tail_in ) << std::endl;

			std::cout << "Head out " << inv ( m_nodes [ n_ ].head_out ) << std::endl;
			std::cout << "Tail out " << inv ( m_nodes [ n_ ].tail_out ) << std::endl;

			std::cout << std::endl;
		}


		Link addNode ( const Node s_ ) { // Add node and incident arc...

			const Arc  a ( static_cast < Arc > ( m_arcs.emplace_back ( true ) - m_arcs.begin ( ) ) ); // Locked arc...
			const Node t ( static_cast < Node > ( m_nodes.emplace_back ( false, a ) - m_nodes.begin ( ) ) );

			m_arcs [ a ].setSourceTarget ( s_, t );
			m_arcs [ a ].data.unlock ( );

			ScopedLock node_lock ( m_nodes [ s_ ].data );

			if ( m_nodes [ s_ ].head_out == invalid_arc ) {

				m_nodes [ s_ ].tail_out = m_nodes [ s_ ].head_out = a;
			}

			else {

				Arc & ref = m_nodes [ s_ ].tail_out;

				ScopedLock arc_lock ( m_arcs [ ref ].data );

				ref = m_arcs [ ref ].next_out = a;
			}

			return Link ( a, t );
		}


		Link addNodeUnsafe ( const Node s_ ) { // Add node and incident arc...

			// Non-locking addNode...

			const  Arc a ( static_cast < Arc > ( m_arcs.emplace_back ( false ) - m_arcs.begin ( ) ) );
			const Node t ( static_cast < Node > ( m_nodes.emplace_back ( false, a ) - m_nodes.begin ( ) ) );

			m_arcs [ a ].setSourceTarget ( s_, t );

			if ( m_nodes [ s_ ].head_out == invalid_arc ) {

				m_nodes [ s_ ].tail_out = m_nodes [ s_ ].head_out = a;
			}

			else {

				m_nodes [ s_ ].tail_out = m_arcs [ m_nodes [ s_ ].tail_out ].next_out = a;
			}

			return Link ( a, t );
		}

		// Iterating...

	private:

		template < ItType I, LockType L >
		class ArcItBase {

		protected:

			Tree & g;
			Arc arc;

			ArcItBase ( Tree & g_, const Node n_ ) noexcept : g ( g_ ), arc ( I == ItType::in ? g.m_nodes [ n_ ].head_in : g.m_nodes [ n_ ].head_out ) { }
			virtual ~ArcItBase ( ) { } // Base class must have virtual destructor...

			Arc getNext ( const Arc a_ ) const noexcept { return I == ItType::in ? g.m_arcs [ a_ ].next_in : g.m_arcs [ a_ ].next_out; }
			Arc getNext ( ) const noexcept { return I == ItType::in ? g.m_arcs [ arc ].next_in : g.m_arcs [ arc ].next_out; }

			void lock ( const Arc a_ ) const noexcept { L == LockType::write ? g.m_arcs [ a_ ].data.lock ( ) : g.m_arcs [ a_ ].data.lockRead ( ); }
			void lock ( ) const noexcept { L == LockType::write ? g.m_arcs [ arc ].data.lock ( ) : g.m_arcs [ arc ].data.lockRead ( ); }

			bool tryLock ( const Arc a_ ) const noexcept { return L == LockType::write ? g.m_arcs [ a_ ].data.tryLock ( ) : g.m_arcs [ a_ ].data.tryLockRead ( ); }
			bool tryLock ( ) const noexcept { return L == LockType::write ? g.m_arcs [ arc ].data.tryLock ( ) : g.m_arcs [ arc ].data.tryLockRead ( ); }

			void unlock ( const Arc a_ ) const noexcept { L == LockType::write ? g.m_arcs [ a_ ].data.unlock ( ) : g.m_arcs [ a_ ].data.unlockRead ( ); }
			void unlock ( ) const noexcept { L == LockType::write ? g.m_arcs [ arc ].data.unlock ( ) : g.m_arcs [ arc ].data.unlockRead ( ); }

			bool valid ( const Arc a_ ) const noexcept { return a_ != g.invalid_arc; }
			bool valid ( ) const noexcept { return arc != g.invalid_arc; }

			bool invalid ( const Arc a_ ) const noexcept { return a_ == g.invalid_arc; }
			bool invalid ( ) const noexcept { return arc == g.invalid_arc; }

		public:

			Arc get ( ) const noexcept { return arc; }

			const bool operator != ( const Arc rhs_ ) const noexcept { return arc != rhs_; }
			const bool operator == ( const Arc rhs_ ) const noexcept { return arc == rhs_; }

			ArcDataType const & operator * ( ) const noexcept { return g.m_arcs [ arc ].data; }
			ArcDataType & operator * ( ) noexcept { return g.m_arcs [ arc ].data; }

			ArcDataType const * operator -> ( ) const noexcept { return &g.m_arcs [ arc ].data; }
			ArcDataType * operator -> ( ) noexcept { return &g.m_arcs [ arc ].data; }

			static const Arc end ( ) noexcept { return Tree::invalid_arc; }
		};

	public:

		template < ItType I, LockType L >
		class QueingArcIt_ : public ArcItBase < I, L > {

			boost::container::deque < Arc > m_queue;
			bool m_not_all_touched = true;

		public:

			QueingArcIt_ ( Tree & g_, const Node n_ ) noexcept : ArcItBase < I, L > ( g_, n_ ) {

				m_queue.push_back ( g_.invalid_arc );

				if ( ArcItBase < I, L >::valid ( ) ) {

					ArcItBase < I, L >::lock ( );
				}
			}

			Arc queuePop ( ) noexcept { const Arc a = m_queue.front ( ); m_queue.pop_front ( ); return a; }

			QueingArcIt_ & operator ++ ( ) noexcept {

				Arc next = ArcItBase < I, L >::getNext ( );

				ArcItBase < I, L >::unlock ( );

				while ( ArcItBase < I, L >::valid ( next ) ) {

					if ( ArcItBase < I, L >::tryLock ( next ) ) {

						break;
					}

					else {

						m_queue.push_back ( next );

						if ( m_not_all_touched ) {

							next = ArcItBase < I, L >::getNext ( next );

							if ( ArcItBase < I, L >::invalid ( next ) ) {

								m_not_all_touched = false;

								next = queuePop ( );
							}
						}

						else {

							next = queuePop ( );
						}
					}
				}

				ArcItBase < I, L >::arc = next;

				return * this;
			}

			// auto operator ++ ( index_t ) -> decltype ( * this ) = delete;
		};

		template < ItType I, LockType L >
		class ArcIt_ : public ArcItBase < I, L > {

		public:

			ArcIt_ ( Tree & g_, const Node n_ ) : ArcItBase < I, L > ( g_, n_ ) {

				if ( ArcItBase < I, L >::valid ( ) ) {

					ArcItBase < I, L >::lock ( );
				}
			}

			ArcIt_ & operator ++ ( ) noexcept {

				const Arc next = ArcItBase < I, L >::getNext ( );

				ArcItBase < I, L >::unlock ( );

				if ( ArcItBase < I, L >::valid ( next ) ) {

					ArcItBase < I, L >::lock ( next );
				}

				ArcItBase < I, L >::arc = next;

				return * this;
			}

			// auto operator ++ ( index_t ) -> decltype ( * this ) = delete;
		};

		class InIt {

			const Tree & g;
			Arc arc;

		public:

			InIt ( const Tree & g_, const Node n_ ) noexcept : g ( g_ ), arc ( g.m_nodes [ n_ ].head_in ) { }

			InIt & operator ++ ( ) noexcept {

				arc = g.m_arcs [ arc ].next_in;

				return * this;
			}

			// auto operator ++ ( index_t ) -> decltype ( * this ) = delete;
			const bool operator != ( const Arc rhs_ ) const noexcept { return arc != rhs_; }

			Arc get ( ) const noexcept { return arc; }
			static const Arc end ( ) noexcept { return Tree::invalid_arc; }
		};

		class OutIt {

			const Tree & g;
			Arc arc;

		public:

			OutIt ( const Tree &g_, const Node n_ ) noexcept : g ( g_ ), arc ( g.m_nodes [ n_ ].head_out ) { }

			OutIt & operator ++ ( ) noexcept {

				arc = g.m_arcs [ arc ].next_out;

				return * this;
			}

			// auto operator ++ ( index_t ) -> decltype ( * this ) = delete;
			const bool operator != ( const Arc rhs_ ) const noexcept { return arc != rhs_; }

			Arc get ( ) const noexcept { return arc; }
			static const Arc end ( ) noexcept { return Tree::invalid_arc; }
		};

		typedef QueingArcIt_ < ItType::in, LockType::write > QWrInIt;
		typedef QueingArcIt_ < ItType::out, LockType::write > QWrOutIt;
		typedef QueingArcIt_ < ItType::in, LockType::read > QRdInIt;
		typedef QueingArcIt_ < ItType::out, LockType::read > QRdOutIt;

		typedef ArcIt_ < ItType::in, LockType::write > WrInIt;
		typedef ArcIt_ < ItType::out, LockType::write > WrOutIt;
		typedef ArcIt_ < ItType::in, LockType::read > RdInIt;
		typedef ArcIt_ < ItType::out, LockType::read > RdOutIt;

		class ArcIt {

			Tree & g;
			Arc a;

		public:

			ArcIt ( Tree & g_ ) noexcept : g ( g_ ), a ( g.root_arc ) { }

			ArcIt & operator ++ ( ) noexcept { ++a; return * this; }
			// auto operator ++ ( index_t ) -> decltype ( * this ) = delete;

			const bool operator == ( const Arc rhs_ ) const noexcept { return a == rhs_; }
			const bool operator != ( const Arc rhs_ ) const noexcept { return a != rhs_; }

			Arc const operator * ( ) const noexcept { return a; }
			Arc operator * ( ) noexcept { return a; }

			Arc begin ( ) noexcept { return g.root_arc; }
			Arc end ( ) noexcept { return static_cast < Arc > ( g.m_arcs.end ( ) - g.m_arcs.begin ( ) ); }
		};

		class NodeIt {

			Tree & g;
			Node n;

		public:

			NodeIt ( Tree & g_ ) noexcept : g ( g_ ), n ( g.root_node ) { }

			NodeIt & operator ++ ( ) noexcept { ++n; return * this; }
			// auto operator ++ ( index_t ) -> decltype ( * this ) = delete;

			const bool operator == ( const Node rhs_ ) const noexcept { return n == rhs_; }
			const bool operator != ( const Node rhs_ ) const noexcept { return n != rhs_; }

			Node const operator * ( ) const noexcept { return n; }
			Node operator * ( ) noexcept { return n; }

			Node begin ( ) noexcept { return g.root_node; }
			Node end ( ) noexcept { return static_cast < Node > ( g.m_nodes.end ( ) - g.m_nodes.begin ( ) ); }
		};

		// Data access...

		ArcDataType const & operator [ ] ( const Arc i_ ) const noexcept { return m_arcs [ i_ ].data; }
		ArcDataType & operator [ ] ( const Arc i_ ) noexcept { return m_arcs [ i_ ].data; }

		template < typename It > ArcDataType const & operator [ ] ( const It & it_ ) const noexcept { return m_arcs [ it_.get ( ) ].data; }
		template < typename It > ArcDataType & operator [ ] ( const It & it_ ) noexcept { return m_arcs [ it_.get ( ) ].data; }

		NodeDataType const & operator [ ] ( const Node i_ ) const noexcept { return m_nodes [ i_ ].data; }
		NodeDataType & operator [ ] ( const Node i_ ) noexcept { return m_nodes [ i_ ].data; }

		template < typename It > ArcType const & operator ->* ( const It & it_ ) const noexcept { return m_arcs [ it_.get ( ) ]; }
		template < typename It > ArcType & operator ->* ( const It & it_ ) noexcept { return m_arcs [ it_.get ( ) ]; }

		// Connectivity...

		Node source ( const Arc a_ ) const noexcept { return m_arcs [ a_ ].source; }
		Node target ( const Arc a_ ) const noexcept { return m_arcs [ a_ ].target; }

		template < typename It > Node source ( const It & it_ ) const noexcept { return m_arcs [ it_.get ( ) ].source; }
		template < typename It > Node target ( const It & it_ ) const noexcept { return m_arcs [ it_.get ( ) ].target; }

		Link link ( const Arc a_ ) const noexcept { return Link ( a_, m_arcs [ a_ ].target ); }
		template < typename It > Link link ( const It & it_ ) const noexcept { return Link ( it_.get ( ), m_arcs [ it_.get ( ) ].target ); }

		bool isLeaf ( const Node n_ ) const noexcept { return m_nodes [ n_ ].head_out == invalid_arc; }
		bool isInternal ( const Node n_ ) const noexcept { return not ( isLeaf ( n_ ) ); }

		bool valid ( const Arc a_ ) const noexcept { return a_ != invalid_arc; }
		bool invalid ( const Arc a_ ) const noexcept { return a_ == invalid_arc; }

		bool valid ( const Node n_ ) const noexcept { return n_ != invalid_node; }
		bool invalid ( const Node n_ ) const noexcept { return n_ == invalid_node; }

		template < typename It > bool valid ( const It & it_ ) const noexcept { return it_.get ( ) != invalid_arc; }
		template < typename It > bool invalid ( const It & it_ ) const noexcept { return it_.get ( ) == invalid_arc; }


		// Locking...

		void lock ( const Arc a_ ) noexcept { m_arcs [ a_ ].data.lock ( ); }
		bool tryLock ( const Arc a_ ) noexcept { return m_arcs [ a_ ].data.tryLock ( ); }
		void unlock ( const Arc a_ ) noexcept { m_arcs [ a_ ].data.unlock ( ); }

		void lockRead ( const Arc a_ ) noexcept { m_arcs [ a_ ].data.lockRead ( ); }
		bool tryLockRead ( const Arc a_ ) noexcept { return m_arcs [ a_ ].data.tryLockRead ( ); }
		void unLockRead ( const Arc a_ ) noexcept { m_arcs [ a_ ].data.unlockRead ( ); }

		template < typename It > void lock ( const It & it_ ) noexcept { m_arcs [ it_.get ( ) ].data.lock ( ); }
		template < typename It > bool tryLock ( const It & it_ ) noexcept { return m_arcs [ it_.get ( ) ].data.tryLock ( ); }
		template < typename It > void unlock ( const It & it_ ) noexcept { m_arcs [ it_.get ( ) ].data.unlock ( ); }

		template < typename It > void lockRead ( const It & it_ ) noexcept { m_arcs [ it_.get ( ) ].data.lockRead ( ); }
		template < typename It > bool tryLockRead ( const It & it_ ) noexcept { return m_arcs [ it_.get ( ) ].data.tryLockRead ( ); }
		template < typename It > void unLockRead ( const It & it_ ) noexcept { m_arcs [ it_.get ( ) ].data.unlockRead ( ); }

		void lock ( const Node n_ ) noexcept { m_nodes [ n_ ].data.lock ( ); }
		bool tryLock ( const Node n_ ) noexcept { return m_nodes [ n_ ].data.tryLock ( ); }
		void unlock ( const Node n_ ) noexcept { m_nodes [ n_ ].data.unlock ( ); }

		void lockRead ( const Node n_ ) noexcept { m_nodes [ n_ ].data.lockRead ( ); }
		bool tryLockRead ( const Node n_ ) noexcept { return m_nodes [ n_ ].data.tryLockRead ( ); }
		void unlockRead ( const Node n_ ) noexcept { m_nodes [ n_ ].data.unlockRead ( ); }


		// Stats...

		size_t nodeNum ( ) const { return m_nodes.size ( ); }
		size_t arcNum ( ) const { return m_arcs.size ( ); }


		// Arcs and Nodes...

		Arc arc ( const Node s_, const Node t_ ) const {

			for ( Arc a = m_nodes [ s_ ].head_out; a != invalid_arc; a = m_arcs [ a ].next_out ) { // Out-iteration...

				if ( m_arcs [ a ].target == t_ ) {

					return a;
				}
			}

			return invalid_arc;
		}


		Node randomNode ( ) const noexcept {

			static jimi::XoRoShiRo128Plus rng;

			return ( Node ) boost::random::uniform_int_distribution < index_t > ( 1, m_nodes.size ( ) - 1 ) ( rng );
		}


		// Slice...

		void sliceDF ( Tree & new_srd_, const Node & old_node_ ) const {

			if ( invalid ( old_node_ ) ) { // New root did not exist...

				return;
			}

			// auto_timer t ( milliseconds );

			// Not thread-safe...

			typedef std::vector < Node > Visited; // New m_nodes by old_index...
			typedef boost::container::deque < Node > Stack;

			Visited visited ( nodeNum ( ), Tree::invalid_node );

			visited [ old_node_ ] = root_node;

			Stack stack;

			stack.emplace_back ( old_node_ );

			new_srd_ [ root_node ] = ( * this ) [ old_node_ ];

			while ( not ( stack.empty ( ) ) ) {

				const Node parent = stack.back ( );

				stack.pop_back ( );

				for ( Arc a = m_nodes [ parent ].head_out; a != invalid_arc; a = m_arcs [ a ].next_out ) {

					const Node child ( target ( a ) );

					if ( visited [ child ] == Tree::invalid_node ) { // Not visited yet...

						const Link link = new_srd_.addNodeUnsafe ( visited [ parent ] );

						visited [ child ] = link.target;
						stack.emplace_back ( child );

						new_srd_ [ link.arc ] = ( * this ) [ a ];
						new_srd_ [ link.target ] = ( * this ) [ child ];
					}

					else {

						new_srd_ [ new_srd_.addArcUnsafe ( visited [ parent ], visited [ child ] ).arc ] = ( * this ) [ a ];
					}
				}
			}
		}


		void sliceBF ( Tree & new_srd_, const Node & old_node_ ) const {

			if ( invalid ( old_node_ ) ) { // New root did not exist...

				return;
			}

			// Not thread-safe...

			typedef std::vector < Node > Visited; // New m_nodes by old_index...
			typedef boost::container::deque < Node > Queue;

			Visited visited ( nodeNum ( ), Tree::invalid_node );

			visited [ old_node_ ] = root_node;

			Queue queue;

			queue.emplace_back ( old_node_ );

			new_srd_ [ root_node ] = ( * this ) [ old_node_ ];

			while ( not ( queue.empty ( ) ) ) {

				const Node parent = queue.front ( );

				queue.pop_front ( );

				for ( Arc a = m_nodes [ parent ].head_out; a != invalid_arc; a = m_arcs [ a ].next_out ) {

					const Node child ( target ( a ) );

					if ( visited [ child ] == Tree::invalid_node ) { // Not visited yet...

						const Link link = new_srd_.addNodeUnsafe ( visited [ parent ] );

						visited [ child ] = link.target;
						queue.emplace_back ( child );

						new_srd_ [ link.arc ] = ( * this ) [ a ];
						new_srd_ [ link.target ] = ( * this ) [ child ];
					}

					else {

						new_srd_ [ new_srd_.addArcUnsafe ( visited [ parent ], visited [ child ] ).arc ] = ( * this ) [ a ];
					}
				}
			}
		}
	};

	template < typename ArcDataType, typename NodeDataType >
	typename Tree < ArcDataType, NodeDataType >::Arc  const  Tree < ArcDataType, NodeDataType >::invalid_arc = static_cast <  Tree<ArcDataType, NodeDataType>::Arc > ( INT_MIN + 0 );
	template < typename ArcDataType, typename NodeDataType >
	typename Tree < ArcDataType, NodeDataType >::Node const Tree < ArcDataType, NodeDataType >::invalid_node = static_cast < Tree<ArcDataType, NodeDataType>::Node > ( INT_MIN + 1 );


	template < typename Graph >
	struct Link {

		typedef typename Graph::Arc   Arc;
		typedef typename Graph::Node Node;

		Arc arc; Node target;

		Link ( ) noexcept { }
		Link ( const Link & l_ ) noexcept : arc ( l_.arc ), target ( l_.target ) { }
		Link ( const Arc a_, const Node t_ ) noexcept : arc ( a_ ), target ( t_ ) { }
		Link ( const Arc a_ ) noexcept : arc ( a_ ) { }
		Link ( const Node t_ ) noexcept : target ( t_ ) { }

		bool operator == ( const Link & rhs_ ) { return arc == rhs_.arc && target == rhs_.target; }
		bool operator != ( const Link & rhs_ ) { return arc != rhs_.arc || target != rhs_.target; }
	};


	template < typename Graph >
	class Path {

		typedef typename Graph::Arc   Arc;
		typedef typename Graph::Node Node;

		typedef Link < Graph > Link;

		boost::container::deque < Link > m_path;

	public:

		Path ( ) : m_path ( ) { }
		Path ( const Link & l_ ) : m_path ( 1, l_ ) { }
		Path ( const Arc a_, const Node t_ ) : m_path ( 1, Link ( a_, t_ ) ) { }

		void push ( const Link & l_ ) noexcept { m_path.emplace_back ( l_ ); }
		void push ( const Arc a_, const Node t_ ) noexcept { m_path.emplace_back ( a_, t_ ); }
		Link pop ( ) noexcept { const Link r = m_path.back ( ); m_path.pop_back ( ); return r; }

		auto back ( ) -> decltype ( m_path.back ( ) ) const { return m_path.back ( ); }
		auto begin ( ) -> decltype ( m_path.begin ( ) ) const { return m_path.begin ( ); }
		auto end ( ) -> decltype ( m_path.end ( ) ) const { return m_path.end ( ); }

		void resize ( const size_t s_ ) noexcept { m_path.resize ( s_ ); }
		void reserve ( const size_t s_ ) noexcept { m_path.reserve ( s_ ); }
	};
}

#pragma warning ( pop )


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

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>

#include <ska_sort.hpp>


namespace pa {

	namespace detail {

		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		inline typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::size_type
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::pad_pointer ( const DataPtr p_, const size_type align_ )
			const noexcept
		{
			uintptr_t result = reinterpret_cast< uintptr_t > ( p_ );
			return ( ( align_ - result ) % align_ );
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::chunk_pool_allocator ( )
			noexcept
			: m_current_block ( nullptr )
			, m_current_slot ( nullptr )
			, m_last_slot ( nullptr )
			, m_free_slots ( nullptr )
		{
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::chunk_pool_allocator ( const chunk_pool_allocator & chunk_pool_ )
			noexcept :
		chunk_pool_allocator ( )
		{
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::chunk_pool_allocator ( chunk_pool_allocator && chunk_pool_ )
			noexcept
			: m_current_block ( chunk_pool_.m_current_block )
			, m_current_slot ( chunk_pool_.m_current_slot )
			, m_last_slot ( chunk_pool_.m_last_slot )
			, m_free_slots ( chunk_pool_.m_free_slots )
		{
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		template<typename U>
		chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::chunk_pool_allocator ( const chunk_pool_allocator<U, ChunkCount, PageSize, MinAlloc> & chunk_pool_ )
			noexcept :
		chunk_pool_allocator ( )
		{
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>&
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::operator = ( chunk_pool_allocator && chunk_pool_ )
			noexcept
		{
			if ( this != &chunk_pool_ ) {

				std::swap ( m_current_block, chunk_pool_.m_current_block );
				m_current_slot = chunk_pool_.m_current_slot;
				m_last_slot = chunk_pool_.m_last_slot;
				m_free_slots = chunk_pool_.freeSlots;
			}

			return *this;
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::~chunk_pool_allocator ( )
			noexcept
		{
            // This destructor does not handle the case of non-trivially-destructible
            // objects in case the ChunkCount > 1.  This can be implemented, but adds
            // overhead, which is guessed to outweigh the benefits of using a pool...

			if constexpr ( std::negation<std::is_trivially_destructible<T>>::value ) {

				if constexpr ( ChunkCount == 1 ) {

					SlotPtrVector sorted_free_vector = pointer_vector ( m_free_slots );

					SlotPointer curr = m_current_block, last = m_current_slot, prev = nullptr, ptr = nullptr;

					if ( sorted_free_vector.empty ( ) ) {

						while ( curr != nullptr ) {

							prev = curr->next;
							ptr = first_slot ( curr );

							while ( ptr < last ) {

								reinterpret_cast<pointer>( ptr++ )->~value_type ( );
							}

							virtual_free ( curr );

							curr = prev;
							last = last_slot ( curr );
						}
					}

					else {

						ska_sort ( std::begin ( sorted_free_vector ), std::end ( sorted_free_vector ) );

						while ( curr != nullptr ) {

							prev = curr->next;
							ptr = first_slot ( curr );

							while ( ptr < last ) {

								if ( ptr != *std::lower_bound ( std::cbegin ( sorted_free_vector ), std::cend ( sorted_free_vector ), ptr ) ) {

									reinterpret_cast<pointer> ( ptr )->~value_type ( );
								}

								++ptr;
							}

							virtual_free ( curr );

							curr = prev;
							last = last_slot ( curr );
						}
					}
				}

				else {

					static_assert ( bool ( std::negation<std::is_trivially_destructible<T>>::value ) && ChunkCount == 1, "Destructor will not be called." );
				}
			}

			else if constexpr ( std::is_trivially_destructible<T>::value ) {

				SlotPointer curr = m_current_block;

				while ( curr != nullptr ) {

					SlotPointer prev = curr->next;
					virtual_free ( curr );
					curr = prev;
				}
			}

			m_current_block = nullptr;
			m_current_slot = nullptr;
			m_last_slot = nullptr;
			m_free_slots = nullptr;
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		void
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::allocate_block ( )
		{
			// Allocate space for the new block and store a pointer to the previous one...

			SlotPointer new_block = virtual_alloc ( block_size ( ) );
			new_block->next = m_current_block;
			m_current_block = new_block;

			// Pad block body to satisfy the alignment requirements for elements...

			DataPtr body = reinterpret_cast<DataPtr> ( new_block ) + sizeof ( SlotPointer );
			m_current_slot = reinterpret_cast<SlotPointer> ( body + pad_pointer ( body, alignof ( SlotType ) ) );
			m_last_slot = reinterpret_cast<SlotPointer> ( reinterpret_cast<DataPtr> ( m_current_slot ) + chunk_size ( ) );
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		inline typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::pointer
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::allocate ( size_type n_, const_pointer hint_ )
		{
			if ( m_free_slots != nullptr ) {

				const pointer result = reinterpret_cast<pointer> ( m_free_slots );
				m_free_slots = m_free_slots->next;
				return result;
			}

			else {

				if ( m_current_slot >= m_last_slot ) {

					allocate_block ( );
				}

				const pointer result = reinterpret_cast<pointer> ( m_current_slot );
				m_current_slot += ChunkCount;
				return result;
			}
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		inline void
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::deallocate ( pointer p_, size_type n_ )
		{
			if ( p_ != nullptr ) {

				reinterpret_cast<SlotPointer> ( p_ )->next = m_free_slots;
				m_free_slots = reinterpret_cast<SlotPointer> ( p_ );
			}
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		constexpr typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::size_type
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::chunk_count ( )
			noexcept
		{
			return ChunkCount;
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		constexpr typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::size_type
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::no_chunks ( )
			noexcept
		{
			return ( block_size ( ) - sizeof ( SlotPointer ) - alignof ( value_type ) ) / chunk_size ( );
		}

		// chunk_size in bytes...

		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		constexpr typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::size_type
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::chunk_size ( )
			noexcept
		{
			return ChunkCount * sizeof ( value_type );
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		constexpr typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::size_type
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::block_size ( )
			noexcept
		{
			const size_type minimum_required_block_size = sizeof ( SlotPointer ) + MinAlloc * chunk_size ( ) + alignof ( value_type );
			size_type bs = PageSize;

			while ( bs < minimum_required_block_size ) {

				bs += PageSize;
			}

			return bs;
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		constexpr typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::size_type
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::max_size ( )
			noexcept
		{
			const size_type max_blocks = -1 / block_size ( );
			return block_size ( ) * max_blocks;
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::size_type
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::memory_size ( )
			const noexcept
		{
			SlotPointer sp = m_current_block;
			size_type ms = 0;

			while ( sp != nullptr ) {

				ms += block_size ( );
				sp = sp->next;
			}

			return ms;
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		inline typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::SlotPointer
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::first_slot ( const SlotPointer block_ )
			const noexcept
		{
			const DataPtr body = reinterpret_cast<DataPtr>( block_ ) + sizeof ( SlotPointer );
			return reinterpret_cast<SlotPointer> ( body + pad_pointer ( body, alignof ( SlotType ) ) );
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		inline typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::SlotPointer
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::last_slot ( const SlotPointer block_ )
			const noexcept
		{
			const DataPtr body = reinterpret_cast<DataPtr>( block_ ) + sizeof ( SlotPointer );
			return reinterpret_cast<SlotPointer> ( body + pad_pointer ( body, alignof ( SlotType ) ) + chunk_size ( ) );
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::SlotPtrVector
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::pointer_vector ( SlotPointer sp_ )
			const noexcept
		{
			SlotPtrVector v;
			v.reserve ( PageSize / ( sizeof ( SlotPointer ) * ChunkCount ) );

			while ( sp_ != nullptr ) {

				v.push_back ( sp_ );
				sp_ = sp_->next;
			}

			return v;
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		inline typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::SlotPtrVector
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::block_vector ( )
			const noexcept
		{
			return pointer_vector ( m_current_block );
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		inline typename chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::SlotPointer
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::virtual_alloc ( const size_type size_ )
			const noexcept
		{
            return reinterpret_cast<SlotPointer> ( VirtualAlloc ( 0, ( unsigned long long ) size_, 0x00001000 | 0x00002000, 0x04 ) );
		}



		template<typename T, std::size_t ChunkCount, std::size_t PageSize, std::size_t MinAlloc>
		inline void
			chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::virtual_free ( const SlotPointer block_ )
			const noexcept
		{
			VirtualFree ( reinterpret_cast< void* > ( block_ ), 0, 0x00008000 );
		}


		template<typename T, std::size_t PageSize, std::size_t MinAlloc>
		template<typename U, typename ... Args>
		inline void
			node_pool_allocator<T, PageSize, MinAlloc>::construct ( U* p_, Args && ... args_ )
		{
			new ( p_ ) U ( std::forward<Args> ( args_ )... );
		}



		template<typename T, std::size_t PageSize, std::size_t MinAlloc>
		template<typename U>
		inline void
			node_pool_allocator<T, PageSize, MinAlloc>::destroy ( U *p_ )
		{
			if constexpr ( std::negation<std::is_trivially_destructible<T>>::value ) {

				p_->~U ( );
			}
		}



		template<typename T, std::size_t PageSize, std::size_t MinAlloc>
		template<typename ... Args>
		inline typename node_pool_allocator<T, PageSize, MinAlloc>::pointer
			node_pool_allocator<T, PageSize, MinAlloc>::new_element ( Args && ... args_ )
		{
			pointer result = base_allocator_type::allocate ( );
			construct<value_type> ( result, std::forward<Args> ( args_ ) ... );
			return result;
		}



		template<typename T, std::size_t PageSize, std::size_t MinAlloc>
		inline void
			node_pool_allocator<T, PageSize, MinAlloc>::delete_element ( pointer p_ )
		{
			if ( p_ != nullptr ) {

				if constexpr ( std::negation<std::is_trivially_destructible<T>>::value ) {

					p_->~value_type ( );
				}

                base_allocator_type::deallocate ( p_ );
			}
		}
	}
}


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
#include <climits>
#include <cstddef>

#include <memory>
#include <type_traits>
#include <vector>


extern "C" {

   void* __cdecl VirtualAlloc ( void*, unsigned long long, unsigned long, unsigned long );
   int __cdecl VirtualFree ( void*, unsigned long long, unsigned long );
}

namespace pa {

	namespace detail {

		template<typename T, std::size_t ChunkCount, std::size_t PageSize = 4096, std::size_t MinAlloc = 8>
		class chunk_pool_allocator {

		    public:

            using allocator_type = chunk_pool_allocator;
            using value_type = T;
            using pointer = value_type*;
            using const_pointer = typename std::pointer_traits<pointer>::template rebind<value_type const>;
            using void_pointer = typename std::pointer_traits<pointer>::template rebind<void>;
            using const_void_pointer = typename std::pointer_traits<pointer>::template rebind<const void>;
            using difference_type = typename std::pointer_traits<pointer>::difference_type;
            using size_type = std::make_unsigned_t<difference_type>;

			using propagate_on_container_copy_assignment = std::false_type;
			using propagate_on_container_move_assignment = std::true_type;
			using propagate_on_container_swap = std::true_type;

			template<typename U> struct rebind { using other = chunk_pool_allocator<U, ChunkCount, PageSize, MinAlloc>; };

			chunk_pool_allocator ( ) noexcept;
			chunk_pool_allocator ( const chunk_pool_allocator & chunk_pool_ ) noexcept;
			chunk_pool_allocator ( chunk_pool_allocator && chunk_pool_ ) noexcept;
			template<typename U> chunk_pool_allocator ( const chunk_pool_allocator<U, ChunkCount, PageSize, MinAlloc> & chunk_pool_ ) noexcept;

			~chunk_pool_allocator ( ) noexcept;

			chunk_pool_allocator& operator = ( const chunk_pool_allocator & ) = delete;
			chunk_pool_allocator& operator = ( chunk_pool_allocator && chunk_pool_ ) noexcept;

			// Allocates one chunk of ChunkCount objects at a time. n and hint are ignored...

			pointer allocate ( size_type n_ = 1, const_pointer hint_ = 0 );
			void deallocate ( pointer p_, size_type n_ = 1 );

			static constexpr size_type chunk_count ( ) noexcept;
			static constexpr size_type no_chunks ( ) noexcept;
			static constexpr size_type chunk_size ( ) noexcept;
			static constexpr size_type block_size ( ) noexcept;
			static constexpr size_type max_size ( ) noexcept;

			size_type memory_size ( ) const noexcept;

		    private:

			union Slot;

			using DataPtr = char*;
			using SlotType = Slot;
			using SlotPointer = Slot*;
			using SlotPtrVector = std::vector<SlotPointer>;

			union Slot {

				value_type element;
				SlotPointer next;

                ~Slot ( ) = delete;
			};

			SlotPointer m_current_block;
			SlotPointer m_current_slot;
			SlotPointer m_last_slot;
			SlotPointer m_free_slots;

			size_type pad_pointer ( const DataPtr p_, const size_type align_ ) const noexcept;
			void allocate_block ( );
			SlotPointer first_slot ( const SlotPointer block_ ) const noexcept;
			SlotPointer last_slot ( const SlotPointer block_ ) const noexcept;
			inline SlotPtrVector pointer_vector ( SlotPointer sp_ ) const noexcept;
			SlotPtrVector block_vector ( ) const noexcept;

			inline SlotPointer virtual_alloc ( const size_type size_ ) const noexcept;
			inline void virtual_free ( const SlotPointer block_ ) const noexcept;
		};



		template<typename T, std::size_t PageSize = 4096, std::size_t MinAlloc = 8>
		class node_pool_allocator : public chunk_pool_allocator<T, 1, PageSize, MinAlloc> {

            using base_allocator_type = typename chunk_pool_allocator<T, 1, PageSize, MinAlloc>::allocator_type;

		    public:

            using allocator_type = base_allocator_type;
            using value_type = typename base_allocator_type::value_type;
            using pointer = typename base_allocator_type::pointer;
            using const_pointer = typename base_allocator_type::const_pointer;
            using void_pointer = typename base_allocator_type::void_pointer;
            using const_void_pointer = typename base_allocator_type::const_void_pointer;
            using difference_type = typename base_allocator_type::difference_type;
            using size_type = typename base_allocator_type::size_type;

            using propagate_on_container_copy_assignment = typename base_allocator_type::propagate_on_container_copy_assignment;
            using propagate_on_container_move_assignment = typename base_allocator_type::propagate_on_container_move_assignment;
            using propagate_on_container_swap = typename base_allocator_type::propagate_on_container_swap;

 			template<typename U, typename ... Args> void construct ( U *p, Args && ... args_ );
			template<typename U> void destroy ( U *p_ );

			template<typename ... Args> pointer new_element ( Args && ... args_ );
			void delete_element ( pointer p_ );
		};
	}



	template<typename T, std::size_t ChunkCount = 1, std::size_t PageSize = 4096, std::size_t MinAlloc = 8>
	class pool_allocator : public detail::chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc> {

        using base_allocator_type = typename detail::chunk_pool_allocator<T, ChunkCount, PageSize, MinAlloc>::allocator_type;

        public:

        using allocator_type = base_allocator_type;
        using value_type = typename base_allocator_type::value_type;
        using pointer = typename base_allocator_type::pointer;
        using const_pointer = typename base_allocator_type::const_pointer;
        using void_pointer = typename base_allocator_type::void_pointer;
        using const_void_pointer = typename base_allocator_type::const_void_pointer;
        using difference_type = typename base_allocator_type::difference_type;
        using size_type = typename base_allocator_type::size_type;

        using propagate_on_container_copy_assignment = typename base_allocator_type::propagate_on_container_copy_assignment;
        using propagate_on_container_move_assignment = typename base_allocator_type::propagate_on_container_move_assignment;
        using propagate_on_container_swap = typename base_allocator_type::propagate_on_container_swap;
    };



	template<typename T, std::size_t PageSize, std::size_t MinAlloc>
	class pool_allocator<T, 1, PageSize, MinAlloc> : public detail::node_pool_allocator<T, PageSize, MinAlloc> {

        using base_allocator_type = typename detail::node_pool_allocator<T, PageSize, MinAlloc>::allocator_type;

        public:

        using allocator_type = base_allocator_type;
        using value_type = typename base_allocator_type::value_type;
        using pointer = typename base_allocator_type::pointer;
        using const_pointer = typename base_allocator_type::const_pointer;
        using void_pointer = typename base_allocator_type::void_pointer;
        using const_void_pointer = typename base_allocator_type::const_void_pointer;
        using difference_type = typename base_allocator_type::difference_type;
        using size_type = typename base_allocator_type::size_type;

        using propagate_on_container_copy_assignment = typename base_allocator_type::propagate_on_container_copy_assignment;
        using propagate_on_container_move_assignment = typename base_allocator_type::propagate_on_container_move_assignment;
        using propagate_on_container_swap = typename base_allocator_type::propagate_on_container_swap;
    };
}

#include "pool_allocator.inl"

#include "Allocator.h"

Pointer::Pointer() : Pointer{ nullptr, nullptr }
{
}

Pointer::Pointer(std::shared_ptr<void*> ptr, std::shared_ptr<size_t> size) : ptr{ ptr }, size{ size }
{
}

Pointer::Pointer(const Pointer& src): Pointer { src.ptr, src.size }
{
}

void* Pointer::get() const
{
	if (ptr != nullptr)
	{
		return *ptr;
	}
	else
	{
		return nullptr;
	}
}

void Pointer::set(void* data, size_t size)
{
	if (size > 0 && size < *(this->size))
	{
		memcpy(*ptr, data, size);
	}
	else
	{
		std::runtime_error("Couldn't set this amount of memory. Block size exceeded");
	}
}

Allocator::Allocator(void *base, size_t size) : base_ptr{ base }, max_size{ size }
{
	free_blocks.emplace(base_ptr, size);
}

Pointer Allocator::alloc(size_t N)
{
	size_t min_size = SIZE_MAX;
	void* ptr = nullptr;
	for (auto block : free_blocks)
	{
		if (block.second >= N && block.second < min_size)
		{
			min_size = block.second;
			ptr = block.first;
		}
	}

	if (ptr != nullptr)
	{
		size_t rest_size = min_size - N;
		free_blocks.erase(ptr);
		if (rest_size > 0)
		{
			free_blocks.emplace(static_cast<char*>(ptr) + N, rest_size);
		}

		std::shared_ptr<void*> addr(new void*);
		std::shared_ptr<size_t> sz(new size_t);
		*addr = ptr;
		*sz = N;
		allocated_blocks.emplace(ptr, ptr_info(addr, sz));
		return Pointer(addr, sz);
	}
	else
	{
		throw AllocError(AllocErrorType::NoMemory, "Out of memory");
	}
}

void Allocator::realloc(Pointer &p, size_t N)
{
	free(p);
	p = alloc(N);
}

void Allocator::free(Pointer &p)
{
	void* address_to_free = p.get();
	auto existing_block = allocated_blocks.find(address_to_free);
	if (existing_block == allocated_blocks.end())
	{
		return;
		//throw AllocError(AllocErrorType::InvalidFree, "Free of non-allocated block");
	}
	size_t existing_block_size = *(existing_block->second.second);

	for (auto free_block : free_blocks)
	{
		void* next_byte_ptr = static_cast<char*>(existing_block->first) + existing_block_size;
		if (free_block.first == next_byte_ptr)
		{
			void* new_free_block_ptr = existing_block->first;
			size_t new_free_block_size = free_block.second + existing_block_size;
			free_blocks.erase(free_block.first);
			free_blocks.emplace(new_free_block_ptr, new_free_block_size);
			
			*(existing_block->second.first) = nullptr;
			*(existing_block->second.second) = 0;
			
			allocated_blocks.erase(address_to_free);
			return;
		}
	}
	free_blocks.emplace(existing_block->first, existing_block_size);

	*(existing_block->second.first) = nullptr;
	*(existing_block->second.second) = 0;

	allocated_blocks.erase(address_to_free);
}

void Allocator::defrag()
{
	std::map<void*, ptr_info> new_heap;
	size_t heap_size = 0;
	char* current_ptr = static_cast<char*>(base_ptr);
	for (auto block : allocated_blocks)
	{
		ptr_info info = block.second;
		size_t block_size = *(info.second);
		*(info.first) = current_ptr;
		
		new_heap.emplace(current_ptr, ptr_info(info.first, info.second));
		current_ptr += block_size;
		heap_size += block_size;
	}
	allocated_blocks = new_heap;
	free_blocks.clear();
	if (heap_size < max_size)
	{
		free_blocks.emplace(
			static_cast<char*>(base_ptr) + heap_size,
			max_size - heap_size
		);
	}
}

std::string Allocator::dump()
{
	return "";
}
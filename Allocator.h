#pragma once
#include <stdexcept>
#include <string>
#include <map>
#include <memory>

typedef std::pair<std::shared_ptr<void*>, size_t> ptr_info;

enum class AllocErrorType {
	InvalidFree,
	NoMemory,
};

class AllocError : std::runtime_error {
private:
	AllocErrorType type;

public:
	AllocError(AllocErrorType _type, std::string message) :
		runtime_error(message),
		type(_type)
	{}

	AllocErrorType getType() const { return type; }
};

class Allocator;

class Pointer {
public:
	Pointer();
	Pointer(std::shared_ptr<void*> ptr);
	Pointer(const Pointer& src);
	inline void *get() const;
private:
	std::shared_ptr<void*> ptr;
};

class Allocator {
public:
	Allocator(void *base, size_t size);

	Pointer alloc(size_t N);
	void realloc(Pointer &p, size_t N);
	void free(Pointer &p);

	void defrag();
	std::string dump();
private:
	std::pair<void*, size_t> find_fittest_block(size_t N);
	void free_impl(std::map<void*, ptr_info>::iterator existing_block);
	void* base_ptr;
	size_t max_size;
	std::map<void*, ptr_info> allocated_blocks;
	std::map<void*, size_t> free_blocks;
};

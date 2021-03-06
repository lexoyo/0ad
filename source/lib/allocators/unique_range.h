#ifndef INCLUDED_ALLOCATORS_UNIQUE_RANGE
#define INCLUDED_ALLOCATORS_UNIQUE_RANGE

#include "lib/lib_api.h"

// we usually don't hold multiple references to allocations, so unique_ptr
// can be used instead of the more complex (ICC generated incorrect code on
// 2 occasions) and expensive shared_ptr.
// a custom deleter is required because allocators such as ReserveAddressSpace need to
// pass the size to their deleter. we want to mix pointers from various allocators, but
// unique_ptr's deleter is fixed at compile-time, so it would need to be general enough
// to handle all allocators.
// storing the size and a function pointer would be one such solution, with the added
// bonus of no longer requiring a complete type at the invocation of ~unique_ptr.
// however, this inflates the pointer size to 3 words. if only a few allocator types
// are needed, we can replace the function pointer with an index stashed into the
// lower bits of the pointer (safe because allocations are always aligned to the
// word size).
typedef intptr_t IdxDeleter;

// no-op deleter (use when returning part of an existing allocation)
// must be zero because reset() sets address (which includes idxDeleter) to zero.
static const IdxDeleter idxDeleterNone = 0;

static const IdxDeleter idxDeleterAligned = 1;

// (temporary value to prevent concurrent calls to AddUniqueRangeDeleter)
static const IdxDeleter idxDeleterBusy = -IdxDeleter(1);

// governs the maximum number of IdxDeleter and each pointer's alignment requirements
static const IdxDeleter idxDeleterBits = 0x7;

typedef void (*UniqueRangeDeleter)(void* pointer, size_t size);

/**
 * @return the next available IdxDeleter and associate it with the deleter.
 * halts the program if the idxDeleterBits limit has been reached.
 *
 * thread-safe, but no attempt is made to detect whether the deleter has already been
 * registered (would require a mutex). each allocator must ensure they only call this once.
 **/
LIB_API IdxDeleter AddUniqueRangeDeleter(UniqueRangeDeleter deleter);

LIB_API void CallUniqueRangeDeleter(void* pointer, size_t size, IdxDeleter idxDeleter) throw();


// unfortunately, unique_ptr allows constructing without a custom deleter. to ensure callers can
// rely upon pointers being associated with a size, we introduce a `UniqueRange' replacement.
// its interface is identical to unique_ptr except for the constructors, the addition of
// size() and the removal of operator bool (which avoids implicit casts to int).
class UniqueRange
{
public:
	typedef void* pointer;
	typedef void element_type;

	UniqueRange()
	{
		Set(0, 0, idxDeleterNone);
	}

	UniqueRange(pointer p, size_t size, IdxDeleter deleter)
	{
		Set(p, size, deleter);
	}

	UniqueRange(RVALUE_REF(UniqueRange) rvalue)
	{
		UniqueRange& lvalue = LVALUE(rvalue);
		address_ = lvalue.address_;
		size_ = lvalue.size_;
		lvalue.address_ = 0;
	}

	UniqueRange& operator=(RVALUE_REF(UniqueRange) rvalue)
	{
		UniqueRange& lvalue = LVALUE(rvalue);
		if(this != &lvalue)
		{
			Delete();
			address_ = lvalue.address_;
			size_ = lvalue.size_;
			lvalue.address_ = 0;
		}
		return *this;
	}

	~UniqueRange()
	{
		Delete();
	}

	pointer get() const
	{
		return pointer(address_ & ~idxDeleterBits);
	}

	IdxDeleter get_deleter() const
	{
		return IdxDeleter(address_ & idxDeleterBits);
	}

	size_t size() const
	{
		return size_;
	}

	// side effect: subsequent get_deleter will return idxDeleterNone
	pointer release()	// relinquish ownership
	{
		pointer ret = get();
		address_ = 0;
		return ret;
	}

	void reset()
	{
		Delete();
		address_ = 0;
	}

	void reset(pointer p, size_t size, IdxDeleter deleter)
	{
		Delete();
		Set(p, size, deleter);
	}

	void swap(UniqueRange& rhs)
	{
		std::swap(address_, rhs.address_);
		std::swap(size_, rhs.size_);
	}

	// don't define construction and assignment from lvalue,
	// but the declarations must be accessible
	UniqueRange(const UniqueRange&);
	UniqueRange& operator=(const UniqueRange&);

private:
	void Set(pointer p, size_t size, IdxDeleter deleter)
	{
		ASSERT((uintptr_t(p) & idxDeleterBits) == 0);
		ASSERT(deleter <= idxDeleterBits);

		address_ = uintptr_t(p) | deleter;
		size_ = size;

		ASSERT(get() == p);
		ASSERT(get_deleter() == deleter);
		ASSERT(this->size() == size);
	}

	void Delete()
	{
		CallUniqueRangeDeleter(get(), size(), get_deleter());
	}

	// (IdxDeleter is stored in the lower bits of address since size might not even be a multiple of 4.)
	uintptr_t address_;
	size_t size_;
};

namespace std {

static inline void swap(UniqueRange& p1, UniqueRange& p2)
{
	p1.swap(p2);
}

static inline void swap(RVALUE_REF(UniqueRange) p1, UniqueRange& p2)
{
	p2.swap(LVALUE(p1));
}

static inline void swap(UniqueRange& p1, RVALUE_REF(UniqueRange) p2)
{
	p1.swap(LVALUE(p2));
}

}

#endif	// #ifndef INCLUDED_ALLOCATORS_UNIQUE_RANGE

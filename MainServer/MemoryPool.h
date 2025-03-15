#pragma once
constexpr uint64_t MEMORYPOOL_SIZE = 100000000;
constexpr uint64_t BLOCK_SIZE = 8192;

struct MemoryBlock
{
	bool isbAllocated;
	uint8_t* ptr;
	uint64_t writtenSize;

	MemoryBlock()
		: isbAllocated(false)
		, ptr(new uint8_t[BLOCK_SIZE])
		, writtenSize(0)
	{
		ZeroMemory(ptr, BLOCK_SIZE);
	}

	~MemoryBlock()
	{
		delete[] ptr;
	}
};

class MemoryPool final
{
public:
	MemoryPool();
	~MemoryPool();

	MemoryPool(const MemoryPool& rhs) = delete;
	MemoryPool& operator==(const MemoryPool& rhs) = delete;

	MemoryBlock* Allocate();
	void DeAllocate(MemoryBlock* memoryBlock);

private:
	bool isAllocatable() const;

private:
	SRWLOCK mSRWLock;

	uint64_t mBlockCount;
	uint64_t mAllocatedBlocks;
	MemoryBlock* mMemoryBlocks;
	MemoryBlock* mFrontBlock;
	MemoryBlock* mRearBlock;
};


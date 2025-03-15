#include "stdafx.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool()
	: mSRWLock()
	, mBlockCount(MEMORYPOOL_SIZE / BLOCK_SIZE)
	, mAllocatedBlocks(0)
	, mMemoryBlocks(new MemoryBlock[mBlockCount])
	, mFrontBlock(nullptr)
	, mRearBlock(nullptr)
{
	InitializeSRWLock(&mSRWLock);
	mFrontBlock = &mMemoryBlocks[0];
	mRearBlock = &mMemoryBlocks[mBlockCount - 1];
}

MemoryPool::~MemoryPool()
{
	for (int i = 0; i < mBlockCount; i++)
	{
		delete mMemoryBlocks[i].ptr;
	}
	delete[] mMemoryBlocks;
}

MemoryBlock* MemoryPool::Allocate()
{
	AcquireSRWLockExclusive(&mSRWLock);
	mAllocatedBlocks++;

	MemoryBlock* ret = nullptr;

	if (isAllocatable() == false)
	{
		ret = nullptr;
	}

	if (mFrontBlock == &mMemoryBlocks[mBlockCount - 1])
	{
		mFrontBlock = &mMemoryBlocks[0];
		ret = &mMemoryBlocks[mBlockCount - 1];
	}
	else
	{
		ret = mFrontBlock++;
	}

	ReleaseSRWLockExclusive(&mSRWLock);

	return ret;
}

void MemoryPool::DeAllocate(MemoryBlock* memoryBlock)
{
	AcquireSRWLockExclusive(&mSRWLock);
	mRearBlock = memoryBlock;
	mAllocatedBlocks--;

	if (mRearBlock == &mMemoryBlocks[mBlockCount - 1])
	{
		mRearBlock = &mMemoryBlocks[0];
	}
	else
	{
		mRearBlock++;
	}

	ReleaseSRWLockExclusive(&mSRWLock);
}

bool MemoryPool::isAllocatable() const
{
	return mFrontBlock != mRearBlock;
}

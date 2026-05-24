#pragma once

#include <iostream>
#include <vector>
#include <atomic>
#include <thread>

template<typename T>
class LockFreeQueue
{
public:
	explicit LockFreeQueue(size_t capacity)
		: mCapacity(capacity), mBuffer(capacity)
	{
	}

	// Multi-Producer: 여러 스레드가 동시에 호출 가능
	bool push(const T& value)
	{
		size_t head = mHead.load(std::memory_order_relaxed);

		while (true)
		{
			// 현재 tail 값을 읽어와 큐가 꽉 찼는지 검사
			// 이 검사는 100% 정확하지 않을 수 있으나, CAS 루프에서 최종 결정됨
			if (head - mTail.load(std::memory_order_acquire) >= mCapacity)
			{
				return false; // 큐가 꽉 참
			}

			// CAS 루프: head 값을 1 증가시키려는 시도를 함
			// 다른 스레드가 먼저 head를 바꾸었다면 실패하고 루프를 다시 시작
			if (mHead.compare_exchange_weak(head, head + 1, std::memory_order_release, std::memory_order_relaxed))
			{
				break; // 슬롯 확보 성공!
			}
		}

		// 확보한 슬롯에 데이터 쓰기
		mBuffer[head % mCapacity] = value;
		return true;
	}

	// Single-Consumer: 단 하나의 스레드만 호출해야 함
	bool pop(T& value)
	{
		size_t tail = mTail.load(std::memory_order_relaxed);

		// head 값을 읽어와 큐가 비었는지 검사
		if (tail >= mHead.load(std::memory_order_acquire))
		{
			return false; // 큐가 비었음
		}

		// 데이터 읽기
		value = mBuffer[tail % mCapacity];

		// tail 인덱스 업데이트 (다른 스레드에게 이 슬롯이 비었음을 알림)
		mTail.store(tail + 1, std::memory_order_release);
		return true;
	}

private:
	const size_t mCapacity;
	std::vector<T> mBuffer;

	// false sharing을 막기 위해 cache line 크기에 맞춰 정렬
	alignas(64) std::atomic<size_t> mHead{ 0 };
	alignas(64) std::atomic<size_t> mTail{ 0 };
};
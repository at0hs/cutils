#pragma once

#include "ExtendedTypes/LinkedList.h"

template<class T>
class LinkedListWrapper : public LinkedList_t {
public:
	LinkedListWrapper() {
		LinkedList_Init(this, nullptr, sizeof(T));
	}
	~LinkedListWrapper() {
		LinkedList_Destroy(this);
	}
	std::size_t Count() {
		return LinkedList_Count(this);
	}
	bool IsEmpty() {
		return LinkedList_IsEmpty(this);
	}
	void Add(const T &object) {
		LinkedList_Add(this, &object);
	}
	void Add(T &&object) {
		LinkedList_Add(this, &object);
	}
	T At(std::uint32_t index) {
		return *static_cast<const T *>(LinkedList_At(this, index));
	}
	bool Contains(const T &object) {
		return LinkedList_Contains(this, const_cast<T *>(&object));
	}
	bool Remove(const T &object) {
		return LinkedList_Remove(this, &object);
	}
	bool RemoveAt(std::uint32_t index) {
		return LinkedList_RemoveAt(this, index);
	}
	void Clear() {
		LinkedList_Clear(this);
	}
};

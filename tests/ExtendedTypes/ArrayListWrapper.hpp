#pragma once

#include "ExtendedTypes/ArrayList.h"
#include <vector>

template<class T>
class ArrayListWrapper : public ArrayList {
private:
	void cb(uint64_t index, const void *object) {

	}
public:
	ArrayListWrapper() {
		ArrayList_Init(this, nullptr, sizeof(T));
	}
	~ArrayListWrapper() {
		ArrayList_Destroy(this);
	}
	std::size_t Capacity() {
		return ArrayList_Capacity(this);
	}
	std::size_t Length() {
		return ArrayList_Length(this);
	}

	bool IsEmpty() {
		return ArrayList_IsEmpty(this);
	}
	void Resize(std::size_t newSize) {
		ArrayList_Resize(this, newSize);
	}
	T At(uint64_t index) {
		return *static_cast<const T *>(ArrayList_At(this, index));
	}
	void Add(T &object) {
		ArrayList_Add(this, &object);
	}
	void Add(T &&object) {
		ArrayList_Add(this, &object);
	}
	void AddReference(T &object) {
		ArrayList_AddReference(this, &object);
	}
	void AddRange(std::vector<T> objects) {
		ArrayList_AddRange(this, objects.data(), objects.size());
	}
	void Append(const ArrayList &list) {
		ArrayList_Append(this, &list);
	}
	void Insert(uint64_t index, const T &object) {
		ArrayList_Insert(this, index, &object);
	}
	void Insert(uint64_t index, T &&object) {
		ArrayList_Insert(this, index, &object);
	}
	uint64_t IndexOf(const T &object) {
		return ArrayList_IndexOf(this, &object);
	}
	bool Contains(const T object) {
		return ArrayList_Contains(this, &object);
	}
	bool Remove(const T &object) {
		return ArrayList_Remove(this, &object);
	}
	bool Remove(const T &&object) {
		return ArrayList_Remove(this, &object);
	}
	bool RemoveAt(uint64_t index) {
		return ArrayList_RemoveAt(this, index);
	}
	void Clear() {
		ArrayList_Clear(this);
	}
	void ForEach(void (*callback)(uint64_t index, const void *object)) {
		ArrayList_ForEach(this, callback);
	}
};

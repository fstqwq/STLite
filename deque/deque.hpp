#pragma GCC optimize(3,"Ofast","inline")
#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP
#include "exceptions.hpp"
#include <cstddef>
#include <cstring>

namespace sjtu {
template<typename T, int blockSize = 1333, int wasteRatio = 5>
class deque {
private:
	static const int maxW = blockSize / wasteRatio;
	struct block;
	class allocator;//pool
public :
	class iterator;
	class const_iterator;

private:
	allocator M;
	block* End;
	int sz;
#define Begin End->nxt
#define Last End->pre
	struct block {
		block* pre, *nxt;
		T* v;
		int sz, l, r;
		char mem[sizeof(T) * blockSize];
		void reset() {
			pre = nxt = NULL;
			l = sz = r = 0;
			v = (T*)mem;
		}
		block () {
			reset();
		}
		int lsiz() {
			return l;
		}
		int rsiz() {
			return blockSize - r;
		}
		void dell() {
			(v + l++)->~T();
			sz--;
		}
		void delr() {
			(v + --r)->~T();
			sz--;
		}
#define cp(a, b) memcpy(a, b, sizeof(T))
		void addl(const T& x) {
			if (!sz) l = r = blockSize;

			if (!lsiz()) {
				int b = (blockSize - sz + 1) / 2; //O(log wasteRatio) totally

				for (int i = r - 1; i >= l; i--) cp(v + i + b, v + i);

				l += b, r += b;
			}

			--l, sz++;
			new(v + l) T(x);
		}
		void addr(const T& x) {
			if (!sz) l = r = 0;

			if (!rsiz()) {
				int b = (blockSize - sz + 1) / 2;

				for (int i = l; i < r; i++) cp(v + i - b, v + i);

				l -= b, r -= b;
			}

			new(v + r) T(x);
			r++, sz++;
		}
		void insert(int p, const T& x) {
			if ((p <= sz - p && lsiz()) || !rsiz()) {
				--l;

				for (int i = 0; i < p; i++) cp(v + l + i, v + l + i + 1);
			} else {
				r++;

				for (int i = sz - 1; i >= p; i--) cp(v + l + i + 1, v + l + i);
			}

			new(v + l + p) T(x);
			sz++;
		}
		void erase(int p) {
			(v + l + p)->~T();

			if (p <= sz - p) {
				l++;

				for (int i = p - 1; i >= 0; i--) cp(v + l + i, v + l + i - 1);
			} else {
				r--;

				for (int i = p; i < sz - 1; i++) cp(v + l + i, v + l + i + 1);
			}

			sz--;
		}
#undef cp
	};
	class allocator {
		block* pool;
	public:
		allocator () {
			pool = NULL;
		}
		~allocator () {
			while (pool) {
				block* nxt = pool->nxt;
				delete pool;
				pool = nxt;
			}
		}
		block* New(bool isEnd = 0) {
			block* ret;

			if (pool) ret = pool, pool = pool->nxt, ret->reset();
			else ret = new block;

			if (isEnd) ret->sz = ret->r = blockSize, ret->v = 0;

			return ret;
		}
		block* New(block* pre, block* nxt) {
			block* ret;

			if (pool) ret = pool, pool = pool->nxt, ret->reset();
			else ret = new block;

			ret->pre = pre, ret->nxt = nxt, pre->nxt = ret, nxt->pre = ret;
			return ret;
		}
		void Del(block* x) {
			x->nxt->pre = x->pre, x->pre->nxt = x->nxt, x->nxt = pool, pool = x;
		}
	};

public:
	class iterator {
	public:
		const deque* id;
		int rk;
		block* b;
		int p;
		iterator () {}
		iterator (const deque* i, int r) noexcept : id(i), rk(r) {
			int c = 0;
			b = id->End, p = 0;

			for (block* x = i->Begin; x != i->End; c += x->sz, x = x->nxt)
				if (c + x->sz > rk) {
					b = x, p = rk - c;
					return;
				}
		}
		iterator (const deque* i, int r, block* u, int v) : id(i), rk(r), b(u), p(v) {}
		iterator operator+(const int& n) const {
			return iterator(id, rk + n);
		}
		iterator operator-(const int& n) const {
			return iterator(id, rk - n);
		}
		int operator-(const iterator& o) const {
			if (id != o.id) throw invalid_iterator();

			return rk - o.rk;
		}
		iterator operator+=(const int& n) {
			return (*this = *this + n);
		}
		iterator operator-=(const int& n) {
			return (*this = *this - n);
		}
		iterator& operator++() {
			if (p + 1 < b->sz) return *this = iterator(id, rk + 1, b, p + 1);
			else return *this = iterator(id, rk + 1, b->nxt, 0);
		}
		iterator operator++(int) {
			iterator ret = *this;
			return ++*this, ret;
		}
		iterator& operator--() {
			if (p) return *this = iterator(id, rk - 1, b, p - 1);
			else return *this = iterator(id, rk - 1, b->pre, b->pre->sz - 1);
		}
		iterator operator--(int) {
			iterator ret = *this;
			return --*this, ret;
		}
		T& operator*() const {
			if (!b->v) throw index_out_of_bound();

			return b->v[b->l + p];
		}
		T* operator->() const {
			if (!b->v) throw index_out_of_bound();

			return b->v + b->l + p;
		}
		bool operator==(const iterator& o) const {
			return id == o.id && rk == o.rk;
		}
		bool operator!=(const iterator& o) const {
			return !(id == o.id && rk == o.rk);
		}
	};

	class const_iterator {
		iterator it;
	public:
		const_iterator() {}
		const_iterator(const const_iterator& o) : it(o.it) {}
		const_iterator(const iterator& o) : it(o) {}
		const T& operator*() const {
			return *it;
		}
		const T* operator->() const {
			return &*it;
		}
		int operator-(const const_iterator& o) const {
			return it - o.it;
		}
		const_iterator operator+(const int& n) const {
			return const_iterator(it + n);
		}
		const_iterator operator-(const int& n) const {
			return const_iterator(it - n);
		}
		const_iterator operator+=(const int& n) {
			return (*this = *this + n);
		}
		const_iterator operator-=(const int& n) {
			return (*this = *this - n);
		}
		const_iterator operator++ () {
			++it;
			return *this;
		}
		const_iterator operator++ (int) {
			return const_iterator(it++);
		}
		const_iterator operator-- () {
			--it;
			return *this;
		}
		const_iterator operator-- (int) {
			return const_iterator(it--);
		}
		bool operator==(const const_iterator& o) const {
			return it == o.it;
		}
		bool operator!=(const const_iterator& o) const {
			return it != o.it;
		}
	};
	void clear() {
		for (block* x = Begin; x != End; x = x->nxt, M.Del(x->pre))
			for (int i = 0; i < x->sz; i++)	(x->v + x->l + i)->~T();

		Begin = Last = End, sz = 0;
	}
	deque& operator=(const deque& o) {
		if (this == &o) return *this;

		clear();

		for (block* x = o.Begin; x != o.End; x = x->nxt)
			for (int i = 0; i < x->sz; i++)	push_back(x->v[x->l + i]);

		return *this;
	}
	deque() {
		End = M.New(1), Begin = Last = End, sz = 0;
	}
	deque(const deque& o) {
		End = M.New(1), Begin = Last = End, sz = 0, *this = o;
	}
	~deque() {
		clear();
		delete End;
	}

private:
	void suck(block* x) {
		while(x->nxt->sz) x->addr(x->nxt->v[x->nxt->l]), x->nxt->dell();

		M.Del(x->nxt);
	}
	void burst(block* x) {
		M.New(x, x->nxt);

		for (int i = 0; i < x->sz; i++) x->nxt->addl(x->v[x->l + x->sz - 1]), x->delr();
	}

	T& get(const unsigned p) {
		unsigned i = 0;

		for (block* x = Begin; x != End; i += x->sz, x = x->nxt)
			if (i + x->sz > p) return x->v[x->l + p - i];

		throw index_out_of_bound();
	}
	const T& get(const unsigned p) const {
		unsigned i = 0;

		for (block* x = Begin; x != End; i += x->sz, x = x->nxt)
			if (i + x->sz > p) return x->v[x->l + p - i];

		throw index_out_of_bound();
	}

public:
	T& at(const int& p) {
		return get(p);
	}
	const T& at(const int& p) const {
		return get(p);
	}
	T& operator[](const int& p) {
		return get(p);
	}
	const T& operator[](const int& p) const {
		return get(p);
	}
	const T& front() const {
		if (sz == 0) throw container_is_empty();

		return Begin->v[Begin->l];
	}
	const T& back() const {
		if (sz == 0) throw container_is_empty();

		return Last->v[Last->r - 1];
	}
	iterator begin() const {
		return iterator(this, 0, Begin, 0);
	}
	const_iterator cbegin() const {
		return const_iterator(begin());
	}
	iterator end() const {
		return iterator(this, sz, End, 0);
	}
	const_iterator cend() const {
		return const_iterator(end());
	}
	iterator last() const {
		return iterator(this, sz - 1, Last, Last->sz - 1);
	}
	bool empty() const {
		return sz == 0;
	}
	int size() const {
		return sz;
	}

	iterator insert(iterator p, const T& value) {
		if (this != p.id) throw invalid_iterator();

		if (p.rk == 0) return push_front(value), begin();
		else if (p.rk < 0 || p.rk > sz) throw index_out_of_bound();
		else if (p.rk == sz) return push_back(value), last();

		if (p.b->sz < blockSize) return sz++, p.b->insert(p.p, value), p;

		return burst(p.b), insert(iterator(p.id, p.rk), value);
	}
	iterator erase(iterator p) {
		if (this != p.id) throw invalid_iterator();

		if (p.rk < 0 || p.rk >= sz) throw index_out_of_bound();
		else if (p.rk == 0) return pop_front(), begin();
		else if (p.rk == sz - 1) return pop_back(), end();

		sz--, p.b->erase(p.p);

		if (p.b->sz == 0) M.Del(p.b);
		else if (p.b->sz + p.b->nxt->sz <= blockSize * (wasteRatio - 1) / wasteRatio) suck(p.b);

		return iterator(p.id, p.rk);
	}
	void push_back(const T& value) {
		if (Last->rsiz() == 0 && Last->lsiz() < maxW) M.New(Last, End);

		sz++, Last->addr(value);
	}
	void push_front(const T& value) {
		if (Begin->lsiz() == 0 && Begin->rsiz() < maxW) M.New(End, Begin);

		sz++, Begin->addl(value);
	}
	void pop_back() {
		if (sz == 0) throw container_is_empty();

		sz--, Last->delr();

		if (Last->sz == 0) M.Del(Last);
	}
	void pop_front() {
		if (sz == 0) throw container_is_empty();

		sz--, Begin->dell();

		if (Begin->sz == 0) M.Del(Begin);
	}
#undef Begin
#undef Last
};
}
#endif //Am I shortest?

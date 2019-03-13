#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP
#pragma GCC optimize("-Ofast") 
#include "exceptions.hpp"
#include <cstddef>
#include <cstring>

namespace sjtu { 

template<class T, int bSiz = 1333, int wRatio = 3>
class deque {
private:
	static const int maxW = bSiz / wRatio;
	struct Block;
	class Allocator; // pool
public :
	class iterator;
	class const_iterator;

private:
	Allocator Mem;
	Block *End; int sz;
	#define Begin End->nxt
	#define Last End->pre

	struct Block {
		Block *pre, *nxt;
		T* v;
		int sz, l, r;
		char mem[sizeof(T) * bSiz];

		void reset() {pre = nxt = NULL; l = sz = r = 0; v = (T*)mem;}
		Block () {reset();}
		~Block () {}
		int lsiz() {return l;}
		int rsiz() {return bSiz - r;}
		void dell() {(v + l++)->~T(); sz--;}
		void delr() {(v + --r)->~T(); sz--;}

		void addl(const T& x) {
			if (!sz) l = r = bSiz; 
			if (!lsiz()) {
				int b = (bSiz - sz + 1) / 2; // O(log wRatio) in total
				for (int i = r - 1; i >= l; i--) memcpy(v + i + b, v + i, sizeof(T));
				l += b, r += b;
			}
			--l, sz++;
			new(v + l) T(x);
		}
		void addr(const T& x) {
			if (!sz) l = r = 0;
			if (!rsiz()) {
				int b = (bSiz - sz + 1) / 2;
				for (int i = l; i < r; i++) memcpy(v + i - b, v + i, sizeof(T));
				l -= b, r -= b;
			}
			new(v + r) T(x);
			r++, sz++;
		}
		void insert(int p, const T& x) {
			if ((p <= sz - p && lsiz()) || !rsiz()) {
				--l;
				for (int i = 0; i < p; i++) memcpy(v + l + i, v + l + i + 1, sizeof(T));
			} else {
				r++;
				for (int i = sz - 1; i >= p; i--) memcpy(v + l + i + 1, v + l + i, sizeof(T));
			}
			new(v + l + p) T(x);
			sz++;
		}
		void erase(int p) {
			(v + l + p)->~T();
			if (p <= sz - p) {
				l++;
				for (int i = p - 1; i >= 0; i--) memcpy(v + l + i, v + l + i - 1, sizeof(T));
			} else {
				r--;
				for (int i = p; i < sz - 1; i++) memcpy(v + l + i, v + l + i + 1, sizeof(T));
			}
			sz--;
		}
	};

	class Allocator {
		Block *pool;
		public:
		Allocator () {pool = NULL;}
		~Allocator () {
			while (pool) {
				Block *nxt = pool->nxt;
				delete pool;
				pool = nxt;
			}
		}
		Block* New(bool isEnd = 0) {  
			Block* ret;
			if (pool) ret = pool, pool = pool->nxt;
			else ret = new Block;
			ret->reset();
			if (isEnd) ret->sz = ret->r = bSiz, ret->v = 0;
			return ret;
		}
		Block* New(Block *pre, Block *nxt) { // add block between pre and nxt
			Block* ret;
			if (pool) ret = pool, pool = pool->nxt, ret->reset();
			else ret = new Block;
			ret->pre = pre, ret->nxt = nxt;
			pre->nxt = ret, nxt->pre = ret;
			return ret;
		}
		void Del(Block *x) {
			x->nxt->pre = x->pre;
			x->pre->nxt = x->nxt;
			x->nxt = pool, pool = x;
		}
	};

public:
	class iterator {
	public:
		const deque* id;
		int rk; Block* b; int pos;
		iterator () {}
		iterator (const deque* i, int r) noexcept : id(i), rk(r) {
			int c = 0; b = id->End, pos = 0;
			for (Block *x = i->Begin; x != i->End; c += x->sz, x = x->nxt) 
				if (c + x->sz > rk) {
					b = x, pos = rk - c; 
					return;
				}
		}
		iterator (const deque *i, int r, Block *u, int v) : id(i), rk(r), b(u), pos(v) {}
		iterator operator+(const int &n) const {return iterator(id, rk + n);}
		iterator operator-(const int &n) const {return iterator(id, rk - n);}
		int operator-(const iterator &rhs) const {if (id != rhs.id) throw invalid_iterator(); return rk - rhs.rk;}
		iterator operator+=(const int &n) {return (*this = *this + n);}
		iterator operator-=(const int &n) {return (*this = *this - n);}
		iterator& operator++() {
			if (pos + 1 < b->sz) return *this = iterator(id, rk + 1, b, pos + 1); 
			else return *this = iterator(id, rk + 1, b->nxt, 0);
		}
		iterator operator++(int) {iterator ret = *this; return ++*this, ret;}
		iterator& operator--() {
			if (pos) return *this = iterator(id, rk - 1, b, pos - 1); 
			else return *this = iterator(id, rk - 1, b->pre, b->pre->sz - 1);
		}
		iterator operator--(int) {iterator ret = *this; return --*this, ret;}
		T& operator*() const {if (!b->v) throw index_out_of_bound(); return b->v[b->l + pos];}
		T* operator->() const {if (!b->v) throw index_out_of_bound(); return b->v + b->l + pos;}
		bool operator==(const iterator &rhs) const {return id == rhs.id && rk == rhs.rk;}
		bool operator!=(const iterator &rhs) const {return !(id == rhs.id && rk == rhs.rk);}
	};

	class const_iterator {
	private:
		iterator it;
	public:
		const_iterator() {}
		const_iterator(const const_iterator &o) : it(o.it) {}
		const_iterator(const iterator &o) : it(o) {}
		const T& operator*() const {return *it;}
		const T* operator->() const {return &*it;}
		int operator-(const const_iterator &rhs) const {return it - rhs.it;}
		const_iterator operator+(const int &n) const {return const_iterator(it + n);}
		const_iterator operator-(const int &n) const {return const_iterator(it - n);}
		const_iterator operator+=(const int &n) {return (*this = *this + n);}
		const_iterator operator-=(const int &n) {return (*this = *this - n);}
		const_iterator operator ++ () {++it; return *this;}
		const_iterator operator ++ (int) {return const_iterator(it++);}
		const_iterator operator -- () {--it; return *this;}
		const_iterator operator -- (int) {return const_iterator(it--);}
		bool operator==(const const_iterator &rhs) const {return it == rhs.it;}
		bool operator!=(const const_iterator &rhs) const {return it != rhs.it;}
	};
	deque() {
		End = Mem.New(1);
		Begin = Last = End, sz = 0;
	}
	deque(const deque &o) {
		End = Mem.New(1);
		Begin = Last = End, sz = 0;
		for (Block* x = o.Begin; x != o.End; x = x->nxt)
			for (int i = 0; i < x->sz; i++) 
				push_back(x->v[x->l + i]);
	}
	void clear() {
		for (Block *x = Begin; x != End; x = x->nxt, Mem.Del(x->pre)) 
			for (int i = 0; i < x->sz; i++)
				(x->v + x->l + i)->~T();
		Begin = End, Last = End, sz = 0;
	}
	~deque() {
		clear();
		delete End;
	}
	deque &operator=(const deque &o) {
		if (this == &o) return *this;
		clear();
		for (Block* x = o.Begin; x != o.End; x = x->nxt)
			for (int i = 0; i < x->sz; i++) 
				push_back(x->v[x->l + i]);
		return *this;
	}

private:
	void suck(Block *x) {
		while(x->nxt->sz) x->addr(x->nxt->v[x->nxt->l]), x->nxt->dell();
		Mem.Del(x->nxt);
	}
	void burst(Block *x) {
		Mem.New(x, x->nxt);
		for (int i = 0; i < x->sz; i++) x->nxt->addl(x->v[x->l + x->sz - 1]), x->delr();  
	}

	T& get(const size_t pos) {
		int i = 0;
		for (Block *x = Begin; x != End; x = x->nxt) {
			if (i + x->sz > pos) return x->v[x->l + pos - i];
			i += x->sz;
		}
		throw index_out_of_bound();
	}
	const T& get(const size_t pos) const {
		int i = 0;
		for (Block *x = Begin; x != End; x = x->nxt) {
			if (i + x->sz > pos) return x->v[x->l + pos - i];
			i += x->sz;
		}
		throw index_out_of_bound();
	}

public:
	T & at(const size_t &pos) {return get(pos);}
	const T & at(const size_t &pos) const {return get(pos);}
	T & operator[](const size_t &pos) {return get(pos);}
	const T & operator[](const size_t &pos) const {return get(pos);}
	const T & front() const {if (sz == 0) throw container_is_empty(); return Begin->v[Begin->l];}
	const T & back() const {if (sz == 0) throw container_is_empty(); return Last->v[Last->r - 1];}
	iterator begin() const {return iterator(this, 0, Begin, 0);}
	const_iterator cbegin() const {return const_iterator(begin());}
	iterator end() const {return iterator(this, sz, End, 0);}
	const_iterator cend() const {return const_iterator(end());}
	iterator last() const {return iterator(this, sz - 1, Last, Last->sz - 1);}
	bool empty() const {return sz == 0;}
	size_t size() const {return sz;}

	iterator insert(iterator pos, const T &value) {
		if (this != pos.id) throw invalid_iterator();
		if (pos.rk == 0) return push_front(value), begin();
		else if (pos.rk < 0 || pos.rk > sz) throw index_out_of_bound();
		else if (pos.rk == sz) return push_back(value), last();
		if (pos.b->sz < bSiz) return sz++, pos.b->insert(pos.pos, value), pos;
		return burst(pos.b), insert(iterator(pos.id, pos.rk), value);
	}
	iterator erase(iterator pos) {
		if (this != pos.id) throw invalid_iterator();
		if (pos.rk < 0 || pos.rk >= sz) throw index_out_of_bound();
		else if (pos.rk == 0) 	   return pop_front(), begin();
		else if (pos.rk == sz - 1) return pop_back(), end();
		sz--, pos.b->erase(pos.pos);
		if (pos.b->sz == 0) Mem.Del(pos.b);
		else if (pos.b->sz + pos.b->nxt->sz <= bSiz * (wRatio - 1) / wRatio) suck(pos.b);
		return iterator(pos.id, pos.rk);
	}
	void push_back(const T &value) {
		if (Last->rsiz() == 0 && Last->lsiz() < maxW) Mem.New(Last, End);
		sz++, Last->addr(value);
	}
	void push_front(const T &value) {
		if (Begin->lsiz() == 0 && Begin->rsiz() < maxW) Mem.New(End, Begin);
		sz++, Begin->addl(value);
	}
	void pop_back() {
		if (sz == 0) throw container_is_empty();
		sz--, Last->delr();
		if (Last->sz == 0) Mem.Del(Last);
	}
	void pop_front() {
		if (sz == 0) throw container_is_empty();
		sz--, Begin->dell();
		if (Begin->sz == 0) Mem.Del(Begin);
	}

	#undef Begin
	#undef Last
};
}
#endif // Am I shortest?

/**
 * implement a container like std::map
 */

#pragma GCC optimize("O3")
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
	class Key,
	class T,
	class Compare = std::less<Key>
> class map {

private:
	class allocator;
	struct Node;

public:
	class const_iterator;
	class iterator;
	typedef sjtu::pair<const Key, T> value_type;

private:

	allocator M;
	Node end_node, nil, *null, *rt, *End;
	int sz;

	struct Node {
		Node *c[2], *fa; // as a splay node
		Node *pre, *nxt; // as a list node
		char mem[sizeof(value_type)];
		bool aux;
		Node () {aux = true;}
		Node (const value_type &z) {
			aux = false;
			new(mem) value_type(z);
		}
		~Node () {
			if (!aux) v().~value_type();
		}
		inline value_type & v() {
			return *(value_type*)(mem);
		}
		inline const value_type & v() const {
			return *(value_type*)(mem);
		}
		bool operator < (const Node &b) const {
			return aux == b.aux ? Compare()(v().first, b.v().first) : aux < b.aux;
		}
		friend bool operator < (const Key &a, const Node &n) {
			return n.aux ? 1 : Compare()(a, n.v().first);
		}
		
		friend bool operator < (const Node &n, const Key &a) {
			return n.aux ? 0 : Compare()(n.v().first, a);
		}
		bool d() const{
			return this == fa->c[1];
		}
		void setc(Node *ch, const int& p) {
			c[p] = ch; ch->fa = this;
		}
	};

	class allocator {
		// It is only a homework, so I don't want to implement a robust memory pool
		Node *pool;
	public:
		allocator () {pool = NULL;}
		~allocator () {
			while (pool) {
				Node *x = pool;
				pool = pool->nxt;
				x->aux = 1;
				delete x;
			}
		}
		Node *New(const value_type &v, Node *null = NULL) {
			Node *ret = pool;
			if (pool) {
				pool = pool->nxt;
				new(ret->mem) value_type(v);
			} else ret = new Node(v);
			ret->fa = ret->c[0] = ret->c[1] = null;
			ret->pre = ret->nxt = ret;
			return ret;
		}
		void Del(Node *x) {
			x->~Node();
			x->nxt = pool, pool = x;
		}
	};

	void add(Node* const x, Node* const fa, const int d) {
		Node* const pre = d ? fa : fa->pre;
		Node* const nxt = d ? fa->nxt : fa;
		fa->setc(x, d);
		x->pre = pre, x->nxt = nxt;
		pre->nxt = x; nxt->pre = x;
		sz++;
	}

	void del(Node *x) { // x as a leaf
		x->fa->setc(null, x->d());
		x->nxt->pre = x->pre, x->pre->nxt = x->nxt;
		M.Del(x);
		sz--;
	}
	void del_(Node *x) { // x as a list node when clear
		x->nxt->pre = x->pre, x->pre->nxt = x->nxt;
		M.Del(x);
		sz--;
	}

	void rot(Node *x) {
		Node *fa = x->fa;
		int d = x->d();
		fa->fa->setc(x, fa->d());
		fa->setc(x->c[!d], d);
		x->setc(fa, !d);
		if (fa == rt) rt = x;
	}
	
	Node* splay(Node *x, Node *fa = 0) {
		if (!fa) fa = null;
		while (x->fa != fa) {
			if (x->fa->fa == fa) rot(x);
			else x->d() == x->fa->d() ? (rot(x->fa), rot(x)) : (rot(x), rot(x));
		}
		return x;
	}

#define binary_search() while (x != null) {\
			if (k < *x) y = x, x = x->c[0];\
			else if (*x < k) y = x, x = x->c[1];\
			else return splay(x);\
		}
	
	inline Node* get(const Key &k) { // return pointer or End
		Node *x = rt, *y = null;
		binary_search();
		if (y != null) splay(y);
		return End;
	}

	inline Node* tget(const Key &k) { // [throw] return pointer or throw
		Node *x = rt, *y = null;
		binary_search();
		if (y != null) splay(y);
		throw index_out_of_bound();
	}

	inline Node* nget(const Key &k) { // [new] return pointer and new if needed
		Node *x = rt, *y = null;
		binary_search();
		bool d = *y < k;
		add(x = M.New(value_type(k, T()), null), y, d);
		return splay(x);
	}


public:
	class iterator {
		friend map;
	private:
		Node *x, *e;
	public:
		iterator() {}
		iterator(Node *y, Node *z) : x(y), e(z) {}
		iterator(const iterator &other) : x(other.x), e(other.e) {}
		iterator & operator++() {if (x == e) throw invalid_iterator(); x = x->nxt; return *this;}
		iterator operator++(int) {
			iterator ret = *this;
			++(*this);
			return ret;
		}
		iterator & operator--() {if (x->pre == e) throw invalid_iterator(); x = x->pre; return *this;}
		iterator operator--(int) {
			iterator ret = *this;
			--(*this);
			return ret;
		}
		value_type & operator*() const {if (x == e) throw invalid_iterator(); return x->v();}
		value_type* operator->() const noexcept {return &(x->v());}
		bool operator==(const iterator &rhs) const {return x == rhs.x;}
		bool operator!=(const iterator &rhs) const {return x != rhs.x;}
	};

	class const_iterator {
		private:
			iterator x;
		public:
			const_iterator() {}
			const_iterator(const iterator &other) : x(other) {}
			const_iterator(const const_iterator &other) : x(other.x) {}
			const_iterator & operator ++ ()  {++x; return *this;}
			const_iterator operator ++ (int) {const_iterator ret(*this); ++x; return ret;}
			const_iterator & operator -- ()  {--x; return *this;}
			const_iterator operator -- (int) {const_iterator ret(*this); --x; return ret;}
			const value_type & operator*() const {return *x;}
			const value_type* operator->() const noexcept {return &(*x);}
			bool operator==(const const_iterator &rhs) const {return x == rhs.x;}
			bool operator!=(const const_iterator &rhs) const {return x != rhs.x;}
			friend bool operator==(const const_iterator &lhs, const iterator &rhs) {return lhs.x == rhs;}
			friend bool operator==(const iterator &lhs, const const_iterator &rhs) {return lhs == rhs.x;}
			friend bool operator!=(const const_iterator &lhs, const iterator &rhs) {return lhs.x != rhs;}
			friend bool operator!=(const iterator &lhs, const const_iterator &rhs) {return lhs != rhs.x;}
	};

	inline iterator iter(Node* x) const {
		return iterator(x, End);
	}
	inline void insert_all(const map &other) {
		for (auto v : other) insert(v);
	}
	void clear() {
		while (End->nxt != End) del_(End->nxt);
		sz = 0;
		rt = End;
		rt->pre = rt->nxt = rt;
		rt->fa = rt->c[0] = rt->c[1] = null;
	}
	inline void shared_construct() {
		sz = 0;

		null = &nil;
		null->nxt = null->pre = null;
		null->fa = null->c[0] = null->c[1] = null;
		End = &end_node;

		End->nxt = End->pre = End;
		End->fa = End->c[0] = End->c[1] = null;
		rt = End;
	}
	map() {
		shared_construct();
	}
	map(const map &other) {
		shared_construct();
		insert_all(other);
	}
	map & operator=(const map &other) {
		if (this != &other) { 
			clear();
			insert_all(other);
		}
		return *this;
	}
	~map() {
		clear();
	}
	T & at(const Key &key) {return tget(key)->v().second;}
	const T & at(const Key &key) const {return const_cast<map*>(this)->tget(key)->v().second;}
	T & operator[](const Key &key) {return nget(key)->v().second;}
	const T & operator[](const Key &key) const {return const_cast<map*>(this)->tget(key)->v().second;}
	iterator begin() const {return iter(End->nxt);}
	const_iterator cbegin() const {return const_iterator(begin());}
	iterator end() const {return iter(End);}
	const_iterator cend() const {return const_iterator(end());}
	bool empty() const {return sz == 0;}
	int size() const {return sz;}
	pair<iterator, bool> insert(const value_type &value) {
		Node *x = rt, *y = null;
		while (x != null) {
			if (value.first < *x) y = x, x = x->c[0];
			else if (*x < value.first) y = x, x = x->c[1];
			else return {iter(splay(x)), 0};
		}

		x = M.New(value, null);
		add(x, y, *y < value.first);
		return {iter(splay(x)), 1};
	}
	void erase(iterator pos) {
		if (End != pos.e || End == pos.x) throw invalid_iterator();
		Node *x = pos.x;
		if (x->pre == End) {
			splay(x->nxt);
		}
		else {
			splay(x->pre);
			splay(x->nxt, x->pre);
		}
		del(x);
	}
	size_t count(const Key &key) const {return const_cast<map*>(this)->get(key) != End;}
	iterator find(const Key &key) {return iter(get(key));}
	const_iterator find(const Key &key) const {
		return const_cast<map*>(this)->find(key); // surely const function!
	}
};
}

#endif

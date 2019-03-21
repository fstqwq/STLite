/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
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
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::map as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;

private:
	class allocator;
	struct Node;

public:
	class const_iterator;
	class iterator;

private:

	allocator M;
	Node end_node, nil, *null, *rt, *End;
	int sz;

	class auto_set_ptr {
		auto_set_ptr () {
			null.aux = 1;
			end_node.aux = 1;
			null = &nil;
			End = &end_node;
			rt = End;
			sz = 0;
		}
	};
	auto_set_ptr _enabled;

	struct Node {
		Node *c[2], *fa; // as a splay node
		Node *pre, *nxt; // as a list node
		value_type v; bool aux;
		bool operator < (const Node &b) const {
			return aux == b.aux ? Compare()(v.first, b.v.first) : aux < b.aux;
		}
		friend bool operator < (const Key &a, const Node &n) {
			return Compare()(a, n.v.first);
		}
		bool d() const{
			return this == fa->c[1];
		}
		void setc(const Node *ch, const int& p) {
			c[p] = ch; ch->fa = this;
		}
	};

	class allocator {
		Node *pool;
		allocator () {pool = NULL;}
		~allocator () {
			while (pool) {
				Node *x = pool;
				pool = pool->nxt;
				delete x;
			}
		}
		Node *New() {
			Node *ret = pool;
			if (pool) pool = pool->nxt;
			else ret = new Node;
			return ret;
		}
		void Del(Node *x) {
			x->nxt = pool, pool = x;
		}
	};

	void add(Node* const x, Node* const fa, const int d, const Node *pre, const Node *nxt) {
		fa->setc(x, d);
		x->pre = pre, x->nxt = nxt;
		pre->nxt = pre; nxt->pre = nxt;
	}

	void del(Node *x) {
		x->pre->nxt = x->nxt, x->nxt->pre = x->pre;
		x->fa->setc(null);
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

	
	Node* get(const Key &k) { // return pointer or End
		Node *x = rt;
		while (x != null) {
			if (k < *x) x = x->c[0];
			else if (*x < k) x = x->c[1];
			else return x;
		}
		return End;
	}

	Node* tget(const Key &k) { // return pointer or throw
		Node *x = rt;
		while (x != null) {
			if (k < *x) x = x->c[0];
			else if (*x < k) x = x->c[1];
			else return x;
		}
		throw index_out_of_bound();
	}

	Node* nget(const Key &k) { // return pointer and new if needed
		Node *x = rt, *y = rt;
		while (x != null) {
			if (k < *x) y = x, x = x->c[0];
			else if (*x < k) y = x, x = x->c[1];
			else return splay(x);
		}
		x = M.New();
		if (k < *y) y->setc(x, 0);
		else y->setc(x, 1);
		return splay(x);
	}


public:
	 /* if there is anything wrong throw invalid_iterator.*/
	class iterator {
	friend map;
	private:
		Node *x, *e;
	public:
		iterator() {}
		iterator(const Node *y, const Node *z) : x(y), e(z) {}
		iterator(const iterator &other) : x(other.x), e(other.e) {}
		iterator & operator++() {if (x == e) throw invalid_iterator(); return x = x->nxt;}
		iterator operator++(int) {
			iterator ret = *this;
			++(*this);
			return ret;
		}
		iterator & operator--() {if (x->pre == e) throw invalid_iterator(); return x = x->pre;}
		iterator operator--(int) {
			iterator ret = *this;
			--(*this);
			return ret;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {if (x == e) throw invalid_iterator(); return x->v;}
		bool operator==(const iterator &rhs) const {return x == rhs.x;}
		bool operator==(const const_iterator &rhs) const {return x == rhs.x.x;}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {return x != rhs.x;}
		bool operator!=(const const_iterator &rhs) const {return x != rhs.x.x;}

		/**
		 * for the support of it->first. 
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {return &(*x);}
	};
	class const_iterator {
		private:
			iterator x;
		public:
			const_iterator() {}
			const_iterator(const iterator &other) : x(other) {}
			const_iterator(const const_iterator &other) : x(other.x) {}
			const_iterator & operator ++ ()  {++x; return *this;}
			const_iterator operator ++ (int) {const_iterator ret(*this); ++x; return *this;}
			const_iterator & operator -- ()  {--x; return *this;}
			const_iterator operator -- (int) {const_iterator ret(*this); --x; return *this;}
			const value_type & operator*() const {return *x;}
			const value_type* operator->() const noexcept {return &(*x);}
			bool operator==(const const_iterator &rhs) const {return x == rhs.x.x;}
			bool operator!=(const const_iterator &rhs) const {return x != rhs.x.x;}
	};
	/**
	 * TODO two constructors
	 */
	iterator iter(const Node* &x) {
		return iterator(x, End);
	}
	void insert_all(const map &other) {
		for (auto it : other) insert(it->v);
	}
	void clear() {
		for (auto it : *this) {
			if (it->pre != End) M.Del(it->pre);
		}
		rt = End;
		sz = 0;
	}
	map() {}
	map(const map &other) {
		insert_all(other);
	}
	map & operator=(const map &other) {
		clear();
		insert_all(other);
	}
	/**
	 * TODO Destructors
	 */
	~map() {
		clear();
	}
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {return tget(key);}
	const T & at(const Key &key) const {return tget(key);}
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {return nget(key);}
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {return tget(key);}
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {return iter(End->nxt);}
	const_iterator cbegin() const {return const_iterator(begin());}
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {return iter(End);}
	const_iterator cend() const {return const_iterator(end());}
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {return sz == 0;}
	/**
	 * returns the number of elements.
	 */
	int size() const {return sz;}
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		Node *x = rt, *y = rt;
		while (x != null) {
			if (value < *x) y = x, x = x->c[0];
			else if (*x < value) y = x, x = x->c[1];
			else return make_pair(iter(splay(x)), 0);
		}
		x = M.New();
		if (value < *y) y->setc(x, 0);
		else y->setc(x, 1);
		return make_pair(iter(splay(x)), 1);
	}
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if (End != pos.e || End == pos.x) throw invalid_iterator();
		Node *x = splay(pos.x);
		splay(x->nxt);
		splay(x->pre);
		M.Del(x);
	}
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 * The default method of check the equivalence is !(a < b || b > a)
	 */
	size_t count(const Key &key) const {return get(key) != End;}
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {return iter(get(key));}
	const_iterator find(const Key &key) const {return const_iterator(find(key));}
};

}

#endif

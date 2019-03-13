#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {
template <class T> void swap(T &a, T &b) {
	if (&a == &b) return;
	T x = a; a = b; b = x;
}


/**
 * a container like std::priority_queue which is a heap internal.
 * it should be based on the vector written by yourself.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct Node {
		Node *l, *r; T v;
		Node (const T &a) : v(a){
			l = NULL; r = NULL;
		}
	};

	Node *rt; size_t sz;

	Node *newnode(const T &a) {
		Node *x = new Node(a);
		return x;
	}

	void del(Node *x) {
		if (x) {
			del(x->l);
			del(x->r);
		}
		delete x;
	}
	
	Node *cp(Node *y) {
		if (y) {
			Node *x = new Node(y->v);
			x->l = cp(y->l);
			x->r = cp(y->r);
			return x;
		}
		else return NULL;
	}

	Node *mer(Node *x, Node *y) {
		if (!x) return y;
		if (!y) return x;
		if (Compare()(x->v, y->v)) swap(x, y);
		swap(x->l, x->r);
		x->l = mer(x->l, y);
		return x;
	}

	void free_clr() {
		del(rt);
		rt = NULL;
		sz = 0;
	}
	
	void protect_clr() {
		rt = NULL;
		sz = 0;
	}

public:
	/**
	 * TODO constructors
	 */
	priority_queue() {rt = NULL; sz = 0;}
	priority_queue(const priority_queue &other) {
		rt = cp(other.rt);		
		sz = other.sz;
	}
	/**
	 * TODO deconstructor
	 */
	~priority_queue() {free_clr();}
	/**
	 * TODO Assignment operator
	 */
	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this; 
		free_clr();
		rt = cp(other.rt);		
		sz = other.sz;
		return *this;
	}
	/**
	 * get the top of the queue.
	 * @return a reference of the top element.
	 * throw container_is_empty if empty() returns true;
	 */
	const T & top() const {
		if (sz) return rt->v;
		else throw container_is_empty();
	}
	/**
	 * TODO
	 * push new element to the priority queue.
	 */
	void push(const T &e) {
		rt = mer(rt, newnode(e));
		sz++;
	}
	/**
	 * TODO
	 * delete the top element.
	 * throw container_is_empty if empty() returns true;
	 */
	void pop() {
		if (sz) rt = mer(rt->l, rt->r);
		else throw container_is_empty();
		sz--;
	}
	/**
	 * return the number of the elements.
	 */
	size_t size() const {
		return sz;
	}
	/**
	 * check if the container has at least an element.
	 * @return true if it is empty, false if it has at least an element.
	 */
	bool empty() const {
		return sz == 0;
	}
	/**
	 * return a merd priority_queue with at least O(logn) complexity.
	 */
	void merge(priority_queue &other) {
		rt = mer(rt, other.rt);
		sz += other.sz;
		other.protect_clr();
	}
};

}

#endif

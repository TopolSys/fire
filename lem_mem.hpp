#ifndef LEM_MEM_HPP
#define LEM_MEM_HPP
#include <cstddef>
#include <limits>
#include <assert.h>
#include <iostream>
#include <vector>
#include <list>
template<typename T>
class Lem_Node_t {
public:
	T data;
	Lem_Node_t * prev, * next;
	Lem_Node_t():prev(NULL),next(NULL){}
	Lem_Node_t( const T& val ):prev(NULL),next(NULL),data(val){}
};
template<typename T>
class Lem_Page_t: public std::vector<Lem_Node_t<T> > {
};

template<typename T>
class Lem_Alloc {
public:
	// type definitions
	typedef T        value_type;
	typedef T*       pointer;
	typedef const T* const_pointer;
	typedef T&       reference;
	typedef const T& const_reference;
	typedef std::size_t    size_type;
	typedef std::ptrdiff_t difference_type;

	typedef Lem_Node_t<T>   Node_t;

	// rebind allocator to type U
	template <class U>
	struct rebind {
		typedef Lem_Alloc<U> other;
	};

	// return address of values
	pointer address (reference value) const {
		return &value;
	}
	const_pointer address (const_reference value) const {
		return &value;
	}

	/* constructors and destructor
	* - nothing to do because the allocator has no state
	*/
	Lem_Alloc() throw() {
		_init();
	}
	Lem_Alloc(const Lem_Alloc&) throw() {
		_init();
	}
	template <class U>
	Lem_Alloc (const Lem_Alloc<U>&) throw() {
		_init();
	}
	~Lem_Alloc() throw() {
	}

	// return maximum number of elements that can be allocated
	size_type max_size () const throw() {
		return std::numeric_limits<std::size_t>::max() / sizeof(T);
	}

	// allocate but don't initialize num elements of type T
	pointer allocate (size_type num, const void* = 0) {
		assert(num==1);
		// print message and allocate memory with global new
		//Node_t * pN = (Node_t *) malloc(num*sizeof(Node_t));//::operator new(num*sizeof(Lem_Node_t<T>));
		Node_t * pN = getNode();
		pointer ret = (pointer) pN;
		return ret;
	}

	// initialize elements of allocated storage p with value value
	void construct (pointer p, const T& value) {
		// initialize memory with placement new
		Node_t * pN = (Node_t*) p;
		new((void*)pN)Node_t(value);
	}

	// destroy elements of initialized storage p
	void destroy (pointer p) {
		// destroy objects by calling their destructor
		Node_t * pN = (Node_t*) p;
		pN->~Lem_Node_t<T>();
	}

	// deallocate storage p of deleted elements
	void deallocate (pointer p, size_type num) {
		assert(num==1);
		// print message and deallocate memory with global delete
		Node_t * pN = (Node_t*) p;
		delNode(pN);
	}
private:
	void _init(){
		pList = NULL;
		page = PageList.end();
	}
	Node_t * pList;
	typedef std::list<Lem_Page_t<Node_t> > PageList_t;
	PageList_t PageList;
	typename PageList_t::iterator page;
	int NodeTop;
	Node_t * getNode(){
		// search node from list
		if( pList ){
			Node_t * ret = pList;
			pList = pList? pList->next: NULL;
			return ret;
		}
		// search node from page
		if( PageList.end() == page? true: (NodeTop >= page->size()) ){
			PageList.push_back(Lem_Page_t<Node_t>());
			PageList.back().resize(6144);
			NodeTop = 0;
			page = --PageList.end();
			return (Node_t*) &(*page)[NodeTop++];
		}
		return (Node_t*) &(*page)[NodeTop++];
	}
	void     delNode( Node_t * pNode ){
		// concatenate node to list
		pNode->next = pList;
		pList = pNode;
	}
};


#endif
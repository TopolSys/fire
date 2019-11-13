#ifndef LEM_MAC_HPP
#define LEM_MAC_HPP


#define Lem_Iterate(CONTAINER,ItrName)\
	for( __typeof__(CONTAINER.begin()) ItrName = CONTAINER.begin();\
		ItrName!=CONTAINER.end(); ItrName++ )

#define Lem_IterateCount(CONTAINER,ItrName,nMove)\
	for( __typeof__(CONTAINER.begin()) ItrName = CONTAINER.begin();\
		ItrName!=CONTAINER.end(); ItrName++, nMove++ )


template <typename T>
inline T Lem_ItrPrev( T itr ){ T ret = itr; return --ret; }

#endif
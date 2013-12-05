//
// simpleList.h
//

#ifndef _SIMPLE_LIST_OBJECTS_H_
#define _SIMPLE_LIST_OBJECTS_H_

#include <cstddef>  // include for pre c++11 defines

struct simpleListObject {
	simpleListObject *next;
	simpleListObject *previous;
	
	simpleListObject();
	~simpleListObject();
}; // end of struct simpleListObject

struct simpleList {
	simpleListObject *first;
	simpleListObject *last;
	unsigned long nelements;
	
	simpleList();
	~simpleList();
	
	void add(simpleListObject *elem, simpleListObject *after=NULL);
	void insert(simpleListObject *elem, simpleListObject *before=NULL);
	void remove(simpleListObject *elem);
	simpleListObject *findByIndex(unsigned long index);
}; // end of struct simpleList

#endif

// END OF simpleList.h
 
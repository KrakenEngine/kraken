//
// simpleList.cpp
//

#include "simpleList.h"

simpleListObject::simpleListObject() { next = previous = NULL; }
simpleListObject::~simpleListObject() { }

simpleList::simpleList() {
	first = last = NULL;
	nelements = 0;
};

simpleList::~simpleList() { while((first)) remove(first); }

void simpleList::add(simpleListObject *elem, simpleListObject *after) {
	if (NULL == elem) return;
	if (NULL == first) {
		first = last = elem;
		elem->next = elem->previous = NULL;
	}
	else {
		if (NULL == after) { after = last; }
		if (last == after) {
			elem->next = NULL;
			elem->previous = last;
			last->next = elem;
			last = elem;
		}
		else {
			elem->previous = after;
			elem->next = after->next;
			after->next->previous = elem;
			after->next = elem;
		}
	}
	nelements++;
}

void simpleList::insert(simpleListObject *elem, simpleListObject *before) {
	if (NULL == elem) return;
	if (NULL == first) {
		first = last = elem;
		elem->next = elem->previous = NULL;
	}
	else {
		if (NULL == before) { before = first; }
		if (first == before) {
			elem->previous = NULL;
			elem->next = first;
			first->previous = elem;
			first = elem;
		}
		else {
			elem->previous = before->previous;
			elem->next = before;
			before->previous->next = elem;
			before->previous = elem;
		}
	}
	nelements++;
}

void simpleList::remove(simpleListObject *elem) {
	if (0 == nelements) return;
	if (NULL == elem) return;
	if (NULL != elem->previous) elem->previous->next = elem->next;
	if (NULL != elem->next) elem->next->previous = elem->previous;
	if (elem == last) last = elem->previous;
	if (elem == first) first = elem->next;
	elem->previous = elem->next = NULL;
	nelements--;
}

simpleListObject *simpleList::findByIndex(unsigned long index) {
	simpleListObject *result = NULL;
	if (index < nelements) {
		simpleListObject *current = first;
		for (unsigned long i = 0; (i < index) && (NULL != current); i++)
			current = current->next;
		result = current;
	}
	return result;
}

// END OF simpleList.cpp

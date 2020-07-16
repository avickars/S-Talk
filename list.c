#include "list.h"
#include <stdio.h>
#include <assert.h>

// Declaring a static array of list heads, and a static integer numHeads that counts the number of heads currently in use.
static List heads[LIST_MAX_NUM_HEADS];
static int numHeads = 0;

// Declaring a static array of list nodes, and a static integer numNodes that counts the number of heads currently in use.
static Node nodes[LIST_MAX_NUM_NODES];
static int numNodes = 0;

// Declaring a pointer to the first element in a singly linked list of available heads.
static Node *availableNodes;

// Declaring a pointer to the first element in a singly linked list of available nodes.
static List *availableHeads;

// Declaring an indicator that indicates whether or not the client is performing their first List_create()
static bool firstCreate = true;

// Creates two singly linked lists.  One of the available nodes, and one of the available heads.
static void Constructor() {
    // Creating initial singly linked list of available nodes
    availableNodes = &nodes[numNodes];
    Node *nodePtr = availableNodes;
    for (int i = 1; i < LIST_MAX_NUM_NODES; ++i) {
        nodePtr->next = &nodes[i];
        nodePtr = nodePtr->next;
    }
    nodePtr->next = NULL;

    // Creating initial singly linked list of available heads
    availableHeads = &heads[numHeads];
    List *headPtr = availableHeads;
    for (int j = 1; j < LIST_MAX_NUM_HEADS; ++j) {
        headPtr->next = &heads[j];
        headPtr = headPtr->next;
    }
    headPtr->next = NULL;
}

// This function takes a list head and initializes is values
static void initializeHead(List *pList) {
    assert(pList != NULL);
    pList->current = NULL;
    pList->currentOutOfBoundsBack = true; // These 2 values are set to be both true only in the case when pList has no nodes, which in this case the we say the current item is both
    // before the list head and after the list tail
    pList->currentOutOfBoundsFront = true;
    pList->head = NULL;
    pList->size = 0;
    pList->tail = NULL;
    pList->next = NULL;
}

static void initializeNode(Node *pNode, void *pItem) {
    pNode->next = NULL;
    pNode->previous = NULL;
    pNode->item = pItem;
}

static void *Get_new_node(void *pItem) {
    assert(numNodes < LIST_MAX_NUM_NODES);
    Node  * newNode = availableNodes;
    availableNodes = availableNodes->next;
    numNodes++;

    initializeNode(newNode, pItem);
    return newNode;

}

// This function removes a list head from the linked list of available heads and returns a pointer to it.
static void *get_new_head(){
    assert(numHeads < LIST_MAX_NUM_HEADS); // Checking to ensure there is an available head.  I use an assert here because if the program gets here while there are no more heads,
    // something bad has gone wrong
    List *newHead = availableHeads;
    availableHeads = availableHeads->next;  // Removing the head from the list of available heads
    numHeads++; // Incrementing the counter of the number of heads in use
    initializeHead(newHead); // Initializing the new list head by passing its pointer to the initializeHead() function

    return newHead;
}

// This function accepts a pointer to a Node and returns it the list of available nodes.
static void Return_node(Node *pNode) {
    pNode->next = availableNodes;
    availableNodes = pNode;
    pNode = NULL;
    numNodes--;
}

// This function accepts a pointer to a Head and returns it the list of available Heads.
static void Return_head(List *head) {
    initializeHead(head);
    head->next = availableHeads;
    availableHeads = head;
    head = NULL;
    numHeads--;
}

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
List* List_create() {
    if (numHeads >= LIST_MAX_NUM_HEADS) // If their are no more heads free heads available, function returns null
        return NULL;
    if (firstCreate) { // Testing if this is the first time a client has called List_create().  If yes it will call the Constructor() method for some extra setup
        Constructor();
        firstCreate = false;
    }
    List *newList = get_new_head(); // Retrieves an available head from the linked list of available heads by calling the get_new_head() function
    return newList;
}

// Returns the number of items in pList.
int List_count(List* pList) {
    assert(pList != NULL);
    return pList->size;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
    assert(pList != NULL);
    if (pList->size == 0) { //Testing if pList is empty
        pList->current = NULL;
        return NULL;
    } else {
        // Testing if the current item is set either beyond the front or back of pList, and if so we designate that it no longer is either of these cases
        if (pList->currentOutOfBoundsBack || pList->currentOutOfBoundsFront) {
            pList->currentOutOfBoundsFront = false;
            pList->currentOutOfBoundsBack = false;
        }
        pList->current = pList->head;
        return pList->current->item;
    }
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
    assert(pList != NULL);
    if (pList->size == 0) { // Testing if pList is empty
        pList->current = NULL;
        return NULL;
    } else {
        // Testing if the current item is set either beyond the front or back of pList, and if so we designate that it no longer is either of these cases
        if (pList->currentOutOfBoundsBack || pList->currentOutOfBoundsFront) {
            pList->currentOutOfBoundsFront = false;
            pList->currentOutOfBoundsBack = false;
        }
        pList->current = pList->tail;
        return pList->current->item;
    }
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList) {
    assert(pList != NULL);
    if (pList->currentOutOfBoundsFront) {
        // Testing if the current item is before the front of pList.  If so we automatically set the current item to the front of pList, and designate that the current
        // item is no longer before the front of pList
        pList->current = pList->head;
        pList->currentOutOfBoundsFront = false;
        return pList->current->item;
    } else if (pList->currentOutOfBoundsBack || pList->current->next == NULL) {
        // Testing if the current item is beyond the end of pList or if advancing the current item by one will set the current item beyond the end of pList
        // In either case the result is the same, hence the following lines of code are used for both.
        pList->currentOutOfBoundsBack = true;
        pList->current = NULL;
        return NULL;
    } else {
        // Advancing the current item by one.
        pList->current = pList->current->next;
        return pList->current->item;
    }
}

// Backs up pList's current item by one, and returns a pointer to the new current item.
// If this operation backs up the current item beyond the start of the pList, a NULL pointer
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList) {
    assert(pList != NULL);
    if (pList->currentOutOfBoundsBack) {
        // Testing if the current item is beyond the end of pList.  If so we automatically set the current item to the back of pList, and designate that the current
        // item is no longer before the front of pList
        pList->current = pList->tail;
        pList->currentOutOfBoundsBack = false;
        return pList->current->item;
    } else if (pList->currentOutOfBoundsFront || pList->current->previous == NULL ) {
        // Testing if the current item is before the front of pList or if backing the current item by one will set the current item before the front of pList.
        // In either case the result is the same, hence the following lines of code are used for both.
        pList->currentOutOfBoundsFront = true;
        pList->current = NULL;
        return NULL;
    } else {
        // Backing up the current item by one.
        pList->current = pList->current->previous;
        return pList->current->item;
    }
}

// Returns a pointer to the current item in pList.
// Returns NULL if current is before the start of the pList, or after the end of the pList.
void* List_curr(List* pList) {
    assert(pList != NULL);
    if (pList->currentOutOfBoundsBack || pList->currentOutOfBoundsFront)
        // Testing if the current item is before the start or after the end of pList.
        return NULL;
    else
        return pList->current->item;
}

// Adds the new item to pList directly after the current item, and makes item the current item.
// If the current pointer is before the start of the pList, the item is added at the start. If
// the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_add(List* pList, void* pItem) {
    assert(pList != NULL);
    if (pList->currentOutOfBoundsBack || pList->current == pList->tail) {
        // Testing if the current item is beyond the end of pList or if it is set to the tail of the list.  In either case an item is added at the end of the list.  Hence
        // List_append() is called to perform this.
        return List_append(pList, pItem);
    } else if (pList->currentOutOfBoundsFront) {
        // Testing if the current item is before the front of pList.  If so we can simply call List_prepend() to insert pItem at the beginning of the list.
        return List_prepend(pList, pItem);
    } else {
        if (numNodes >= LIST_MAX_NUM_NODES) {
            // Testing if there is an available node.  If so -1 will be returned to designate a failure.
            return -1;
        }

        // Inserting pItem after the current item.  To do this we retrieve a new node from the list of available nodes using Get_new_note(), and adjust the pointers of pList, and
        // the current node as required.
        Node *newNode = Get_new_node(pItem);
        newNode->next = pList->current->next;
        newNode->previous = pList->current;
        pList->current->next->previous = newNode;
        pList->current->next = newNode;
        pList->current = newNode;
        pList->size++;
        return 0;
    }

}

// Adds item to pList directly before the current item, and makes the new item the current one.
// If the current pointer is before the start of the pList, the item is added at the start.
// If the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem) {
    assert(pList != NULL);
    if (pList->currentOutOfBoundsBack) {
        // Testing if the current item is beyond the end of pList.  If so we can simply call List_append() to insert pItem at the end of the list.
        return List_append(pList, pItem);
    } else if (pList->currentOutOfBoundsFront || pList->current == pList->head) {
        // Testing if the current item is before the front of pList or if it is set to the head of the list.  In either case an item is added at the front of the list.  Hence
        // List_prepend() is called to perform this.
        return List_prepend(pList, pItem);
    } else {
        if (numNodes >= LIST_MAX_NUM_NODES) {
            // Testing if there is an available node.  If so -1 will be returned to designate a failure.
            return -1;
        }
        // Inserting pItem before the current item.  To do this we retrieve a new node from the list of available nodes using Get_new_note(), and adjust the pointers of pList, and
        // the current node as required.
        Node *newNode = Get_new_node(pItem);
        newNode->next = pList->current;
        newNode->previous = pList->current->previous;
        pList->current->previous->next = newNode;
        pList->current->previous = newNode;
        pList->size++;
        pList->current = newNode;
        return 0;
    }
}

// Adds item to the end of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
    assert(pList != NULL);
    if (numNodes >= LIST_MAX_NUM_NODES) {
        // Testing if there is an available node
        return -1;
    } else if (pList->size == 0) {
        // Testing if pList is empty.  If true, some extra work is required.  We perform a call to Get_new_node() to extract a new node from the list of available nodes, and
        // initialize the head of pList accordingly.
        pList->current = Get_new_node(pItem);
        pList->head = pList->current;
        pList->tail = pList->current;
        pList->size++;
        pList->currentOutOfBoundsFront = false;
        pList->currentOutOfBoundsBack = false;
    } else {
        // Adding an element to the end of pList.
        pList->current = Get_new_node(pItem);
        pList->current->previous = pList->tail;
        pList->tail->next = pList->current;
        pList->tail = pList->current;
        pList->size++;
        if (pList->currentOutOfBoundsBack || pList->currentOutOfBoundsFront) {
            // Testing if the current item was beyond or before pList, and designating it that it is no longer the case.
            pList->currentOutOfBoundsFront = false;
            pList->currentOutOfBoundsBack = false;
        }
    }
    return 0;
}


// Adds item to the front of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
    assert(pList != NULL);
    if (numNodes >= LIST_MAX_NUM_NODES) {
        // Testing if there is an available node
        return -1;
    } else if (pList->size == 0) {
        // Testing if pList is empty.  If true, some extra work is required.  We perform a call to Get_new_node() to extract a new node from the list of available nodes, and
        // initialize the head of pList accordingly.
        pList->current = Get_new_node(pItem);
        pList->head = pList->current;
        pList->tail = pList->current;
        pList->size++;
        pList->currentOutOfBoundsFront = false;
        pList->currentOutOfBoundsBack = false;
    } else {
        // Adding an element to the front of pList.
        pList->current = Get_new_node(pItem);
        pList->current->next = pList->head;
        pList->head->previous = pList->current;
        pList->head = pList->current;
        pList->size++;
        if (pList->currentOutOfBoundsBack || pList->currentOutOfBoundsFront) {
            // Testing if the current item was beyond or before pList, and designating it that it is no longer the case.
            pList->currentOutOfBoundsFront = false;
            pList->currentOutOfBoundsBack = false;
        }
    }
    return 0;
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList) {
    assert(pList != NULL);
    if (pList->currentOutOfBoundsFront || pList->currentOutOfBoundsBack) {
        // Testing if the current item is before the front of the list or beyond the end of the list.  In either case NULL is returned
        return NULL;
    } else {
        void *data = pList->current->item;
        if (pList->size == 1) {
            // Testing if the size of the pList is 1.  If so, we return the current node to the list of available nodes by calling Return_node().  Then since pList has no more
            // nodes, we can simply reinitialize it as required by just passing pList into initializeHead() that way it is ready to accept new nodes or items again.
            Return_node(pList->current);
            initializeHead(pList);
        } else if (pList->current == pList->head) {
            // Testing if the current item is the head of pList.  If so, we must change the current head of pList.  Then, we return the current node using Return_node()
            pList->head = pList->current->next;
            pList->head->previous = NULL;
            Return_node(pList->current);
            pList->current = pList->head;
            pList->size--;
        } else if(pList->current == pList->tail) {
            // Testing if the current item is the tail of pList.  If so, we must change the current tail of pList.  Then, we return the current node using Return_node()
            pList->tail = pList->current->previous;
            pList->tail->next = NULL;
            Return_node(pList->current);
            pList->current = pList->tail;
            List_next(pList);
            pList->size--;
        } else {
            // Removing and returning the current node.
            Node *temp = pList->current->next;
            pList->current->previous->next = pList->current->next;
            pList->current->next->previous = pList->current->previous;
            Return_node(pList->current);
            pList->current = temp;
            pList->size--;

        }
        return data;
    }
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1.
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2) {
    assert(pList1 != NULL && pList2 != NULL);
    if (pList1->size == 0) { // Testing if pList1 is empty, which then we can just copy pList2 to pList1, and adjust the current pointer to NULL (since the current pointer of
        // pList1 is NULL)
        pList1 = pList2;
        pList1->current = NULL;
        Return_head(pList2);
    } else if (pList2->size == 0) { // Testing if pList2 is empty, which then we dont have to do anything except return the head of pList2 to the list of available heads
        Return_head(pList2);
        return;
    } else {
        // Concating pList1, and pList2.  At the end we return pList2 to the list of available heads using Return_head()
        pList1->tail->next = pList2->head;
        pList2->head->previous = pList1->tail;
        pList1->tail = pList2->tail;
        pList1->size += pList2->size;
        Return_head(pList2);
    }
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item.
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are
// available for future operations.
void List_free(List* pList, FREE_FN pItemFreeFn) {
    assert(pList != NULL);
    // Function accepts pList, and passes the items contained in each node to the client defined function pItemFreeFn to free the item.  Then each node is returned to the
    // list of available nodes by calling Return_node().  Finally, we return the head for pList to the list of available available by calling Return_head().
    Node *tempNode = pList->head;
    Node *tempNode2;
    while (tempNode != NULL) {
        (*pItemFreeFn)(tempNode->item);
        tempNode2 = tempNode;
        tempNode = tempNode->next;
        Return_node(tempNode2);
    }
    printf("\n");

    Return_head(pList);

}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList) {
    assert(pList != NULL);
    if (pList->size == 0) {
        // Testing if the size of pList is 0.  In this case NULL is returned
        return NULL;
    } else {
        Node *tempNode = pList->tail;
        pList->current = pList->tail->previous;
        if (pList->current == NULL) {
            // Testing if the pList has size zero (i.e. the current item is NULL).  In this case, the last node
            // (tempNode) is returned to the list of available nodes by Return_node(), and since pList now has no nodes
            // we initialize is with initializeHead to prepare it to accept new nodes again.
            Return_node(tempNode);
            initializeHead(pList);
            return tempNode->item;
        }
        // Removing the last node from the list and returning its item
        pList->size--;
        pList->current->next = NULL;
        pList->tail = pList->current;
        Return_node(tempNode);
        return tempNode->item;
    }
}

// Search pList, starting at the current item, until the end is reached or a match is found.
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match,
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator.
//
// If a match is found, the current pointer is left at the matched item and the pointer to
// that item is returned. If no match is found, the current pointer is left beyond the end of
// the list and a NULL pointer is returned.
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    assert(pList != NULL);
    Node *tempNode = pList->current; // Set tempNode to the current node, to start search from the current node.
    while (tempNode != NULL) { // Continue the search until either the end of the list is reached or if the pComparisonArg is found.
        if ((*pComparator)(tempNode->item, pComparisonArg)) {
            pList->current = tempNode;
            return pList->current->item;
        }
        tempNode = tempNode->next;
    }

    // If not found, current is set to be beyond the end of the list
    pList->current = NULL;
    pList->currentOutOfBoundsBack = true;
    return NULL;
}

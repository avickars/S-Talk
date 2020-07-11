//
// Created by aidan on 2020-05-25.
//

#include "list.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define CHECK(condition) do{ \
    if (!(condition)) { \
        printf("ERROR: %s (@%d): failed condition \"%s\"\n", __func__, __LINE__, #condition); \
        exit(1);\
    }\
} while(0)

// For checking the "free" function called
static int complexTestFreeCounter = 0;
static void complexTestFreeFn(void* pItem)
{
    CHECK(pItem != NULL);
    complexTestFreeCounter++;
}

// For searching
static bool itemEquals(void* pItem, void* pArg)
{
    return (pItem == pArg);
}


static void testComplex() {
    // Testing the functionality of List_create()
    List *pListArr[LIST_MAX_NUM_HEADS + 1];
    for (int i = 0; i < LIST_MAX_NUM_HEADS + 1; ++i) {
        pListArr[i] = List_create();
    }
    CHECK(pListArr[0] != NULL);
    CHECK(List_count(pListArr[0]) == 0); // Testing that the list head is initialized properly
    CHECK(pListArr[LIST_MAX_NUM_HEADS] == NULL); // Testing that if there are no more heads available, NULL is returned

    // Inserting some elements into the list pListArr[0]
    int sameElementForAll = 5;
    for (int m = 0; m < sameElementForAll; ++m) {
        List_insert(pListArr[0], &sameElementForAll);
    }

    // Freeing the heads for further use.  Also  testing to ensure that all the nodes are also freed as well.  This is done
    // by checking the amount of times complexTestFreeFn was used by checking the value of complexTestFreeCounter
    for (int j = 0; j < LIST_MAX_NUM_HEADS; ++j) {
        List_free(pListArr[j], complexTestFreeFn);
    }
    CHECK(complexTestFreeCounter == sameElementForAll);

    // Creating a List for testing
    List *pList = List_create();
    CHECK(pList != NULL);

    int zero = 0;
    int one = 1;
    int two = 2;
    int three = 3;
    // Testing List_add() by adding the maximum number of nodes to pList, and then attempting to add 1 node too many to ensure
    // it returns -1
    for (int k = 0; k < LIST_MAX_NUM_NODES; ++k) {
        switch (k % 4) {
            case 0:
                CHECK(List_add(pList, &zero) == 0);
                break;
            case 1:
                CHECK(List_add(pList, &one) == 0);
                break;
            case 2:
                CHECK(List_add(pList, &two) == 0);
                break;
            case 3:
                CHECK(List_add(pList, &three) == 0);
                break;
        }
    }
    CHECK(List_add(pList, &three) == -1);

    // Testing the functionality of List_count()
    CHECK(List_count(pList) == LIST_MAX_NUM_NODES);

    // Testing to ensure the functionality of List_remove.  That it removes the current node from the List, returns the item, and
    // sets the current node to the next node.  If List_remove is correctly implemented it will follow the same pattern in the previous
    // List_add() statements
    CHECK(List_first(pList) == &zero);
    for (int l = 0; l < LIST_MAX_NUM_NODES - 1; ++l) {
        switch (l % 4) {
            case 0:
                CHECK(List_remove(pList) == &zero);
                break;
            case 1:
                CHECK(List_remove(pList) == &one);
                break;
            case 2:
                CHECK(List_remove(pList) == &two);
                break;
            case 3:
                CHECK(List_remove(pList) == &three);
                break;
        }
    }

    // Testing the functionality of List_count()
    CHECK(List_count(pList) == 1);

    // Testing the functionality of List_insert() and List_curr.
    List_insert(pList,&two);
    List_insert(pList,&one);
    List_insert(pList,&zero);
    CHECK(List_curr(pList) == &zero);

    // Testing the functionality of List_prev, and the order of the list.  If correctly implemented, the items in list order should be 0,1,2,3
    CHECK(List_prev(pList) == NULL);
    CHECK(List_prev(pList) == NULL);
    CHECK(List_next(pList) == &zero);
    CHECK(List_next(pList) == &one);
    CHECK(List_next(pList) == &two);
    CHECK(List_next(pList) == &three);
    CHECK(List_next(pList) == NULL);
    CHECK(List_next(pList) == NULL);
    CHECK(List_prev(pList) == &three);

    // Testing the functionality of List_first() and List_last().  If correctly implemented they will set the current item to the first and
    // last item respectively as well as return the current item.
    CHECK(List_first(pList) == &zero);
    CHECK(List_curr(pList) == &zero);
    CHECK(List_last(pList) == &three);
    CHECK(List_curr(pList) == &three);


    // Testing List_prepend() and List_first()
    int negOne = -1;
    CHECK(List_prepend(pList, &negOne) == 0);
    List_next(pList); // shifting the current item by one to get a better test of List_first()
    CHECK(List_first(pList) == &negOne);
    CHECK(List_remove(pList) == &negOne);

    // Testing List_append() and List_last()
    int numberFour = 4;
    CHECK(List_append(pList, &numberFour) == 0);
    List_prev(pList); // shifting the current item by one to get a better test of List_last()
    CHECK(List_last(pList) == &numberFour);
    CHECK(List_remove(pList) == &numberFour);


    // Testing List_search()
    List_first(pList);
    CHECK(List_search(pList, itemEquals, &two) == &two);
    CHECK(List_search(pList, itemEquals, &two) == &two);
    CHECK(List_search(pList, itemEquals, &zero) == NULL);

    // Testing List Concat
    int four = 4;
    int five = 5;
    int six = 6;
    List *pList2 = List_create();
    List_add(pList2, &four);
    List_add(pList2, &five);
    List_add(pList2, &six);
    List_concat(pList,pList2);
    CHECK(List_last(pList) == &six);

    // Testing to ensure the head for pList2 was returned to the pool of available heads
    List *pListArr2[LIST_MAX_NUM_HEADS - 1];
    for (int i = 0; i < LIST_MAX_NUM_HEADS - 1; ++i) {
        pListArr2[i] = List_create();
        CHECK(pListArr2[i] != NULL);
    }

    // Testing the functionality of List_trim().
    List_trim(pList);
    List_trim(pList);
    List_trim(pList);
    List_prev(pList);
    CHECK(List_trim(pList) == &three);
    CHECK(List_curr(pList) == &two);
    List_trim(pList);
    List_trim(pList);
    CHECK(List_trim(pList) == &zero);
    CHECK(List_trim(pList) == NULL);

}

int main() {

    testComplex();

    printf("********************************\n");
    printf("           PASSED\n");
    printf("********************************\n");






    return 0;
}


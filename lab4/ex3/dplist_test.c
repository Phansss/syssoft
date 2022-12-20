/**
 * \author Pieter Hanssens
 */
#define _GNU_SOURCE

#include "dplist.h"
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
    int id;
    char* name;
} my_element_t;

dplist_t *list_null;                     //| 
dplist_t *list_empty;                    //|
dplist_t *list_one_element_pointer;      //| 
dplist_t *list_ten_elements_pointer;     //| 
dplist_t *list_one_element_deepCopy;     //| 
dplist_t *list_ten_elements_deepCopy;    //| 
                                         //|
my_element_t* el_one = NULL;                    //|          
my_element_t* el0 = NULL;                       //| Global pointers to lists and elements in this test suite.
my_element_t* el1 = NULL;                       //| See comments of setup() for their use cases.
my_element_t* el2 = NULL;                       //| 
my_element_t* el3 = NULL;                       //|
my_element_t* el4 = NULL;                       //|
my_element_t* el5 = NULL;                       //|
my_element_t* el6 = NULL;                       //|
my_element_t* el7 = NULL;                       //|
my_element_t* el8 = NULL;                       //|
my_element_t* el9 = NULL;                       //|
my_element_t* el_insert = NULL;                 //|
my_element_t* el_deepCopy = NULL;        //|

void* element_copy(void * element);                                             //|
void element_free(void ** element);                                             //| Callback Functions in the original file for lab 4.
int element_compare(void * x, void * y);                                        //|

void element_print(void * element);                                             //| Personal defined callback function.
static my_element_t* element_create(my_element_t* element, int id, char name[]);//| Personal defined function (not a callback).

void * element_copy(void * element) {
    my_element_t* copy = malloc(sizeof (my_element_t));
    char* new_name;
    asprintf(&new_name,"%s",((my_element_t*)element)->name);
    assert(copy != NULL);
    copy->id = ((my_element_t*)element)->id;
    copy->name = new_name;
    return (void *) copy;
}

void element_free(void ** element) {
    if (*element == NULL) return;
    else {
        free((((my_element_t*)*element))->name);
        free(*element);
        *element = NULL;
    }
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 : (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);
}

/**
 * Callback function to print the contents of an element of type my_element_t inside a dplist.
 * \param element pointer to the element
 * \return Nothing. The content is printed on stdout
*/
void element_print(void* element) {
    printf("(id=%d, name=%s)\n", ((my_element_t*)element)->id, ((my_element_t*)element)->name);
}

/**
* Create a my_element_t element on heap and assign the given id and name.
* \param element new null-pointer, initialized on the caller-stack
* \param id the id of the new element.
* \param name the name of the new element.
* \return 
*/
static my_element_t* element_create(my_element_t* element, int id, char name[]) {
    assert(element==NULL);
    element = malloc(sizeof(my_element_t));
    element->id = id;
    element->name = malloc(sizeof(&name));
    assert(element->name != NULL);
    strcpy(element->name, name);
    return element;
}


/*
* Setup() is called before each TEST. 
* The following lists are 'freshly' available in each test:
*   NULL list   (case 1)
*   empty list  (case 2)
*   list with one element, not a deepcopy (case 3.1)
*   list with one element, deepcopy       (case 3.2)
*   list with multiple (10) elements, no deepcopies (case 4.1)
*   list with multiple (10) elements, deepcopies    (case 4.2)
*/
void setup(void) {
    
    el_one = element_create(el_one, 0, "el_one"); // | el_one is used to populate the list with one element (cases 3.1 and 3.2) 

    el0 = element_create(el0, 0, "el_0");         // |
    el1 = element_create(el1, 1, "el_1");         // |
    el2 = element_create(el2, 2, "el_2");         // |
    el3 = element_create(el3, 3, "el_3");         // |
    el4 = element_create(el4, 4, "el_4");         // | elx are used to populate the list with multiple elements 
    el6 = element_create(el6, 6, "el_6");         // | (cases 4.1 and 4.2)
    el5 = element_create(el5, 5, "el_5");         // |
    el7 = element_create(el7, 7, "el_7");         // |
    el8 = element_create(el8, 8, "el_8");         // |
    el9 = element_create(el9, 9, "el_9");         // |

    el_insert = element_create(el_insert, 99,     // | el_insert is used to test  
                                "el_insert");     // | dpl_insert_index().


    list_null = NULL;                                               //|  NULL List (case 1)         
    
    list_empty = dpl_create(element_copy,                           //|    
                            element_free,                           //|  Empty list
                            element_compare                        //|  (case 2)
                            );                         //|

/*NOTE: comment following code out if you're not 
        yet sure whether dpl_insert_at_index() already works.*/                                                                                                                       
    list_one_element_pointer = dpl_create(element_copy,             // |
                            element_free,                           // |  List with one element where the element  
                            element_compare                        // |  is a pointer to the element on stack.
                            );                         // |  (case 3.1)
    dpl_insert_at_index(list_one_element_pointer, el_one, 0, false);// |  

    list_one_element_deepCopy = dpl_create(element_copy,            // |
                            element_free,                           // |  List with one element where the element
                            element_compare                        // |  is a deep copy of the element on stack.
                            );                         // |  (case 3.2)
    dpl_insert_at_index(list_one_element_deepCopy, el_one, 0, true);// |


/*NOTE: comment following code out if you're not 
        yet sure whether dpl_insert_at_index() already works.*/  
    list_ten_elements_pointer = dpl_create(element_copy,            // |
                            element_free,                           // |
                            element_compare                        // | 
                            );                         // |  
    dpl_insert_at_index(list_ten_elements_pointer, el0, 0, false);  // |
    dpl_insert_at_index(list_ten_elements_pointer, el1, 1, false);  // | List with ten elements where the elements
    dpl_insert_at_index(list_ten_elements_pointer, el2, 2, false);  // | are pointers to the elements on stack.
    dpl_insert_at_index(list_ten_elements_pointer, el3, 3, false);  // | (case 4.1)
    dpl_insert_at_index(list_ten_elements_pointer, el4, 4, false);  // |
    dpl_insert_at_index(list_ten_elements_pointer, el5, 5, false);  // |
    dpl_insert_at_index(list_ten_elements_pointer, el6, 6, false);  // |
    dpl_insert_at_index(list_ten_elements_pointer, el7, 7, false);  // |
    dpl_insert_at_index(list_ten_elements_pointer, el8, 8, false);  // |
    dpl_insert_at_index(list_ten_elements_pointer, el9, 9, false);  // |

    list_ten_elements_deepCopy = dpl_create(element_copy,           // |
                            element_free,                           // |
                            element_compare                        // |
                            );                         // |
    dpl_insert_at_index(list_ten_elements_deepCopy, el0, 0, true);  // |
    dpl_insert_at_index(list_ten_elements_deepCopy, el1, 1, true);  // | List with ten elements where the elements
    dpl_insert_at_index(list_ten_elements_deepCopy, el2, 2, true);  // | are a deep copy of the elements on stack.
    dpl_insert_at_index(list_ten_elements_deepCopy, el3, 3, true);  // | (case 4.2)
    dpl_insert_at_index(list_ten_elements_deepCopy, el4, 4, true);  // |
    dpl_insert_at_index(list_ten_elements_deepCopy, el5, 5, true);  // |
    dpl_insert_at_index(list_ten_elements_deepCopy, el6, 6, true);  // |
    dpl_insert_at_index(list_ten_elements_deepCopy, el7, 7, true);  // |
    dpl_insert_at_index(list_ten_elements_deepCopy, el8, 8, true);  // |
    dpl_insert_at_index(list_ten_elements_deepCopy, el9, 9, true);  // |  

    //dpl_print(list_one_element_pointer);
}

void teardown(void) {
    // Implement post-test teardown
    dpl_free(&list_null, true);
    dpl_free(&list_empty, true);
    dpl_free(&list_one_element_pointer, true);
    dpl_free(&list_one_element_deepCopy, true);
    dpl_free(&list_ten_elements_pointer, true);
    dpl_free(&list_ten_elements_deepCopy, true);
    el0= NULL;
    el1= NULL;
    el2= NULL;
    el3= NULL;
    el4= NULL;
    el5= NULL;
    el6= NULL;
    el7= NULL;
    el8= NULL;
    el9= NULL;
    el_one = NULL;
    el_insert = NULL;
    el_deepCopy = NULL;
}

/**********************************************************************************************
 * -------------------------------------DPL_FREE-----------------------------------------------
***********************************************************************************************/
// dpl_free(): Do not use the element_free() callback function. 
START_TEST(test_ListFree_noCallback)
    {
        dpl_free(&list_null, false);                        //| ________ Free list_null ________
        ck_assert_msg(list_null == NULL,                    //|
            "Failure: expected result to be NULL, got %p",  //|
            list_null);                                     //|
        
        dpl_free(&list_empty, false);                       //| ________ Free list_empty ________
        ck_assert_msg(list_empty == NULL,                   //|
            "Failure: expected result to be NULL, got %p",  //|
            list_empty);                                    //|

        dpl_free(&list_one_element_pointer, false);         //| ________ Free list_one_element ________
        ck_assert_msg(list_one_element_pointer == NULL,     //| 
            "Failure: expected result to be NULL, got %p",  //|
            list_one_element_pointer);                      //|          
        
        dpl_free(&list_ten_elements_pointer, false);        //| ________ Free list_ten_elements ________
        ck_assert_msg(list_ten_elements_pointer == NULL,    //| 
            "Failure: expected result to be NULL, got %p",  //|
            list_ten_elements_pointer);                     //|   

/*NOTE: AVOID FOLLOWING SITUATION -> Big chance that you loose memory
        (unless you have created stack pointers to the deepcopied elements).*/
        dpl_free(&list_one_element_deepCopy, false);        //| ________ Free list_one_element ________
        ck_assert_msg(list_one_element_deepCopy == NULL,    //| 
            "Failure: expected result to be NULL, got %p",  //|
            list_one_element_deepCopy);                     //|       
/*NOTE: AVOID FOLLOWING SITUATION -> Big chance that you loose memory
        (unless you have created stack pointers to the deepcopied elements).*/
        dpl_free(&list_ten_elements_deepCopy, false) ;      //| ________ Free list_ten_elements ________
        ck_assert_msg(list_ten_elements_deepCopy == NULL,   //| 
            "Failure: expected result to be NULL, got %p",  //|
            list_ten_elements_deepCopy);                    //|
    }           
END_TEST
// dpl_free(): Use the element_free() callback function.
START_TEST(test_ListFree_callback) {

        dpl_free(&list_null, true);                        //|________ Free list_null ________
        ck_assert_msg(list_null == NULL,                   //| 
            "Failure: expected result to be NULL, got %p", //| Test whether list_null stack pointer 
            list_null);                                    //| points to NULL.

        dpl_free(&list_empty, true);                         //|________ Free list_empty ________
        ck_assert_msg(list_empty == NULL,                    //|
            "Failure: expected result to be NULL, got %p",   //| Test whether list_empty stack pointer 
            list_empty);                                     //| points to NULL.

        dpl_free(&list_one_element_pointer, true);           //|________ Free list_one_element containing pointers to stack elements. ________
        ck_assert_msg(list_one_element_pointer == NULL,             //| 
            "Failure: expected result to be NULL, got %p",          //| Test whether list_one_element_pointer stack pointer 
            list_one_element_pointer);                              //| points to NULL.
//TO DO: more thorough check whether el_one became invalid.         //|
        ck_assert_msg(el_one->id != 0,                              //|
            "Failure: expected result to be not 0 anymore, got %d", //| Test whether el_one became invalid.
            el_one->id);                                            //|
        el_one = NULL;                                              //| Don't forget to clear the pointer in stack when memory 
                                                                    //| from heap is freed!
        
        dpl_free(&list_ten_elements_pointer, true);          //|________ Free list_ten_elements containing pointers to stack elements. ________
        ck_assert_msg(list_ten_elements_pointer == NULL,                   //| 
            "Failure: expected result to be NULL, got %p",                 //| Test whether list_ten_elements_pointer stack pointer
            list_ten_elements_pointer);                                    //| points to NULL.                                                           
//TO DO: more thorough check whether elx became invalid.                   //|
        ck_assert_msg(el0-> id != 0,                                       //|
            "Failure: expected result to be not 0 anymore, got %d",        //|
            el0->id);                                                      //|
        ck_assert_msg(el1-> id != 1,                                       //|
            "Failure: expected result to be not 1 anymore, got %d",        //|
            el1->id);                                                      //|
        ck_assert_msg(el2-> id != 2,                                       //|
            "Failure: expected result to be not 2 anymore, got %d",        //|
            el2->id);                                                      //|
        ck_assert_msg(el3-> id != 3,                                       //|
            "Failure: expected result to be not 3 anymore, got %d",        //| 
            el3->id);                                                      //|
        ck_assert_msg(el4-> id != 4,                                       //| Test whether elx became invalid.
            "Failure: expected result to be not 4 anymore, got %d",        //|
            el4->id);                                                      //|
        ck_assert_msg(el5-> id != 5,                                       //|
            "Failure: expected result to be not 5 anymore, got %d",        //|
            el5->id);                                                      //|
        ck_assert_msg(el6-> id != 6,                                       //|
            "Failure: expected result to be not 6 anymore, got %d",        //|
            el6->id);                                                      //|
        ck_assert_msg(el7-> id != 7,                                       //|
            "Failure: expected result to be not 7 anymore, got %d",        //|
            el7->id);                                                      //|
        ck_assert_msg(el8-> id != 8,                                       //|
            "Failure: expected result to be not 8 anymore, got %d",        //|
            el8->id);                                                      //|
        ck_assert_msg(el9-> id != 9,                                       //|
            "Failure: expected result to be not 9 anymore, got %d",        //|
            el9->id);                                                      //|
        el0 = NULL;                                                        //|
        el1 = NULL;                                                        //|
        el2 = NULL;                                                        //|
        el3 = NULL;                                                        //| 
        el4 = NULL;                                                        //| Don't forget to clear the pointer 
        el5 = NULL;                                                        //| in stack when memory from heap is freed!
        el6 = NULL;                                                        //|
        el7 = NULL;                                                        //|
        el8 = NULL;                                                        //|
        el9 = NULL;                                                        //|

        my_element_t* el_one_deepcopy = dpl_get_element_at_index(list_one_element_deepCopy, 0); //|
        ck_assert_msg(el_one_deepcopy->id == 0,                                                 //| Retrieving pointers to the deepcopied 
            "Failure: expected result to be 0, got %d",                                         //| elements and test their ID values.
            el_one_deepcopy->id);                                                               //|
        dpl_free(&list_one_element_deepCopy, true);   //|________ Free list_one_element containing a deep copy. ________
        ck_assert_msg(list_one_element_pointer == NULL,             //|
            "Failure: expected result to be NULL, got %p",          //| Test whether list_one_element_pointer stack pointer
            list_one_element_pointer);                              //| points to NULL.
//TO DO: more thorough check whether el_one_deepcopy became invalid.//|
        ck_assert_msg(el_one_deepcopy->id != 0,                     //|
            "Failure: expected result to be not 0 anymore, got %d", //| Test whether el_one_deepcopy became invalid.
            el_one_deepcopy->id);                                   //|
        el_one_deepcopy = NULL;

        my_element_t* el0_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 0); //|
        ck_assert_msg(el0_deepcopy->id == 0,                                                  //|
            "Failure: expected result to be 0, got %d",                                       //|
            el0_deepcopy->id);                                                                //|
        my_element_t* el1_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 1); //|
        ck_assert_msg(el1_deepcopy->id == 1,                                                  //|
            "Failure: expected result to be 1, got %d",                                       //|
            el1_deepcopy->id);                                                                //|
        my_element_t* el2_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 2); //|
        ck_assert_msg(el2_deepcopy->id == 2,                                                  //|
            "Failure: expected result to be 2, got %d",                                       //|
            el2_deepcopy->id);                                                                //|
        my_element_t* el3_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 3); //|
        ck_assert_msg(el3_deepcopy->id == 3,                                                  //|
            "Failure: expected result to be 3, got %d",                                       //|
            el3_deepcopy->id);                                                                //|
        my_element_t* el4_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 4); //| Retrieving pointers to the deepcopied
        ck_assert_msg(el4_deepcopy->id == 4,                                                  //| elements and test their ID values
            "Failure: expected result to be 4, got %d",                                       //|
            el4_deepcopy->id);                                                                //|
        my_element_t* el5_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 5); //|
        ck_assert_msg(el5_deepcopy->id == 5,                                                  //|
            "Failure: expected result to be 5, got %d",                                       //|
            el5_deepcopy->id);                                                                //|
        my_element_t* el6_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 6); //|
        ck_assert_msg(el6_deepcopy->id == 6,                                                  //|
            "Failure: expected result to be 6, got %d",                                       //|
            el6_deepcopy->id);                                                                //|
        my_element_t* el7_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 7); //|
        ck_assert_msg(el7_deepcopy->id == 7,                                                  //|
            "Failure: expected result to be 7, got %d",                                       //|
            el7_deepcopy->id);                                                                //|
        my_element_t* el8_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 8); //|
        ck_assert_msg(el8_deepcopy->id == 8,                                                  //|
            "Failure: expected result to be 8, got %d",                                       //|
            el8_deepcopy->id);                                                                //|
        my_element_t* el9_deepcopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 9); //|
        ck_assert_msg(el9_deepcopy->id == 9,                                                  //|
            "Failure: expected result to be 9, got %d",                                       //|
            el9_deepcopy->id);                                                                //|
        dpl_free(&list_ten_elements_deepCopy, true);        //|________ Free list_ten_elements containing deep copies. ________ 
        ck_assert_msg(list_ten_elements_deepCopy == NULL,   //|
            "Failure: expected result to be NULL, got %p",  //| Test whether list_ten_elements_deepCopy stack pointer
            list_ten_elements_deepCopy);                    //| points to NULL.
        ck_assert_msg(el0_deepcopy->id != 0,                         //| 
            "Failure: expected result to be not 0 anymore, got %d",  //|
            el0_deepcopy->id);                                       //|
        ck_assert_msg(el1_deepcopy->id != 1,                         //| 
            "Failure: expected result to be not 1 anymore, got %d",  //|
            el1_deepcopy->id);                                       //|
        ck_assert_msg(el2_deepcopy->id != 2,                         //| 
            "Failure: expected result to be not 2 anymore, got %d",  //|
            el2_deepcopy->id);                                       //|
        ck_assert_msg(el3_deepcopy->id != 3,                         //|  
            "Failure: expected result to be not 3 anymore, got %d",  //| 
            el3_deepcopy->id);                                       //|   
        ck_assert_msg(el4_deepcopy->id != 4,                         //| Check whether the ID values 
            "Failure: expected result to be not 4 anymore, got %d",  //| of the deepcopied have been changed
            el4_deepcopy->id);                                       //| during dpl_free.
        ck_assert_msg(el5_deepcopy->id != 5,                         //| 
            "Failure: expected result to be not 5 anymore, got %d",  //| If they have changed, 
            el5_deepcopy->id);                                       //| memory was succesfully freed.
        ck_assert_msg(el6_deepcopy->id != 6,                         //| 
            "Failure: expected result to be not 6 anymore, got %d",  //|
            el6_deepcopy->id);                                       //|
        ck_assert_msg(el7_deepcopy->id != 7,                         //| 
            "Failure: expected result to be not 7 anymore, got %d",  //|
            el7_deepcopy->id);                                       //|
        ck_assert_msg(el8_deepcopy->id != 8,                         //| 
            "Failure: expected result to be not 8 anymore, got %d",  //|
            el8_deepcopy->id);                                       //|
        ck_assert_msg(el9_deepcopy->id != 9,                         //| 
            "Failure: expected result to be not 9 anymore, got %d",  //|
            el9_deepcopy->id);                                       //|       
}



/**********************************************************************************************
 * ------------------------------INSERT_AT_INDEX-----------------------------------------------
***********************************************************************************************/
/*____________________INSERT LIST_NULL____________________________________*/
// dpl_insert_at_index() - null list - pointer
START_TEST(test_ListInsertAtIndexListNull_noCallback) {
    ck_assert_msg(dpl_size(list_null) == -1,                            //|  
        "Failure, expected -1, got %d instead",                         //| Test whether the list points to Null
        dpl_size(list_null));                                           //| 
    
    dpl_insert_at_index(list_null, el_insert, 0, false);                //|________ Insert the given element as a pointer. ________

    ck_assert_msg(dpl_get_element_at_index(list_null, 0) == NULL,       //|  
        "Failure, expected NULL, got %p",                               //| 
        dpl_get_element_at_index(list_null, 0));                        //| 
    ck_assert_msg(list_null == NULL,                                    //| Test whether no element was inserted
        "Failure, expected NULL, got %p",                               //| and list_null returns null
        list_null);                                                     //|
    ck_assert_msg(dpl_size(list_null) == -1,                            //| 
        "Failure, expected -1, got %d",                                 //| 
        dpl_size(list_null));                                           //|  
}
END_TEST
// dpl_insert_at_index() - null list - deep copy
START_TEST(test_ListInsertAtIndexListNull_callback) {
    ck_assert_msg(dpl_size(list_null) == -1,                            //|  
        "Failure, expected -1, got %d instead",                         //| Test whether the list points to Null
        dpl_size(list_null));                                           //| 

    dpl_insert_at_index(list_null, el_insert, 0, true);                 //|________ Insert the given element as a deep copy. ________

    ck_assert_msg(dpl_get_element_at_index(list_null, 0) == NULL,       //|  
        "Failure, expected NULL, got %p",                               //| 
        dpl_get_element_at_index(list_null, 0));                        //| 
    ck_assert_msg(list_null == NULL,                                    //| Test whether no element was inserted
        "Failure, expected NULL, got %p",                               //| and list_null returns null
        list_null);                                                     //|
    ck_assert_msg(dpl_size(list_null) == -1,                            //| 
        "Failure, expected -1, got %d",                                 //| 
        dpl_size(list_null));                                           //|                                                          
    
}
END_TEST

/*____________________INSERT LIST_EMPTY____________________________________*/
// dpl_insert_at_index() - empty list - pointer
START_TEST(test_ListInsertAtIndexListEmpty_noCallback) {
    ck_assert_msg(dpl_size(list_empty) == 0,                            //|  
        "Failure, expected 0, got %d instead",                          //| Test whether the list is empty
        dpl_size(list_empty));                                          //| 
    
    dpl_insert_at_index(list_empty, el_insert, 0, false); //|________ Insert the given element as a pointer. ________

    ck_assert_msg(dpl_get_element_at_index(list_empty, 0) == el_insert, //| Test whether the inserted element-pointer 
        "Failure, expected %p, got %p instead",                       //| points to the same address as
        el_insert, dpl_get_element_at_index(list_empty, 0));                       //| the stack element-pointer.
}
END_TEST
// dpl_insert_at_index() - empty list - deep copy
START_TEST(test_ListInsertAtIndexListEmpty_callback) {
    ck_assert_msg(dpl_size(list_empty) == 0,                            //|  
        "Failure, expected 0, got %d instead",                          //| Test whether the list is empty
        dpl_size(list_empty));                                          //| 

    dpl_insert_at_index(list_empty, el_insert, 0, true); //|________ Insert the given element as a deep copy. ________

    el_deepCopy = dpl_get_element_at_index(list_empty, 0);

    ck_assert_msg(el_deepCopy != el_insert,                                         //| Test whether the inserted element-pointer 
        "Failure, expected addresses to differ, got el_deepCopy == el_insert (%p)", //| does not point to the same address as
        el_deepCopy);                                                               //| the stack element-pointer.
    ck_assert_msg(strcmp(el_deepCopy->name,el_insert->name) == 0,                             //| 
        "Failure, expected names to be equal, got el_deepCopy (\"%s\") and el_insert (\"%s\")", //| 
        el_deepCopy->name, el_insert->name);                                            //| Test whether the id and name of the
    ck_assert_msg(el_deepCopy->id == el_insert->id,                                     //| deep copy match the stack element. 
        "Failure, expected ids to be equal, got el_deepCopy (%d) and el_insert (%d)",   //| 
        el_deepCopy->id, el_insert->id);                                                //|  
}
END_TEST

/*____________________INSERT LIST_ONE_ELEMENT____________________________________*/

// dpl_insert_at_index() - list one element - pointer
START_TEST(test_ListInsertAtIndexListOne_noCallback) {
    ck_assert_msg(dpl_size(list_one_element_pointer) == 1,              //|  
        "Failure, expected 1, got %d instead",                          //| Test whether the list contains one element.
        dpl_size(list_one_element_pointer));                            //| 
    
    dpl_insert_at_index(list_one_element_pointer, el_insert, 0, false); //|________ Insert the given element as a pointer. ________

    ck_assert_msg(dpl_get_element_at_index(list_one_element_pointer, 0) == el_insert, //| Test whether the inserted element-pointer 
        "Failure, expected %p, got %p instead",                                     //| points to the same address as
        el_insert, dpl_get_element_at_index(list_one_element_pointer, 0));                       //| the stack element-pointer.
}
END_TEST
// dpl_insert_at_index() - list one element - deep copy
START_TEST(test_ListInsertAtIndexListOne_callback) {
    
    ck_assert_msg(dpl_size(list_one_element_pointer) == 1,                            //|  
        "Failure, expected 1, got %d instead",                                        //| Test whether the list contains one element.
        dpl_size(list_one_element_pointer));                                          //| 

    dpl_insert_at_index(list_one_element_pointer, el_insert, 0, true); //|________ Insert the given element as a deep copy. ________

    el_deepCopy = dpl_get_element_at_index(list_one_element_pointer, 0);

    ck_assert_msg(el_deepCopy != el_insert,                                         //| Test whether the inserted element-pointer 
        "Failure, expected addresses to differ, got el_deepCopy == el_insert (%p)", //| does not point to the same address as
        el_deepCopy);                                                               //| the stack element-pointer.
    ck_assert_msg(strcmp(el_deepCopy->name,el_insert->name) == 0,                             //| 
        "Failure, expected names to be equal, got el_deepCopy (%s) and el_insert (%s)", //| 
        el_deepCopy->name, el_insert->name);                                            //| Test whether the id and name of the
    ck_assert_msg(el_deepCopy->id == el_insert->id,                                     //| deep copy match the stack element. 
        "Failure, expected ids to be equal, got el_deepCopy (%d) and el_insert (%d)",   //| 
        el_deepCopy->id, el_insert->id);                                               
}
END_TEST

/*____________________INSERT LIST_TEN_ELEMENTS____________________________________*/

// dpl_insert_at_index() - list ten elements - pointer
START_TEST(test_ListInsertAtIndexListMulti_noCallback) {

    ck_assert_msg(dpl_size(list_ten_elements_pointer) == 10,                            //|  
        "Failure, expected 10, got %d instead",                         //| Test whether the list contains ten elements.
        dpl_size(list_ten_elements_pointer));                                           //| 
    
    dpl_insert_at_index(list_ten_elements_pointer, el_insert, 7, false); //|________ Insert the given element as a pointer. ________

    ck_assert_msg(dpl_get_element_at_index(list_ten_elements_pointer, 7) == el_insert, //| Test whether the inserted element-pointer 
        "Failure, expected %p, got %p instead",                                        //| points to the same address as
        el_insert, dpl_get_element_at_index(list_ten_elements_pointer, 7));            //| the stack element-pointer.

}
END_TEST
// dpl_insert_at_index() - list ten elements - deep copy
START_TEST(test_ListInsertAtIndexListMulti_callback) {

    ck_assert_msg(dpl_size(list_ten_elements_deepCopy) == 10,                            //| Check whether the list 
        "Failure, expected 10, got %d instead", dpl_size(list_ten_elements_deepCopy));   //| has 10 elements.
    
    dpl_insert_at_index(list_ten_elements_deepCopy, el_insert, 7, true); //________ Insert the given element as a deep copy. ________

    el_deepCopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 7);                              //|
    ck_assert_msg(el_deepCopy != el_insert,                                                             //| 
        "Failure, pointers to differ got %p and %p instead",                                            //| 
        el_deepCopy, el_insert);                                                                        //| 
    ck_assert_msg(el_deepCopy->id == el_insert->id,                                                     //| Test whether a deep copy
        "Failure, expected ids to be the same, got deepCopy id (%d) and el_insert id (%d) instead",     //| was made of the stack 
        el_deepCopy->id, el_insert->id);                                                                //| element.    
    ck_assert_msg(strcmp(el_deepCopy->name, el_insert->name) == 0,                                           //| 
        "Failure, expected names to be the same, got deepCopy (\"%s\") and el_insert (\"%s\") instead", //| 
        el_deepCopy->name, el_insert->name);                                                                //|     

}
END_TEST

/**********************************************************************************************
 * ------------------------------REMOVE_AT_INDEX-----------------------------------------------
***********************************************************************************************/
/*____________________REMOVE LIST_NULL____________________________________*/
// dpl_remove_at_index() - null list - leave element
START_TEST(test_ListRemoveAtIndexListNull_noCallback) {
         ck_assert_msg(dpl_remove_at_index(list_null, 0, false) == NULL,   //| ________ Remove (non-existent) element 0 ________
            "Failure: expected result to be NULL, got %p",                  //| Test whether remove_at_index() returns NULL.
            list_null);                                                     //| 
}
END_TEST
// dpl_remove_at_index() - null list - free element
START_TEST(test_ListRemoveAtIndexListNull_callback) {
                                                                            
    ck_assert_msg(dpl_remove_at_index(list_null, 0, true) == NULL,     //| ________ Remove & free (non-existent) element 0 ________
        "Failure: expected result to be NULL, got %p",                  //| Test whether remove_at_index() returns NULL.
        list_null);                                                     //| 
}
END_TEST


/*____________________REMOVE LIST_EMPTY____________________________________*/
// dpl_remove_at_index() - empty list - leave element - pointer
START_TEST(test_ListRemoveAtIndexListEmpty_noCallback) {
    dplist_t* temp_list = list_empty;
    ck_assert_msg(dpl_remove_at_index(list_empty, 0, false) == temp_list,     //| ________ Remove (non-existent) element 0 ________
            "Failure: expected result to be %p, got %p",                      //| Test whether remove_at_index() returns list_empty
            temp_list, list_empty);                                           //| 
}
END_TEST

// dpl_remove_at_index() - empty list - free element - pointer
START_TEST(test_ListRemoveAtIndexListEmpty_callback) {
    dplist_t* temp_list = list_empty;
    ck_assert_msg(dpl_remove_at_index(list_empty, 0, true) == temp_list, //| ________ Remove & free (non-existent) element 0 ________
        "Failure: expected result to be %p, got %p",                      //| Test whether remove_at_index() 
        temp_list, list_empty);                                           //| returns list_empty.
}
END_TEST


/*____________________REMOVE LIST_ONE_ELEMENT____________________________________*/
// dpl_remove_at_index() - list one element - leave element - pointer
START_TEST(test_ListRemoveAtIndexListOne_noCallback_pointer) {
    dplist_t* temp_list = dpl_remove_at_index(list_one_element_pointer, 0, false); //| ________ Remove element 0 ________
    ck_assert_msg(temp_list == list_one_element_pointer,                            //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                //| returns list_one_element_pointer
        temp_list, list_one_element_pointer);                                       //|
    ck_assert_msg(dpl_get_element_at_index(list_one_element_pointer, 0) == NULL,    //| Test whether el_one is removed  
        "Failure: expected result to be NULL, got %p",                              //| from the list. 
        dpl_get_element_at_index(list_one_element_pointer, 0));                                      //|                                    
}
END_TEST
//NOTE: AVOID FOLLOWING CASE!
// dpl_remove_at_index() - list one element - leave element - deep copy
START_TEST(test_ListRemoveAtIndexListOne_noCallback_deepCopy) {
    my_element_t* temp_el = dpl_get_element_at_index(list_one_element_deepCopy, 0);
    dplist_t* temp_list = dpl_remove_at_index(list_one_element_deepCopy, 0, false);  //| ________ Remove element 0 ________
    ck_assert_msg(temp_list == list_one_element_deepCopy,                             //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                  //| returns list_one_element_pointer
        temp_list, list_one_element_deepCopy);                                        //|
    ck_assert_msg(dpl_get_element_at_index(list_one_element_deepCopy, 0) == NULL,     //| Test whether el_one is removed  
        "Failure: expected result to be NULL, got %p",                                //| from the list. 
        dpl_get_element_at_index(list_one_element_deepCopy, 0));                                       //|
                                                               
}   
END_TEST

// dpl_remove_at_index() - list one element - free element - pointer
START_TEST(test_ListRemoveAtIndexListOne_callback_pointer) {
    my_element_t* temp_el = dpl_get_element_at_index(list_one_element_pointer, 0);  //|
    dplist_t* temp_list = dpl_remove_at_index(list_one_element_pointer, 0, true);  //| ________ Remove element 0 ________
    ck_assert_msg(temp_list == list_one_element_pointer,                            //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                //| returns list_one_element_deepCopy
        temp_list, list_one_element_pointer);                                       //|
    ck_assert_msg(dpl_get_element_at_index(list_one_element_pointer, 0) == NULL,    //| Test whether el_one is removed  
        "Failure: expected result to be NULL, got %p",                              //| from the list. 
        list_one_element_pointer, list_empty);                                      //|
}
END_TEST

// dpl_remove_at_index() - list one element - free element - deep copy
START_TEST(test_ListRemoveAtIndexListOne_callback_deepCopy) {
    my_element_t* temp_el = dpl_get_element_at_index(list_one_element_deepCopy, 0);
    dplist_t* temp_list = dpl_remove_at_index(list_one_element_deepCopy, 0, true);  //| ________ Remove element 0 ________
    ck_assert_msg(temp_list == list_one_element_deepCopy,                            //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                 //| returns list_one_element_deepCopy
        temp_list, list_one_element_deepCopy);                                       //|
    ck_assert_msg(dpl_get_element_at_index(list_one_element_deepCopy, 0) == NULL,    //| Test whether el_one is removed  
        "Failure: expected result to be NULL, got %p",                               //| from the list. 
        list_one_element_deepCopy, list_empty);                                      //|
//TO DO: More thorough check?
    ck_assert_msg(temp_el->id > 1000,                                               //| 
        "Failure: expected resultlarger than 1000 got %d",                          //| Test whether memory is freed.
        temp_el->id);                                                               //| 
}
END_TEST


/*____________________REMOVE LIST_TEN_ELEMENTS____________________________________*/
// dpl_remove_at_index() - list ten elements - leave element - pointer
START_TEST(test_ListRemoveAtIndexListMulti_noCallback_pointer) {
    dplist_t* temp_list = dpl_remove_at_index(list_ten_elements_pointer, 0, false); //| ________ Remove element 0 (start of list) ________
    ck_assert_msg(temp_list == list_ten_elements_pointer,                            //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                 //| returns list_ten_elements_pointer
        temp_list, list_ten_elements_pointer);
    my_element_t* pointer = dpl_get_element_at_index(list_ten_elements_pointer, 0);  //|
    ck_assert_msg(pointer->id == 1,                                                  //|   
        "Failure: expected result to be 1, got %d",                                  //| 
        pointer->id);                                                                //| Test whether el0 is removed
    ck_assert_msg(strcmp(pointer->name, "el1"),                                      //| from the list. 
        "Failure: expected result to be \"el1\", got %s",                            //| 
        pointer->name);                                                              //|
    
    temp_list = dpl_remove_at_index(list_ten_elements_pointer, 4, false); //| ________ Remove element 4 (middle of list) ________
    ck_assert_msg(temp_list == list_ten_elements_pointer,                            //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                 //| returns list_ten_elements_pointer
        temp_list, list_ten_elements_pointer);
    pointer = dpl_get_element_at_index(list_ten_elements_pointer, 4);  //|
    ck_assert_msg(pointer->id == 6,                                                  //|   
        "Failure: expected result to be 6, got %d",                                  //| 
        pointer->id);                                                                //| Test whether el5 is removed
    ck_assert_msg(strcmp(pointer->name, "el6"),                                      //| from the list. 
        "Failure: expected result to be \"el6\", got %s",                            //| 
        pointer->name);                                                              //|

    temp_list = dpl_remove_at_index(list_ten_elements_pointer, 50, false); //| ________ Remove element 9 (end of list) ________
    ck_assert_msg(temp_list == list_ten_elements_pointer,                            //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                 //| returns list_ten_elements_pointer
        temp_list, list_ten_elements_pointer);
    pointer = dpl_get_element_at_index(list_ten_elements_pointer, 50);  //|
    ck_assert_msg(pointer->id == 8,                                                  //|   
        "Failure: expected result to be 8, got %d",                                  //| 
        pointer->id);                                                                //| Test whether el9 is removed
    ck_assert_msg(strcmp(pointer->name, "el8"),                                      //| from the list. 
        "Failure: expected result to be \"el8\", got %s",                            //| 
        pointer->name);                                                              //|
}
END_TEST

// dpl_remove_at_index() - list ten elements - leave element - deep copy
//NOTE: AVOID FOLLOWING CASE!
START_TEST(test_ListRemoveAtIndexListMulti_noCallback_deepCopy) {
    my_element_t* temp_el = dpl_get_element_at_index(list_ten_elements_deepCopy, 0);
    dplist_t* temp_list = dpl_remove_at_index(list_ten_elements_deepCopy, 0, false);  //| ________ Remove element 0 (start of list) ________
    ck_assert_msg(temp_list == list_ten_elements_deepCopy,                             //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                   //| returns list_ten_elements_pointer
        temp_list, list_one_element_deepCopy); 
    my_element_t* deepCopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 0);  //|
    ck_assert_msg(deepCopy->id == 1,                                                   //|   
        "Failure: expected result to be 1, got %d",                                    //|  
        deepCopy->id);                                                                 //| Test whether el0 is removed
    ck_assert_msg(strcmp(deepCopy->name, "el1"),                                       //| from the list.  
        "Failure: expected result to be \"el1\", got %s",                              //| 
        deepCopy->name);                                                               //|
    ck_assert_msg(dpl_size(list_ten_elements_deepCopy) == 9,                           //|   
        "Failure: expected result to be 9, got %d",                                    //| 
        dpl_size(list_ten_elements_deepCopy));                                         //|      
                                                                                       
    ck_assert_msg(temp_el->id == 0,                                                    //|
        "Failure: expected temp_el id to be 0, got %d",                                //|
        temp_el->id);                                                                  //| Test whether temp_el (el0) 
    ck_assert_msg(strcmp(temp_el->name, "el0"),                                        //| is still reachable.
        "Failure: expected temp_el name to be \"el0\", got %s",                        //| 
        temp_el->name);                                                                //|
    
    temp_el = dpl_get_element_at_index(list_ten_elements_deepCopy, 4);
    temp_list = dpl_remove_at_index(list_ten_elements_deepCopy, 4, false);  //| ________ Remove element 4 (middle of list) ________
    ck_assert_msg(temp_list == list_ten_elements_deepCopy,                             //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                   //| returns list_ten_elements_pointer
        temp_list, list_one_element_deepCopy); 
    deepCopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 4);  //|
    ck_assert_msg(deepCopy->id == 6,                                                   //|   
        "Failure: expected result to be 6, got %d",                                    //|  
        deepCopy->id);                                                                 //| Test whether el5 is removed
    ck_assert_msg(strcmp(deepCopy->name, "el6"),                                       //| from the list.  
        "Failure: expected result to be \"el6\", got %s",                              //| 
        deepCopy->name);                                                               //|
    ck_assert_msg(dpl_size(list_ten_elements_deepCopy) == 8,                           //|   
        "Failure: expected result to be 8, got %d",                                    //| 
        dpl_size(list_ten_elements_deepCopy));                                         //|      
                                                                                       
    ck_assert_msg(temp_el->id == 5,                                                    //|
        "Failure: expected temp_el id to be 0, got %d",                                //|
        temp_el->id);                                                                  //| Test whether temp_el (el5) 
    ck_assert_msg(strcmp(temp_el->name, "el5"),                                        //| is still reachable.
        "Failure: expected temp_el name to be \"el5\", got %s",                        //| 
        temp_el->name);                                                                //|

    temp_el = dpl_get_element_at_index(list_ten_elements_deepCopy, 50);
    temp_list = dpl_remove_at_index(list_ten_elements_deepCopy, 50, false);  //| ________ Remove element 9 (end of list)________
    ck_assert_msg(temp_list == list_ten_elements_deepCopy,                             //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                   //| returns list_ten_elements_pointer
        temp_list, list_one_element_deepCopy); 
    deepCopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 50);  //|
    ck_assert_msg(deepCopy->id == 8,                                                   //|   
        "Failure: expected result to be 8, got %d",                                    //|  
        deepCopy->id);                                                                 //| Test whether el9 is removed
    ck_assert_msg(strcmp(deepCopy->name, "el8"),                                       //| from the list.  
        "Failure: expected result to be \"el8\", got %s",                              //| 
        deepCopy->name);                                                               //|
    ck_assert_msg(dpl_size(list_ten_elements_deepCopy) == 7,                           //|   
        "Failure: expected result to be 7, got %d",                                    //| 
        dpl_size(list_ten_elements_deepCopy));                                         //|      
                                                                                       
    ck_assert_msg(temp_el->id == 9,                                                    //|
        "Failure: expected temp_el id to be 9, got %d",                                //|
        temp_el->id);                                                                  //| Test whether temp_el (el9) 
    ck_assert_msg(strcmp(temp_el->name, "el9"),                                        //| is still reachable.
        "Failure: expected temp_el name to be \"el9\", got %s",                        //| 
        temp_el->name);                                                                //|


}
END_TEST

// dpl_remove_at_index() - list ten elements - free element - pointer
START_TEST(test_ListRemoveAtIndexListMulti_callback_pointer) {
    my_element_t* pointer = dpl_get_element_at_index(list_ten_elements_pointer, 0);
    dplist_t* temp_list = dpl_remove_at_index(list_ten_elements_pointer, 0, true);       //| ________ Remove element 0 ________
    ck_assert_msg(temp_list == list_ten_elements_pointer,                                //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                     //| returns list_ten_elements_pointer
        temp_list, list_ten_elements_pointer);
    my_element_t* el1_pointer = dpl_get_element_at_index(list_ten_elements_pointer, 0);  //|
    ck_assert_msg(el1_pointer->id == 1,                                                  //|   
        "Failure: expected result to be 1, got %d",                                      //| 
        el1_pointer->id);                                                                //| Test whether el0 is removed
    ck_assert_msg(strcmp(el1_pointer->name, "el1"),                                      //| from the list. 
        "Failure: expected result to be \"el1\", got %s",                                //| 
        el1_pointer->name); 
    ck_assert_msg((pointer->id) > 1000,                                                  //| 
        "Failure: expected result to be >1000, got %d",                                  //| Test whether el0 element was freed.
        (pointer->id));                                                              //|

    pointer = dpl_get_element_at_index(list_ten_elements_pointer, 4);
    temp_list = dpl_remove_at_index(list_ten_elements_pointer, 4, true); //| ________ Remove element 4 (middle of list) ________
    ck_assert_msg(temp_list == list_ten_elements_pointer,                                //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                     //| returns list_ten_elements_pointer
        temp_list, list_ten_elements_pointer);                                           //|
    my_element_t* el6_pointer = dpl_get_element_at_index(list_ten_elements_pointer, 4);  //|
    ck_assert_msg(el6_pointer->id == 6,                                                  //|   
        "Failure: expected result to be 6, got %d",                                      //| 
        el6_pointer->id);                                                                //| Test whether el5 is removed
    ck_assert_msg(strcmp(el6_pointer->name, "el6"),                                      //| from the list. 
        "Failure: expected result to be \"el6\", got %s",                                //| 
        el6_pointer->name);                                                              //|
    
    ck_assert_msg((pointer->id) > 1000,                                              //| 
        "Failure: expected result to be >1000, got %d",                              //| Test whether el5 element was freed.
        (pointer->id));                                                              //|

    pointer = dpl_get_element_at_index(list_ten_elements_pointer, 50);
    temp_list = dpl_remove_at_index(list_ten_elements_pointer, 50, true); //| ________ Remove element 9 (end of list) ________
    ck_assert_msg(temp_list == list_ten_elements_pointer,                            //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                 //| returns list_ten_elements_deepCopy
        temp_list, list_ten_elements_pointer);
    my_element_t* el8_pointer = dpl_get_element_at_index(list_ten_elements_pointer, 50);  //|
    ck_assert_msg(el8_pointer->id == 8,                                                   //|   
        "Failure: expected result to be 8, got %d",                                       //| 
        el8_pointer->id);                                                                 //| Test whether el9 is removed
    ck_assert_msg(strcmp(el8_pointer->name, "el8"),                                       //| from the list. 
        "Failure: expected result to be \"el8\", got %s",                                 //| 
        pointer->name);                                                                   //|

    ck_assert_msg((pointer->id) > 1000,                                              //| 
        "Failure: expected result to be >1000, got %d",                              //| Test whether el9 element was freed.
        (pointer->id));                                                              //|
}
END_TEST

// dpl_remove_at_index() - list ten elements - free element - deep copy
START_TEST(test_ListRemoveAtIndexListMulti_callback_deepCopy) {
    my_element_t* pointer = dpl_get_element_at_index(list_ten_elements_deepCopy, 0);
    dplist_t* temp_list = dpl_remove_at_index(list_ten_elements_deepCopy, 0, true);      //| ________ Remove element 0 ________
    ck_assert_msg(temp_list == list_ten_elements_deepCopy,                               //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                     //| returns list_ten_elements_deepCopy
        temp_list, list_one_element_deepCopy); 
    
    my_element_t* el1_pointer = dpl_get_element_at_index(list_ten_elements_deepCopy, 0);    //|
    ck_assert_msg(el1_pointer->id == 1,                                                     //|   
        "Failure: expected result to be 1, got %d",                                         //|  
        el1_pointer->id);                                                                   //| Test whether el0 is removed
    ck_assert_msg(strcmp(el1_pointer->name, "el1"),                                         //| from the list.  
        "Failure: expected result to be \"el1\", got %s",                                   //| 
        el1_pointer->name);                                                                 //|

    ck_assert_msg((pointer->id) > 1000,                                                     //| 
        "Failure: expected result to be >1000, got %d",                                     //| Test whether el0 element was freed.
        (pointer->id));                                                                     //|
    
    pointer = dpl_get_element_at_index(list_ten_elements_deepCopy, 4);
    temp_list = dpl_remove_at_index(list_ten_elements_deepCopy, 4, true); //| ________ Remove element 4 (middle of list) ________
    ck_assert_msg(temp_list == list_ten_elements_deepCopy,                                //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                      //| returns list_ten_elements_deepCopy
        temp_list, list_ten_elements_deepCopy);                                           //|
    my_element_t* el6_pointer = dpl_get_element_at_index(list_ten_elements_deepCopy, 4);  //|
    ck_assert_msg(el6_pointer->id == 6,                                                   //|   
        "Failure: expected result to be 6, got %d",                                       //| 
        el6_pointer->id);                                                                 //| Test whether el5 is removed
    ck_assert_msg(strcmp(el6_pointer->name, "el6"),                                       //| from the list. 
        "Failure: expected result to be \"el6\", got %s",                                 //| 
        el6_pointer->name);                                                               //|
    
    ck_assert_msg((pointer->id) > 1000,                                              //| 
        "Failure: expected result to be >1000, got %d",                              //| Test whether el5 element was freed.
        (pointer->id));                                                              //|

    pointer = dpl_get_element_at_index(list_ten_elements_deepCopy, 50);
    temp_list = dpl_remove_at_index(list_ten_elements_deepCopy, 50, true); //| ________ Remove element 9 (end of list) ________
    ck_assert_msg(temp_list == list_ten_elements_deepCopy,                            //| Test whether remove_at_index() 
        "Failure: expected result to be %p, got %p",                                  //| returns list_ten_elements_deepCopy
        temp_list, list_ten_elements_deepCopy);
    my_element_t* el8_pointer = dpl_get_element_at_index(list_ten_elements_deepCopy, 50);  //|
    ck_assert_msg(el8_pointer->id == 8,                                                   //|   
        "Failure: expected result to be 8, got %d",                                       //| 
        el8_pointer->id);                                                                 //| Test whether el9 is removed
    ck_assert_msg(strcmp(el8_pointer->name, "el8"),                                       //| from the list. 
        "Failure: expected result to be \"el8\", got %s",                                 //| 
        el8_pointer->name);                                                                   //|

    ck_assert_msg((pointer->id) > 1000,                                              //| 
        "Failure: expected result to be >1000, got %d",                              //| Test whether el9 element was freed.
        (pointer->id));                                                              //|
}
END_TEST

/**********************************************************************************************
 * ------------------------------GET_REFERENCE_AT_INDEX----------------------------------------
***********************************************************************************************/
START_TEST(test_getReferenceAtIndexNULL) {
    dplist_node_t* node = dpl_get_reference_at_index(list_null, 0);
    my_element_t* element = dpl_get_element_at_reference(list_null, node);
    ck_assert_msg( node == NULL,
        "Failure: expected NULL, got %p",
        node);
    ck_assert_msg( element == NULL,
        "Failure: expected NULL, got %p",
        element);
}
END_TEST

START_TEST(test_getReferenceAtIndexEmpty) {
    dplist_node_t* node = dpl_get_reference_at_index(list_empty, 0);
    my_element_t* element = dpl_get_element_at_reference(list_empty, node);
    ck_assert_msg( node == NULL,
        "Failure: expected NULL, got %p",
        node);
    ck_assert_msg( element == NULL,
        "Failure: expected NULL, got %p",
        element);
}
END_TEST

START_TEST(test_getReferenceAtIndexOne_pointer) {
    dplist_node_t* node = dpl_get_reference_at_index(list_one_element_pointer, 0);
    my_element_t* element = dpl_get_element_at_reference(list_one_element_pointer, node);

    ck_assert_msg(element == el_one,
        "Failure: expected %p, got %p",
        el_one, element);
}
END_TEST

START_TEST(test_getReferenceAtIndexOne_deepCopy) {
    dplist_node_t* node = dpl_get_reference_at_index(list_one_element_deepCopy, 0);
    my_element_t* element = dpl_get_element_at_reference(list_one_element_deepCopy, node);
    ck_assert_msg(element != el_one,
        "Failure: expected difference in pointers, got %p and %p",
        el_one, element);
    ck_assert_msg(element->id == el_one->id,
        "Failure: expected equal reference element ids, got %d and %d",
        el_one->id, element->id);
    ck_assert_msg(element->id == el_one->id,
        "Failure: expected equal reference element names, got %s and %s",
        el_one->name, element->name);
}
END_TEST

START_TEST(test_getReferenceAtIndexMulti_pointer) {
    dplist_node_t* node = dpl_get_reference_at_index(list_ten_elements_pointer, 0);
    my_element_t* element = dpl_get_element_at_reference(list_ten_elements_pointer, node);
    ck_assert_msg(element == el0,
        "Failure: expected %p, got %p",
        el0, element);
    node = dpl_get_reference_at_index(list_ten_elements_pointer, 4);
    element = dpl_get_element_at_reference(list_ten_elements_pointer, node);

    ck_assert_msg(element == el4,
        "Failure: expected %p, got %p",
        el4, element);
    node = dpl_get_reference_at_index(list_ten_elements_pointer, 9);
    element = dpl_get_element_at_reference(list_ten_elements_pointer, node);
    ck_assert_msg(element == el9,
        "Failure: expected %p, got %p",
        el9, element);
    node = dpl_get_reference_at_index(list_ten_elements_pointer, 50);
    element = dpl_get_element_at_reference(list_ten_elements_pointer, node);

    ck_assert_msg(element == el9,
        "Failure: expected %p, got %p",
        el9, element);
    
}

END_TEST

START_TEST(test_getReferenceAtIndexMulti_deepCopy) {
    dplist_node_t* node = dpl_get_reference_at_index(list_ten_elements_deepCopy, 0);
    my_element_t* element = dpl_get_element_at_reference(list_ten_elements_deepCopy, node);
    ck_assert_msg(element != el0,
        "Failure: expected difference in pointers, got %p and %p",
        el0, element);
    ck_assert_msg(element->id == el0->id,
        "Failure: expected equal reference element ids, got %d and %d",
        el0->id, element->id);
    ck_assert_msg(element->id == el0->id,
        "Failure: expected equal reference element names, got %s and %s",
        el0->name, element->name);

    node = dpl_get_reference_at_index(list_ten_elements_deepCopy, 4);
    element = dpl_get_element_at_reference(list_ten_elements_deepCopy, node);
    ck_assert_msg(element != el4,
        "Failure: expected difference in pointers, got %p and %p",
        el4, element);
    ck_assert_msg(element->id == el4->id,
        "Failure: expected equal reference element ids, got %d and %d",
        el4->id, element->id);
    ck_assert_msg(element->id == el4->id,
        "Failure: expected equal reference element names, got %s and %s",
        el4->name, element->name);

    node = dpl_get_reference_at_index(list_ten_elements_deepCopy, 9);
    element = dpl_get_element_at_reference(list_ten_elements_deepCopy, node);
    ck_assert_msg(element != el9,
        "Failure: expected difference in pointers, got %p and %p",
        el9, element);
    ck_assert_msg(element->id == el9->id,
        "Failure: expected equal reference element ids, got %d and %d",
        el9->id, element->id);
    ck_assert_msg(element->id == el9->id,
        "Failure: expected equal reference element names, got %s and %s",
        el9->name, element->name);

    node = dpl_get_reference_at_index(list_ten_elements_deepCopy, (50));
    element = dpl_get_element_at_reference(list_ten_elements_deepCopy, node);
    ck_assert_msg(element != el9,
        "Failure: expected difference in pointers, got %p and %p",
        el9, element);
    ck_assert_msg(element->id == el9->id,
        "Failure: expected equal reference element ids, got %d and %d",
        el9->id, element->id);
    ck_assert_msg(element->id == el9->id,
        "Failure: expected equal reference element names, got %s and %s",
        el9->name, element->name);
} END_TEST


/**********************************************************************************************
 * ------------------------------GET_ELEMENT_AT_INDEX------------------------------------------
***********************************************************************************************/
START_TEST(test_getElementAtIndexNULL) {
    ck_assert_msg(dpl_get_element_at_index(list_null, 0) == NULL, 
        "Failure: expected NULL, got %p instead.",
         dpl_get_element_at_index(list_null, 5));
}
END_TEST

START_TEST(test_getElementAtIndexEmpty) {
    ck_assert_msg(dpl_get_element_at_index(list_empty, 0) == NULL, 
        "Failure: expected NULL, got %p instead.",
         dpl_get_element_at_index(list_empty, 0));
}
END_TEST

START_TEST(test_getElementAtIndexOne_pointer) {
    ck_assert_msg(dpl_get_element_at_index(list_one_element_pointer, 0) == el_one, 
        "Failure: expected %p, got %p instead.",
         el_one, dpl_get_element_at_index(list_one_element_pointer, 0));
}
END_TEST

START_TEST(test_getElementAtIndexOne_deepCopy) {
    my_element_t* deepCopy = dpl_get_element_at_index(list_one_element_deepCopy, 0);
    ck_assert_msg(deepCopy != el_one, 
        "Failure: expected element pointers to differ, got %p and %p instead.",
         deepCopy, el_one);
    ck_assert_msg(deepCopy->id == el_one->id, 
        "Failure: expected equal element ids, got %d and %d instead.",
         deepCopy->id, el_one->id);
    ck_assert_msg(strcmp(deepCopy->name, el_one->name) == 0, 
        "Failure: expected equal element names, got %s and %s instead.",
         strcmp(deepCopy->name, el_one->name) );
}
END_TEST

START_TEST(test_getElementAtIndexMulti_pointer) {
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements_pointer, 0) == el0, 
        "Failure: expected %p, got %p instead.",
         el0, dpl_get_element_at_index(list_ten_elements_pointer, 0));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements_pointer, 7) == el7, 
        "Failure: expected %p, got %p instead.",
         el7, dpl_get_element_at_index(list_ten_elements_pointer, 7));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements_pointer, 50) == el9, 
        "Failure: expected %p, got %p instead.",
         el9, dpl_get_element_at_index(list_ten_elements_pointer, 50));
}
END_TEST

START_TEST(test_getElementAtIndexMulti_deepCopy) {
    my_element_t* deepCopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 0);
    ck_assert_msg(deepCopy != el0, 
        "Failure: expected element pointers to differ, got %p and %p instead.",
         deepCopy, el0);
    ck_assert_msg(deepCopy->id == el0->id, 
        "Failure: expected equal element ids, got %d and %d instead.",
         deepCopy->id, el0->id);
    ck_assert_msg(strcmp(deepCopy->name, el0->name) == 0, 
        "Failure: expected equal element names, got %s and %s instead.",
         strcmp(deepCopy->name, el0->name) );

    deepCopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 7);
    ck_assert_msg(deepCopy != el7, 
        "Failure: expected element pointers to differ, got %p and %p instead.",
         deepCopy, el7);
    ck_assert_msg(deepCopy->id == el7->id, 
        "Failure: expected equal element ids, got %d and %d instead.",
         deepCopy->id, el7->id);
    ck_assert_msg(strcmp(deepCopy->name, el7->name) == 0, 
        "Failure: expected equal element names, got %s and %s instead.",
         strcmp(deepCopy->name, el7->name) );

    deepCopy = dpl_get_element_at_index(list_ten_elements_deepCopy, 50);
    ck_assert_msg(deepCopy != el9, 
        "Failure: expected element pointers to differ, got %p and %p instead.",
         deepCopy, el9);
    ck_assert_msg(deepCopy->id == el9->id, 
        "Failure: expected equal element ids, got %d and %d instead.",
         deepCopy->id, el9->id);
    ck_assert_msg(strcmp(deepCopy->name, el9->name) == 0, 
        "Failure: expected equal element names, got %s and %s instead.",
         strcmp(deepCopy->name, el9->name) );
}
END_TEST


/**********************************************************************************************
 * ------------------------------GET_INDEX_OF_ELEMENT------------------------------------------
***********************************************************************************************/

START_TEST(test_getIndexOfElementNULL) {
    ck_assert_msg(dpl_get_index_of_element(list_null, el_one) == -1,
        "Failure: expected -1, got %d instead.",
        dpl_get_index_of_element(list_null, el_one));
}
END_TEST

START_TEST(test_getIndexOfElementEmpty) {
    ck_assert_msg(dpl_get_index_of_element(list_empty, el_one) == -1,
        "Failure: expected -1, got %d instead.",
        dpl_get_index_of_element(list_empty, el_one));
}
END_TEST

START_TEST(test_getIndexOfElementOne_pointer) {
    ck_assert_msg(dpl_get_index_of_element(list_one_element_pointer, el_one) == 0,
        "Failure: expected 0, got %d instead.",
        dpl_get_index_of_element(list_empty, el_one));
}
END_TEST

START_TEST(test_getIndexOfElementOne_deepCopy) {
    ck_assert_msg(dpl_get_index_of_element(list_one_element_deepCopy, el_one) == 0,
        "Failure: expected 0, got %d instead.",
        dpl_get_index_of_element(list_empty, el_one));
}
END_TEST

START_TEST(test_getIndexOfElementMulti_pointer) {
    ck_assert_msg(dpl_get_index_of_element(list_ten_elements_pointer, el0) == 0,
        "Failure: expected 0, got %d instead.",
        dpl_get_index_of_element(list_ten_elements_pointer, el0));
    ck_assert_msg(dpl_get_index_of_element(list_ten_elements_pointer, el7) == 7,
        "Failure: expected 7, got %d instead.",
        dpl_get_index_of_element(list_ten_elements_pointer, el0));
    ck_assert_msg(dpl_get_index_of_element(list_ten_elements_pointer, el9) == 9,
        "Failure: expected 9, got %d instead.",
        dpl_get_index_of_element(list_ten_elements_pointer, el9));
    ck_assert_msg(dpl_get_index_of_element(list_ten_elements_pointer, el_insert) == -1,
        "Failure: expected -1, got %d instead.",
        dpl_get_index_of_element(list_ten_elements_pointer, el_insert));
}
END_TEST

START_TEST(test_getIndexOfElementMulti_deepCopy) {
    ck_assert_msg(dpl_get_index_of_element(list_ten_elements_deepCopy, el0) == 0,
        "Failure: expected 0, got %d instead.",
        dpl_get_index_of_element(list_ten_elements_deepCopy, el0));
    ck_assert_msg(dpl_get_index_of_element(list_ten_elements_deepCopy, el7) == 7,
        "Failure: expected 7, got %d instead.",
        dpl_get_index_of_element(list_ten_elements_deepCopy, el0));
    ck_assert_msg(dpl_get_index_of_element(list_ten_elements_deepCopy, el9) == 9,
        "Failure: expected 9, got %d instead.",
        dpl_get_index_of_element(list_ten_elements_deepCopy, el9));
    ck_assert_msg(dpl_get_index_of_element(list_ten_elements_deepCopy, el_insert) == -1,
        "Failure: expected -1, got %d instead.",
        dpl_get_index_of_element(list_ten_elements_deepCopy, el_insert));
}
END_TEST

/**********************************************************************************************
 * ------------------------------GET_ELEMENT_AT_REFERENCE--------------------------------------
***********************************************************************************************/
//NOTE: This function is tested in GET_REFERENCE_AT INDEX (lines 891 - 1019)

int main(void) {
    Suite *s1 = suite_create("LIST_EX4");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);
    
    // Free
    tcase_add_test(tc1_1, test_ListFree_noCallback);
    tcase_add_test(tc1_1, test_ListFree_callback);
    
    // InsertAtIndex
    tcase_add_test(tc1_1, test_ListInsertAtIndexListNull_noCallback);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListNull_callback);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListEmpty_noCallback);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListEmpty_callback);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListOne_noCallback);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListOne_callback);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListMulti_noCallback);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListMulti_callback);

    // RemoveAtIndex
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListNull_noCallback);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListNull_callback);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListEmpty_noCallback);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListEmpty_callback);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListOne_noCallback_pointer);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListOne_noCallback_deepCopy);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListOne_callback_pointer);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListOne_callback_deepCopy);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListMulti_noCallback_pointer);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListMulti_noCallback_deepCopy);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListMulti_callback_pointer);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListMulti_callback_deepCopy);  

    // getReferenceAtIndex
    tcase_add_test(tc1_1, test_getReferenceAtIndexNULL);
    tcase_add_test(tc1_1, test_getReferenceAtIndexEmpty);
    tcase_add_test(tc1_1, test_getReferenceAtIndexOne_pointer);
    tcase_add_test(tc1_1, test_getReferenceAtIndexOne_deepCopy);
    tcase_add_test(tc1_1, test_getReferenceAtIndexMulti_pointer);
    tcase_add_test(tc1_1, test_getReferenceAtIndexMulti_deepCopy);

    // GetElementAtIndex
    tcase_add_test(tc1_1, test_getElementAtIndexNULL);
    tcase_add_test(tc1_1, test_getElementAtIndexEmpty);
    tcase_add_test(tc1_1, test_getElementAtIndexOne_pointer);
    tcase_add_test(tc1_1, test_getElementAtIndexOne_deepCopy);
    tcase_add_test(tc1_1, test_getElementAtIndexMulti_pointer);
    tcase_add_test(tc1_1, test_getElementAtIndexMulti_deepCopy);

    // GetIndexOfElement
    tcase_add_test(tc1_1, test_getIndexOfElementNULL);
    tcase_add_test(tc1_1, test_getIndexOfElementEmpty);
    tcase_add_test(tc1_1, test_getIndexOfElementOne_pointer);
    tcase_add_test(tc1_1, test_getIndexOfElementOne_deepCopy);
    tcase_add_test(tc1_1, test_getIndexOfElementMulti_pointer);
    tcase_add_test(tc1_1, test_getIndexOfElementMulti_deepCopy);

    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}




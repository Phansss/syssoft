/**
 * \author Pieter Hanssens
 */
#define _GNU_SOURCE

#include "dplist.h"
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define EL_NAME_SIZE 10


typedef struct {
    int id;
    char* name;
} my_element_t;

dplist_t *list_null; //NULL List
dplist_t *list_empty; //Empty List
dplist_t *list_one_element; //List with one element
dplist_t *list_ten_elements; //List with 10 elements

char el_one_name[] = "el_one";
char el0_name[] = "el0";
char el1_name[] = "el1";
char el2_name[] = "el2";
char el3_name[] = "el3";
char el4_name[] = "el4456789";


void* element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);

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
    free((((my_element_t*)*element))->name);
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 : (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);
}


void setup(void) {
    // Implement pre-test setup
    my_element_t* el_one = malloc(sizeof(my_element_t));
    assert(el_one != NULL);
    el_one->id = 1;
    el_one->name = malloc(sizeof(&el_one_name));
    assert(el_one->name != NULL);
 
    my_element_t* el0 = malloc(sizeof(my_element_t));
    assert(el0 != NULL);
    el0->id = 0;
    el0->name = malloc(sizeof(&el0_name));
    assert(el0->name != NULL);
    my_element_t* el1 = malloc(sizeof(my_element_t));
    assert(el1 != NULL);
    el1->id = 1;
    el1->name = malloc(sizeof(&el1_name));
    assert(el1->name != NULL);
    my_element_t* el2 = malloc(sizeof(my_element_t));
    assert(el2 != NULL);
    el2->id = 2;
    el2->name = malloc(sizeof(&el2_name));
    assert(el2->name != NULL);
    my_element_t* el3 = malloc(sizeof(my_element_t));
    assert(el3 != NULL);
    el3->id = 3;
    el3->name = malloc(sizeof(&el3_name));
    assert(el3->name != NULL);
    my_element_t* el4 = malloc(sizeof(my_element_t));
    assert(el4 != NULL);
    el4->id = 4;
    el4->name = malloc(sizeof(el4_name)); 
    assert(el4->name != NULL);
    strcpy(el4->name, el4_name);
    printf("\nhello %s my old friend.\n", el4->name);
    
    list_null = NULL;                 //NULL List

    list_empty = dpl_create(element_copy, 
                            element_free, 
                            element_compare);         //Empty List

    list_one_element = dpl_create(element_copy, 
                            element_free, 
                            element_compare);         //List with one element
    dpl_insert_at_index(list_one_element, &el_one, 0, true);   

    list_ten_elements = dpl_create(element_copy, 
                            element_free, 
                            element_compare);         //List with 10 elements
    /* char element = 'A';
    for (int i = 0; i<10; i++) {   
        dpl_insert_at_index(list_ten_elements, element + i, i, true);
    } */
}

void teardown(void) {
    // Implement post-test teardown
    dpl_free(&list_null, true);
    dpl_free(&list_empty, true);
    dpl_free(&list_one_element, true);
    dpl_free(&list_ten_elements, true);
}
START_TEST(test_ListFree)
    {
        // Test free NULL, don't use callback
        dpl_free(&list_null, false);
        ck_assert_msg(list_null == NULL, "Failure: expected result to be NULL");

        // Test free NULL, use callback
        dpl_free(&list_null, true);
        ck_assert_msg(list_null == NULL, "Failure: expected result to be NULL");

        // Test free empty list, don't use callback
        list_empty = dpl_create(element_copy, element_free, element_compare);
        dpl_free(&list_empty, false);
        ck_assert_msg(list_empty == NULL, "Failure: expected result to be NULL");

        // Test free empty list, use callback
        list_empty = dpl_create(element_copy, element_free, element_compare);
        dpl_free(&list_empty, true);
        ck_assert_msg(list_empty == NULL, "Failure: expected result to be NULL");

        // TODO : Test free with one element, also test if inserted elements are set to NULL

        // TODO : Test free with multiple element, also test if inserted elements are set to NULL

    }
END_TEST

//START_TEST(test_nameOfYourTest)
//  Add other testcases here...
//END_TEST

int main(void) {
    Suite *s1 = suite_create("LIST_EX3");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);
    tcase_add_test(tc1_1, test_ListFree);
    // Add other tests here...

    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * \author Pieter Hanssens
 */

#include "dplist.h"
#include <check.h>
#include <stdlib.h>
#include <stdio.h>

dplist_t *list_null; //NULL List
dplist_t *list_empty; //Empty List
dplist_t *list_one_element; //List with one element
dplist_t *list_ten_elements; //List with 10 elements

void setup(void) {
    // Implement pre-test setup
    list_null = NULL;                 //NULL List

    list_empty = dpl_create();         //Empty List

    list_one_element = dpl_create();         //List with one element
    dpl_insert_at_index(list_one_element, "A", 0);   
    
    list_ten_elements = dpl_create();         //List with 10 elements
    char *element = "A";

    

    for (int i = 1; i<11; i++) {   
        dpl_insert_at_index(list_ten_elements, element + i, i);
    }
    

    //dpl_print(list_ten_elements);
}

void teardown(void) {
    // Implement post-test teardown
    dpl_free(&list_null);
    dpl_free(&list_empty);
    dpl_free(&list_one_element);
    dpl_free(&list_ten_elements);
}


START_TEST(test_ListFree) 
{        // Test free NULL
        dpl_free(&list_null);
        ck_assert_msg(list_null == NULL, "Failure: expected result to be NULL");
        // Test free empty list
        dpl_free(&list_empty);
        ck_assert_msg(list_empty == NULL, "Failure: expected result to be NULL");
        //Test free with one element
        dpl_free(&list_one_element);
        ck_assert_msg(list_one_element == NULL, "Failure: expected result to be NULL");
        //Test free with multiple element
        dpl_free(&list_ten_elements);
        ck_assert_msg(list_ten_elements == NULL, "Failure: expected result to be NULL");
    }
END_TEST

START_TEST(test_ListInsertAtIndexListNULL) {
        // Test inserting at index -1
        dpl_insert_at_index(list_null, "B", -1);
        ck_assert_msg(dpl_size(list_null) == -1, "Failure: NULL expected");
        // Test inserting at index 0
        dpl_insert_at_index(list_null, "B", 0);
        ck_assert_msg(dpl_size(list_null) == -1, "Failure: NULL expected");
        // Test inserting at index 7
        dpl_insert_at_index(list_null, "B", 7);
        ck_assert_msg(dpl_size(list_null) == -1, "Failure: NULL expected");
        // Test inserting at index 99
        dpl_insert_at_index(list_null, "B", 99);
        ck_assert_msg(dpl_size(list_null) == -1, "Failure: NULL expected");
    }
END_TEST

START_TEST(test_ListInsertAtIndexListEmpty) //list_empty
{
    // Test inserting at index -1
    printf("\n%d\n", dpl_size(list_empty));
    dpl_insert_at_index(list_empty, "B", -1);
    ck_assert_msg(dpl_size(list_empty) == 1, "Failure: expected list to have size of 1, got a size of %d",
                                         dpl_size(list_empty));
    dpl_remove_at_index(list_empty, -1);
    
    //dpl_free(&list);
    // Test inserting at index 0
    dpl_insert_at_index(list_empty, "B", 0);
    ck_assert_msg(dpl_size(list_empty) == 1, "Failure: expected list to have size of 1, got a size of %d",
                                         dpl_size(list_empty));
    dpl_remove_at_index(list_empty, 0);

    // Test inserting at index 7
    dpl_insert_at_index(list_empty, "B", 7);
    ck_assert_msg(dpl_size(list_empty) == 1, "Failure: expected list to have size of 1, got a size of %d",
                                         dpl_size(list_empty));
    dpl_remove_at_index(list_empty, 7);

    // Test inserting at index 99
    dpl_insert_at_index(list_empty, "B", 99);
    ck_assert_msg(dpl_size(list_empty) == 1, "Failure: expected list to have size of 1, got a size of %d",
                                         dpl_size(list_empty));
    dpl_remove_at_index(list_empty, 99);

 
}
END_TEST

START_TEST(test_ListInsertAtIndexListOne) //list3
{
    // Test inserting at index -1
    printf("\n%d\n", dpl_size(list_one_element));
    dpl_insert_at_index(list_one_element, "B", -1);
    ck_assert_msg(dpl_size(list_one_element) == 2, "Failure: expected list to have size of 2, got a size of %d",
                                         dpl_size(list_one_element));
    dpl_remove_at_index(list_one_element, -1);
    
    //dpl_free(&list);
    // Test inserting at index 0
    dpl_insert_at_index(list_one_element, "B", 0);
    ck_assert_msg(dpl_size(list_one_element) == 2, "Failure: expected list to have size of 2, got a size of %d",
                                         dpl_size(list_one_element));
    dpl_remove_at_index(list_one_element, 0);

    // Test inserting at index 7
    dpl_insert_at_index(list_one_element, "B", 7);
    ck_assert_msg(dpl_size(list_one_element) == 2, "Failure: expected list to have size of 2, got a size of %d",
                                         dpl_size(list_one_element));
    dpl_remove_at_index(list_one_element, 7);

    // Test inserting at index 99
    dpl_insert_at_index(list_one_element, "B", 99);
    ck_assert_msg(dpl_size(list_one_element) == 2, "Failure: expected list to have size of 2, got a size of %d",
                                         dpl_size(list_one_element));
    dpl_remove_at_index(list_one_element, 99);
} END_TEST

START_TEST(test_ListInsertAtIndexListMulti) //list4
{
    // Test inserting at index -1

    dpl_insert_at_index(list_ten_elements, "B", -1);
    ck_assert_msg(dpl_size(list_ten_elements) == 11, "Failure: expected list to have size of 11, got a size of %d",
                                         dpl_size(list_ten_elements));
    dpl_remove_at_index(list_ten_elements, -1);
    
    //dpl_free(&list);
    // Test inserting at index 0
    dpl_insert_at_index(list_ten_elements, "B", 0);
    ck_assert_msg(dpl_size(list_ten_elements) == 11, "Failure: expected list to have size of 11, got a size of %d",
                                         dpl_size(list_ten_elements));
    dpl_remove_at_index(list_ten_elements, 0);

    // Test inserting at index 7
    dpl_insert_at_index(list_ten_elements, "B", 7);
    ck_assert_msg(dpl_size(list_ten_elements) == 11, "Failure: expected list to have size of 11, got a size of %d",
                                         dpl_size(list_ten_elements));
    dpl_remove_at_index(list_ten_elements, 7);

    // Test inserting at index 99
    dpl_insert_at_index(list_ten_elements, "B", 99);
    ck_assert_msg(dpl_size(list_ten_elements) == 11, "Failure: expected list to have size of 11, got a size of %d",
                                         dpl_size(list_ten_elements));
    dpl_remove_at_index(list_ten_elements, 99);
} END_TEST

START_TEST(test_ListRemoveAtIndexListNULL) {
    // remove at index == -1
    dpl_remove_at_index(list_null, -1);
    ck_assert_msg(list_null == NULL, "Failure: expected list to be NULL");
    // remove at index == 0
    dpl_remove_at_index(list_null, 0);
    ck_assert_msg(list_null == NULL, "Failure: expected list to be NULL");
    // remove at index == 99
    dpl_remove_at_index(list_null, 99);
    ck_assert_msg(list_null == NULL, "Failure: expected list to be NULL");
}
END_TEST

START_TEST(test_ListRemoveAtIndexListEmpty) {
    // remove at index == -1
    dpl_remove_at_index(list_empty, -1);
    ck_assert_msg(dpl_size(list_empty) == 0, "Failure: expected list to be size 0, got %d", dpl_size(list_empty));
    // remove at index == 0
    dpl_remove_at_index(list_empty, 0);
    ck_assert_msg(dpl_size(list_empty) == 0, "Failure: expected list to be size 0, got %d", dpl_size(list_empty));
    // remove at index == 99
    dpl_remove_at_index(list_empty, 99);
    ck_assert_msg(dpl_size(list_empty) == 0, "Failure: expected list to be size 0, got %d", dpl_size(list_empty));
}
END_TEST

START_TEST(test_ListRemoveAtIndexListOne) {
    // remove at index == -1
    dpl_remove_at_index(list_one_element, -1);
    ck_assert_msg(dpl_size(list_one_element) == 0, "Failure: expected list to be size 9, got %d", dpl_size(list_empty));
    // remove at index == 0
    dpl_remove_at_index(list_one_element, 0);
    ck_assert_msg(dpl_size(list_one_element) == 0, "Failure: expected list to be size 9, got %d", dpl_size(list_empty));
    // remove at index == 99
    dpl_remove_at_index(list_one_element, 99);
    ck_assert_msg(dpl_size(list_one_element) == 0, "Failure: expected list to be size 9, got %d", dpl_size(list_empty));
}
END_TEST

START_TEST(test_ListRemoveAtIndexListMulti) {
    // remove at index == -1
    dpl_remove_at_index(list_ten_elements, -1);
    ck_assert_msg(dpl_size(list_ten_elements) == 9, "Failure: expected list to be size 9, got %d", dpl_size(list_ten_elements));
    dpl_insert_at_index(list_ten_elements, "A", -1);
    // remove at index == 0
    dpl_remove_at_index(list_ten_elements, 0);
    ck_assert_msg(dpl_size(list_ten_elements) == 9, "Failure: expected list to be size 9, got %d", dpl_size(list_ten_elements));
    dpl_insert_at_index(list_ten_elements, "A", 0);
    // remove at index == 7
    dpl_remove_at_index(list_ten_elements, 7);
    ck_assert_msg(dpl_size(list_ten_elements) == 9, "Failure: expected list to be size 9, got %d", dpl_size(list_ten_elements));
    dpl_insert_at_index(list_ten_elements, "A", 7);
    // remove at index == 99
    dpl_remove_at_index(list_ten_elements, 99);
    ck_assert_msg(dpl_size(list_ten_elements) == 9, "Failure: expected list to be size 9, got %d", dpl_size(list_ten_elements));
    dpl_insert_at_index(list_ten_elements, "A", 99);
}
END_TEST


/*______dpl_size()_________*/
START_TEST(test_DplSizeNULL) {
    ck_assert_msg(dpl_size(list_null) == -1, "Failure: expected dpl_size to be -1, got %d", dpl_size(list_null));
}
END_TEST

START_TEST(test_DplSizeEmpty) {
    ck_assert_msg(dpl_size(list_empty) == 0, "Failure: expected dpl_size to be 0, got %d", dpl_size(list_empty));
}
END_TEST

START_TEST(test_DplSizeOne) {
    ck_assert_msg(dpl_size(list_one_element) == 1, "Failure: expected dpl_size to be 1, got %d", dpl_size(list_one_element));
}
END_TEST

START_TEST(test_DplSizeMulti) {
    ck_assert_msg(dpl_size(list_ten_elements) == 10, "Failure: expected dpl_size to be 10, got %d", dpl_size(list_ten_elements));
}
END_TEST



/* START_TEST(test_getReferenceAtIndexNULL) {
    ck_assert_msg(dpl_get_reference_at_index(list_null, -1) == 0x00, "Failure: expected dpl_reference to be NULL, got %s", 
                                                                        dpl_get_reference_at_index(list_null, -1));
    ck_assert_msg(dpl_get_reference_at_index(list_null, 0) == 0x00, "Failure: expected dpl_reference to be NULL, got %s", 
                                                                        dpl_get_reference_at_index(list_null, 0));
    ck_assert_msg(dpl_get_reference_at_index(list_null, 7) == 0x00, "Failure: expected dpl_reference to be NULL, got %s", 
                                                                        dpl_get_reference_at_index(list_null, 7));
    ck_assert_msg(dpl_get_reference_at_index(list_null, 99) == 0x00, "Failure: expected dpl_reference to be NULL, got %s", 
                                                                        dpl_get_reference_at_index(list_null, 99));                                                                                                                                        
}
END_TEST

START_TEST(test_getReferenceAtIndexEmpty) {
    ck_assert_msg(dpl_get_reference_at_index(list_empty, -1) == NULL, "Failure: expected dpl_reference to be NULL, got %s", 
                                                                        dpl_get_reference_at_index(list_empty, -1));
    ck_assert_msg(dpl_get_reference_at_index(list_empty, 0) == NULL, "Failure: expected dpl_reference to be NULL, got %s", 
                                                                        dpl_get_reference_at_index(list_empty, 0));
    ck_assert_msg(dpl_get_reference_at_index(list_empty, 7) == NULL, "Failure: expected dpl_reference to be NULL, got %s", 
                                                                        dpl_get_reference_at_index(list_empty, 7));
    ck_assert_msg(dpl_get_reference_at_index(list_empty, 99) == NULL, "Failure: expected dpl_reference to be NULL, got %s", 
                                                                        dpl_get_reference_at_index(list_empty, 99));  
}
END_TEST

START_TEST(test_getReferenceAtIndexOne) {
    ck_assert_msg(dpl_get_reference_at_index(list_one_element, -1) == 0, "Failure: expected dpl_reference to be 0, got %s", 
                                                                        dpl_get_reference_at_index(list_one_element, -1));
    ck_assert_msg(dpl_get_reference_at_index(list_one_element, 0) == 0, "Failure: expected dpl_reference to be 0, got %s", 
                                                                        dpl_get_reference_at_index(list_one_element, 0));
    ck_assert_msg(dpl_get_reference_at_index(list_one_element, 7) == 0, "Failure: expected dpl_reference to be 0, got %s", 
                                                                        dpl_get_reference_at_index(list_one_element, 7));
    ck_assert_msg(dpl_get_reference_at_index(list_one_element, 99) == 0, "Failure: expected dpl_reference to be 0, got %s", 
                                                                        dpl_get_reference_at_index(list_one_element, 99));
}
END_TEST

START_TEST(test_getReferenceAtIndexMulti) {
    
}
END_TEST */


/*______dpl_get_index_of_element()_________*/
START_TEST(test_getIndexOfElementNULL) {
    ck_assert_msg(dpl_get_index_of_element(list_null, "A") == -1, "Failure: expected -1, got %d", 
                                                                dpl_get_index_of_element(list_null, "A"));
}
END_TEST

START_TEST(test_getIndexOfElementEmpty) {
            ck_assert_msg(dpl_get_index_of_element(list_empty, "A") == -1, "Failure: expected -1, got %d", 
                                                                    dpl_get_index_of_element(list_empty, "A"));
}
END_TEST

START_TEST(test_getIndexOfElementOne) {
            ck_assert_msg(dpl_get_index_of_element(list_one_element, "A") == 0, "Failure: expected 0, got %d", 
                                                                    dpl_get_index_of_element(list_one_element, "A"));
            ck_assert_msg(dpl_get_index_of_element(list_one_element, "B") == -1, "Failure: expected -1, got %d", 
                                                                    dpl_get_index_of_element(list_one_element, "B"));
}
END_TEST

START_TEST(test_getIndexOfElementMulti) {
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "A") == 0, "Failure: expected 0, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "A"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "B") == 1, "Failure: expected 1, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "B"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "C") == 2, "Failure: expected 2, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "C"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "D") == 3, "Failure: expected 3, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "D"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "E") == 4, "Failure: expected 4, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "E"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "F") == 5, "Failure: expected 5, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "F"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "G") == 6, "Failure: expected 6, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "G"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "H") == 7, "Failure: expected 7, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "H"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "I") == 8, "Failure: expected 8, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "I"));
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "J") == 9, "Failure: expected 9, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "J"));                                                                                                                                                                                                         
            ck_assert_msg(dpl_get_index_of_element(list_ten_elements, "Z") == -1, "Failure: expected -1, got %d", 
                                                                    dpl_get_index_of_element(list_ten_elements, "Z"));

}
END_TEST



START_TEST(test_getElementAtIndexNULL) {
    ck_assert_msg(dpl_get_element_at_index(list_null, -1) == 0, "Failure: expected 0, got %c", 
                                                                    dpl_get_element_at_index(list_null, -1));

    ck_assert_msg(dpl_get_element_at_index(list_null, 0) == 0, "Failure: expected 0, got %c", 
                                                                    dpl_get_element_at_index(list_null, 0));
}
END_TEST

START_TEST(test_getElementAtIndexEmpty) {
    ck_assert_msg(dpl_get_element_at_index(list_empty, -1) == 0, "Failure: expected 0, got %c", 
                                                                    dpl_get_element_at_index(list_empty, -1));
    ck_assert_msg(dpl_get_element_at_index(list_empty, 0) == 0, "Failure: expected 0, got %c", 
                                                                    dpl_get_element_at_index(list_empty, 0));
    ck_assert_msg(dpl_get_element_at_index(list_empty, 1) == 0, "Failure: expected 0, got %c", 
                                                                    dpl_get_element_at_index(list_empty, 1));

}
END_TEST

START_TEST(test_getElementAtIndexOne) {
    ck_assert_msg(dpl_get_element_at_index(list_one_element, -1) == "A", "Failure: expected \"A\", got %d", 
                                                                    dpl_get_element_at_index(list_one_element, -1));
    ck_assert_msg(dpl_get_element_at_index(list_one_element, 0) == "A", "Failure: expected \"A\", got %d", 
                                                                    dpl_get_element_at_index(list_one_element, 0)); 
    ck_assert_msg(dpl_get_element_at_index(list_one_element, 1) == "A", "Failure: expected \"A\", got %d", 
                                                                    dpl_get_element_at_index(list_one_element, 1)); 
}
END_TEST

START_TEST(test_getElementAtIndexMulti) {
    dpl_print(list_ten_elements);
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 0) == "A", "Failure: expected \"A\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 1) == "B", "Failure: expected \"B\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 2) == "C", "Failure: expected \"C\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 3) == "D", "Failure: expected \"D\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 4) == "E", "Failure: expected \"E\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 5) == "F", "Failure: expected \"F\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 6) == "G", "Failure: expected \"G\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 7) == "H", "Failure: expected \"H\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 8) == "I", "Failure: expected \"I\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 9) == "J", "Failure: expected \"J\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, 99) == "J", "Failure: expected \"J\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
    ck_assert_msg(dpl_get_element_at_index(list_ten_elements, -1) == "A", "Failure: expected \"A\", got %c", 
                                                                    dpl_get_element_at_index(list_ten_elements, 1));
}
END_TEST



int main(void) {
    Suite *s1 = suite_create("LIST_EX1");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);
    tcase_add_test(tc1_1, test_ListFree);

    //InsertAtIndex
    tcase_add_test(tc1_1, test_ListInsertAtIndexListNULL);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListEmpty);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListOne);
    tcase_add_test(tc1_1, test_ListInsertAtIndexListMulti);

    // //RemoveAtIndex
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListNULL);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListEmpty);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListOne);
    tcase_add_test(tc1_1, test_ListRemoveAtIndexListMulti);

    // //DplSize
    tcase_add_test(tc1_1, test_DplSizeNULL);
    tcase_add_test(tc1_1, test_DplSizeEmpty);
    tcase_add_test(tc1_1, test_DplSizeOne);
    tcase_add_test(tc1_1, test_DplSizeMulti);

    // //getReferenceAtIndex
    // tcase_add_test(tc1_1, test_getReferenceAtIndexNULL);
    // tcase_add_test(tc1_1, test_getReferenceAtIndexEmpty);
    // tcase_add_test(tc1_1, test_getReferenceAtIndexOne);
    // tcase_add_test(tc1_1, test_getReferenceAtIndexMulti);

    

    // //GetElementAt
    tcase_add_test(tc1_1, test_getElementAtIndexNULL);
    tcase_add_test(tc1_1, test_getElementAtIndexEmpty);
    tcase_add_test(tc1_1, test_getElementAtIndexOne);
    tcase_add_test(tc1_1, test_getElementAtIndexMulti);

    // Add other tests here...

    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

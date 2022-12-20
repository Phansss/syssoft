
#define _GNU_SOURCE
#define DEBUG


#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "config.h"
#include "datamgr.h"
#include "lib/dplist.h"
#include <errno.h>

FILE* f_sensor_map;
FILE* f_sensor_data;
extern dplist_t* sensor_list; 

void setup(void) {
    f_sensor_map = fopen("room_sensor.map", "r");
    f_sensor_data = fopen("sensor_data", "r");
    //fprintf(f_sensor_data, "%);
    /* int c;
    while(1) {
      c = fgetc(f_sensor_map);
      if( feof(f_sensor_map) ) { 
         break ;
      }
      printf("%c", c);
    }

    while(1) {
      c = fgetc(f_sensor_data);
      if( feof(f_sensor_data) ) { 
         break ;
      }
      printf("%c", c);
    } */

    
}

void teardown(void) {
  fclose(f_sensor_map);
  fclose(f_sensor_data);
}

START_TEST(test_main) {
    int c;


    /* while(1) {
      c = fgetc(f_sensor_map);
      if( feof(f_sensor_map) ) { 
         break ;
      }
      printf("%c", c);
    } */
    datamgr_parse_sensor_files(f_sensor_map, f_sensor_data);
    

} 





int main(void) {
    Suite *s1 = suite_create("LIST_EX4");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);
    
    // Free
    tcase_add_test(tc1_1, test_main);

    srunner_run_all(sr, CK_VERBOSE);

    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}



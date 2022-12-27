#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "sensor_db.h"
#include "config.h"

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main() {
    printf("Running lab 6 with sqlite3 version: %s\n", sqlite3_libversion());
    
    FILE* binary_data = fopen("sensor_data", "r");
    DBCONN* db = init_connection(1);


    time_t curr_time = time(NULL);
    //insert_sensor_from_file(db, binary_data);

    int x = 0;
    x = insert_sensor(db, 20,50, time(NULL));
    x = insert_sensor(db, 21,121, time(NULL));
    x = insert_sensor(db, 22,122, time(NULL));
    x = insert_sensor(db, 23,123, time(NULL));
    x = insert_sensor(db, 24,124, curr_time);

  
    // x = find_sensor_all(db, callback);
    // printf("%s %i\n","query_all_data:",x);
    
    // x = find_sensor_by_value(db, 121, callback);
    // printf("%s %i\n","query_value_data:",x);
    // x= find_sensor_exceed_value(db, 122, callback);
    // printf("%s %i\n","query_valuegt_data:",x);
    x= find_sensor_by_timestamp(db, curr_time, callback);
    printf("%s %i\n","query_ts_data:",x);

    x = find_sensor_after_timestamp(db,1661791192,callback);
    x = find_sensor_after_timestamp(db,1681791192,callback); 
    printf("%s %i\n","query_tsgt_data:",x);

    fclose(binary_data);
    return 0;
}
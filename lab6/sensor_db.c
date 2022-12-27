#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include "sensor_db.h"
#include "config.h"



/**
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME  
 * \param clear_up_flag if the table existed, clear up the existing data when clear_up_flag is set to 1
 * \return the connection for success, NULL if an error occurs
 */
DBCONN *init_connection(char clear_up_flag) {
    printf("Initiating connection...\n");
    sqlite3 *db;
    char *err_msg = 0;
    char* query_reset_table = NULL;

    //open the database connection
    if (sqlite3_open(TO_STRING(DB_NAME), &db) != SQLITE_OK) {
        fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    //RESET TABLE
    if (clear_up_flag == 1) {
        if (asprintf(&query_reset_table, "%s %s %s %s %s",
            "DROP TABLE IF EXISTS", TO_STRING(TABLE_NAME), "; CREATE TABLE",TO_STRING(TABLE_NAME),
            "(Id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_id INT, sensor_value NUMERIC, timestamp INT);") == -1) {
                fprintf(stderr, "Failed to allocate memory for query_reset_table: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                return NULL;
        }
        if (sqlite3_exec(db, query_reset_table, NULL, NULL, &err_msg) != SQLITE_OK) {
            fprintf(stderr, "Failed to execute query_reset_table: %s\n ", err_msg);
        }
        free(query_reset_table);
        query_reset_table = NULL;
        free(err_msg);
        err_msg = NULL;

        }  
    return db;
    
}

/**
 * Disconnect from the database server
 * \param conn pointer to the current connection
 */
void disconnect(DBCONN *conn) {
    sqlite3_close(conn);
}

/**
 * Write an INSERT query to insert a single sensor measurement
 * \param conn pointer to the current connection
 * \param id the sensor id
 * \param value the measurement value
 * \param ts the measurement timestamp
 * \return zero for success, and non-zero if an error occurs
 */
int insert_sensor(DBCONN *conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    char* query_insert_srow;
    char* err_msg;
    if (asprintf(&query_insert_srow, "%s %s %s%d%s %lf%s %li%s", 
            "INSERT INTO", TO_STRING(TABLE_NAME), "(sensor_id, sensor_value, timestamp) VALUES (",
            id,",",value,",", ts,");") ==-1) {
                fprintf(stderr, "Failed to allocate memory for query_insert_srow: %s\n", sqlite3_errmsg(conn));
                return 1;
        }
    printf("%s\n", query_insert_srow);
    if (sqlite3_exec(conn, query_insert_srow, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query_insert_srow: %s\n ", err_msg);
        return 1;
    };
    free(query_insert_srow);
    query_insert_srow = NULL;
    free(err_msg);
    err_msg = NULL;
    return 0;
}

/**
 * Write an INSERT query to insert all sensor measurements available in the file 'sensor_data'
 * \param conn pointer to the current connection
 * \param sensor_data a file pointer to binary file containing sensor data
 * \return zero for success, and non-zero if an error occurs
 */
int insert_sensor_from_file(DBCONN *conn, FILE *sensor_data) {
    sensor_data_t buffer;
    char* query_insert_sdata;
    char* err_msg;

    int i = 0;
    while (fread(&((&buffer)->id), sizeof(sensor_id_t), 1, sensor_data)!= 0) {
        i++;
        fread(&((&buffer)->value), sizeof(sensor_value_t), 1, sensor_data);
        fread(&((&buffer)->ts), sizeof(sensor_ts_t), 1, sensor_data);
        if (asprintf(&query_insert_sdata, "%s %s %s%d%s %lf%s %li%s", 
            "INSERT INTO", TO_STRING(TABLE_NAME), "(sensor_id, sensor_value, timestamp) VALUES (",
            (&buffer)->id,",",(&buffer)->value,",", (&buffer)->ts,");") ==-1) {
                fprintf(stderr, "Failed to allocate memory for query_insert_data: %s\n", sqlite3_errmsg(conn));
                return 1;
        }
        printf("%s\n", query_insert_sdata);
        if (sqlite3_exec(conn, query_insert_sdata, NULL, NULL, &err_msg) != SQLITE_OK) {
            fprintf(stderr, "Failed to execute query_insert_data: %s\n ", err_msg);
            return 1;
        }
    }
    free(query_insert_sdata);
    query_insert_sdata = NULL;
    free(err_msg);
    err_msg = NULL;
    return 0;
}

/**
  * Write a SELECT query to select all sensor measurements in the table 
  * The callback function is applied to every row in the result
  * \param conn pointer to the current connection
  * \param f function pointer to the callback method that will handle the result set
  * \return zero for success, and non-zero if an error occurs
  */
int find_sensor_all(DBCONN *conn, callback_t f) {
    char* query_all_data;
    char* err_msg;
    
    if (asprintf(&query_all_data, "%s %s %s",
        "SELECT * FROM",TO_STRING(TABLE_NAME),";") == -1) {
        fprintf(stderr, "Failed to allocate memory for query_all_data: %s\n", sqlite3_errmsg(conn)); 
        return 1;
    }
    if (sqlite3_exec(conn, query_all_data, f, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query_all_data: %s\n ", err_msg);
        return 1;
    }
    free(query_all_data);
    query_all_data = NULL;
    free(err_msg);
    err_msg = NULL;
    return 0;
}
/**
 * Write a SELECT query to return all sensor measurements having a temperature of 'value'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param value the value to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_value(DBCONN *conn, sensor_value_t value, callback_t f) {
    char* query_value_data;
    char* err_msg;
    
    if (asprintf(&query_value_data, "%s %s %s %lf %s",
        "SELECT * FROM",TO_STRING(TABLE_NAME)," WHERE sensor_value =",value,";") == -1) {
        fprintf(stderr, "Failed to allocate memory for query_value_data: %s\n", sqlite3_errmsg(conn)); 
        return 1;
    }
    if (sqlite3_exec(conn, query_value_data, f, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query_value_data: %s\n ", err_msg);
        return 1;
    }
    free(query_value_data);
    query_value_data = NULL;
    free(err_msg);
    err_msg = NULL;
    return 0;
}

/**
 * Write a SELECT query to return all sensor measurements of which the temperature exceeds 'value'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param value the value to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_exceed_value(DBCONN *conn, sensor_value_t value, callback_t f) {
    char* query_valuegt_data;
    char* err_msg;
    if (asprintf(&query_valuegt_data, "%s %s %s %lf %s",
        "SELECT * FROM",TO_STRING(TABLE_NAME)," WHERE sensor_value >",value,";") == -1) {
        fprintf(stderr, "Failed to allocate memory for query_valuegt_data: %s\n", sqlite3_errmsg(conn)); 
        return 1;
    }
    if (sqlite3_exec(conn, query_valuegt_data, f, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query_valuegt_data: %s\n ", err_msg);
        return 1;
    }
    free(query_valuegt_data);
    query_valuegt_data = NULL;
    free(err_msg);
    err_msg = NULL;
    return 0;
}

/**
 * Write a SELECT query to return all sensor measurements having a timestamp 'ts'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param ts the timestamp to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f) {
    char* query_ts_data;
    char* err_msg;
    
    if (asprintf(&query_ts_data, "%s %s %s %li %s",
        "SELECT * FROM",TO_STRING(TABLE_NAME)," WHERE timestamp =",ts,";") == -1) {
        fprintf(stderr, "Failed to allocate memory for query_ts_data: %s\n", sqlite3_errmsg(conn)); 
        return 1;
    }
    if (sqlite3_exec(conn, query_ts_data, f, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query_ts_data: %s\n ", err_msg);
        return 1;
    }
    free(query_ts_data);
    query_ts_data = NULL;
    free(err_msg);
    err_msg = NULL;
    return 0;
}

/**
 * Write a SELECT query to return all sensor measurements recorded after timestamp 'ts'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param ts the timestamp to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_after_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f) {
    char* query_tsgt_data;
    char* err_msg;
    
    if (asprintf(&query_tsgt_data, "%s %s %s %li %s",
        "SELECT * FROM",TO_STRING(TABLE_NAME)," WHERE timestamp >",ts,";") == -1) {
        fprintf(stderr, "Failed to allocate memory for query_tsgt_data: %s\n", sqlite3_errmsg(conn)); 
        return 1;
    }
    if (sqlite3_exec(conn, query_tsgt_data, f, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query_tsgt_data: %s\n ", err_msg);
        return 1;
    }
    free(query_tsgt_data);
    query_tsgt_data = NULL;
    free(err_msg);
    err_msg = NULL;
    return 0;
}

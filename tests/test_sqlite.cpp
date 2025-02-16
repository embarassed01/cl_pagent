#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main() {
    printf("test sqlite3\n");
    printf("sqlite3 version: %s\n", sqlite3_libversion());  // 3.48.0

    // write to db
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open("foo.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return EXIT_FAILURE;
    }
    char *sql = "DROP TABLE IF EXISTS Cars;"
        "CREATE TABLE Cars(Id INT, Name TEXT, Price INT);"
        "INSERT INTO Cars VALUES(1, 'Audi', 52642);"
        "INSERT INTO Cars VALUES(2, 'Me', 111111)";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return EXIT_FAILURE;
    }
    sqlite3_close(db);

    // read from db
    rc = sqlite3_open("foo.db", &db);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return EXIT_FAILURE;
    }
    sql = "SELECT * FROM Cars";
    rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return EXIT_FAILURE;
    } 
    sqlite3_close(db);
    /*
    callback output:
Id = 1
Name = Audi
Price = 52642

Id = 2
Name = Me
Price = 111111
    */
    return 0;
}
#include <assert.h> // assert
#include <stdio.h>  // printf
#include <string.h> // snprintf
#include <sqlite3.h>
#include "flib.h"
#include "database.h"
#include "model.h"

struct database {
    sqlite3 *conn;
    sqlite3_stmt *stmt;
    const char *err;
    int failed;
};

static void check_failure(struct database *db, int last_result)
{
    assert(db);
    
    if (db->failed)
        return;
    
    switch (last_result) {
        case SQLITE_OK:
        case SQLITE_ROW:
        case SQLITE_DONE:
        break;
        default:
        db->err = sqlite3_errmsg(db->conn);
        db->failed = 1;
        printf("query error %s\n", db->err);
        break;
    }
}

static void begin_query(struct database *db, char *query)
{
    assert(db);
    assert(query);
    // open conn if required.
    if (!db->conn) {
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX;
        int result = sqlite3_open_v2("db", &db->conn, flags, 0);
        check_failure(db, result);
    }
    printf("query '%s'\n", query);
    check_failure(db, sqlite3_prepare_v2(db->conn, query, -1, &db->stmt, 0));
}

static int execute(struct database *db)
{
    assert(db);
    int result = sqlite3_step(db->stmt);
    check_failure(db, result);
    return result;
}

static void end_query(struct database *db)
{
    assert(db);
    if (db->stmt) {
        check_failure(db, sqlite3_finalize(db->stmt));
        db->stmt = 0;
    }
    if (db->conn) {
        check_failure(db, sqlite3_close(db->conn));
        db->conn = 0;
    }
}

static void bind_double(struct database *db, char *key, double value)
{
    assert(db);
    assert(key);
    int key_index = sqlite3_bind_parameter_index(db->stmt, key);
    check_failure(db, sqlite3_bind_double(db->stmt, key_index, value));
}

static void bind_long(struct database *db, char *key, long long value)
{
    assert(db);
    assert(key);
    int key_index = sqlite3_bind_parameter_index(db->stmt, key);
    check_failure(db, sqlite3_bind_int64(db->stmt, key_index, value));
}

static void bind_int(struct database *db, char *key, int value)
{
    assert(db);
    assert(key);
    int key_index = sqlite3_bind_parameter_index(db->stmt, key);
    check_failure(db, sqlite3_bind_int(db->stmt, key_index, value));
}

static void bind_text(struct database *db, char *key, char *value)
{
    assert(db);
    assert(key);
    assert(value);
    int key_index = sqlite3_bind_parameter_index(db->stmt, key);
    check_failure(db, sqlite3_bind_text(db->stmt, key_index, value, -1, 0));
}

static void bind_text_n(struct database *db, char *key, char *value, int n)
{
    assert(db);
    assert(key);
    assert(value);
    int key_index = sqlite3_bind_parameter_index(db->stmt, key);
    check_failure(db, sqlite3_bind_text(db->stmt, key_index, value, n, 0));
}

static void fields_to(void *dest, 
                      struct database *db, 
                      char *table, 
                      struct field *fields,
                      unsigned long long fields_count)
{
    assert(dest);
    assert(db);
    assert(fields);
    int columns = sqlite3_column_count(db->stmt);
    for (int i = 0; i < columns; i++) {
        const char *colum_name = sqlite3_column_name(db->stmt, i);
        for (unsigned long long j = 0; j < fields_count; j++) {
            if (!fields[j].name ||
                !colum_name ||
                !str_ends_with((char *) fields[j].table, (char *) table) ||
                !str_equals((char *) fields[j].name, (char *) colum_name))
                continue;
            switch (fields[j].type) {
                case type_float:
                *(float *)(dest + fields[j].offset) = (float) sqlite3_column_double(db->stmt, i);
                break;
                
                case type_double:
                *(double *)(dest + fields[j].offset) = sqlite3_column_double(db->stmt, i);
                break;
                
                case type_int:
                *(int *)(dest + fields[j].offset) = sqlite3_column_int(db->stmt, i);
                break;
                
                case type_long:
                *(long long *)(dest + fields[j].offset) = sqlite3_column_int64(db->stmt, i);
                break;
                
                case type_text: {
                    const char *src = sqlite3_column_text(db->stmt, i);
                    if (src)
                        strncpy(dest + fields[j].offset, src, fields[j].size);
                } break;
                
                default:
                break;
            }
        }
    }
}

int database_find(void *dest, 
                  unsigned long long stride, 
                  struct database *db, 
                  struct field *fields,
                  unsigned long long fields_count,
                  unsigned long long limit, char *table,
                  char *query, 
                  ...)
{
    va_list va;
    va_start(va, query);
    char *parsed_query = sqlite3_vmprintf(query, va);
    va_end(va);
    
    // check if failed to allocate memory.
    if (!parsed_query)
        return 0;
    
    if (!db)
        db = &(struct database) {0};
    
    begin_query(db, parsed_query);
    // just wants to execute the query and
    // ignore the result.
    if (limit == 0)
        execute(db);
    for (unsigned long long i = 0; i < limit; i++) {
        execute(db);
        fields_to(dest, db, table, fields, fields_count);
        (byte *)dest += stride;
    }
    sqlite3_free(parsed_query);
    end_query(db);
    
    return !db->failed;
}

int database_create(struct database *db, 
                    char *table, 
                    struct field *fields, 
                    unsigned long long fields_count, 
                    void *src)
{
    assert(table);
    assert(fields);
    assert(src);
    
    if (!db)
        db = &(struct database) {0};
    
    unsigned int column_count = 0;
    struct field *columns = 0;
    
    // find columns.
    for (unsigned long long i = 0; i < fields_count; i++) {
        if (!str_ends_with((char *) fields[i].table, (char *) table))
            continue;
        columns = fields + i;
        column_count++;
    }
    // no columns found?
    if (!columns)
        return 0;
    // go back to the first column.
    columns -= column_count;
    columns++;
    // build "?, ?, ? ..." string (each ? represents a column)
    char column_names[128] = {0};
    char values[32] = {0};
    for (unsigned i = 0; i < column_count; i++) {
        strf_ex(column_names, 
                "%s%s%s",
                column_names,
                columns[i].name,
                i + 1 < column_count ? "," : "");
        strf_ex(values, 
                "%s?%s",
                values,
                i + 1 < column_count ? "," : "");
    }
    char query[256] = {0};
    strf_ex(query, 
            "insert into %s(%s) values (%s);",
            table,
            column_names,
            values);
    
    begin_query(db, query);
    
    for (unsigned int i = 0; i < column_count; i++) {
        char column_key[64] = {0};
        snprintf(column_key, sizeof(column_key) - 1, ":%s", columns[i].name);
        void *field = (src + columns[i].offset);
        switch (columns[i].type) {
            case type_float:
            check_failure(db, sqlite3_bind_double(db->stmt, i+1, *(float *) field));
            break;
            
            case type_double:
            check_failure(db, sqlite3_bind_double(db->stmt, i+1, *(double *) field));
            break;
            
            case type_int:
            check_failure(db, sqlite3_bind_int(db->stmt, i+1, *(int *) field));
            break;
            
            case type_long:
            check_failure(db, sqlite3_bind_int64(db->stmt, i+1, *(long long *) field));
            break;
            
            case type_text:
            check_failure(db, sqlite3_bind_text(db->stmt, i+1, field, -1, 0));
            break;
            
            default:
            break;
        }
    }
    execute(db);
    end_query(db);
    return !db->failed;
}

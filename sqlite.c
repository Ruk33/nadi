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
    
    if (!parsed_query)
        return 0;
    
    if (!db)
        db = &(struct database) {0};
    
    begin_query(db, parsed_query);
    for (unsigned long long i = 0; i < limit; i++) {
        execute(db);
        fields_to(dest, db, table, fields, fields_count);
        (byte *)dest += stride;
    }
    sqlite3_free(parsed_query);
    end_query(db);
    
    return !db->failed;
}

int database_create(struct database *db, char *table, struct field *fields, int fields_count, void *src)
{
    assert(table);
    assert(fields);
    assert(src);
    
    if (!db)
        db = &(struct database) {0};
    
    char query[256] = {0};
    int written = 0;
    int tmp = snprintf(query, sizeof(query) - 1, "insert into %s (", table);
    tmp = tmp == -1 ? 0 : tmp;
    written += tmp;
    for (int i = 0; i < fields_count; i++) {
        tmp = snprintf(query + written, 
                       sizeof(query) - written - 1, 
                       "%s%s", 
                       fields[i].name,
                       i + 1 < fields_count ? "," : "");
        tmp = tmp == -1 ? 0 : tmp;
        written += tmp;
    }
    tmp = snprintf(query + written, sizeof(query) - written - 1, ") values (");
    tmp = tmp == -1 ? 0 : tmp;
    written += tmp;
    for (int i = 0; i < fields_count; i++) {
        tmp = snprintf(query + written, 
                       sizeof(query) - written - 1, 
                       ":%s%s", 
                       fields[i].name,
                       i + 1 < fields_count ? "," : "");
        tmp = tmp == -1 ? 0 : tmp;
        written += tmp;
    }
    tmp = snprintf(query + written, sizeof(query) - written - 1, ");");
    tmp = tmp == -1 ? 0 : tmp;
    written += tmp;
    
    begin_query(db, query);
    
    for (int i = 0; i < fields_count; i++) {
        char column_key[64] = {0};
        snprintf(column_key, sizeof(column_key) - 1, ":%s", fields[i].name);
        switch (fields[i].type) {
            case type_float:
            bind_double(db, column_key, *(float *)(src + fields[i].offset));
            break;
            
            case type_double:
            bind_double(db, column_key, *(double *)(src + fields[i].offset));
            break;
            
            case type_int:
            bind_int(db, column_key, *(int *)(src + fields[i].offset));
            break;
            
            case type_long:
            bind_long(db, column_key, *(long *)(src + fields[i].offset));
            break;
            
            case type_text:
            bind_text_n(db, column_key, (char *)(src + fields[i].offset), fields[i].size);
            break;
            
            default:
            break;
        }
    }
    execute(db);
    end_query(db);
    return !db->failed;
}

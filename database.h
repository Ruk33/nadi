// will be defined by the database
// implementation, for example, sqlite.
struct database;

struct field {
    const char *table;
    // name of the column/field.
    const char *name;
    // offset to property/field.
    unsigned long long offset;
    // how much can be stored/copied.
    unsigned long long size;
    // size of the whole parent.
    // unsigned long long table_size;
    // type of value.
    enum {
        type_float,
        type_double,
        type_int,
        type_long,
        type_text,
    } type;
};

#define field_float(type, name) \
{#type, #name, offsetof(type, name), sizeof(((type *)(0))->name), type_float,}
#define field_double(type, name) \
{#type, #name, offsetof(type, name), sizeof(((type *)(0))->name), type_double,}
#define field_int(type, name) \
{#type, #name, offsetof(type, name), sizeof(((type *)(0))->name), type_int,}
#define field_long(type, name) \
{#type, #name, offsetof(type, name), sizeof(((type *)(0))->name), type_long,}
#define field_text(type, name) \
{#type, #name, offsetof(type, name), sizeof(((type *)(0))->name), type_text,}
#define field_text_n(type, name, size) \
{#type, #name, offsetof(type, name), size, type_text,}

#define find(dest, type, ...) \
database_find((dest), sizeof(type), 0, fields, sizeof(fields) / sizeof(*fields), 1, #type, __VA_ARGS__)

#define find_many(dest, type, n, ...) \
database_find((dest), sizeof(type), 0, fields, sizeof(fields) / sizeof(*fields), (n), #type, __VA_ARGS__)

#define query(...) \
database_find(0, 0, 0, 0, 0, 0, 0, __VA_ARGS__)

#define create(table, src) \
database_create(0, #table, fields, sizeof(fields)/sizeof(*fields), src)

int database_find(void *dest, 
                  unsigned long long stride, 
                  struct database *db, 
                  struct field *fields,
                  unsigned long long fields_count,
                  unsigned long long limit, 
                  char *table,
                  char *query, 
                  ...);

int database_create(struct database *db, 
                    char *table, 
                    struct field *fields, 
                    unsigned long long fields_count, 
                    void *src);

// will be defined by the database
// implementation, for example, sqlite.
struct database;

struct field {
    // name of the column/field.
    const char *name;
    // offset to property/field.
    unsigned long long offset;
    // how much can be stored/copied.
    unsigned long long size;
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
{#name, offsetof(type, name), sizeof(((type *)(0))->name), type_float,}
#define field_double(type, name) \
{#name, offsetof(type, name), sizeof(((type *)(0))->name), type_double,}
#define field_int(type, name) \
{#name, offsetof(type, name), sizeof(((type *)(0))->name), type_int,}
#define field_long(type, name) \
{#name, offsetof(type, name), sizeof(((type *)(0))->name), type_long,}
#define field_text(type, name) \
{#name, offsetof(type, name), sizeof(((type *)(0))->name) - 1, type_text,}
#define field_text_n(type, name, size) \
{#name, offsetof(type, name), size, type_text,}

#define find(dest, table, id) \
database_find(dest, 0, #table, id, table##_fields, sizeof(table##_fields)/sizeof(*table##_fields))

#define create(table, src) \
database_create(0, #table, table##_fields, sizeof(table##_fields)/sizeof(*table##_fields), src)

int database_find(void *dest, struct database *db, char *table, int id, struct field *fields, int fields_count);

int database_create(struct database *db, char *table, struct field *fields, int fields_count, void *src);

struct user {
    int id;
    char name[32];
    char pass[32];
    int age;
};

struct log {
    char severity[32];
    char message[256];
};

static struct field fields[] = {
    field_int(struct user, id),
    field_text(struct user, name),
    field_text(struct user, pass),
    field_int(struct user, age),
    
    field_text(struct log, severity),
    field_text(struct log, message),
};

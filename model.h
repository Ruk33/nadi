struct user {
    int id;
    char name[32];
    char pass[32];
    int age;
};

struct field user_fields[] = {
    field_int(struct user, id),
    field_text(struct user, name),
    field_text(struct user, pass),
    field_int(struct user, age),
};

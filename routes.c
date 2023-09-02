#include "nadi.h"

void create_user(struct http_response *response)
{
    struct user new_user = {.id=0, .name="hola", .pass="", .age=42};
    create(user, &new_user);
    log_info("new user %s created", new_user.name);
    
    header(response, "Content-Type", "text/html");
    response(response, 
             "200 OK", 
             "
             the new user is %s
             <b>and we can send some html too!</b>
             ", 
             new_user.name);
}

void get_user(struct http_response *response)
{
    struct user first_user = {0};
    find(&first_user, 
         struct user, 
         "select * from user where name = '%q' limit 1;", 
         "franco");
    
    query("select 1;");
    // log_info("new user %s created", new_user.name);
    
    struct user first_two_users[2] = {0};
    int to_find = 2;
    find_many(first_two_users,
              struct user,
              to_find,
              "select * from user limit %d;", 
              to_find);
    
    header(response, "Content-Type", "text/html");
    response(response, "200 OK", "first user is %s", first_user.name);
}

void not_found(struct http_response *response)
{
    printf("404 hit!" nl);
    
    header(response, "Content-Type", "text/html");
    response(response, "404 Not found", "%s", "oops?...");
}

void routes(struct route_list *routes)
{
    get(routes, "/create", create_user);
    get(routes, "/get", get_user);
}
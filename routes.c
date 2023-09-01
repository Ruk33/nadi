#include "nadi.h"

void create_user(struct http_response *response)
{
    header(response, "Content-Type", "text/html");
    
    struct user new_user = {.id=0, .name="foo", .pass="", .age=42};
    create(user, &new_user);
    response(response, "200 OK", "the new user is %s", new_user.name);
}

void get_user(struct http_response *response)
{
    header(response, "Content-Type", "text/html");
    
    struct user first_user = {0};
    find(&first_user, user, 0);
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
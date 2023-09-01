#include "nadi.h"

void greeting(struct http_response *response)
{
    header(response, "Content-Type", "text/html");
    response(response, 0, "hello! this is my response!!", 0);
}

void not_found(struct http_response *response)
{
    printf("404 hit!." nl);
    
    struct user franco = {0};
    // find(&franco, user, 42);
    strf_ex(franco.name, "%s", "yay!");
    create(user, &franco);
    
    header(response, "Content-Type", "text/html");
    response(response, "404 Not found", "name is %s", franco.name);
}

void routes(struct route_list *routes)
{
    get(routes, "/greeting", greeting);
}
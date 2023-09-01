struct http_response;
typedef void route_cb(struct http_response *);

struct route {
    char path[256];
    route_cb *cb;
    int verb;
};

struct route_list {
    struct route routes[32];
    int count;
};

#define get(dest, path, cb) \
dest->routes[dest->count++] = (struct route) {path, cb, METHOD_GET}
#define post(dest, path, cb) \
dest->routes[dest->count++] = (struct route) {path, cb, METHOD_POST}
#define put(dest, path, cb) \
dest->routes[dest->count++] = (struct route) {path, cb, METHOD_PUT}
#define delete(dest, path, cb) \
dest->routes[dest->count++] = (struct route) {path, cb, METHOD_DELETE}

void not_found(struct http_response *dest);
void routes(struct route_list *dest);

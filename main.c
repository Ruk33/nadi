#include <assert.h> // assert
#include <stdlib.h> // calloc, size_t
#include <unistd.h> // close
#include <string.h> // memcpy
#include <stdio.h> // printf
#include "asocket.h"
#include "http_request.h"
#include "http_response.h"
#include "flib.h"
#include "database.h"

// max routes that can be defined.
#define max_routes 64
// max body size in bytes.
#define max_body 1024
// max requests to be handled
// at the same time.
#define max_requests 8
// max headers to receive/send per request.
#define max_headers 32

struct body {
    char buf[max_body];
    uint size;
};

struct response {
    struct body body;
};

struct request {
    int fd;
    struct body body;
    struct response response;
    uint written;
    uint to_write;
    struct http_header headers[max_headers];
    uint headers_count;
    uint used;
};

typedef void route_cb(struct request *);
void not_found(struct request *);

struct uri {
    char buf[64];
};

struct route {
    struct uri path;
    route_cb *cb;
    enum http_request_method verb;
};

struct program {
    // routes
    struct route routes[max_routes];
    uint routes_count;
    
    // requests
    struct request requests[max_requests];
};

static struct program *program = 0;

// just some nice route aliasing.
#define get(path, cb) route(&(struct route) {path, cb, METHOD_GET})
#define post(path, cb) route(&(struct route) {path, cb, METHOD_POST})
#define put(path, cb) route(&(struct route) {path, cb, METHOD_PUT})
#define delete(path, cb) route(&(struct route) {path, cb, METHOD_DELETE})

void route(struct route *src)
{
    assert(src);
    program->routes[program->routes_count++] = *src;
}

void header(struct request *req, char *key, char *val)
{
    if (!req || !key || !val)
        return;
    if (req->headers_count + 1 >= max_headers)
        return;
    struct http_header *h = req->headers + req->headers_count;
    strncpy(h->key, key, sizeof(h->key) - 1);
    strncpy(h->val, val, sizeof(h->val) - 1);
    req->headers_count++;
}

void response2(struct request *req, char *status, char *content)
{
    if (!req)
        return;
    if (!status)
        status = "200 OK";
    if (!content)
        content = "";
    req->to_write = http_response_encode(req->response.body.buf,
                                         sizeof(req->response.body.buf), 
                                         status, 
                                         req->headers,
                                         req->headers_count, 
                                         content);
}

#define response(req, status, content, ...) \
do { \
response2(req, status, 0); \
req->to_write += strf(req->response.body.buf + req->to_write, \
sizeof(req->response.body.buf) - req->to_write, \
content, \
__VA_ARGS__); \
} while (0)

#define find(dest, table, id) \
database_find(dest, 0, #table, id, table##_fields, sizeof(table##_fields)/sizeof(*table##_fields))

#define create(table, src) \
database_create(0, #table, table##_fields, sizeof(table##_fields)/sizeof(*table##_fields), src)

struct request *get_or_create_request_for(int fd)
{
    struct request *request = 0;
    for_each(struct request, req, program->requests) {
        // found the request used by this fd.
        if (req->fd == fd)
            return req;
        // found a free request that can be used
        // in case we don't find a request with this
        // fd.
        if (!request && !req->used)
            request = req;
    }
    // at full capacity?
    if (!request)
        return 0;
    // clean the new request so we start fresh.
    memset(request, 0, sizeof(*request));
    request->fd = fd;
    request->used = 1;
    return request;
}

void handle_event(int fd, enum asocket_event ev, void *buf, size_t len)
{
    switch (ev) {
        case ASOCKET_NEW_CONN: {
            struct request *request = get_or_create_request_for(fd);
            if (!request)
                close(fd);
        } break;
        
        case ASOCKET_CLOSED: {
            struct request *request = get_or_create_request_for(fd);
            if (request)
                memset(request, 0, sizeof(*request));
        } break;
        
        case ASOCKET_READ: {
            struct request *request = get_or_create_request_for(fd);
            assert(request);
            // don't read more than what the buffer can hold.
            len = min((size_t) (max_body - request->body.size), len);
            // read.
            memcpy(request->body.buf + request->body.size, buf, len);
            // update size and guarantee null terminator.
            request->body.size += len;
            request->body.buf[request->body.size] = 0;
            // only handle complete requests.
            if (http_request_is_partial(request->body.buf))
                return;
            // search for route or 404.
            for_each(struct route, route, program->routes) {
                if (http_request_get_method(request->body.buf) != route->verb)
                    continue;
                if (!http_request_matches_path(request->body.buf, route->path.buf))
                    continue;
                // handle request and prevent 404.
                route->cb(request);
                return;
            }
            // 404.
            not_found(request);
        } break;
        
        case ASOCKET_CAN_WRITE: {
            struct request *request = get_or_create_request_for(fd);
            assert(request);
            if (request->to_write == request->written)
                return;
            request->written += asocket_write(fd, request->response.body.buf + request->written, request->to_write - request->written);
            if (request->to_write == request->written) {
                memset(request, 0, sizeof(*request));
                close(fd);
            }
        } break;
        
        default:
        break;
    }
}

///////

void greeting(struct request *req)
{
    header(req, "Content-Type", "text/html");
    response(req, 0, "hello! this is my response!!", 0);
}

struct user {
    char name[32];
    // struct user_name { char buf[32]; } name;
    char pass[32];
    int age;
};

struct field user_fields[] = {
    field_text(struct user, name),
    field_text(struct user, pass),
    field_int(struct user, age),
};

void not_found(struct request *req)
{
    printf("404 hit!." nl);
    
    struct user franco = {0};
    // find(&franco, user, 42);
    strf_ex(franco.name, "%s", "yay!");
    create(user, &franco);
    
    header(req, "Content-Type", "text/html");
    response(req, "404 Not found", "name is %s", franco.name);
}

///////

int main()
{
    program = calloc(1, sizeof(*program));
    assert(program);
    
    // run migrations.
    // migrations();
    
    // routes
    get("/greeting", greeting);
    
    // start!
    int fd = asocket_port(8080);
    asocket_listen(fd, handle_event);
    
    return 0;
}

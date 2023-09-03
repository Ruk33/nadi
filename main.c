#include <assert.h> // assert
#include <stdlib.h> // calloc, size_t
#include <unistd.h> // close
#include <string.h> // memcpy
#include "asocket.h"
#include "nadi.h"

struct conn {
    int fd;
    char request[1024];
    int request_size;
    struct http_response response;
    int written;
    int used;
};

struct program {
    struct route_list routes;
    struct conn connections[8];
};

static struct program *program = 0;

struct conn *get_or_create_conn_for(int fd)
{
    struct conn *free_conn = 0;
    for_each(struct conn, conn, program->connections) {
        // found the request used by this fd.
        if (conn->fd == fd)
            return conn;
        // found a free request that can be used
        // in case we don't find a request with this
        // fd.
        if (!free_conn && !conn->used)
            free_conn = conn;
    }
    // at full capacity?
    if (!free_conn)
        return 0;
    // clean the new request so we start fresh.
    memset(free_conn, 0, sizeof(*free_conn));
    free_conn->fd = fd;
    free_conn->used = 1;
    return free_conn;
}

void handle_event(int fd, enum asocket_event ev, void *buf, size_t len)
{
    struct conn *conn = get_or_create_conn_for(fd);
    switch (ev) {
        case ASOCKET_NEW_CONN: {
            if (!conn)
                close(fd);
        } break;
        
        case ASOCKET_CLOSED: {
            if (conn)
                memset(conn, 0, sizeof(*conn));
        } break;
        
        case ASOCKET_READ: {
            if (!conn)
                return;
            // don't read more than what the buffer can hold.
            len = min((size_t) (sizeof(conn->request) - conn->request_size), 
                      len);
            // read.
            memcpy(conn->request + conn->request_size, buf, len);
            // update size and guarantee null terminator.
            conn->request_size += len;
            conn->request[conn->request_size] = 0;
            // only handle complete requests.
            if (http_request_is_partial(conn->request))
                return;
            // search for route or 404.
            for_each(struct route, route, program->routes.routes) {
                if (http_request_get_method(conn->request) != route->verb)
                    continue;
                if (!http_request_matches_path(conn->request, route->path))
                    continue;
                // handle request and prevent 404.
                route->cb(&conn->response);
                return;
            }
            // 404.
            not_found(&conn->response);
        } break;
        
        case ASOCKET_CAN_WRITE: {
            if (!conn)
                return;
            if (conn->response.body_size == conn->written)
                return;
            void *resume_from = conn->response.body + conn->written;
            size_t to_be_sent = conn->response.body_size - conn->written;
            conn->written += asocket_write(fd, resume_from, to_be_sent);
            // when the entire response has been sent, close 
            // the connection.
            if (conn->response.body_size == conn->written) {
                memset(conn, 0, sizeof(*conn));
                close(fd);
            }
        } break;
        
        default:
        break;
    }
}

int main()
{
    program = calloc(1, sizeof(*program));
    assert(program);
    
    // reserve some memory
    memory_reserve(10 mb);
    
    // routes
    routes(&program->routes);
    
    // start!
    int fd = asocket_port(8080);
    asocket_listen(fd, handle_event);
    
    return 0;
}

struct http_header {
    char key[256];
    char val[8192];
};

struct http_response {
    struct http_header headers[32];
    int headers_count;
    
    char body[1024];
    int body_size;
};

#define response(dest, status, content, ...) \
do { \
response_raw(dest, status, 0); \
dest->body_size += strf(dest->body + dest->body_size, \
sizeof(dest->body) - dest->body_size, \
content, \
__VA_ARGS__); \
} while (0)

void header(struct http_response *dest, char *key, char *val);

void response_raw(struct http_response *dest, char *status, char *content);

// return number of bytes written to dest (INCLUDING null terminator)
int http_response_encode(struct http_response *dest, char *status, char *body);

#include <string.h> // strncpy
#include <stdio.h> // snprintf
#include "flib.h"
#include "http_response.h"

void header(struct http_response *dest, char *key, char *val)
{
    if (!dest || !key || !val)
        return;
    if (dest->headers_count + 1 >= array_length(dest->headers))
        return;
    struct http_header *header = dest->headers + dest->headers_count;
    strncpy(header->key, key, sizeof(header->key) - 1);
    strncpy(header->val, val, sizeof(header->val) - 1);
    dest->headers_count++;
}

void response_raw(struct http_response *dest, char *status, char *content)
{
    if (!dest)
        return;
    if (!status)
        status = "200 OK";
    if (!content)
        content = "";
    dest->body_size = http_response_encode(dest, status, content);
}

int http_response_encode(struct http_response *dest, char *status, char *body)
{
    if (!dest)
        return 0;
    if (!status)
        status = "200 OK";
    if (!body)
        body = "";
    int result = snprintf(dest->body, sizeof(dest->body) - 1, "HTTP/1.1 %s\r\n", status);
    result = result == -1 ? 0 : result;
    dest->body_size = result;
    for (size_t i = 0; i < dest->headers_count; i++) {
        result = snprintf(dest->body + dest->body_size,  sizeof(dest->body) - dest->body_size - 1, "%s: %s\r\n", dest->headers[i].key, dest->headers[i].val);
        result = result == -1 ? 0 : result;
        dest->body_size += result;
    }
    result = snprintf(dest->body + dest->body_size,  sizeof(dest->body) - dest->body_size - 1, "\r\n%s", body);
    result = result == -1 ? 0 : result;
    dest->body_size += result;
    // null terminator.
    dest->body[sizeof(dest->body) - 1] = 0;
    return dest->body_size;
}

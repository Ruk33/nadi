#include <stdio.h> // snprintf
#include "http_response.h"

int http_response_encode(char *dest, size_t dest_size, char *status, struct http_header *headers, size_t headers_count, char *body)
{
    int written = 0;
    if (!dest)
        return written;
    if (!status)
        return written;
    if (!headers)
        headers_count = 0;
    if (!body)
        body = "";
    int result = 0;
    result = snprintf(dest, dest_size, "HTTP/1.1 %s\r\n", status);
    result = result == -1 ? 0 : result;
    written += result;
    dest += result;
    dest_size -= result;
    for (size_t i = 0; i < headers_count; i++) {
        result = snprintf(dest, dest_size, "%s: %s\r\n", headers[i].key, headers[i].val);
        result = result == -1 ? 0 : result;
        written += result;
        dest += result;
        dest_size -= result;
    }
    // empty space for null terminator.
    result = snprintf(dest, dest_size, "\r\n%s ", body);
    result = result == -1 ? 0 : result;
    written += result;
    dest += result;
    dest_size -= result;
    *dest = 0;
    return written;
}

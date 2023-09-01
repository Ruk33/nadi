struct http_header {
    char key[256];
    char val[8192];
};

// return number of bytes written to dest (INCLUDING null terminator)
int http_response_encode(char *dest, size_t dest_size, char *status, struct http_header *headers, size_t headers_count, char *body);

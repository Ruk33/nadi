#include <assert.h>
#include <stdio.h>
#include "database.h"
#include "model.h"
#include "flib.h"
#include "http_request.h"
#include "http_response.h"
#include "routes.h"
#include "memory.h"

// as seconds
#define minutes * 60
#define minute minutes
#define days * 60 * 24
#define day days
#define weeks * 60 * 24 * 7
#define week weeks

// as bytes
#define kb * 1024
#define mb * 1024 * 1024

#define make(type, name) \
type *name = memory_get(sizeof(type))

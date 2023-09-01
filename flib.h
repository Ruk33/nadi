#include <stdarg.h>

// sometimes i get tired of writing "unsigned"...
typedef unsigned char byte;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#define array_length(x) \
((sizeof(x) / sizeof(*(x))))

#define for_each(type, x, arr) \
for (type *x = (arr); x < (arr) + array_length(arr); x++)

#define for_each_n(type, x, arr, n) \
for (type *x = (arr); x < (arr) + min(array_length(arr), (n)); x++)

#define for_each_reverse(type, x, arr) \
for (type *x = (arr) + array_length(arr) - 1; x >= (arr); x--)

#define for_each_reverse_n(type, x, arr, n) \
for (type *x = (arr) + array_length(arr) - 1; x >= (arr) + array_length(arr) - (n); x--)

#define nl "\n"

#define abs(x) \
((x) < 0 ? (-(x)) : (x))
#define min(a, b) \
((a) < (b) ? (a) : (b))
#define max(a, b) \
((a) > (b) ? (a) : (b))
#define clamp(x, a, b) \
(min(max((x), min((a), (b))), max((a), (b))))
#define lerp(x, a, b) \
((a) * (1 - (x)) + ((b) * (x)))
#define sqr(x) \
((x) * (x))

// coroutine/fiber/green-thread/task
// example:
// void my_coroutine(struct coroutine *ctx)
// {
//      coroutine(ctx) {
//          // on first call, this will be executed.
//          // then exit.
//          yield(1);
//          // on second call, this will be executed.
//          // then exit.
//          yield(2)
//          // third call, this chunk will execute, restarting
//          // the coroutine state to start from the beginning.
//          reset;
//      }
// }
#define coroutine(ctx) \
struct coroutine *__coro = (ctx); \
switch (__coro->state) \
case 0:

// yield and update the state of the coroutine.
// id must be unique.
// id could be replaced with __LINE__ if you don't
// hot-reload your code. if you do though, you
// can't since any change can update your line
// numbers and produce incorrect results.
#define yield(id) \
do { \
__coro->state = (id); \
return; \
case (id):; \
} while (0)

// sleep and yield until timeout has past.
// dt = how much time has past (if you are writing
//      a game, this would be the delta time of each
//      frame)
#define syield(id, _timeout, dt) \
do { \
__coro->timeout = (_timeout); \
yield(id); \
__coro->timeout -= (dt); \
if (__coro->timeout > 0) \
return; \
} while(0)

// reset the coroutine state to start from the beginning.
#define reset \
do { \
*__coro = (struct coroutine){0}; \
} while (0)

// str format using fixed array as destination.
#define strf_ex(dest, format, ...) \
(strf((dest), sizeof(dest), (format), __VA_ARGS__))

#define id_get_ex(dest, ids) \
(id_get((dest), (ids), array_length(ids)))

typedef union v2 {
    struct { float x, y; };
    struct { float w, h; };
    float f[2];
} v2;

struct coroutine {
    unsigned int state;
    float timeout;
};

// fast inverse square root.
// https://en.wikipedia.org/wiki/Fast_inverse_square_root
float q_rsqrt(float number);

v2 v2_add(v2 x1, v2 x2);
v2 v2_sub(v2 x1, v2 x2);
v2 v2_scale(v2 x, float n);
// get normalized/direction from src to dest.
v2 v2_direction_to(v2 src, v2 dest);
v2 v2_normalize(v2 x);
float v2_dot(v2 x1, v2 x2);
float v2_sqr_length(v2 x);
float v2_sqr_distance(v2 src, v2 dest);

// check circle c1 with radius r1 collides with circle c2 with radius r2.
int cc_hit(v2 c1, v2 c2, float r1, float r2);
// check if point p is inside circle c with radius r.
int cp_hit(v2 c, float r, v2 p);
// check if point p is inside rectangle r with width w and height h.
int rp_hit(v2 r, float w, float h, v2 p);
// check rectangle r1 collides with rectangle r2.
int rr_hit(v2 r1, v2 r2, float w1, float h1, float w2, float h2);

// check if string src starts with match.
// both strings need to be null terminated.
// src and match can be null.
int str_starts_with(char *src, char *match);
int str_starts_with_n(char *src, char *match, unsigned int n);
// check if string src ends with match.
// both strings need to be null terminated.
// src and match can be null.
int str_ends_with(char *src, char *match);
int str_ends_with_n(char *src, char *match, unsigned int n);
// check if string a and b are equal.
// both strings need to be null terminated.
// a and b can be null.
int str_equals(char *a, char *b);
int str_equals_n(char *a, char *b, unsigned int n);
// get length of src.
// src must be null terminated.
// src can be null.
unsigned int str_length(char *src);
// convert integer x to string.
// dest can be null.
// dest will be null terminated.
// the amount of characters written is returned (including null terminator)
unsigned int str_int(char *dest, int x, unsigned int base, unsigned int n);
// convert double x to string.
// dest can be null.
// dest will be null terminated.
// the amount of characters written is returned (including null terminator)
unsigned int str_double(char *dest, double x, unsigned int n);
// parse int from src and store it in dest.
// dest can be null.
// returns the number of bytes scanned.
unsigned int str_parse_int(int *dest, char *src);
unsigned int str_parse_double(double *dest, char *src);
// build a hash from a string.
unsigned int str_hash(char *src);
// write up to n bytes of formatted string into dest.
// dest will be null terminated.
// returns number of bytes used/written (INCLUDING null terminator)
//
// formats:
// %%  = escapes % and only writes one %.
// %c  = writes character.
// %s  = writes strings up to null terminator.
// %*s = writes n bytes of string. 
//       example: strf(dest, n, "%*s", 2, "lorem")
//       only writes 2 bytes/chars, meaning, "lo"
// %x  = writes hexadecimal.
// %d  = writes int.
// %f  = writes float/double with 2 decimals.
// %v  = writes vector with format "(x:y)"
// %?  = use custom function to do the writting. IMPORTANT, make sure
//       your function returns the proper amount of bytes written, 
//       INCLUDING the null terminator.
//       example: strf(dest, n, "%?", custom_func, pointer_to_val);
//       unsigned int custom_func(char *dest, void *v, usize n)
//       {
//           // write to dest and return how many bytes were written.
//           unsigned int written = ...
//           return written;
//       }
unsigned int strf(char *dest, unsigned int n, char *format, ...);
unsigned int vstrf(char *dest, unsigned int n, char *format, va_list va);

// scan string from src using the pattern from pattern
// if the string doesn't conform to the pattern,
// the function will return at the first mismatch.
//
// formats:
// %d  = reads integer
// %f  = reads double
// %*s = reads up to n bytes of string or first space or tab
//       if the entire string can't be read/stored
//       into the buffer, what can be stored will be stored
//       and the rest is discarded. the string is guaranteed
//       to be null terminated.
// 
// example: 
// int i = 0;
// double d = 0;
// char str[4] = {0};
// str_scan("int is 42, double is 42.5, str is randomstring", "int is %d, double is %f, str is %*s", &i, &d, (uint) sizeof(str), str);
// 
// after this call, i will be 42, d will be 42.5, and str will be "ran" (notice the last byte was used for the null terminator)
//
// returns how many bytes were parsed/scanned from src.
unsigned int vstr_scan(char *src, char *pattern, va_list va);
unsigned int str_scan(char *src, char *pattern, ...);

// get a reusable id from ids without exceeding n ids.
// example, if you have a list of ids like [1, 2, 3]
// the first time you call this function, you will get 1.
// the second time 2. now let's say you are done with number 1.
// this id (1) will get recycled with id_recycle. when you
// call id_get again, you won't get 3, but instead, you
// will get 1, since this id got recycled and it's ready to be used again.
// if you call the function again, now you will get 3.
// 0 = error: no more space left; dest is null or ids is null.
// 1 = success, using recycled id.
// 2 = success, using new id.
int id_get(unsigned int *dest, unsigned int *ids, unsigned int n);
// mark the id as done so it can be recycled when
// id_get gets called again.
void id_recycle(unsigned int *ids, unsigned int id);

// random integer.
// seed can be null in which case, a hard coded
// seed will be used instead.
unsigned int random_int(unsigned int *seed);
// random integer between lower and upper.
// seed can be null.
unsigned int random_int_ex(unsigned int *seed, unsigned int lower, unsigned int upper);

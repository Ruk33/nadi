#include <stdarg.h>
#include "flib.h"

float q_rsqrt(float number)
{
	union {
		float f;
		unsigned int i;
	} conv = { .f = number };
	conv.i  = 0x5f3759df - (conv.i >> 1);
	conv.f *= 1.5f - (number * 0.5f * conv.f * conv.f);
	return conv.f;
}

v2 v2_add(v2 x1, v2 x2)
{
	v2 r = {{x1.f[0] + x2.f[0], x1.f[1] + x2.f[1]}};
	return r;
}

v2 v2_sub(v2 x1, v2 x2)
{
	v2 r = {{x1.f[0] - x2.f[0], x1.f[1] - x2.f[1]}};
	return r;
}

v2 v2_scale(v2 x, float n)
{
	v2 r = {{x.f[0] * n, x.f[1] * n}};
	return r;
}

v2 v2_direction_to(v2 src, v2 dest)
{
	v2 r = v2_normalize(v2_sub(dest, src));
	return r;
}

v2 v2_normalize(v2 x)
{
	float len_sqr = v2_sqr_length(x);
	v2 r = x;
	if (len_sqr > 0)
		r = v2_scale(r, q_rsqrt(len_sqr));
	return r;
}

float v2_dot(v2 x1, v2 x2)
{
	float r = (x1.f[0] * x2.f[0]) + (x1.f[1] * x2.f[1]);
	return r;
}

float v2_sqr_length(v2 x)
{
	float r = sqr(x.f[0]) + sqr(x.f[1]);
	return r;
}

float v2_sqr_distance(v2 src, v2 dest)
{
	float r = sqr(src.f[0] - dest.f[0]) + sqr(src.f[1] - dest.f[1]);
	return r;
}

int cc_hit(v2 c1, v2 c2, float r1, float r2)
{
	float d2 = v2_sqr_distance(c1, c2);
	int r = sqr(r1 + r2) >= d2;
	return r;
}

int cp_hit(v2 c, float r, v2 p)
{
	float d2 = v2_sqr_distance(c, p);
	int result = d2 < sqr(r);
	return result;
}

int rp_hit(v2 r, float w, float h, v2 p)
{
	int result = (p.x >= min(r.x, r.x + w) && p.x <= max(r.x, r.x + w) &&
                  p.y >= min(r.y, r.y + h) && p.y <= max(r.y, r.y + h));
	return result;
}

int rr_hit(v2 r1, v2 r2, float w1, float h1, float w2, float h2)
{
	int xhit = (max(r1.x, r1.x + w1) >= min(r2.x, r2.x + w2) &&
                min(r1.x, r1.x + w1) <= max(r2.x, r2.x + w2));
	int yhit = (max(r1.y, r1.y + h1) >= min(r2.y, r2.y + h2) &&
                min(r1.y, r1.y + h1) <= max(r2.y, r2.y + h2));
	int r = xhit && yhit;
	return r;
}

int str_starts_with(char *src, char *match)
{
	if (!src || !match)
		return 0;
	while (*match && 
           *src && 
           *src == *match && 
           (src++, match++, 1));
	int r = *match == 0;
	return r;
}

int str_starts_with_n(char *src, char *match, unsigned int n)
{
	if (!src || !match)
		return 0;
	unsigned int checked = 0;
	while (checked < n && 
           *match && 
           *src && 
           *src == *match && 
           (src++, match++, checked++, 1));
	int r = n == checked;
	return r;
}

int str_ends_with(char *src, char *match)
{
	if (!src || !match)
		return 0;
	char *src_end = src;
	char *match_end = match;
	// find end of src.
	while (*src_end && (src_end++, 1));
	// find end of match.
	while (*match_end && (match_end++, 1));
	// now check if they match.
	while (src < src_end && 
           match < match_end && 
           *src_end == *match_end && 
           (src_end--, match_end--, 1));
	int r = match == match_end && *src_end == *match_end;
	return r;
}

int str_ends_with_n(char *src, char *match, unsigned int n)
{
	if (!src || !match)
		return 0;
	char *src_end = src;
	char *match_end = match;
	// find end of src.
	while (*src_end && (src_end++, 1));
	// find end of match.
	while (*match_end && (match_end++, 1));
	// now check if they match.
	unsigned int checked = 0;
	while (checked < n &&
           src < src_end && 
           match < match_end && 
           *src_end == *match_end && 
           (src_end--, match_end--, checked++, 1));
	int r = *src_end == *match_end && n == checked;
	return r;
}

int str_equals(char *a, char *b)
{
	if (!a || !b)
		return 0;
	while (*a && *b && *a == *b && (a++, b++, 1));
	int r = *a == 0 && *b == 0;
	return r;
}

int str_equals_n(char *a, char *b, unsigned int n)
{
	if (!a || !b || n == 0)
		return 0;
	while (n > 0 && *a && *b && *a == *b && (a++, b++, n--, 1));
	int r = n == 0;
	return r;
}

unsigned int str_length(char *src)
{
	if (!src)
		return 0;
	unsigned int r = 0;
	while (*src && (r++, src++, 1));
	return r;
}

unsigned int str_int(char *dest, int x, unsigned int base, unsigned int n)
{
	if (!dest || !n || !base)
		return 0;
	int negative = x < 0;
	// if zero, make sure we at least have 1 digit.
	unsigned int r = x ? 0 : 1;
	x = abs(x);
	// count digits in number.
	for (int i = x; i; i /= base)
		r++;
	// if we can't store the whole number just return.
	if (r + negative >= n)
		return 0;
	// add negative symbol.
	if (negative)
		*dest++ = '-';
	for (unsigned int i = 0; i < r; i++) {
		int rem = x % base;
		x = x / base;
		*(dest + r - i - 1) = rem < 10 ? rem + '0' : 'a' + rem - 10;
	}
	// null terminator.
	*(dest + r) = 0;
	// +1 null terminator.
	return r + 1 + negative;
}

unsigned int str_double(char *dest, double x, unsigned int n)
{
	if (!dest || !n)
		return 0;
	int negative = x < 0;
	unsigned int r = 0;
	x = abs(x);
	int d = (int) x;
	int dec = (int) (x * 100) % 100;
	if (negative)
		*dest++ = '-';
	while (r + negative < n && d > 0) {
		*dest++ = (d % 10) + '0';
		r++;
		d /= 10;
	}
	// +1 = .
	// +2 = decimals
	// +1 = null
	// +4 total.
	if (r + 4 + negative >= n) {
		// if there isn't enough space, roll back what we wrote.
		for (unsigned int i = 0; i < r + negative; i++)
			*dest-- = 0;
		return 0;
	}
	for (unsigned int i = 0, m = r / 2; i < m; i++) {
		char last = *(dest - r + i);
		*(dest - r + i) = *(dest - (i + 1));
		*(dest - (i + 1)) = last;
	}
	*dest++ = '.';
	*dest++ = (dec / 10) + '0';
	*dest++ = (dec % 10) + '0';
	// null terminator.
	*dest = 0;
	return r + 4 + negative;
}

unsigned int str_parse_int(int *dest, char *src)
{
    char *head = src;
    if (!src)
        return 0;
    int result = 0;
    int sign = 1;
    // get rid of white space.
    while (*src == ' ')
        src++;
    // check sign.
    if (*src == '-' || *src == '+') {
        sign = (*src == '-') ? -1 : 1;
        src++;
    }
    while (*src >= '0' && *src <= '9') {
        result = result * 10 + (*src - '0');
        src++;
    }
    result = result * sign;
    if (dest)
        *dest = result;
    unsigned int bytes_parsed = src - head;
    return bytes_parsed;
}

unsigned int str_parse_double(double *dest, char *src) {
    char *head = src;
    if (!src)
        return 0;
    double result = 0.0;
    double sign = 1.0;
    double fraction = 1.0;
    // get rid of white space.
    while (*src == ' ')
        src++;
    // check sign.
    if (*src == '-' || *src == '+') {
        sign = (*src == '-') ? -1.0 : 1.0;
        src++;
    }
    while (*src >= '0' && *src <= '9') {
        result = result * 10.0 + (*src - '0');
        src++;
    }
    if (*src == '.') {
        src++;
        while (*src >= '0' && *src <= '9') {
            result = result * 10.0 + (*src - '0');
            fraction *= 10.0;
            src++;
        }
    }
    result = sign * result / fraction;
    if (dest)
        *dest = result;
    unsigned int bytes_parsed = src - head;
    return bytes_parsed;
}

// djb2 by Dan Bernstein.
unsigned int str_hash(char *src)
{
	if (!src)
		return 0;
	unsigned int hash = 5381;
	int c = 0;
	while ((c = *src++))
		hash = ((hash << 5) + hash) + c;
	return hash;
}

unsigned int strf(char *dest, unsigned int n, char *format, ...)
{
	va_list va;
	va_start(va, format);
	unsigned int r = vstrf(dest, n, format, va);
	va_end(va);
	return r;
}

unsigned int vstrf(char *dest, unsigned int n, char *format, va_list va)
{
	if (!dest || !format)
		return 0;
	char *head = dest;
	// va_list va;
	// va_start(va, format);
	while (*format && (dest - head) < n) {
		int is_command = *format == '%';
		if (is_command && *(format + 1) == '%') {
			// skip %%.
			format++;
			format++;
			*dest++ = '%';
			continue;
		}
		if (is_command && *(format + 1) == 'c') {
			// skip %c.
			format++;
			format++;
			char c = (char) va_arg(va, int);
			*dest++ = c;
			continue;
		}
		if (is_command && *(format + 1) == 's') {
			// skip %s.
			format++;
			format++;
			char *buf = va_arg(va, char *);
			if (!buf)
				buf = "(null)";
			unsigned int len = str_length(buf);
			for (unsigned int i = 0; i < len; i++) {
				*dest++ = *buf++;
				if (dest - head >= n)
					goto finish;
			}
			continue;
		}
		if (is_command && *(format + 1) == '*' && *(format + 2) == 's') {
			// skip %*s.
			format++;
			format++;
			format++;
			unsigned int len = va_arg(va, unsigned int);
			char *buf = va_arg(va, char *);
			if (!buf) {
				len = sizeof("(null)") - 1;
				buf = "(null)";
			}
			for (unsigned int i = 0; i < len; i++) {
				*dest++ = *buf++;
				if (dest - head >= n)
					goto finish;
			}
			continue;
		}
		if (is_command && *(format + 1) == 'x') {
			// skip %x.
			format++;
			format++;
			int x = va_arg(va, int);
			unsigned int r = str_int(dest, x, 16, n - (dest - head));
			if (!r)
				*dest++ = '?';
			// don't include the null terminator yet.
			else
				dest += r - 1;
			continue;
		}
		if (is_command && *(format + 1) == 'd') {
			// skip %d.
			format++;
			format++;
			int d = va_arg(va, int);
			unsigned int r = str_int(dest, d, 10, n - (dest - head));
			if (!r)
				*dest++ = '?';
			// don't include the null terminator yet.
			else
				dest += r - 1;
			continue;
		}
		if (is_command && *(format + 1) == 'f') {
			// skip %f.
			format++;
			format++;
			double f = va_arg(va, double);
			unsigned int r = str_double(dest, f, n - (dest - head));
			if (!r)
				*dest++ = '?';
			// don't include the null terminator yet.
			else
				dest += r - 1;
			continue;
		}
		if (is_command && *(format + 1) == 'v') {
			// skip %v.
			format++;
			format++;
			*dest++ = '(';
			if (dest - head >= n)
				goto finish;
			v2 v = va_arg(va, v2);
			unsigned int d = str_double(dest, v.f[0], n - (dest - head));
			if (!d)
				*dest++ = '?';
			// don't include the null terminator yet.
			else
				dest += d - 1;
			if (dest - head >= n)
				goto finish;
			*dest++ = ':';
			if (dest - head >= n)
				goto finish;
			d = str_double(dest, v.f[1], n - (dest - head));
			if (!d)
				*dest++ = '?';
			// don't include the null terminator yet.
			else
				dest += d - 1;
			if (dest - head >= n)
				goto finish;
			*dest++ = ')';
			continue;
		}
		if (is_command && *(format + 1) == '?') {
			// skip %?
			format++;
			format++;
			unsigned int (*f)(char *, void *, unsigned int) = va_arg(va, void *);
			void *v = va_arg(va, void *);
			unsigned int written = f(dest, v, n - (dest - head));
			// expect null terminator but don't include it yet.
			dest += written - 1;
			continue;
		}
		*dest++ = *format++;
	}
	finish:
	// va_end(va);
	// null terminator.
	if ((dest - head) < n)
		*dest++ = 0;
	else
		*(dest - 1) = 0;
	unsigned int r = dest - head;
	return r;
}

unsigned int str_scan(char *src, char *pattern, ...)
{
	va_list va;
	va_start(va, pattern);
	unsigned int bytes_parsed = vstr_scan(src, pattern, va);
	va_end(va);
	return bytes_parsed;
}

unsigned int vstr_scan(char *src, char *pattern, va_list va)
{
    char *head = src;
    if (!src || !pattern)
        return 0;
    while (*src && *pattern) {
        int is_command = *pattern == '%';
        if (!is_command) {
            // exit when the pattern is no longer matched.
            if (*src != *pattern)
                break;
            pattern++;
            src++;
            continue;
        }
        // parse int.
        if (*(pattern + 1) == 'd') {
            int *dest = va_arg(va, int *);
            src += str_parse_int(dest, src);
            // advance %d.
            pattern++;
            pattern++;
            continue;
        }
        // parse double.
        if (*(pattern + 1) == 'f') {
            double *dest = va_arg(va, double *);
            src += str_parse_double(dest, src);
            // advance %f.
            pattern++;
            pattern++;
            continue;
        }
        // parse string
        if (*(pattern + 1) == '*' && *(pattern + 2) == 's') {
            // advance %*s
            pattern++;
            pattern++;
            pattern++;
            unsigned int i = 0;
            unsigned int n = va_arg(va, unsigned int);
            char *dest = va_arg(va, char *);
            while (*src && 
                   *src != ' ' &&
                   *src != '\t' &&
                   // +1, make sure the is room for null termiantor.
                   i+1 < n) {
                *dest++ = *src++;
                i++;
            }
            // make sure the entire string
            // is consumed even if dest
            // isn't big enough to hold it.
            while (*src && 
                   *src != ' ' &&
                   *src != '\t')
                src++;
            // null terminator.
            *dest = 0;
            continue;
        }
        // break when the pattern is no longer matched.
        if (*src != *pattern)
            break;
        pattern++;
        src++;
    }
    unsigned int bytes_parsed = src - head;
    return bytes_parsed;
}

int id_get(unsigned int *dest, unsigned int *ids, unsigned int n)
{
	int result = 0;
	if (!ids)
		return result;
	if (!dest)
		return result;
	if (!n)
		return result;
	unsigned int id = ids[0];
	if (id >= n)
		return 0;
	if (ids[id]) {
		*dest = id;
		ids[0] = ids[id];
		result = 1;
	} else {
		if (id + 1 > n) {
			result = 0;
		} else {
			*dest = id;
			ids[0] = id + 1;
			result = 2;
		}
	}
	return result;
}

void id_recycle(unsigned int *ids, unsigned int id)
{
	if (!ids)
		return;
	ids[id] = ids[0];
	ids[0]  = id;
}

unsigned int random_int(unsigned int *seed)
{
	static unsigned int debug_seed = -324516438;
	if (!seed)
		seed = &debug_seed;
	*seed = (214013 * (*seed) + 2531011);
	unsigned int r = (*seed >> 16) & 0x7FFF;
	return r;
}

unsigned int random_int_ex(unsigned int *seed, unsigned int lower, unsigned int upper)
{
	lower = min(lower, upper);
	upper = max(lower, upper);
	unsigned int r = (random_int(seed) % (upper - lower + 1)) + lower;
	return r;
}

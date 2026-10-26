/* string.c - minimal string / memory helpers (no libc) */

#include "string.h"

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && (*a == *b)) { a++; b++; n--; }
    if (n == 0) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = '\0';
    return dst;
}

char *strcat(char *dst, const char *src) {
    char *d = dst + strlen(dst);
    while ((*d++ = *src++));
    return dst;
}

char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == (char)c) return (char *)str;
        str++;
    }
    return (c == '\0') ? (char *)str : NULL;
}

void *memset(void *ptr, int value, size_t size) {
    unsigned char *p = (unsigned char *)ptr;
    while (size--) *p++ = (unsigned char)value;
    return ptr;
}

void *memcpy(void *dst, const void *src, size_t size) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (size--) *d++ = *s++;
    return dst;
}

int memcmp(const void *a, const void *b, size_t size) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    while (size--) {
        if (*pa != *pb) return *pa - *pb;
        pa++; pb++;
    }
    return 0;
}

/* Convert unsigned integer to string in given base (2-16) */
void uint_to_str(uint32_t n, char *buf, int base) {
    static const char digits[] = "0123456789ABCDEF";
    char tmp[33];
    int i = 0;
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (n) {
        tmp[i++] = digits[n % (uint32_t)base];
        n /= (uint32_t)base;
    }
    /* Reverse */
    int j = 0;
    while (i--) buf[j++] = tmp[i];
    buf[j] = '\0';
}

void int_to_str(int n, char *buf, int base) {
    if (n < 0 && base == 10) {
        buf[0] = '-';
        uint_to_str((uint32_t)(-n), buf + 1, base);
    } else {
        uint_to_str((uint32_t)n, buf, base);
    }
}

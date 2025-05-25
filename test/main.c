#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// Decode a UTF‑8 string 's' into an array of uint32_t codepoints.
// On success, *out_len is set to the number of codepoints and
// the function returns a malloc’d array of length *out_len.
// On failure (invalid UTF‑8), returns NULL and *out_len is 0.
uint32_t* utf8_to_codepoints(const char *s, size_t *out_len) {
    size_t cap = 16;
    size_t len = 0;
    uint32_t *codepoints = malloc(cap * sizeof *codepoints);
    if (!codepoints) return NULL;

    const unsigned char *p = (const unsigned char*)s;
    while (*p) {
        uint32_t cp;
        size_t nbytes;

        if (*p < 0x80) {
            // 1‑byte sequence: 0xxxxxxx
            cp = *p;
            nbytes = 1;
        }
        else if ((p[0] & 0xE0) == 0xC0) {
            // 2‑byte sequence: 110xxxxx 10xxxxxx
            cp = p[0] & 0x1F;
            nbytes = 2;
        }
        else if ((p[0] & 0xF0) == 0xE0) {
            // 3‑byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
            cp = p[0] & 0x0F;
            nbytes = 3;
        }
        else if ((p[0] & 0xF8) == 0xF0) {
            // 4‑byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            cp = p[0] & 0x07;
            nbytes = 4;
        }
        else {
            // invalid leading byte
            free(codepoints);
            *out_len = 0;
            return NULL;
        }

        // Consume continuation bytes
        for (size_t i = 1; i < nbytes; ++i) {
            if ((p[i] & 0xC0) != 0x80) {
                // invalid continuation byte
                free(codepoints);
                *out_len = 0;
                return NULL;
            }
            cp = (cp << 6) | (p[i] & 0x3F);
        }

        // advance pointer
        p += nbytes;

        // append cp to array
        if (len + 1 > cap) {
            cap *= 2;
            uint32_t *tmp = realloc(codepoints, cap * sizeof *tmp);
            if (!tmp) {
                free(codepoints);
                *out_len = 0;
                return NULL;
            }
            codepoints = tmp;
        }
        codepoints[len++] = cp;
    }

    *out_len = len;
    return codepoints;
}

// Example usage
int main(void) {
    const char *utf8str = ":"; // "Hello, world" in Japanese
    size_t n;
    uint32_t *cps = utf8_to_codepoints(utf8str, &n);
    if (!cps) {
        fprintf(stderr, "Invalid UTF‑8 input\n");
        return 1;
    }
    printf("Decoded %zu codepoints:\n", n);
    for (size_t i = 0; i < n; ++i) {
        printf("U+%04X\n", cps[i]);
    }
    free(cps);
    return 0;
}

#ifndef STRING_TREE_H
#define STRING_TREE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>

#include "num_arrays/uint32_array.h"
#include "cstring_array/cstring_array.h"

/*
String trees are a way of storing alternative representations of a tokenized string concisely

Particularly with hyphens, we may want the string "twenty-five" to normalize to both:

twenty five
twentyfive

so when we encounter "twenty-five", we'd propose both alternative representations as possible
normalizations of the token.

string_tree is similar to a CSR (compressed sparse row) sparse matrix.

@tokens - for token i, tree->tokens[i] is the index in strings->indices where token i's alternatives begin
@strings - a contiguous string array which only contains as many tokens as there are alternatives

Since we typically only normalize on mid-word hyphens, periods and non-ASCII characters, a string_tree
might not need to store anything at all in many languages.

*/

typedef struct string_tree {
    uint32_array *token_indices;
    cstring_array *strings;
} string_tree_t;

string_tree_t *string_tree_new(void);
string_tree_t *string_tree_new_size(size_t size);

// get
char *string_tree_get_alternative(string_tree_t *self, size_t token_index, uint32_t alternative);

// finalize
void string_tree_finalize_token(string_tree_t *self);
// terminated
void string_tree_add_string(string_tree_t *self, char *str);
void string_tree_add_string_len(string_tree_t *self, char *str, size_t len);
// unterminated
void string_tree_append_string(string_tree_t *self, char *str);
void string_tree_append_string_len(string_tree_t *self, char *str, size_t len);

void string_tree_clear(string_tree_t *self);

uint32_t string_tree_num_tokens(string_tree_t *self);
uint32_t string_tree_num_strings(string_tree_t *self);

uint32_t string_tree_num_alternatives(string_tree_t *self, uint32_t i);

void string_tree_destroy(string_tree_t *self);

typedef struct string_tree_iterator {
    string_tree_t *tree;
    uint32_t *path;
    uint32_t num_tokens;
    uint32_t remaining;
} string_tree_iterator_t;

string_tree_iterator_t *string_tree_iterator_new(string_tree_t *tree);
void string_tree_iterator_next(string_tree_iterator_t *self);
char *string_tree_iterator_get_string(string_tree_iterator_t *self, uint32_t i);
bool string_tree_iterator_done(string_tree_iterator_t *self);
void string_tree_iterator_destroy(string_tree_iterator_t *self);


#define string_tree_iterator_foreach_token(iter, s, code) {                             \
    string_tree_t *tree = iter->tree;                                                   \
    for (int __pi = 0; __pi < iter->num_tokens; __pi++) {                               \
        (s) = string_tree_get_alternative(tree, __pi, iter->path[__pi]);                \
        code;                                                                           \
    }                                                                                   \
}


#endif

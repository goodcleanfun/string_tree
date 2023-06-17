#include <stdio.h>
#include "string_tree.h"

string_tree_t *string_tree_new_size(size_t size) {
    string_tree_t *self = malloc(sizeof(string_tree_t));
    if (self == NULL) {
        return NULL;
    }

    self->token_indices = uint32_array_new_size(size);
    if (self->token_indices == NULL) {
        free(self);
        return NULL;
    }

    uint32_array_push(self->token_indices, 0);

    self->strings = cstring_array_new();
    if (self->strings == NULL) {
        uint32_array_destroy(self->token_indices);
        free(self);
        return NULL;
    }

    return self;
}

#define DEFAULT_STRING_TREE_SIZE 8

string_tree_t *string_tree_new(void) {
    return string_tree_new_size((size_t)DEFAULT_STRING_TREE_SIZE);
}

inline char *string_tree_get_alternative(string_tree_t *self, size_t token_index, uint32_t alternative) {
    if (token_index >= self->token_indices->n) return NULL;

    uint32_t token_start = self->token_indices->a[token_index];

    return cstring_array_get_string(self->strings, token_start + alternative);
}

inline void string_tree_finalize_token(string_tree_t *self) {
    uint32_array_push(self->token_indices, (uint32_t)cstring_array_num_strings(self->strings));
}

void string_tree_clear(string_tree_t *self) {
    uint32_array_clear(self->token_indices);
    uint32_array_push(self->token_indices, 0);
    cstring_array_clear(self->strings);
}

// terminated
inline void string_tree_add_string(string_tree_t *self, char *str) {
    cstring_array_add_string(self->strings, str);
}

inline void string_tree_add_string_len(string_tree_t *self, char *str, size_t len) {
    cstring_array_add_string_len(self->strings, str, len);
}

// unterminated
inline void string_tree_append_string(string_tree_t *self, char *str) {
    cstring_array_append_string(self->strings, str);
}

inline void string_tree_append_string_len(string_tree_t *self, char *str, size_t len) {
    cstring_array_append_string_len(self->strings, str, len);
}

inline uint32_t string_tree_num_tokens(string_tree_t *self) {
    return (uint32_t)self->token_indices->n - 1;
}

inline uint32_t string_tree_num_strings(string_tree_t *self) {
    return (uint32_t)cstring_array_num_strings(self->strings);
}

inline uint32_t string_tree_num_alternatives(string_tree_t *self, uint32_t i) {
    if (i >= self->token_indices->n) return 0;
    uint32_t n = self->token_indices->a[i + 1] - self->token_indices->a[i];
    return n > 0 ? n : 1;
}

void string_tree_destroy(string_tree_t *self) {
    if (self == NULL) return;

    if (self->token_indices != NULL) {
        uint32_array_destroy(self->token_indices);
    }

    if (self->strings != NULL) {
        cstring_array_destroy(self->strings);
    }

    free(self);
}

string_tree_iterator_t *string_tree_iterator_new(string_tree_t *tree) {
    string_tree_iterator_t *self = malloc(sizeof(string_tree_iterator_t));
    self->tree = tree;

    uint32_t num_tokens = string_tree_num_tokens(tree);
    self->num_tokens = num_tokens;

    // calloc since the first path through the tree is all zeros
    self->path = calloc(num_tokens, sizeof(uint32_t));

    uint32_t permutations = 1;
    uint32_t num_strings;

    for (int i = 0; i < num_tokens; i++) {
        // N + 1 indices stored in token_indices, so this is always valid
        num_strings = string_tree_num_alternatives(tree, i);
        if (num_strings > 0) {
            // 1 or more strings in the string_tree means use those instead of the actual token
            permutations *= num_strings;
        }
    }

    if (permutations > 1) {
        self->remaining = (uint32_t)permutations;
    } else{
        self->remaining = 1;
    }

    return self;
}

void string_tree_iterator_next(string_tree_iterator_t *self) {
    if (self->remaining > 0) {
        int i;
        for (i = self->num_tokens - 1; i >= 0; i--) {
            self->path[i]++;
            if (self->path[i] == string_tree_num_alternatives(self->tree, i)) {
                self->path[i] = 0;
            } else {
                self->remaining--;
                break;
            }
        }

        if (i < 0) {
            self->remaining = 0;
        }
    }
}

char *string_tree_iterator_get_string(string_tree_iterator_t *self, uint32_t i) {
    if (i >= self->num_tokens) {
        return NULL;
    }
    uint32_t base_index = self->tree->token_indices->a[i];
    uint32_t offset = self->path[i];

    return cstring_array_get_string(self->tree->strings, base_index + offset);
}

bool string_tree_iterator_done(string_tree_iterator_t *self) {
    return self->remaining == 0;
}

void string_tree_iterator_destroy(string_tree_iterator_t *self) {
    if (self == NULL) return;

    if (self->path) {
        free(self->path);
    }

    free(self);
}

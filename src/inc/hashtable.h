#pragma once

#include <stdlib.h>

#define HT_MINIMUN_CAPACITY 32

enum ht_status_t { HT_FAILURE, HT_SUCCESS, HT_SHOULD_GROW, HT_INSERTED, HT_UPDATED, HT_FOUND, HT_NOT_FOUND };

#define GENERATE_HASHTABLE(key_t, value_t)                                                                             \
    typedef struct {                                                                                                   \
        key_t key;                                                                                                     \
        value_t value;                                                                                                 \
    } ht_item_##key_t##_##value_t##_t;                                                                                 \
                                                                                                                       \
    typedef size_t (*ht_hash_##key_t##_##value_t##_t)(const key_t*);                                                   \
    typedef int (*ht_cmp_##key_t##_##value_t##_t)(const key_t*, const key_t*);                                         \
                                                                                                                       \
    typedef struct {                                                                                                   \
        size_t size;                                                                                                   \
        ht_item_##key_t##_##value_t##_t* items;                                                                        \
                                                                                                                       \
        size_t capacity;                                                                                               \
        double growth_threshold;                                                                                       \
        ht_hash_##key_t##_##value_t##_t hash;                                                                          \
        ht_cmp_##key_t##_##value_t##_t cmp;                                                                            \
                                                                                                                       \
        ht_item_##key_t##_##value_t##_t* _null_item;                                                                   \
        ht_item_##key_t##_##value_t##_t* _tombstone_item;                                                              \
    } hashtable_##key_t##_##value_t##_t;                                                                               \
                                                                                                                       \
    [[nodiscard]] hashtable_##key_t##_##value_t##_t* ht_create_##key_t##_##value_t(                                    \
        size_t capacity, double growth_threshold, ht_hash_##key_t##_##value_t##_t hash,                                \
        ht_cmp_##key_t##_##value_t##_t cmp) {                                                                          \
        if (capacity < HT_MINIMUN_CAPACITY || growth_threshold > 1 || hash == NULL || cmp == NULL) {                   \
            goto defer;                                                                                                \
        }                                                                                                              \
                                                                                                                       \
        hashtable_##key_t##_##value_t##_t* table = malloc(sizeof(hashtable_##key_t##_##value_t##_t));                  \
        if (table == NULL) {                                                                                           \
            goto defer;                                                                                                \
        }                                                                                                              \
        table->size = 0;                                                                                               \
                                                                                                                       \
        table->capacity = capacity;                                                                                    \
        table->growth_threshold = growth_threshold;                                                                    \
        table->hash = hash;                                                                                            \
        table->cmp = cmp;                                                                                              \
                                                                                                                       \
        if ((table->items = calloc(table->capacity, sizeof(*table->items))) == NULL) {                                 \
            goto defer;                                                                                                \
        }                                                                                                              \
                                                                                                                       \
        if ((table->_null_item = calloc(1, sizeof(*table->items))) == NULL) {                                          \
            goto defer;                                                                                                \
        }                                                                                                              \
                                                                                                                       \
        if ((table->_tombstone_item = malloc(sizeof(*table->items))) == NULL) {                                        \
        defer:                                                                                                         \
            free(table->_null_item);                                                                                   \
            free(table->items);                                                                                        \
            free(table);                                                                                               \
            return NULL;                                                                                               \
        }                                                                                                              \
        memset(table->_tombstone_item, 1, sizeof(*table->items));                                                      \
                                                                                                                       \
        return table;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    void ht_destroy_##key_t##_##value_t(hashtable_##key_t##_##value_t##_t* table) {                                    \
        free(table->_tombstone_item);                                                                                  \
        free(table->_null_item);                                                                                       \
        free(table->items);                                                                                            \
        free(table);                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    int ht_insert_##key_t##_##value_t(hashtable_##key_t##_##value_t##_t* table, const key_t* key,                      \
                                      const value_t* value) {                                                          \
        if (table == NULL || table->items == NULL || key == NULL || value == NULL) {                                   \
            return HT_FAILURE;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        if (table->size >= table->growth_threshold * table->capacity) {                                                \
            return HT_SHOULD_GROW;                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        size_t i = table->hash(key) % table->capacity;                                                                 \
        while (memcmp(table->items + i, table->_null_item, sizeof(*table->items)) != 0                                 \
               && memcmp(table->items + i, table->_tombstone_item, sizeof(*table->items)) != 0) {                      \
            if (table->cmp(key, &table->items[i].key) == 0) {                                                          \
                memcpy(&table->items[i].value, value, sizeof(table->items->value));                                    \
                return HT_UPDATED;                                                                                     \
            }                                                                                                          \
                                                                                                                       \
            ++i;                                                                                                       \
            if (i >= table->capacity) {                                                                                \
                i = 0;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        memcpy(&table->items[i].key, key, sizeof(table->items->key));                                                  \
        memcpy(&table->items[i].value, value, sizeof(table->items->value));                                            \
        ++table->size;                                                                                                 \
                                                                                                                       \
        return HT_INSERTED;                                                                                            \
    }                                                                                                                  \
                                                                                                                       \
    value_t* ht_lookup_##key_t##_##value_t(const hashtable_##key_t##_##value_t##_t* table, const key_t* key) {         \
        if (table == NULL || table->items == NULL || key == NULL) {                                                    \
            return NULL;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        size_t i = table->hash(key) % table->capacity;                                                                 \
        while (memcmp(table->items + i, table->_null_item, sizeof(*table->items)) != 0                                 \
               && memcmp(table->items + i, table->_tombstone_item, sizeof(*table->items)) != 0) {                      \
            if (table->cmp(key, &table->items[i].key) == 0) {                                                          \
                return &table->items[i].value;                                                                         \
            }                                                                                                          \
                                                                                                                       \
            ++i;                                                                                                       \
            if (i >= table->capacity) {                                                                                \
                i = 0;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        return NULL;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    const value_t* ht_const_lookup_##key_t##_##value_t(const hashtable_##key_t##_##value_t##_t* table,                 \
                                                       const key_t* key) {                                             \
        if (table == NULL || table->items == NULL || key == NULL) {                                                    \
            return NULL;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        size_t i = table->hash(key) % table->capacity;                                                                 \
        while (memcmp(table->items + i, table->_null_item, sizeof(*table->items)) != 0                                 \
               && memcmp(table->items + i, table->_tombstone_item, sizeof(*table->items)) != 0) {                      \
            if (table->cmp(key, &table->items[i].key) == 0) {                                                          \
                return &table->items[i].value;                                                                         \
            }                                                                                                          \
                                                                                                                       \
            ++i;                                                                                                       \
            if (i >= table->capacity) {                                                                                \
                i = 0;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        return NULL;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    int ht_contains_##key_t##_##value_t(const hashtable_##key_t##_##value_t##_t* table, const key_t* key) {            \
        if (table == NULL || table->items == NULL || key == NULL) {                                                    \
            return HT_FAILURE;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        size_t i = table->hash(key) % table->capacity;                                                                 \
        while (memcmp(table->items + i, table->_null_item, sizeof(*table->items)) != 0                                 \
               && memcmp(table->items + i, table->_tombstone_item, sizeof(*table->items)) != 0) {                      \
            if (table->cmp(key, &table->items[i].key) == 0) {                                                          \
                return HT_FOUND;                                                                                       \
            }                                                                                                          \
                                                                                                                       \
            ++i;                                                                                                       \
            if (i >= table->capacity) {                                                                                \
                i = 0;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        return HT_NOT_FOUND;                                                                                           \
    }                                                                                                                  \
                                                                                                                       \
    int ht_erase_##key_t##_##value_t(hashtable_##key_t##_##value_t##_t* table, const key_t* key) {                     \
        if (table == NULL || table->items == NULL || key == NULL) {                                                    \
            return HT_FAILURE;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        size_t i = table->hash(key) % table->capacity;                                                                 \
        while (memcmp(table->items + i, table->_null_item, sizeof(*table->items)) != 0                                 \
               && memcmp(table->items + i, table->_tombstone_item, sizeof(*table->items)) != 0) {                      \
            if (table->cmp(key, &table->items[i].key) == 0) {                                                          \
                memcpy(table->items + i, table->_tombstone_item, sizeof(*table->items));                               \
                --table->size;                                                                                         \
                return HT_FOUND;                                                                                       \
            }                                                                                                          \
                                                                                                                       \
            ++i;                                                                                                       \
            if (i >= table->capacity) {                                                                                \
                i = 0;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        return HT_NOT_FOUND;                                                                                           \
    }                                                                                                                  \
                                                                                                                       \
    int ht_clear_##key_t##_##value_t(hashtable_##key_t##_##value_t##_t* table) {                                       \
        if (table == NULL || table->items == NULL) {                                                                   \
            return HT_FAILURE;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        memset(table->items, 0, table->capacity * sizeof(*table->items));                                              \
        table->size = 0;                                                                                               \
                                                                                                                       \
        return HT_SUCCESS;                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    typedef struct {                                                                                                   \
        hashtable_##key_t##_##value_t##_t* _table;                                                                     \
        size_t _index;                                                                                                 \
    } hashtable_it_##key_t##_##value_t##_t;                                                                            \
                                                                                                                       \
    hashtable_it_##key_t##_##value_t##_t* ht_create_iterator_##key_t##_##value_t(                                      \
        hashtable_##key_t##_##value_t##_t* table) {                                                                    \
        hashtable_it_##key_t##_##value_t##_t* it = malloc(sizeof(hashtable_it_##key_t##_##value_t##_t));               \
        if (it == NULL) {                                                                                              \
            return NULL;                                                                                               \
        }                                                                                                              \
        it->_table = table;                                                                                            \
        it->_index = 0;                                                                                                \
                                                                                                                       \
        return it;                                                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    void ht_destroy_iterator_##key_t##_##value_t(hashtable_it_##key_t##_##value_t##_t* it) {                           \
        it->_index = it->_table->capacity;                                                                             \
        it->_table = NULL;                                                                                             \
        free(it);                                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
    int ht_next_##key_t##_##value_t(hashtable_it_##key_t##_##value_t##_t* it, key_t* key, value_t* value) {            \
        for (; it->_index < it->_table->capacity; ++it->_index) {                                                      \
            if (memcmp(it->_table->items + it->_index, it->_table->_null_item, sizeof(*it->_table->items)) != 0        \
                && memcmp(it->_table->items + it->_index, it->_table->_tombstone_item, sizeof(*it->_table->items))     \
                       != 0) {                                                                                         \
                memcpy(key, &it->_table->items[it->_index].key, sizeof(it->_table->items->key));                       \
                memcpy(value, &it->_table->items[it->_index].value, sizeof(it->_table->items->value));                 \
                ++it->_index;                                                                                          \
                return HT_FOUND;                                                                                       \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        return HT_NOT_FOUND;                                                                                           \
    }

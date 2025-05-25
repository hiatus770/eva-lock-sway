#pragma once
#include <stdint.h>
#include "graphics.h"

static uint32_t hash_int(const void* data, size_t length);

typedef struct {
    uint32_t key;
    struct character value;
} entry;

typedef struct {
    int count;
    int capacity;
    entry* entries;
} table;

void init_table(table* table);

static entry* find_entry(entry* entries, int capacity, uint32_t key);

bool table_get(table* table, uint32_t key, struct character* ch);

static void adjust_capacity(table* table, int capacity);

void print_table(table* table);

void free_table(table* table);

bool table_set(table* table, uint32_t key, struct character value);

int grow_capacity(int current_capacity);

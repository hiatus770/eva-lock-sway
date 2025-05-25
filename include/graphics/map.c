#include "map.h"
#include "../logger.h"

#define TABLE_MAX_LOAD 0.75

static uint32_t hash_int(const void* data, size_t length){
    uint32_t hash = 2166136261u;
    const uint8_t* bytes = (const uint8_t*) data;

    for (size_t i = 0; i < length; i++){
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

void init_table(table* table){
    log_debug("INITIALIZING TABLE");
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void free_table(table* table){
    log_debug("FREEING  TABLE");
    if (table != NULL){
        free(table->entries);
    }
    return;
}

void print_table(table* table){
    // This code just prints the table
    log_debug("Printing table ...\n");
    for (int i = 0; i < table->capacity; i++){
        fprintf(stderr, "[ID %d]", table->entries[i].value.TextureID);
    }
    log_debug("end of table");
    return;
}

static entry* find_entry(entry* entries, int capacity, uint32_t key){
    uint32_t index = hash_int(&key, sizeof(uint32_t)) % capacity;

    for (;;){
        entry* entry = &entries[index];
        if ( entry->key == 0 || (entry->key) == key){
            assert(entry != NULL);
            return entry;
        }
        index = (index + 1) % capacity;  /// Make sure we look back around as well in case we go over size
    }
    assert(false); // The code should never reach here
}

bool table_get(table* table, uint32_t key, struct character* ch){
    if (table->count == 0){
        return false;
    }
    entry* en = find_entry(table->entries, table->capacity, key);
    if (en->key == 0){
        return false;
    }
    *ch = en->value;
    return true;
}


// Very basic growing algorithm
int grow_capacity(int current_capacity){
    log_debug("GROWING CAP");
    if (current_capacity == 0){
        current_capacity = 2;
    } else {
        current_capacity *= 2; // Right now we just use powers of two
    }
    return current_capacity;
}

static void adjust_capacity(table* table, int capacity){
    log_debug("ADJUSTING CAP");
    entry* entries = (entry*)calloc(capacity, sizeof(entry));
    for (int i = 0; i < capacity; i++){
        entries[i].key = 0;
        entries[i].value = (struct character){.TextureID=0, .size = {0, 0}, .bearing={0,0}, .advance = 0};
    }
    for (int i = 0; i < table->capacity; i++){
        entry *t_entry = &table->entries[i];
        if (t_entry->key == 0){
            log_debug("Null key found");
            continue;
        }
        // Now we can actually move it along
        // entry* destination; find_entry(entries, capacity, entry->key);
        entry* destination = find_entry(entries, capacity, t_entry->key);
        destination->key = t_entry->key;
        destination->value = t_entry->value;
    }
    // test that we can access

    table->entries = entries;
    table->capacity = capacity;
}

bool table_set(table* table, uint32_t key, struct character value){
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD){
        int capacity = grow_capacity(table->capacity);
        adjust_capacity(table, capacity);
        log_debug("DONE ADJUSTING CAP TO %d", capacity);
    }

    entry *entry = find_entry(table->entries, table->capacity, key);
    bool is_new = entry->key == 0;  // This is probably right
    if (is_new){
        table->count++;
    }

    entry->key = key;
    entry->value = value;
    return is_new;
}

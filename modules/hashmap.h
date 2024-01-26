/*
 * Hash Map Functions
 * To be used by the desktop wm manager
*/
#pragma once
#include "../lib.c"
#include "../smem.h"

typedef struct hashmap_entry {
	char * key;
	void * value;
	struct hashmap_entry * next;
} hashmap_entry_t;

struct hashtable
{
    hashmap_entry_t** array;
    size_t num_elems, size;
};

uint32_t hashmap_string_hash(const void * _key) {
	unsigned int hash = 0;
	char * key = (char *)_key;
	int c;
	/* This is the so-called "sdbm" hash. It comes from a piece of
	 * public domain code from a clone of ndbm. */
	while ((c = *key++)) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}

int hasmap_exist(struct hashtable* table, const void* key) {
    uint32_t hash = hashmap_string_hash(key) % table->size;
    return ((uint32_t**)table->array)[hash] != NULL_PTR;
}

void * hashmap_set(struct hashtable * map, const void * key, void * value) {
	unsigned int hash = hashmap_string_hash(key) % map->size;

	hashmap_entry_t * x = map->array[hash];
	if (!x) {
		hashmap_entry_t * e = kalloc(sizeof(hashmap_entry_t), KERN_MEM);
		e->key   = strdup((char*)key);
		e->value = value;
		e->next = NULL_PTR;
		map->array[hash] = e;
		return NULL_PTR;
	} else {
		hashmap_entry_t * p = NULL_PTR;
		do {
			if ((uint64_t)x->key ==  (uint64_t)key) {
				void * out = x->value;
				x->value = value;
				return out;
			} else {
				p = x;
				x = x->next;
			}
		} while (x);
		hashmap_entry_t * e = kalloc(sizeof(hashmap_entry_t), KERN_MEM);
		e->key   = strdup(key);
		e->value = value;
		e->next = NULL_PTR;

		p->next = e;
		return NULL_PTR;
	}
}

void * hashmap_get(struct hashtable * map, const void * key) {
	unsigned int hash = hashmap_string_hash(key) % map->size;

	hashmap_entry_t * x = map->array[hash];
	if (!x) {
		return NULL_PTR;
	} else {
		do {
			if ((uint64_t)x->key == (uint64_t)key) {
				return x->value;
			}
			x = x->next;
		} while (x);
		return NULL_PTR;
	}
}

void * hashmap_remove(struct hashtable * map, const void * key) {
	unsigned int hash = hashmap_string_hash(key) % map->size;

	hashmap_entry_t * x = map->array[hash];
	if (!x) {
		return NULL_PTR;
	} else {
		if ((uint64_t)x->key == (uint64_t)key) {
			void * out = x->value;
			map->array[hash] = x->next;
			free(x->key);
			free(x);
			return out;
		} else {
			hashmap_entry_t * p = x;
			x = x->next;
			do {
				if ((uint64_t)x->key == (uint64_t)key) {
					void * out = x->value;
					p->next = x->next;
					free(x->key);
					free(x);
					return out;
				}
				p = x;
				x = x->next;
			} while (x);
		}
		return NULL_PTR;
	}
}

int hashmap_has(struct hashtable* map, const void * key) {
	unsigned int hash = hashmap_string_hash(key) % map->size;

	hash_entry_t * x = map->array[hash];
	if (!x) {
		return 0;
	} else {
		do {
			if ((uint64_t)x->key == (uint64_t)key) {
				return 1;
			}
			x = x->next;
		} while (x);
		return 0;
	}
}

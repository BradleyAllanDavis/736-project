#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include "fs.h"

// Map (array) of node access (read) counts
/* int access_counts[20000000]; */

// Settings.
/* enum { BTREE_BLOCK_SIZE = 256}; */
/* enum { BTREE_BLOCK_SIZE = 512}; */
/* enum { BTREE_BLOCK_SIZE = 1024}; */
/* enum { BTREE_BLOCK_SIZE = 2048}; */
/* enum { BTREE_BLOCK_SIZE = 4096}; */
/* enum { BTREE_BLOCK_SIZE = 8192}; */
/* enum { BTREE_BLOCK_SIZE = 16384}; */
enum { BTREE_BLOCK_SIZE = 65536};

/* typedef uint32_t BtreeKey; */
/* typedef uint32_t BtreeValue; */
/* typedef uint64_t BtreeKey; */
/* typedef uint64_t BtreeValue; */
typedef unsigned __int128 BtreeKey;
typedef unsigned __int128 BtreeValue;

#define BTREE_KEY_PRINT PRIu32
#define BTREE_VALUE_PRINT PRIu32
int btree_key_cmp(BtreeKey a, BtreeKey b);

typedef struct Btree Btree;

Btree *btree_new(const char *file_name);
void btree_destroy(Btree *btree);

bool btree_get(Btree *btree, BtreeKey key, BtreeValue *value);
void btree_set(
	Btree *btree, BtreeKey key, BtreeValue value,
	bool *replaced, BtreeValue *old_value);

void btree_print(Btree *btree, FILE *stream);
void btree_walk(
	Btree *btree,
	void (*callback)(BtreeKey, BtreeValue, void *), void *callback_context);

FsStats btree_fs_stats(Btree *btree);

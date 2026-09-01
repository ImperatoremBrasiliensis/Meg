/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Code/Lexicon/IdTable.h>

#include <internal/Orbita.h>

#include <string.h>
#include <xxh3.h>

prosIdTable prosIdTable_new() {
	constexpr size_t INTIAL_TABLE_SIZE = 1024;
	struct prosIdTableBucket *table = malloc(
		INTIAL_TABLE_SIZE * sizeof(struct prosIdTableBucket)
	);

	memset(table, 0, sizeof(struct prosIdTableBucket[INTIAL_TABLE_SIZE]));
	return (prosIdTable){
		.tableSize = INTIAL_TABLE_SIZE,
		.table = table,
		.data = prosArena_new(nullptr),
		.count = 0
	};
}

void prosIdTable_del(prosIdTable *self) {
	free(self->table);
	prosArena_del(&self->data);
	*self = (prosIdTable){};
}

static void idTableDoubleTableSize(prosIdTable *self) {
	size_t newsize = sizeof(struct prosIdTableBucket[self->tableSize]) * 2;
	char *ptr = malloc(newsize * 2);

	memset(ptr, 0, newsize);
	memcpy(ptr, self->table, self->tableSize);

	free(self->table);
	self->table = (void *) ptr;
	self->tableSize *= 2;
}

prosId *prosIdTable_pushId(
	prosIdTable *self,
	prosString identifier,
	size_t len
) {
	if (!identifier || !len)
		pros_panic("prosIdTable_pushId(): Invalid arguments.");

	auto hash = XXH3_64bits(identifier, len);
	size_t pos = hash & self->tableSize - 1;

	// The id already exists?
	auto bkt = &self->table[pos];
	while (bkt->hasValue) {
		prosId *id = bkt->ptr;
		if (id->len == len) {
			if (id->hash == hash) {
				return id;
			}
		}

		bkt = &self->table[++pos];
	}

	prosId *ptr = prosArena_alloc(&self->data, sizeof(prosId) + len + 1);
	*ptr = (prosId){
		.hash = hash,
		.len = len
	};

	memcpy(ptr->str, identifier, len);
	ptr->str[len] = '\0';
	*bkt = (struct prosIdTableBucket){
		.ptr = ptr,
		.hasValue = true
	};

	self->count++;
	if ((double) self->count / self->tableSize >= 0.75)
		idTableDoubleTableSize(self);

	return ptr;
}

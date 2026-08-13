/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_CODE_PARSE_IDTABLE_H
#define PROTOCOLS_INTERNAL_CODE_PARSE_IDTABLE_H

#include <internal/Utilities.h>

PROTOCOLS_EXTERNC_START

/* prosScopeTable */

typedef struct prosId_s {
	uint64_t hash;
	uint32_t len;
	char str[];
} prosId;

typedef struct prosIdTable_s {
	size_t tableSize;
	struct prosIdTableBucket {
		prosId *ptr;
		bool hasValue;
	} *table;
	prosArena data;
	size_t count;
} prosIdTable;

prosIdTable prosIdTable_new();

void prosIdTable_del(prosIdTable *self);

prosId *prosIdTable_pushId(
	prosIdTable *self,
	prosString identifier,
	size_t len
);

prosString prosIdTable_getId(prosIdTable *self, size_t index);

#endif	// PROTOCOLS_INTERNAL_CODE_PARSE_IDTABLE_H

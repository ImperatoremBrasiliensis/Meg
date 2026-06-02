/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Utilities.h>
#include <protocols/Utilities.h>

#include <internal/main.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool prosSyscall_write(int fd, prosString str, size_t size) {
#ifdef _WIN32
	abort();
#else
	ssize_t total = 0;

	while (total < (ssize_t) size) {
		ssize_t n = write(fd, str, size);
		if (n > 0) {
			total += n;
			continue;
		}
		if (n == -1)
			return false;
	}

	return true;
#endif
}

size_t prosString_length(prosString str) {
	int len = 0;
	for (; str[len] != '\0'; len++)
		continue;

	return len;
}

void *pros_memcpy(const void *src, void *dst, size_t n) {
	if (!src || !dst || n == 0)
		return nullptr;

	uint8_t *d = (uint8_t *) dst;
	const uint8_t *s = (uint8_t *) src;

	typedef size_t word_t;
	constexpr size_t W = sizeof(word_t);

	if ((((uintptr_t) d ^ (uintptr_t) s) & W - 1) == 0) {
		// Aligns the pointers.
		while (n && ((uintptr_t) d & (W - 1))) {
			*d++ = *s++;
			n--;
		}

		word_t *dw = (word_t *) d;
		const word_t *sw = (word_t *) s;

		// Copies words.
		while (n >= W) {
			*dw++ = *sw++;
			n -= W;
		}

		d = (uint8_t *) dw;
		s = (uint8_t *) sw;
	}

	// Copies the last bytes.
	while (n--)
		*d++ = *s++;

	return dst;
}

void pros_print(int fd, prosString pfx, prosString str) {
	static char buf[PROTOCOLS_PRINT_BUFFER_SIZE];
	size_t pfxlen = 0;

	if (!str)
		return;

	if (pfx) {
		constexpr size_t DEF_LENGTH = sizeof(prosDEFAULT_PRINT_PREFIX) - 1;
		size_t sublen = prosString_length(pfx);
		if (sublen + 2 >= sizeof(buf) - DEF_LENGTH)
			return;

		// First prefix,
		pros_memcpy(prosDEFAULT_PRINT_PREFIX, buf, DEF_LENGTH);
		pfxlen += DEF_LENGTH;

		if (sublen) {
			// Prefix.
			pros_memcpy(pfx, buf + pfxlen, sublen);
			pfxlen += sublen;
			// Prefix terminator.
			pros_memcpy(": ", buf + pfxlen, 2);
			pfxlen += 2;
		}
	}

	size_t len = prosString_length(str);
	const size_t maxStrLen = sizeof(buf) - pfxlen - 1;
	if (len < maxStrLen) {
		pros_memcpy(str, buf + pfxlen, len);
		len += pfxlen;
	} else {
		pros_memcpy(str, buf + pfxlen, len = maxStrLen);
		len += pfxlen;
	}

	// Adds a `\n` at the end and writes on fd.
	if (str[len] != '\n')
		buf[len++] = '\n';
	prosSyscall_write(fd, buf, len);
}

struct prosVector_s {
	size_t typeSize;
	uint32_t capacity;
	uint32_t size;
	char data[];
};

static inline void *vectorGetAddr(prosVector self, uint32_t index) {
	return self->data + index * self->typeSize;
}

static inline void vectorInsert(prosVector *self, const void *obj, uint32_t index) {
	if ((*self)->size == (*self)->capacity) {
		prosVector_reserve(
			self,
			(*self)->capacity ?
				(*self)->capacity * 2 :
				PROTOCOLS_INITIAL_VECTOR_SIZE
		);
	}

	prosVector s = *self;
	char *p = s->data + (s->size * s->typeSize);
	if (index < s->size) {
		p = vectorGetAddr(*self, index);
		memmove(p + s->typeSize, p, (s->size - index) * s->typeSize);
	}

	memcpy(p, obj, s->typeSize);
	s->size++;
}

static inline bool vectorErase(prosVector self, uint32_t index) {
	if (index < self->size) {
		if (index < --self->size) {
			char *p = vectorGetAddr(self, index);
			memmove(p, p + self->typeSize, (self->size - index) * self->typeSize);
		}
		return true;
	}

	return false;
}

prosVector prosVector_new(size_t typeSize) {
	if (!typeSize)
		pros_panic("prosVector_new(): `typeSize` parameter must not be zero.");

	prosVector object = malloc(sizeof(struct prosVector_s));
	if (!object)
		return nullptr;

	*object = (struct prosVector_s){typeSize, 0, 0};

	return object;
}

void prosVector_del(prosVector *self) {
	if (!self || !*self)
		pros_panic("prosVector_del(): `self` parameter must not be `nullptr`.");

	free(*self);
	*self = nullptr;
}

void prosVector_reserve(prosVector *self, size_t n) {
	if (!self || !*self)
		pros_panic("prosVector_remove(): `self` parameter must not be `nullptr`.");

	if (n <= (*self)->capacity)
		return;

	prosVector temp = realloc(*self, sizeof(**self) + n * (*self)->typeSize);
	if (!temp)
		pros_panic("prosVector_reserve(): `realloc()` standard function failed.");

	*self = temp;
	(**self).capacity = n;
}

void prosVector_pushBack(prosVector *self, const void *obj) {
	if (!self || !*self)
		pros_panic("prosVector_pushBack(): `self` parameter must not be `nullptr`.");
	if (!obj)
		pros_panic("prosVector_pushBack(): `obj` parameter must not be `nullptr`.");

	vectorInsert(self, obj, (**self).size);
}

bool prosVector_popBack(prosVector *self) {
	if (!self || !*self)
		pros_panic("prosVector_popBack(): `self` must not be `nullptr`.");

	if (!(*self)->size)
		return false;

	return vectorErase(*self, (*self)->size - 1);
}

void prosVector_insert(prosVector *self, const void *obj, uint32_t index) {
	if (!self || !*self)
		pros_panic("prosVector_add(): `self` must not be `nullptr`.");
	if (!obj)
		pros_panic("prosVector_add(): `obj` parametet must not be `nullptr`.");

	vectorInsert(self, obj, index);
}

bool prosVector_remove(prosVector *self, uint32_t index) {
	if (!self || !*self)
		pros_panic("prosVector_remove(): `self` must not be `nullptr`.");

	if (!(*self)->size)
		return false;

	return vectorErase(*self, index);
}

void *prosVector_getAt(prosVector *self, uint32_t index) {
	if (!self || !*self)
		pros_panic("prosVector_get(): `self` must not be `nullptr`.");

	if (index < (*self)->size)
		return vectorGetAddr(*self, index);
	return nullptr;
}

void *prosVector_getLastObj(prosVector *self) {
	if (!self || !*self)
		pros_panic("prosVector_getLastObject(): `self` must not be `nullptr`.");
	if (!(**self).size)
		return nullptr;

	return vectorGetAddr(*self, (**self).size - 1);
}

void *prosVector_getData(prosVector *self) {
	if (!self || !*self)
		pros_panic("prosVector_getData(): `self` must not be `nullptr`.");

	if (!(**self).size)
		return nullptr;

	return (**self).data;
}

uint32_t prosVector_getCapacity(prosVector *self) {
	if (!self || !*self)
		pros_panic("prosVector_getData(): `self` must not be `nullptr`.");

	return (**self).capacity;
}

uint32_t prosVector_getSize(prosVector *self) {
	if (!self || !*self)
		pros_panic("prosVector_getSize(): `self` must not be `nullptr`.");

	return (*self)->size;
}

void *prosVector_end(prosVector *self) {
	if (!self || !*self)
		pros_panic("prosVector_end(): `self` must not be `nullptr`.");

	if (!(*self)->size)
		return nullptr;

	return (*self)->data + ((*self)->size * (*self)->typeSize);
}

/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_INTERNAL_UTILITIES_H
#define PROTOCOLS_INTERNAL_UTILITIES_H

#include <protocols/Utilities.h>

PROTOCOLS_EXTERNC_START

#define PROS_MAKE_DEL(fdel) ((void (*)(void*)) fdel)
#define PROS_SIZEOF_ARRAY(array) (sizeof(array) / sizeof(typeof(array[0])))

#ifdef _WIN32
#else
#   include <unistd.h>
#   define PROS_INLINE __attribute__((always_inline))
#   define PROS_PACKED __attribute__((packed))
#endif

#include <threads.h>

/* =============== Utilitaries ===============*/

bool prosSyscall_write(int fd, prosString str, size_t size);

/**
 * @brief Simple formats strings. 
 *
 * This function concatenates strings like
 * `printf()`, but it don't concatenates 
 * numbers or other data types. Use it for
 * simple concatenation and formating.
 * It works replacing evry '$' character by
 * it respective string in the @p va parameter.
 * 
 * @param buf The buffer where to store the 
 * formatted string.
 * @param bsize The size of the buffet pointed 
 * by @p buf parameter.
 * @param msg The string to format.
 * @param va The strings by which the placeholders
 * (the '$' characters) will be replaced.
 *
 * @return The actual numbers of characters 
 * written into the buffer pointed by @p buf
 * parameter.
 */
size_t pros_format(char *buf, size_t bsize, prosString msg, va_list va);

/* =============== Arena Allocator =============== */

static constexpr size_t prosARENA_BLOCK_SIZE = 4096;

typedef struct prosArena_s {
   struct prosAllocator_s *altor;
   struct arenaBlock *blocks, *xblocks;
   size_t level;
} prosArena;

/** 
 * @brief Constructor for the `prosArena` struct.
 *
 * @param initialSize The Arena initial size in 
 * memory, must not be zero.
 * @param growFactor How many the Arena capacity.
 * Per exeample, if it's 2 the capacity doubles
 * when the Arena is full and `prosArena_alloc()`
 * is called.
 */
prosArena prosArena_new(struct prosAllocator_s *allocator);

/**
 * @brief Destructor for the the `prosArena` 
 * struct.
 * 
 * @param self Pointer to the object. May not
 * be null.
 */
void prosArena_del(prosArena *self);

/**
 * @brief Allocates memory in the Arena.
 * 
 * @param self Pointer to the object. May not
 * be null.
 * @param size How many bytes to allocate, may
 * not be zero.
 *
 * @return The index ofvthe allocation in the
 * from the begining of the Arena.
 */
void *prosArena_alloc(prosArena *self, size_t size);

typedef struct prosAllocator_s {
   prosArena data;
   struct allocatorSlab *slabs;
   struct allocatorClasses *classes;
   struct allocatorMemManager *memManager;
} prosAllocator;

prosAllocator prosAllocator_new();

void prosAllocator_del(prosAllocator *self);

void *prosAllocator_alloc(prosAllocator *self, size_t size);

void prosAllocator_free(prosAllocator *self, void *blockp);

PROTOCOLS_EXTERNC_END

#endif  // PROTOCOLS_INTERNAL_UTILITIES_H

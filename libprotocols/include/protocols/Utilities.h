/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#ifndef PROTOCOLS_UTILITIES_H
#define PROTOCOLS_UTILITIES_H

#ifdef __cplusplus
#	define PROTOCOLS_EXTERNC_START extern "C" {
#	define PROTOCOLS_EXTERNC_END }
#else
#	define PROTOCOLS_EXTERNC_START
#	define PROTOCOLS_EXTERNC_END
#endif

#ifndef PROTOCOLS_PRINT_BUFFER_SIZE
#	define PROTOCOLS_PRINT_BUFFER_SIZE 4096
#endif

#ifndef PROTOCOLS_INITIAL_VECTOR_SIZE
#	define PROTOCOLS_INITIAL_VECTOR_SIZE 4
#endif

PROTOCOLS_EXTERNC_START

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

typedef const char *prosString;

/**
 * @brief Returns how many bytes that conform
 * @p str .
 *
 * The number of actual bytes that conform the
 * string pointed by @p str in terms of bytes.
 * 
 * @param str The string from which to count 
 * the letters.
 *
 * @return The number of bytes thst
 * conforms the string, excluding the null-
 * terminator.
 */
size_t prosString_length(prosString str);

/**
 * @brief Copies bytes of memory to an specified 
 * memory address.
 *
 * Copies @p n bytes of memory from @p src to @p dst
 * independently whether the objects overlaps or not.
 * 
 * @param src A pointer to the memory region from 
 * which to copy bytes. May not be `nullptr`.
 * @param dst A pointer to the memory regiom to which
 * paste copied bytes. May not be `nullptr`.
 * @param n Number of bytes to copy.
 *
 * @return Case successful, returns a pointer to
 * @p dest . Case any parameter is `nullptr` or `0`, 
 * returns `nullptr`.
 */
void *pros_memcpy(const void *src, void *dst, size_t n);

// A constant specifying the default
// print prefix, which is printed after
// the prefix.
static constexpr const char prosDEFAULT_PRINT_PREFIX[] = "\033[1;4mProtocols:\033[0m\n";

void pros_print__va(int fd, prosString pfx, prosString msg, va_list va);

/**
 * @brief Prints a string to @p fd with prefix.
 *
 * Prints a string specified in @p str and a 
 * prefix formed by the deafult print prefix
 * (which is "Protocols:") and a prefix spe-
 * cified in @p pfx that acts as a subprefix.
 * 
 * Case @p pfx is `nullptr` no prefix is printed,
 * not even the default print prefix. Case @p pfx
 * is an empty string (which is a string only
 * with a null terminator), only the default 
 * one will be printed.
 *
 * If `nullptr` is passed as @p str this function
 * does nothing.
 * 
 * @param fd A file descriptor to an writable
 * file or to a standard output, as
 * `stdout` or `stderr`.
 * @param pfx The prefix (or subprefix) to be
 * printed below the deafult prefix.
 * @param str The string to be printed after 
 * the prefix. May not be `nullptr`.
 */
void pros_print(int fd, prosString pfx, prosString str, ...);

/* ================= Vector ================= */

/**
 * @brief Handle to an `prosVector` object.
 *
 * This handle is a pointer to `prosVector`
 * object allocated in heap. To create the
 * object is used the method constructor
 * (@ref prosVector_new()).
 *
 * This struct can store smartly data, of 
 * several types, contiguously in memory.
 * It increases its capacity accordding on
 * demand.
 * EXEMPLE
 * @code cpp
 * // Create a vectar to store data of type `int`.
 * prosVector vector = prosVector_new(sizeof(int));
 *
 * // Stores data in it.
 * int number = 6;
 * prosVector_pushBack(&vector, &number);
 *
 * // Don't forget to destroy it.
 * prosVector_del(&vector);
 * @endcode
 */
typedef struct prosVector_s {
	void (*deleter)(void *self);
	char *data;
	uint32_t typeSize, capacity, size;
} prosVector;

/**
 * @brief Constructor for the `prosVector`
 * struct.
 * 
 * This constructor allocates the object in
 * heap and initializes it, returning a handle
 * (or, simply, a pointer) to that object.
 *
 * To know how many memory to allocate, you 
 * should pass the size of the type that you
 * want to as @p typeSize parameter.
 * 
 * @param typeSize The size of the type of the
 * objects that you want to store
 * in the vector, normally is
 * passed using `sizeof()`. May
 * not be zero.
 *
 * @return Returns a handle to the
 * allocated and initialized object. 
 * Might return `nullptr` if the 
 * initialization fails.
 */
[[nodiscard]]
prosVector prosVector_new(size_t typeSize, void (*deleter)(void *self));

/**
 * @brief Destructor for the `prosVector`
 * struct.
 *
 * After using the `prosVector` object, this
 * method should be called to free its 
 * resources. The handle pointed by @p self
 * will be set to `nullptr`.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 */
void prosVector_del(prosVector *self);

/**
 * @brief Reserves @p n unities of capacity
 * in memory tovthe `prosVector` object.
 *
 * This method reallocates the capcity of the
 * `prosVector` object to the size of @p n ,
 * or @p n * type size in bytes of memory.
 * If @p n is less than or equal to vector 
 * capacity this method does nothing.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 * @param n How many unities of capacity to
 * allocate. Normally, should not be equal or
 * less than the vector capacity 
 * (@ref prosVector_getCapacity()).
 */
void prosVector_reserve(prosVector *self, size_t n);

/**
 * @brief Copies the object pointed by @p obj
 * to the end of the vector.
 *
 * What this method actually does is copy
 * the bytes of memory pointed by @p obj 
 * in size of the type and paste it at the
 * end of vector. If the vector is full this
 * method reserves the double of the vetor 
 * capacity.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 * @param obj A pointer to the memory from
 * which copy the memory. May not be `nullptr`.
 */
void prosVector_pushBack(prosVector *self, const void *obj);

/**
 * @brief Removes the last object of the
 * vector.
 *
 * This method removes the last element 
 * of the vector. It does not call the
 * object destructor, if there's one.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 * 
 * @return `true` if successful; `false` 
 * otherwise.
 */
bool prosVector_popBack(prosVector *self);

/**
 * @brief Inserts an object in any position
 * of the vector.
 *
 * This method copies the bytes of memory
 * pointed by @p obj in size of the type
 * and pastes it in the position specified.
 * by @p index . If the vector is full this
 * method reserves  the double of the vetor
 * capacity.
 * 
 * If @p index greater than the vector size
 * the object will be inserted at the end
 * of the vector.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 * @param obj A pointer to the memory from
 * which copy the memory. May not be `nullptr`.
 * @param index Position in which insert the,
 * beginning by 0. 
 *
 * @return `true` if successful; `false` 
 * otherwise.
 */
void prosVector_insert(prosVector *self, const void *obj, uint32_t index);

/**
 * @brief Removes the object located at
 * the specified index.
 *
 * This method removes the object located
 * at @p index in the vector. It does not
 * call the object destructor, if there's
 * one.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 * @param index Position in which the object
 * is located, beginning by 0. Should not be 
 * greater than vector size.
 *
 * @return `true` if successful; `false` 
 * otherwise.
 */
bool prosVector_remove(prosVector *self, uint32_t index);

/**
 * @brief Copies the objects pointed by @p arr
 * to the end of the vector.
 *
 * What this method actually does is copy
 * the bytes of memory pointed by @p arr 
 * in size of the type and paste it at the
 * end of the vector. If the vector is full this
 * method reserves more memory.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 * @param arr A pointer to the memory from
 * which copy the memory. May not be `nullptr`.
 * @param count How many objects are in the 
 * array.
 * 
 * @return `true` if successful; `false` 
 * otherwise.
 */
bool prosVector_pushArray(prosVector *self, void *arr, size_t count);

bool prosVector_erase(prosVector *self, uint32_t index, size_t count);

/**
 * @brief Returns a pointer to the object
 * at the specified index.
 *
 * Returns a `void*` that points to the 
 * object that you want to access, if 
 * theres's one at @p index index in the 
 * vector.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 * @param index Position in which the object
 * is located, beginning by 0. Should not be 
 * greater than vector size.
 *
 * @return A pointer to the object or `nullptr`
 * if there's no object at the specified index.
 */
[[nodiscard]]
void *prosVector_getAt(prosVector *self, uint32_t index);

/**
 * @brief Returns a pointer to the last 
 * object in the vector.
 *
 * Returns a `void*` that points to the 
 * last object in the vector, if theres's
 * at least one.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 *
 * @return A pointer to the last object in
 * the vector or `nullptr` if the vector
 * is empty.
 */
[[nodiscard]]
void *prosVector_getLastObj(prosVector *self);

/**
 * @brief Returns a pointer to the
 * beginning of the vector data.
 *
 * Returns a pointer to the beginning
 * of the vector, that is, to its first
 * element. This pointer can be accessed
 * as an array.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 *
 * @return A pointer to the beginning of the 
 * vector object array or `nullptr` if the
 * vector is empty.
 */
[[nodiscard]]
void *prosVector_getData(prosVector *self);

/**
 * @brief Returns the current vector storage
 * capacity.
 *
 * This method returns the vector storage 
 * capacity in terms of bjects. Per example,
 * if the capacity is 4, up to three objects
 * can fit in the vector memory reserve.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 *
 * @return How many objects fit in 
 * the vector memory reserve.
 */
[[nodiscard]]
uint32_t prosVector_getCapacity(prosVector *self);

/**
 * @brief Returns how many object are in
 * the vector. 
 *
 * This method returns how many objects
 * are stored in the vector.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 *
 * @return How many objects are stored in
 * the vector. 
 */
[[nodiscard]]
uint32_t prosVector_getSize(prosVector *self);

/**
 * @brief Returns a pointer the end of
 * the vector.
 *
 * Returns a pointer past-the-end of
 * the vector.
 * 
 * @param self Pointer to the handle of the 
 * `prosVector` object. May not be `nullptr`.
 *
 * @return Pointer to the end of the vector.
 */
[[nodiscard]]
void *prosVector_end(prosVector *self);

/* ================= Little Vector ================= */

PROTOCOLS_EXTERNC_END

#endif	// PROTOCOLS_UTILITIES_H

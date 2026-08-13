/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <internal/Utilities.h>
#include <protocols/Utilities.h>

#include <internal/Orbita.h>

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xxh3.h>

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

/*
 * The use of `pros_memcpy()` instead of `memcpy()`
 * standard library function is because our function
 * was created to be async-signal-safe in any system.
 * In the same way, there is `prosString_length()` as
 * an alternative to `strlen()`. *Don't use them when
 * they are not necessary*.
 */

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

   constexpr size_t W = sizeof(size_t);

   if ((((uintptr_t) d ^ (uintptr_t) s) & W - 1) == 0) {
      // Aligns the pointers.
      while (n && ((uintptr_t) d & (W - 1))) {
         *d++ = *s++;
         n--;
      }

      size_t *dw = (size_t *) d;
      const size_t *sw = (size_t *) s;

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

size_t pros_format(char *buf, size_t bsize, prosString msg, va_list va) {
   char *bufp = buf;
   const char *msgp = msg;
   size_t msglen = prosString_length(msg);
   size_t copied = 0;
   size_t i = 0;

   for (; i < msglen; i++) {
      if (msg[i] == '$') {
         // Copies `i` bytes of message.
         size_t len = &msg[i] - msgp;
         if (copied + len >= bsize)
            len = bsize - copied;
         pros_memcpy(msgp, bufp, len);
         copied += len;
         bufp += len;
         msgp = &msg[++i];

         // Replaces the placeholder.
         prosString str = va_arg(va, prosString);
         len = prosString_length(str);
         if (copied + len >= bsize)
            len = bsize - copied;
         pros_memcpy(str ? str : "(null)", bufp, len);
         copied += len;
         bufp += len;

         if (copied == bsize - 1)
            return copied;
         continue;
      } else if (i >= msglen - 1) {
         // Copies the rest of `msg`.
         size_t len = &msg[msglen] - msgp;
         if (copied + len >= bsize)
            len = bsize - copied;
         pros_memcpy(msgp, bufp, len);
         copied += len;
         buf[copied] = '\0';
         copied++;
         return copied;
      }
   }

   return copied;
}

void pros_print__va(int fd, prosString pfx, prosString str, va_list va) {
   constexpr size_t BUFSIZE = PROTOCOLS_PRINT_BUFFER_SIZE;
   static char buf[BUFSIZE];
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

   size_t len = pros_format(&buf[pfxlen], (BUFSIZE - pfxlen) - 1, str, va);
   len += pfxlen;
   buf[len++] = '\n';
   buf[len] = '\0';
   prosSyscall_write(fd, buf, len);
}

void pros_print(int fd, prosString pfx, prosString msg, ...) {
   va_list va;
   va_start(va);
   pros_print__va(fd, pfx, msg, va);
   va_end(va);
}

static PROS_INLINE void *vectorGetAddr(prosVector *self, uint32_t index) {
   return self->data + index * self->typeSize;
}

static PROS_INLINE bool
vectorErase(prosVector *self, uint32_t index, size_t count) {
   if (index < self->size) {
      char *p = vectorGetAddr(self, index);

      if (self->deleter)
         for (size_t i = count; i-- > 0;)
            self->deleter(&p[i * self->typeSize]);

      size_t area = self->typeSize * count;
      if (index < self->size - count) {
         memmove(p, p + area, (self->size - index) * self->typeSize - area);
      }
      self->size -= count;
      return true;
   }

   return false;
}

prosVector prosVector_new(size_t typeSize, void (*deleter)(void *self)) {
   if (!typeSize)
      pros_panic("prosVector_new(): `typeSize` parameter must not be zero.");

   return (
      prosVector
   ){.typeSize = typeSize, .deleter = deleter, .size = 0, .capacity = 0};
}

void prosVector_del(prosVector *self) {
   if (!self)
      pros_panic("prosVector_del(): `self` parameter must not be `nullptr`.");

   vectorErase(self, 0, self->size);
   free(self->data);
   *self = (prosVector){};
}

void prosVector_reserve(prosVector *self, size_t n) {
   if (!self)
      pros_panic(
         "prosVector_remove(): `self` parameter must not be `nullptr`."
      );

   if (self->capacity * self->typeSize > 25000000u)
      pros_panic("prosVector_reserve(): Too memory reserved.");

   if (n <= self->capacity)
      return;

   void *temp = realloc(self->data, n * self->typeSize);
   if (!temp)
      pros_panic("prosVector_reserve(): `realloc()` standard function failed.");

   self->data = temp;
   self->capacity = n;
}

void prosVector_pushBack(prosVector *self, const void *obj) {
   if (!self)
      pros_panic(
         "prosVector_pushBack(): `self` parameter must not be `nullptr`."
      );
   if (!obj)
      pros_panic(
         "prosVector_pushBack(): `obj` parameter must not be `nullptr`."
      );

   if (self->size >= self->capacity) {
      prosVector_reserve(
         self,
         self->capacity ? self->capacity * 2 : PROTOCOLS_INITIAL_VECTOR_SIZE
      );
   }

   memcpy(vectorGetAddr(self, self->size), obj, self->typeSize);
   self->size++;
}

bool prosVector_popBack(prosVector *self) {
   if (!self)
      pros_panic("prosVector_popBack(): `self` must not be `nullptr`.");

   if (!self->size)
      return false;

   return vectorErase(self, self->size - 1, 1);
}

void prosVector_insert(prosVector *self, const void *obj, uint32_t index) {
   if (!self)
      pros_panic("prosVector_add(): `self` must not be `nullptr`.");
   if (!obj)
      pros_panic("prosVector_add(): `obj` parameter must not be `nullptr`.");

   if (self->size >= self->capacity) {
      prosVector_reserve(
         self,
         self->capacity ? self->capacity * 2 : PROTOCOLS_INITIAL_VECTOR_SIZE
      );
   }

   char *p;
   if (index < self->size) {
      p = vectorGetAddr(self, index);
      memmove(p + self->typeSize, p, (self->size - index) * self->typeSize);
   } else {
      p = vectorGetAddr(self, self->size);
   }

   memcpy(p, obj, self->typeSize);
   self->size++;
}

bool prosVector_remove(prosVector *self, uint32_t index) {
   if (!self)
      pros_panic("prosVector_remove(): `self` must not be `nullptr`.");

   if (!self->size)
      return false;

   return vectorErase(self, index, 1);
}

bool prosVector_pushArray(prosVector *self, void *arr, size_t count) {
   if (!self)
      pros_panic("prosVector_pushArray(): `self` must not be `nullptr`.");
   if (!arr)
      pros_panic(
         "prosVector_pushArray(): `arr` parameter must not be `nullptr`."
      );

   if (self->capacity - self->size < count) {
      prosVector_reserve(self, self->capacity + count);
   }

   memcpy(vectorGetAddr(self, self->size), arr, count * self->typeSize);
   self->size += count;

   return true;
}

bool prosVector_erase(prosVector *self, uint32_t index, size_t count) {
   if (!self)
      pros_panic("prosVector_earse(): `self` must not be `nullptr`.");

   if (index > self->size || self->size - index < count) {
      pros_panic("prosVector_erase(): Invalid indexes were passed.");
   }

   return vectorErase(self, index, count);
}

void *prosVector_getAt(prosVector *self, uint32_t index) {
   if (!self)
      pros_panic("prosVector_get(): `self` must not be `nullptr`.");

   if (index < self->size)
      return vectorGetAddr(self, index);
   return nullptr;
}

void *prosVector_getLastObj(prosVector *self) {
   if (!self)
      pros_panic("prosVector_getLastObject(): `self` must not be `nullptr`.");
   if (!self->size)
      return nullptr;

   return vectorGetAddr(self, self->size - 1);
}

void *prosVector_getData(prosVector *self) {
   if (!self)
      pros_panic("prosVector_getData(): `self` must not be `nullptr`.");

   if (!self->size)
      return nullptr;

   return self->data;
}

uint32_t prosVector_getCapacity(prosVector *self) {
   if (!self)
      pros_panic("prosVector_getData(): `self` must not be `nullptr`.");

   return self->capacity;
}

uint32_t prosVector_getSize(prosVector *self) {
   if (!self)
      pros_panic("prosVector_getSize(): `self` must not be `nullptr`.");

   return self->size;
}

void *prosVector_end(prosVector *self) {
   if (!self)
      pros_panic("prosVector_end(): `self` must not be `nullptr`.");

   if (!self->size)
      return nullptr;

   return self->data + (self->size * self->typeSize);
}

/* =============== Arena Allocator =============== */

struct arenaBlock {
   struct arenaBlock *next;
   uint32_t size;
   bool malloc;
   char data[];
};

prosArena prosArena_new(prosAllocator *allocator) {
   return (prosArena){
      .altor = allocator
   };
}

void prosArena_del(prosArena *self) {
   struct arenaBlock *block = self->blocks;

   while (block) {
      struct arenaBlock *next = block->next;

      if (block->malloc) {
         free(block);
      } else {
         prosAllocator_free(self->altor, block);
      }

      block = next;
   }

   block = self->xblocks;
   while (block) {
      struct arenaBlock *next = block->next;

      free(block);
      block = next;
   }

   *self = (prosArena){};
}

void *prosArena_alloc(prosArena *self, size_t size) {
   if (!self)
      pros_panic("prosArena_alloc(): `self` parameter is invalid.");
   if (!size)
      pros_panic("prosArena_alloc(): `size` parameter must not be 0.");

   auto cur = self->blocks;

   if (cur) {
      size_t remaining = (prosARENA_BLOCK_SIZE - sizeof(struct arenaBlock)) - self->level;
      size = (size + 7) & ~7;

      if (size <= remaining) {
         void *ptr = &cur->data[self->level];
         self->level += size;
         return ptr;
      }

      // Exclusive block;
      if (size > prosARENA_BLOCK_SIZE) {
         struct arenaBlock *ptr = malloc(sizeof(struct arenaBlock) + size);
         *ptr = (struct arenaBlock){};

         if (self->xblocks)
            self->xblocks->next = ptr;
         self->xblocks = ptr;
         return ptr->data;
      }
   }

   struct arenaBlock *ptr = self->altor ?
      prosAllocator_alloc(self->altor, prosARENA_BLOCK_SIZE) :
      malloc(prosARENA_BLOCK_SIZE);

   *ptr = (struct arenaBlock){.malloc = self->altor == nullptr};
   if (cur)
      cur->next = ptr;
   self->blocks = ptr;
   self->level = size;
   return ptr->data;
}

static constexpr size_t allocatorPAGE_SIZE = 1024 * 16;
static constexpr size_t allocatorBLOCK_SIZE = 1024 * 1024 * 4;  // 4 MB
static constexpr size_t allocatorBLOCK_LIST_SIZE =
   allocatorBLOCK_SIZE / allocatorPAGE_SIZE;

struct allocatorMemBlock {
   struct allocatorMemBlock *prev, *next;
   struct allocatorMemManager *man;
   struct allocatorPage {
      struct allocatorPage *next;
      bool valid;
      struct allocatorSlab {
         struct allocatorSlab *prev, *next;
         struct allocatorClass *class;
         void *free;
         uint16_t capacity, inuse;
      } slab;
   } pages[allocatorBLOCK_LIST_SIZE], *free;
   size_t inuse;
   alignas(allocatorPAGE_SIZE) char mem[];
};

struct allocatorMemManager {
   struct allocatorMemBlock *free, *partial, *full;
   size_t count;
};

struct allocatorClass {
   prosAllocator *altor;
   size_t sizeClass;
   uint16_t total;
   struct allocatorSlab *empty, *partial, *full;
};

static PROS_INLINE void allocatorClassAddSlab(
   struct allocatorSlab **list,
   struct allocatorSlab *slab
) {
   if (*list) {
      (*list)->prev = slab;
   }

   slab->next = *list;
   slab->prev = nullptr;
   *list = slab;
}

static PROS_INLINE void allocatorClassRemoveSlab(
   struct allocatorSlab **list,
   struct allocatorSlab *slab
) {
   if (slab->prev) {
      slab->prev->next = slab->next;
   } else {
      *list = nullptr;
   }

   if (slab->next) {
      slab->next->prev = slab->prev;
   }

   slab->prev = nullptr;
   slab->next = nullptr;
}

static PROS_INLINE void *allocatorBlockGetMemOfPage(
   struct allocatorMemBlock *self,
   struct allocatorPage *page
) {
   size_t index = page - self->pages;
   assert(index < allocatorBLOCK_SIZE / allocatorPAGE_SIZE);

   return &self->mem[index * allocatorPAGE_SIZE];
}

static PROS_INLINE struct allocatorPage *allocatorBlockGetPageOfMem(
   struct allocatorMemBlock *self,
   void *mem
) {
   size_t off = (char *) mem - self->mem;
   assert(off % allocatorPAGE_SIZE == 0 && off < allocatorBLOCK_SIZE);

   size_t idx = off / allocatorPAGE_SIZE;
   return &self->pages[idx];
}

static PROS_INLINE void allocatorMemManagerAddBlock(
   struct allocatorMemBlock **list,
   struct allocatorMemBlock *block
) {
   if (*list) {
      block->next = *list;
      block->next->prev = block;
   } else {
      block->next = nullptr;
   }

   block->prev = nullptr;
   *list = block;
}

static PROS_INLINE void allocatorMemManagerRemoveBlock(
   struct allocatorMemBlock **list,
   struct allocatorMemBlock *block
) {
   if (block->prev) {
      block->prev->next = block->next;
   } else {
      *list = nullptr;
   }

   if (block->next) {
      block->next->prev = block->prev;
   }

   block->prev = nullptr;
   block->next = nullptr;
}

static struct allocatorMemBlock *allocatorBlockNew(
   struct allocatorMemManager *man
) {
   struct allocatorMemBlock *blc = aligned_alloc(allocatorBLOCK_SIZE, allocatorBLOCK_SIZE);
   blc->man = man;
   blc->next = nullptr;
   blc->prev = nullptr;
   blc->free = nullptr;
   blc->inuse = 0;

   auto pages = blc->pages;

   for (size_t i = 0; i < allocatorBLOCK_LIST_SIZE - 1; i++) {
      if (i == 0) {
         /*
          * The first block page is reserved
          * for the block header.
          */

         pages->valid = false;
         pages->next = nullptr;
         continue;
      }

      pages[i].next = blc->free;
      pages[i].valid = false;
      blc->free = &pages[i];
   }

   return blc;
}

static struct allocatorSlab *allocatorBlockNewSlab(
   struct allocatorMemBlock *self,
   struct allocatorClass *class
) {
   size_t capacity = allocatorPAGE_SIZE / class->sizeClass;

   assert(self->free && "Block is full.");

   auto page = self->free;
   self->free = page->next;
   page->next = nullptr;
   page->valid = true;
   page->slab = (struct allocatorSlab){.class = class, .capacity = capacity};

   auto slab = &page->slab;

   void *block = allocatorBlockGetMemOfPage(self, page);
   slab->free = block;

   for (size_t i = 0; i < capacity - 1; i++) {
      void *next = (char *) block + class->sizeClass;
      *(void **) block = next;
      block = next;
   }

   *(void **) block = nullptr;
   self->inuse++;
   return slab;
}

static void *allocatorBlockSlabAlloc(struct allocatorSlab *slab) {
   assert(slab->free && "Free list must not be `nullptr` now.");

   void **mem = slab->free;
   slab->free = *mem;

   slab->inuse++;
   return mem;
}

static void allocatorBlockSlabFree(struct allocatorMemBlock *self, void *ptr) {
   void *mem = (void *) ((uintptr_t) ptr & ~(allocatorPAGE_SIZE - 1));
   auto page = allocatorBlockGetPageOfMem(self, mem);
   auto slab = &page->slab;

   *(void **) ptr = slab->free;
   slab->free = ptr;

   if (slab->inuse-- == slab->capacity) {
      allocatorClassRemoveSlab(&slab->class->full, slab);
      allocatorClassAddSlab(&slab->class->partial, slab);
      return;
   }

   if (!slab->inuse) {  // Slab is empty, destroy it!
      allocatorClassRemoveSlab(&slab->class->partial, slab);
      page->valid = false;
      page->next = self->free;
      self->free = page;
      self->inuse--;

      if (!self->inuse) {  // Block is empty, destroy it!
         allocatorMemManagerRemoveBlock(&self->man->partial, self);
         self->free = nullptr;
         self->man = nullptr;
         free(self);
      }
   }
}

static struct allocatorMemManager *allocatorMemManagerNew(prosAllocator *altor) {
   struct allocatorMemManager *man =
      prosArena_alloc(&altor->data, sizeof(struct allocatorMemManager));

   man->partial = nullptr;
   man->free = nullptr;
   man->full = nullptr;
   man->count = 0;
   return man;
}

static void allocatorMemManagerDel(struct allocatorMemManager *self) {
   if (self->full) {
      auto block = self->full;
      while (block) {
         auto next = block->next;

         *block = (struct allocatorMemBlock){};
         free(block);

         block = next;
      }
   }

   if (self->partial) {
      auto block = self->partial;
      while (block) {
         auto next = block->next;

         *block = (struct allocatorMemBlock){};
         free(block);

         block = next;
      }
   }

   if (self->free) {
      auto block = self->free;
      while (block) {
         auto next = block->next;

         *block = (struct allocatorMemBlock){};
         free(block);

         block = next;
      }
   }

   *self = (struct allocatorMemManager){};
}

static struct allocatorSlab *allocatorMemManagerNewSlab(
   struct allocatorMemManager *self,
   struct allocatorClass *class
) {
   struct allocatorMemBlock *blc;

   // Is there any partial blocks?
   blc = self->partial;
   if (blc) {
      auto slab = allocatorBlockNewSlab(blc, class);

      if (!blc->free) {
         // Moves this block to the full list.
         allocatorMemManagerRemoveBlock(&self->partial, blc);
         allocatorMemManagerAddBlock(&self->full, blc);
      }

      return slab;
   }

   // And is there any free blocks?
   blc = self->free;
   if (blc) {
      // Moves this block to the partial list.
      allocatorMemManagerRemoveBlock(&self->free, blc);
      allocatorMemManagerAddBlock(&self->partial, blc);

      return allocatorBlockNewSlab(blc, class);
   }

   // Creates a new block.
   blc = allocatorBlockNew(self);
   allocatorMemManagerAddBlock(&self->partial, blc);

   return allocatorBlockNewSlab(blc, class);
}

static struct allocatorClass *allocatorClassNew(
   prosAllocator *self,
   size_t sizeClass
) {
   struct allocatorClass *ret =
      prosArena_alloc(&self->data, sizeof(struct allocatorClass));

   *ret = (struct allocatorClass){.altor = self, .sizeClass = sizeClass};
   return ret;
}

static void *allocatorClassAlloc(struct allocatorClass *self) {
   // Is there any partial slabs?
   if (self->partial) {
      auto slab = self->partial;
      void *p = allocatorBlockSlabAlloc(slab);

      if (slab->inuse == slab->capacity) {
         allocatorClassRemoveSlab(&self->partial, slab);
         allocatorClassAddSlab(&self->full, slab);
      }

      return p;
   }

   // Is there any empty slabs?
   if (self->empty) {
      auto slab = self->empty;
      allocatorClassRemoveSlab(&self->empty, slab);
      allocatorClassAddSlab(&self->partial, slab);

      return allocatorBlockSlabAlloc(slab);
   }

   // Creates a new slab.
   struct allocatorSlab *slab =
      allocatorMemManagerNewSlab(self->altor->memManager, self);
   allocatorClassAddSlab(&self->partial, slab);

   void *p = allocatorBlockSlabAlloc(slab);
   return p;
}

struct allocatorClasses {
   // Small scale.
   struct allocatorClass *s8, *s16, *s32, *s64;

   // Medium scale.
   struct allocatorClass *m96, *m128, *m192, *m256;

   // Big scale.
   struct allocatorClass *b384, *b512, *b768, *b1024;

   // Great scale.
   struct allocatorClass *b1536, *b2048, *b3072, *b4096;
};

prosAllocator prosAllocator_new() {
   prosAllocator ret = {.data = prosArena_new(nullptr), .slabs = nullptr};

   ret.classes = prosArena_alloc(&ret.data, sizeof(struct allocatorClasses));
   *ret.classes = (struct allocatorClasses){};
   ret.memManager = allocatorMemManagerNew(&ret);
   return ret;
}

void prosAllocator_del(prosAllocator *self) {
   allocatorMemManagerDel(self->memManager);
   prosArena_del(&self->data);
}

static PROS_INLINE uint32_t allocatorGetClass(uint32_t n) {
   if (n <= 8)
      return 8;

   n--;
   n |= n >> 1;
   n |= n >> 2;
   n |= n >> 4;
   n |= n >> 8;
   n |= n >> 16;
   return n + 1;
}

void *prosAllocator_alloc(prosAllocator *self, size_t size) {
   auto sizeClass = allocatorGetClass(size);
   struct allocatorClass **class = nullptr;

   switch (sizeClass) {
   case 8:
      class = &self->classes->s8;
      break;
   case 16:
      class = &self->classes->s16;
      break;
   case 32:
      class = &self->classes->s32;
      break;
   case 64:
      class = &self->classes->s64;
      break;
   case 128:
      if (size <= 96) {
         sizeClass = 96;
         class = &self->classes->m96;
      } else
         class = &self->classes->m128;
      break;
   case 256:
      if (size <= 192) {
         sizeClass = 192;
         class = &self->classes->m192;
      } else
         class = &self->classes->m256;
      break;
   case 512:
      if (size <= 384) {
         sizeClass = 384;
         class = &self->classes->b384;
      } else
         class = &self->classes->b512;
      break;
   case 1024:
      if (size <= 768) {
         sizeClass = 768;
         class = &self->classes->b768;
      } else
         class = &self->classes->b1024;
      break;
   case 2048:
      if (size <= 1536) {
         sizeClass = 1536;
         class = &self->classes->b1536;
      } else
         class = &self->classes->b2048;
      break;
   case 4096:
      if (size <= 3072) {
         sizeClass = 3072;
         class = &self->classes->b3072;
      } else
         class = &self->classes->b4096;
      break;

   default:
      pros_panic("prosAllocator_alloc(): There are no compatible class.");
   }

   if (!*class) {
      *class = allocatorClassNew(self, sizeClass);
   }

   return allocatorClassAlloc(*class);
}

void prosAllocator_free(prosAllocator *self [[maybe_unused]], void *ptr) {
   // All blocks are aligned at `allocatorBLOCK_SIZE`.
   uintptr_t aligned = (uintptr_t) ptr & ~(allocatorBLOCK_SIZE - 1);

   struct allocatorMemBlock *block = (void *) aligned;
   allocatorBlockSlabFree(block, ptr);
}

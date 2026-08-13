/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <USI/filesystem.hpp>

#include <Debug/debug.hpp>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits.h>

using namespace Meg;

template<typename type>
concept constable_cstring = std::same_as<type, char *> || std::same_as<type, const char>;

/*
 * Turns all alternative separators (like '/' on Windows) to the
 * default system separator ('\' on Windows and '/' on Unix-like systems).
 */
static size_t normalize_separators(char *path) {
   char *p = path, *pend = strlen(path) + p;

   if (!p)
      return 0;

   /*
	 * First, normalize. []
	 * Turns all '/' to '\' (Windows) or '\' to '/' (Unix-like) & 
	 * removes illegal MEG_VIRTUAL_DELEGATE characters.
	 */
   while (*p) {
      switch (*p) {
      case MEG_ALT_PATH_SEP:
         *p = MEG_PATH_SEP;
         break;
      case MEG_VIRTUAL_DELEGATE:
         /*
			 * The Virtual Mode delegated character
			 * (MEG_VIRTUAL_DELEGATE) must be at the beginning of the
			 * path. If this is not the case, break
			 * the path.
			 */
         if (p != path) {
            *p = '\0';
            goto formalize;
         }
         break;
      }
      p++;
   }

// Second, formalze. Only removes consecutive slashes.
formalize:
   p = path;
   while (*p) {
      switch (*p) {
      case MEG_PATH_SEP:
         char *i = p;
         while (*i == MEG_PATH_SEP)
            i++;
         if (i != p)
            memmove(p + 1, i, pend - i + 1);
         break;
      }
      p++;
   }

   return strlen(path);
}

/*
 * Returns true if the entry is a special entry
 * like '..' and '.'.
 */
inline static bool is_special_entry(const char *entry) {
   if (!entry)
      return false;

   if (!strcmp(entry, "..") || !strcmp(entry, "../"))
      return true;
   if (!strcmp(entry, ".") || !strcmp(entry, "./"))
      return true;

   return false;
}

/*
 * Represents a file entry, with its name (including extension)
 * and its extension (without dot). The name and the extension 
 * are just pointers.
 */
template<constable_cstring type>
struct file_composition {
   type name;
   type extension;
};

namespace Meg::filesystem {
   /*
 * Return a `file` object, with a pointer to the file name and to the 
 * extension in the file name, from the `path` string parameter 
 * (including extension).
 */
   template<constable_cstring type>
   static file_composition<type> file_name(type path, size_t size) {
      if (!path || !size)
         return {};

      file_composition<type> ret{};

      char *p = path + size;
      do {
         if (p == path || *(p - 1) == MEG_PATH_SEP || *(p - 1) == MEG_VIRTUAL_DELEGATE) {
            ret.name = p;
            break;
         }
      } while (p-- > path);

      if (!ret.name || !ret.name[0])
         return {};

      for (char *i = path + size; i > ret.name;) {
         if (*i == '.') {
            ret.extension = i;
            break;
         }
         if (*(--i) == MEG_PATH_SEP)
            break;
      }

      return ret;
   }

   path::path(const char *path, size_t n) {
      if (!path || !n)
         return;

      // Reserves enough memory for the string & the terminator.
      if (!reserve(n + 1)) {
         std::cerr << "Could not reserve memory.\n";
         return;
      }

      memmove(buffer, path, n);

      buffer[n] = '\0';
      size = n;

      return;
   }

   path::path(const std::string &path):
      path::path(path.c_str(), path.size()) {
   }

   path::path(const char *str) {
      if (!str)
         return;

      size_t len = strlen(str);
      if (!len)
         return;

      if (!reserve(++len)) {
         std::cerr << "Could not reserve memory.\n";
         return;
      }

      memcpy(buffer, str, len);
      size = normalize_separators(buffer);
      return;
   }

   path::path(const path &other) {
      if (other.is_empty())
         return;

      if (!reserve(other.capacity)) {
         std::cerr << "Could not reserve memory.\n";
         return;
      }

      memcpy(buffer, other.buffer, other.size + 1);
      size = other.size;
      return;
   }

   path path::get_filename() const {
      if (is_empty())
         return {};

      file_composition fcomp = file_name(buffer, size);
      path ret{fcomp.name, static_cast<size_t>((buffer + size) - fcomp.name)};
      if (is_special_entry(ret.get_string()))
         return {};

      return ret;
   }

   path path::get_file_extension() const {
      if (is_empty())
         return {};

      file_composition fcomp = file_name(buffer, size);
      if (is_special_entry(fcomp.name) || fcomp.name > fcomp.extension)
         return {};

      return {fcomp.extension, static_cast<size_t>((buffer + size) - fcomp.extension)};
   }

   path path::get_pure_file_name() const {
      if (is_empty())
         return {};

      file_composition fcomp = file_name(buffer, size);
      if (is_special_entry(fcomp.name))
         return {};

      return {
         fcomp.name,
         static_cast<size_t>((fcomp.extension ? fcomp.extension : buffer + size) - fcomp.name)
      };
   }

   path path::get_directory_name() const {
      if (is_empty())
         return {};

      char *buf = buffer + is_virtual();

      char *end = buffer + size;
      while (end > buf) {
         if (*(end - 1) == MEG_PATH_SEP) {
            if (is_special_entry(end))
               return {end};

            char *beg = end - 1;
            while (beg > buf && *(beg - 1) != MEG_PATH_SEP)
               beg--;

            return {beg, static_cast<size_t>(end - beg)};
         }

         end--;
      }

      return {};
   }

   path path::get_parent_path() const {
      if (is_empty())
         return {};

      char *end = buffer + size;
      while (end > buffer && *(end - 1) == MEG_PATH_SEP)
         end--;

      while (end > buffer && *(end - 1) != MEG_PATH_SEP)
         end--;

      return {buffer, static_cast<size_t>(end - buffer)};
   }

   path path::get_normalized_path() const {
      if (is_empty())
         return {};

      path ret;
      char *p1, *p2 = buffer;
      char *bend = buffer + size;

      if (is_virtual()) {
         ret.virtualize(true);
         p1 = buffer + 1;
         p2 = p1;
      }

      while (p2 < bend) {
         p1 = p2;
         while (p2 < bend && !(*p2 == MEG_PATH_SEP || *p2 == MEG_ALT_PATH_SEP))
            p2++;
         path temp{p1, static_cast<size_t>(++p2 - p1)};

         if ((temp == ".." || temp == "../") && !is_special_entry(ret.get_directory_name().get_string()) && ret.get_entry_count() > 1) {
            ret.remove();
            continue;
         } else if ((temp == "." || temp == "./") && ret.get_entry_count()) {
            continue;
         }

         // If `temp` is not a special path entry simply adds it to `ret`.
         ret.insert(temp.buffer, ret.size);
      }

      ret.size = normalize_separators(ret.buffer);
      return ret;
   }

   size_t path::get_entry_count() const {
      if (is_empty())
         return 0;

      size_t count = 0;
      char *buf_end = buffer + size - 1;
      char *p = buffer + is_virtual();
      while (p <= buf_end) {
         if (*p == MEG_PATH_SEP || p == buf_end)
            count++;
         p++;
      }

      return count;
   }

   constexpr inline auto align(auto n, auto value) {
      return (n + value - 1) & ~(value - 1);
   }

   bool path::reserve(size_t n) {
      constexpr size_t RESERVATION_LIMIT = 4096;

      // Round the value to the mext value multiple of 32 after `n`.
      size_t size = align(n, CAPACITY_UNITY_SIZE);
      if (size > RESERVATION_LIMIT) {
         std::cerr << "Reservation is too large, the liit is: " << RESERVATION_LIMIT << std::endl;
      }

      if (!n) {
         free(buffer);
         buffer = nullptr;
         capacity = 0;
         return true;
      }

      if (!buffer) {  // Allocates.
         buffer = static_cast<char *>(malloc(size));
         if (!buffer)
            return false;

         capacity = size;
         buffer[0] = '\0';
         return true;
      } else {  // or reallocates.
         if (size > capacity || size < capacity / 3) {
            void *temp = realloc(buffer, size);
            if (!temp)
               return false;

            capacity = size;
            buffer = static_cast<char *>(temp);
            buffer[size - 1] = '\0';
         }

         return true;
      }
   }

   bool path::fit_capacity() {
      void *temp = realloc(buffer, size + 1);
      if (!temp)
         return false;

      buffer = static_cast<char *>(temp);
      capacity = size + 1;

      return true;
   }

   bool path::erase(size_t n, size_t pos) {
      size_t rest = size - pos;

      if (is_empty() || pos > size || !n)
         return false;

      char *p = buffer + pos;
      size_t count = n > rest ? rest : n;

      memmove(p, p + count,
         rest + 1 - count);  // 1 extra for the null terminator.
      size -= count;
      return true;
   }

   bool path::remove() {
      if (is_empty())
         return false;

      char *p = buffer + size - 1;
      while (p > buffer) {
         p--;
         if (*p == MEG_PATH_SEP) {
            *(++p) = '\0';
            size = p - buffer;
            return true;
         }
      }

      clear();
      return true;
   }

   bool path::replace_filename(const char *name) {
      if (is_file()) {
         if (!name)
            return remove();

         remove();
         return insert(name, size);
      }

      if (!name)
         return true;
      return insert(name, size);
   }

   bool path::replace_file_extension(const char *new_ext) {
      if (!is_file())
         return false;

      char *buf_end = buffer + size;
      file_composition f = file_name(buffer, size);
      if (!f.name || is_special_entry(f.name))
         return false;

      if (!new_ext || !new_ext[0]) {
         if (!erase(buf_end - f.extension, f.extension - buffer))
            return false;
         return true;
      }

      if (!f.extension) {
         if (insert(".", size))
            return insert(new_ext, size);
      } else {
         if (erase(buf_end - f.extension, f.extension - buffer + 1))
            return insert(new_ext, size);
      }

      return false;
   }

   void path::clear() {
      reserve(0);
      size = 0;
   }

   bool path::insert(const char *str, size_t pos) {
      if (!str)
         return false;
      if (pos > size)
         pos = size;

      size_t len = strlen(str);
      if (!len)
         return true;

      size_t new_size = size + len;
      if (!reserve(new_size + 1)) {
         std::cerr << "Could not reserve memory.\n";
         return false;
      }

      char *dest = buffer + pos;
      memmove(dest + len, dest, (size + 1) - pos);  // This 1 is for the '\0' character.
      memmove(dest, str, len);

      size = new_size;
      return true;
   }

   bool path::assign(const char *path) {
      if (!path || !*path) {
         clear();
         return true;
      }

      size_t len = strlen(path) + 1;
      if (!reserve(len)) {
         std::cerr << "Could not reserve memory.\n";
         return false;
      }

      memmove(buffer, path, len);
      size = len - 1;
      return true;
   }

   bool path::virtualize(bool value) {
      if (value && !is_virtual())
         insert("?", 0);
      else if (is_virtual())
         erase(1, 0);
      else
         return false;

      return true;
   }

   bool path::append(const char *element) {
      if (!element)
         return false;

      size_t len = strlen(element) + 1;  // 1 for the null terminator.
      if (len < 2)
         return false;

      bool is_virt = element[0] == MEG_VIRTUAL_DELEGATE;
      if (is_empty() || element[is_virt] == MEG_PATH_SEP || element[is_virt] == MEG_ALT_PATH_SEP) {
         if (!reserve(len))
            goto alloc_fault;

         memcpy(buffer, element, len);
         size = normalize_separators(buffer);
         return true;
      } else {
         if (is_virt)
            len--, element++;

         size_t offset = !is_directory();
         size_t new_cap = size + len + offset;
         if (!reserve(new_cap))
            goto alloc_fault;

         char *buf_end = buffer + size;
         memcpy(buf_end + offset, element, len);
         if (offset)
            *buf_end = MEG_PATH_SEP;
         size = normalize_separators(buffer);
         return true;
      }

alloc_fault:
      std::cerr << "Could not reserve memory.\n";
      return false;
   }

   bool path::compare(const path &other) const {
      if (is_empty() && other.is_empty())
         return true;
      if (is_empty() || other.is_empty())
         return false;

      return !strcmp(buffer, other.buffer);
   }

   bool path::normalize() {
      return assign(get_normalized_path().get_string());
   }

   bool path::has_directories() const {
      if (is_empty())
         return false;

      for (const path &entry: *this)
         if (entry.is_directory())
            return true;

      return false;
   }

   bool path::is_file() const {
      if (is_empty())
         return false;

      file_composition fcomp = file_name(buffer, size);
      if (fcomp.name && !is_special_entry(fcomp.name))
         return true;

      return false;
   }

   bool path::is_absolute() const {
      if (is_empty())
         return false;

      return buffer[0] == MEG_PATH_SEP ||
         (buffer[0] == MEG_VIRTUAL_DELEGATE && buffer[1] == MEG_PATH_SEP);
   }

   bool path::is_directory() const {
      if (is_empty())
         return false;

      file_composition fcomp = file_name(buffer, size);
      if (!fcomp.name || is_special_entry(fcomp.name))
         return true;

      return false;
   }

   bool path::is_virtual() const {
      if (is_empty())
         return false;

      return *buffer == MEG_VIRTUAL_DELEGATE;
   }

   path &path::operator=(const char *path) {
      assign(path);
      return *this;
   }

   path &path::operator/=(const path &path) {
      append(path.get_string());
      return *this;
   }

   path path::iterator::operator*() const {
      size_t end = pos;

      if (p->is_empty())
         return {};

      while (end < p->size)
         if (p->buffer[end++] == MEG_PATH_SEP)
            break;

      return {p->buffer + pos, end - pos};
   }

   path::iterator &path::iterator::operator++() {
      if (p->is_empty())
         return *this;

      while (pos < p->size)
         if (p->buffer[pos++] == MEG_PATH_SEP)
            break;

      return *this;
   }

   reader::reader(file &f):
      f(f),
      fstream(nullptr),
      offset(0),
      cur_page(-1ull),
      cur_page_size(0),
      buf(nullptr) {

      fstream = fopen(f.filepath.get_string(), "r");
      if (!fstream) {
         dbg::logerr(
            "Couldn't load file. System: '",
            strerror(errno),
            "'. Filepath: '",
            f.filepath.get_string(),
            "'."
         );
         return;
      }

      buf = new char[4096];
   }

   reader::~reader() {
      delete[] buf;
      buf = nullptr;

      if (fstream) {
         fclose(fstream);
         fstream = nullptr;
      }
   }

   size_t reader::load_page_of_index(size_t index) {
      const size_t PAGENUM = index / PAGE_SIZE;
      const size_t BUFIDX = index % PAGE_SIZE;

      if (PAGENUM == cur_page)
         return BUFIDX;

      if (fseek(fstream, PAGE_SIZE * PAGENUM, SEEK_SET)) {
         dbg::logerr("`fseek()` function failed. System: '", strerror(errno), "'.");
         return -1;
      }

      size_t READSIZ = f.size - PAGE_SIZE * PAGENUM;
      if (READSIZ > PAGE_SIZE)
         READSIZ = PAGE_SIZE;

      if (BUFIDX >= READSIZ)
         return -1;

      if (fread(buf, READSIZ, 1, fstream) != 1) {
         dbg::logerr("`fread()` function failed. System: '", strerror(errno), "'.");
         return -1;
      }

      cur_page_size = READSIZ;
      return BUFIDX;
   }

   size_t reader::read(char dest[], size_t n) {
      if (!is_loaded())
         return 0;

      if (f.size - offset < n)
         n = f.size - offset;

      size_t size_read = 0;

      while (n) {
         size_t idx = load_page_of_index(offset + size_read);
         if (idx == -1ull)
            break;

         size_t limit = PAGE_SIZE - idx;
         if (limit > n)
            limit = n;

         memcpy(&dest[size_read], &buf[idx], limit);

         size_read += limit;
         n -= limit;
      }

      offset += size_read;
      return size_read;
   }

   size_t reader::read_line(char dest[], size_t bufsz) {
      if (!is_loaded())
         return 0;

      if (!bufsz) {
         bufsz = f.size - offset;
      } else if (bufsz > f.size - offset) {
         bufsz = f.size - offset;
      }

      size_t size_read = 0;
      size_t idx = load_page_of_index(offset);

      while (size_read < bufsz && idx != -1u) {
         size_t toread = cur_page_size - idx;
         char *c = static_cast<char *>(memchr(&buf[idx], '\n', toread));
         bool foundnl = false;

         if (c) {
            // Newline found in the current page.
            toread = c - &buf[idx];
            foundnl = true;
         }

         if (toread > bufsz - size_read) {
            toread = bufsz - size_read;
         }

         if (dest) {
            memcpy(&dest[size_read], &buf[idx], toread);
         }

         // Try to find the newline in the next page.
         size_read += toread;
         offset += size_read;
         idx = load_page_of_index(offset);

         if (foundnl) {
            break;
         }
      }

      return size_read;
   }

   bool reader::is_loaded() const {
      return this->buf != nullptr;
   }

   bool reader::seek(size_t offset, bool from_end) {
      if (is_loaded() && offset < f.size) {
         this->offset = from_end ? f.size - offset : offset;
         return true;
      }

      return false;
   }

   std::string reader::iterator::operator*() const {
      if (!r)
         dbg::panic("Pointer to the `reader` class is `nullptr`.");

      size_t reader_offset = r->offset;  // Save the reader offset.
      r->seek(offset, false);

      std::string ret;
      ret.reserve(r->read_line(nullptr, 0));
      r->read_line(ret.data(), ret.capacity());

      // Restore the reader offset.
      r->seek(reader_offset, false);
      return ret;
   }

   reader::iterator &reader::iterator::operator++() {
      if (!r)
         dbg::panic("Pointer to the `reader` class is `nullptr`.");

      size_t reader_offset = r->offset;  // Save the reader offset.
      r->seek(offset, false);

      offset += r->read_line(nullptr, 0);

      // Restore the reader offset.
      r->seek(reader_offset, false);
      return *this;
   }

   std::expected<file, error> get_file(const path &fpath) {
      namespace stdfs = std::filesystem;

      if (fpath.is_virtual())
         dbg::panic("Support for the Virtual File System is incomplete.");

      stdfs::path fp{fpath.get_string()};

      if (!stdfs::exists(fp))
         return std::unexpected(error::NO_SUCH_FILE_OR_DIRECTORY);

      if (!stdfs::is_regular_file(fp))
         return std::unexpected(error::NOT_A_FILE);

      stdfs::perms perms = stdfs::status(fp).permissions();
      return file{
         .last_mod = stdfs::last_write_time(fp),
         .filepath = stdfs::absolute(fp).lexically_normal().c_str(),
         .size = stdfs::file_size(fp),
         .isvirt = false,
         .permssions = {
            .r = (perms & stdfs::perms::owner_read) != stdfs::perms::none,
            .w = (perms & stdfs::perms::owner_write) != stdfs::perms::none,
            .x = (perms & stdfs::perms::owner_exec) != stdfs::perms::none,
         }
      };
   }

   error delfile(const path &fpath) {
      if (fpath.is_virtual())
         dbg::panic("Support for the Virtual File System is incomplete.");

      namespace stdfs = std::filesystem;

      stdfs::path fp = fpath.get_string();

      if (stdfs::exists(fp))
         return error::NO_SUCH_FILE_OR_DIRECTORY;

      stdfs::remove(fp);
      return error::NONE;
   }

   error delfile(const file &f) {
      if (f.isvirt)
         dbg::panic("Support for the Virtual File System is incomplete.");

      std::filesystem::remove(f.filepath.get_string());
      return error::NONE;
   }

   error makdir(const path &p) {
      if (p.is_virtual())
         dbg::panic("Support for the Virtual File System is incomplete.");

      std::filesystem::create_directory(p.get_string());
      return error::NONE;
   }

   error deldir(const path &p) {
      if (p.is_virtual())
         dbg::panic("Support for the Virtual File System is incomplete.");

      namespace stdfs = std::filesystem;
      stdfs::path fp = p.get_string();

      if (!stdfs::exists(p.get_string()))
         return error::NO_SUCH_FILE_OR_DIRECTORY;

      if (!stdfs::is_directory(fp))
         return error::NOT_A_DIRECTORY;

      stdfs::remove(p.get_string());
      return error::NONE;
   }
}  // namespace Meg::filesystem

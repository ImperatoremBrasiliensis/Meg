/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#pragma once

#include <Meg/common.hpp>
#include <Meg/debug.hpp>

#include <cstddef>
#include <cstring>

#ifdef _WIN32
#	define MEG_PATH_SEP (char) '\\'
#	define MEG_ALT_PATH_SEP (char) '/'
#else
#	define MEG_PATH_SEP (char) '/'
#	define MEG_ALT_PATH_SEP (char) '\\'
#endif

#define MEG_VIRTUAL_DELEGATE '?'

namespace Meg::filesystem {
	class path {
		size_t size{};
		size_t capacity{};
		char *buffer{};

		path(const char *path, size_t size);

	public:
		/**
       * @brief Iterator over the path entries.
       *
       * Incrementing goes to the next entry, decrementing goes
       * to the previous entry and dereferencing returns the
       * entry as a `path` object.
       */
		class iterator {
		private:
			friend path;

			const path *p;
			size_t pos;

		public:
			/**
          * @brief Constructor for the `iterator` class.
          *
          * **UNIQUE PROPERTY**: If @p pos is an invalid character
          *                      position of @p pos it will be 0.
          * 
          * @param p The `path` object, over which to iterate.
          * @param pos Iterator's positin in @p p , should be 
          * at the beginning of an entry or after a separator.
          */
			iterator(const path *p, size_t pos):
					p(p) {
				this->pos = pos > p->size ? 0 : pos;
			}

			/**
          * @brief Returns the current entry.
          *
          * **UNIQUE PROPERTY**: Returns a `path` object
          *                      sliced from the current 
          *                      position of the iterator 
          *                      to the next separator or
          *                      the end of the path.
          * 
          * @return A `path` object containing the entry which
          * the iterator is pointing/representing.
          */
			path operator*() const;

			/**
          * @brief Goes to the next entry in the path.
          *
          * **UNIQUE PROPERTY**: Goes to the next entry in 
          *                      path. Normally, it goes to
          *                      one past the end of the
          *                      current entry. If its at
          *                      last entry it goes to the
          *                      end of the path (`end()`).
          * 
          * @return The iterator pointing to the next
          * entry in the path.
          */
			iterator &operator++();

			/**
          * @brief Compares two iterators for equality.
          * 
          * @return `true` if both iterators points to the
          * same positon; `false` if not.
          */
			[[nodiscard]]
			bool operator==(const iterator &other) const = default;
		};

		static const constexpr int CAPACITY_UNITY_SIZE = 32;
		/**
       * @brief The default constructor for the `path`
       * class.
       *
       * **UNIQUE PROPERTY**: Zero initializes this `path` object.
       */
		path() = default;

		/**
       * @brief A constructor for `path` class that receives a
       * `std::string` object.
       *
       * **UNIQUE PROPERTY**: This constructor is an implicit call
       *                        to the following constructor:
       *                        ```
       *                        path(const char* str)
       *                        ```
       * 
       * @param path A `std::string` object to initialize
       * this `path` object.
       */
		path(const std::string &path);

		/**
       * @brief A constructor for the `path` class that receives a
       * C-style string.
       *
       * **PROPERTY 1**: If @p str is a valid C-style string, 
       *                 this `path` object will be initialized
       *                   with the parameter.
       *
       * **PROPERTY 2**: If @p str is a valid C-style string and
       *                   the initialization was successful, all the
       *                   now created path will be normalized in this
       *                   way:
       *                   - # Any `MEG_ALT_PATH_SEP` will be turned
       *                        into a `MEG_PATH_SEP`.
       *                   - # If a `MEG_VIRTUAL_DELEGATE` was encoun-
       *                        tered, unless it's in the beginning of
       *                        the path, it will be turned into a null-
       *                        terminator.
       *                   - # Consecutive separators will be removed.
       *
       * **PROPERTY 3**: If @p str is an empty C-style string, that is,
       *                   the first character is a null-terminator (`'\0'`),
       *                    or is `nullptr` this `path` object will be
       *                   zero-initialized.
       * 
       * @param str A C-style string to initialize
       * this `path` object.
       */
		path(const char *str);

		/**
       * @brief The copy constructor for  the `path` class.
       *
       * **PROPERTY 1**: If @p other is non-zero, it will be copied.
       * 
       *   **PROPERTY 2**: If @p other is empty, this `path` object
       *                   will be zero-initializes all even though
       *                    @p other has a memory reserve.
       * 
       * @param other Another `path` object to copy.
       */
		path(const path &other);

		/**
       * @brief The move constructor for the `path`
       * class.
       *
       * **UNIQUE PROPERTY**: All data property of @p path data
       *                      will be moved into this `path` object.
       *                        The origin object should be left with
       *                        all fields as zero.
       * 
       * @param path The object from which the data will be moved.
       */
		inline path(path &&path):
				size(path.size),
				capacity(path.capacity),
				buffer(path.buffer) {
			path.buffer = nullptr;
			path.size = 0;
			path.capacity = 0;
		}

		/**
       * @briefDdestructor for the `path` class.
       *
       * **UNIQUE PROPERTY**: Clears this `path` object
       *                      with an implicit call to clear(),
       *                        all fields must be zero after 
       *                      destruction.
       */
		inline ~path() {
			clear();
		}

		/**
       * @brief Gets the file name from the path object.
       *
       * **PROPERTY 1**: If the path has a valid file name, that is,
       *                 the last path entry not ended by a `PATH_SEP`,
       *                 returns it.
       *
       * **PROPERTY 2**: If the last path entry is a directory or the path
       *                 is empty or null returns a zero-initialized `path`
       *                 object.
       *
       * **PROPERTY 3**: If the last entry is a special, that is either
       *                 "." or ".." special entries, path entry returns a
       *                 zero-initialized `path` object.
       * 
       * @return path A `path` object containing the file name.
       */
		path get_filename() const;

		/**
       * @brief Gets the file extension in the file name.
       * 
       * **PROPERTY 1**: If the path has a valid filename with a
       *                 valid file extension, returns the file
       *                 extension (including the dot). 
       *                 EXAMPLE
       *                 "path/to/file.ext" will return just ".ext".
       *
       * **PROPERTY 2**: If the path filename has no filename or
       *                 file extension returns a zero-initialized
       *                 path. Also, if the path has a dot but wihout
       *                 extension name, still returns the dot.
       *
       * **PROPERTY 3**: The file extension is the last dot of the
       *                 filename (and may not be the first character) and
       *                 all ater it (extension name). So, if the dot of
       *                 the file extension is the first character of the 
       *                 path returns a zero-initialized `path` object.
       *                 EXAMPLE
       *                 ".hiddenfile" has no file name, ".hiddenfile.ext"
       *                 has a file extension (".ext").
       *
       * **PROPERTY 4**: Special directory entries may not be called 
       *                 filename (as "." or ".."), consequently, may
       *                 not have a file extesion. If the path ends in a
       *                 special directory entry returns a zero-initialized 
       *                 `path` object.
       * 
       * @return path A `path` object containing the file extension
       * including the dot.
       */
		path get_file_extension() const;

		/**
       * @brief Gets the file name without its extension.
       *
       * **PROPERTY 1**: If the path have a filename and a valid 
       *                   file extension returns the filename without 
       *                   the file extension.
       *
       * **PROPERTY 2**: If the path has not a filename returns a zero-
       *                   initialized `path` object.
       *
       * **PROPERTY 3**: If the path's filename is a hidden filename, that is
       *                   starts with a dot, the first dot and all after it may
       *                   not be evaluated as an extexnsion.
       * 
       * @return path A `path` object containing the file name without
       * the extension, if there's one, from the path.
       */
		path get_pure_file_name() const;

		/**
       * @brief Gets the last directory name.
       *
       * **PROPERTY 1**: If the path contains at least one directory
       *                 entry, which is any entry ended by a path 
       *                 separator, the name of the last one will
       *                 be returned with the path separator.
       *
       * **PROPERTY 2**: If the path has no directory entries returns a
       *                 zero-initialized `path` object.
       *
       * **PROPERTY 3**: If the last directory entry is a special, that is
       *                 '.' and '..', returns it with no path separator,
       *                 if there's one.
       * 
       * @return path A `path` object containing the directory name. 
       */
		path get_directory_name() const;

		/**
       * @brief Gets the parent of this path.
       *
       * **PROPERTY 1**: If the path is valid and has more than 
       *                   one entry returns the path without the 
       *                   last entry.
       *
       * **PROPERTY 2**: If the path is void or has only one path 
       *                   entry returns a zero-initialized `path`
       *                   object.
       * 
       * @return path A `path` object containing the parent path,
       * of this path.
       */
		path get_parent_path() const;

		/**
       * @brief Gets a lexically resolved path version of this path.
       * 
       * **PROPERTY 1**: If this path is not empty returns a normalized
       *                 version of it. The path will be normalized in this
       *                 way:
       *                 - # Removes any '.' special directory entry as
       *                     long as there's one normal entry before it.
       *                 - # Removes any '..' special directory entry as
       *                     long as there's one normal entry before it
       *                     that will be remove too.
       *                 - # Normalizes all separators, like '\' to '/', and
       *                     turns any virtual delegate that is not the first
       *                     character of the path into a null-terminator 
       *                     character.
       *                 - # Removes consecutive path separators, like
       *                     '////' to '/'.
       *
       * **PROPERTY 2**: If the path is empty returns a zero-initialized
       *                 `path` path object.
       *
       * @return path A `path` object containing the normalized path.
       */
		path get_normalized_path() const;

		/**
       * @brief Gets the size of this `path` object.
       *
       * **UNIQUE PROPERTY**: Returns the size of this path.
       * 
       * @return size_t The size in bytes of the path.
       */
		MEG_INLINE size_t get_path_size() const {
			return size;
		}

		/**
       * @brief Gets the memory reserve of this `path` object.
       *
       * **UNIQUE PROPERTY**: Returns the memory reserve (or capacity)
       *                      of this path in bytes.
       * 
       * @return size_t The memory reserve of this `path` 
       * object in bytes.
       */
		MEG_INLINE size_t get_path_capacity() const {
			return capacity;
		}

		/**
       * @brief Gets the number of entries of this path.
       *
       * **UNIQUE PROPERTY**: Returns the count of entries
       *                        of this path. This count does
       *                        not include the virtual delegate.
       *                        Directory entries always ends in a 
       *                        `MEG_PATH_SEP`, special directory
       *                        entries may too, while file entries
       *                      does not end.
       * 
       * @return size_t The number of path entries in
       * this `path` object.
       */
		size_t get_entry_count() const;

		/**
       * @brief Gets a pointer (C-style string) to the path string.
       *
       * **PROPERTY 1**: Returns a pointer to this path string
       *                   buffer. 
       *
       * **PROPERTY 2**: If the path has no memory reserve returns
       *                   `nullptr`. Also, if the path is empty
       *                   the pointer will point to a null-terminator.
       * 
       * @return const char* The C-style string, can become a dangling
       * pointer if this `path` object it's destroyed.
       */
		MEG_INLINE const char *get_string() const {
			return buffer;
		}

		/**
       * @brief Changes the `path` object storage capcity
       * to @p n bytes of memory.
       *
       * @warning This method always align @p n to `CAPACITY_UNITY_SIZE`.
       *
       * **PROPERTY 1**: If this path has no memory reserve 
       *                   to store the path string this method
       *                   allocates @p n bytes of memory in heap.
       * **PROPERTY 2**: If this path already has a memory reserve
       *                   just reallocates the buffer with @p n as
       *                   the new size. In case @p n is smaller than
       *                   one third of the path capacity, reduces the
       *                   memory reserve to @p n.
       * **PROPERTY 3**: If @p n is 0 the reserve will be freed.
       *
       * @return `true` if successful,`false` otherwise. 
       */
		bool reserve(size_t n);

		/**
       * @brief Changes the `path` object memory capacity
       * to fit to the path size.
       *
       * **UNIQUE PROPERTY**: Fits the size of this path`s 
       *                      memory reserve to the path size
       *                      including the null-terminator.
       * 
       * @return `true` if successful, `false` otherwise.
       */
		bool fit_capacity();

		/**
       * @brief Erases @p n characters from the end
       * of the path.
       *
       * **PROPERTY 1**: If @p pos is less than the path size
       *                 erases @p n characters from @p pos
       *                 towards the end of the path.
       * **PROPERTY 2**: If @p n is greater than the path size
       *                 minus @p pos erases anything from @p pos
       *                 towards the end of the buffer. If @p pos
       *                 is greater than the path size or if @p n
       *                 is 0 this method fails.
       * 
       * @return `true` if successful, `false` if not.
       */
		bool erase(size_t n, size_t pos);

		/**
       * @brief Removes the last entry in the path.
       *
       * **UNIQUE PROPERTY**: If this path has at least one
       *                      entry removes the last one, otherwise
       *                        returns `false`.
       * 
       * @return `true` if successful, `false` if not.
       */
		bool remove();

		/**
       * @brief Replaces, adds, or removes the file name of
       * the path.
       *
       * **UNIQUE PROPERTY**: This method can modify the filename 
       *                      in three ways:
       *                      - # Case this path has no filename, the
       *                          @p name parameter will be added
       *                          as the new one;
       *                      - # Case this path has a filename, the 
       *                          @p name parameter will be added as
       *                          the new one; and
       *                      - # Case @p name is `nullptr` or an 
       *                          empty string, the filename will be
       *                          removed.
       * 
       * @param name The new filename. May be `nullptr`.
       * 
       * @return `true` if successful, `false` if not.
       */
		bool replace_filename(const char *name);

		/**
       * @brief Replaces, adds or removes the file name extension
       * of the path.
       *
       * **PROPERTY 1**: This method can modify the filename 
       *                 extension in three ways:
       *                 - # Case the path has no file extension,
       *                     @p extension will be added as the new
       *                     one.
       *                 - # Case this path has an extension, 
       *                     @p extension will be added as the new 
       *                     one, replacing the previous extension.
       *                 - # Case @p extension is `nullptr` or is an
       *                     empty string, the extension, if there's
       *                     one will be removed.
       * **PROPERTY 2**: If this path has no filename, it has no
       *                 extension. Furthermore, the extension dot
       *                 may not be the first character, per example:
       *                 '.name' has no extension while 'l.name' or,
       *                 even though, '..name' has.
       * 
       * @param extension The new file name extension, without dot.
       *                  May be `nullptr` or an empty string to
       *                  remove the extension.
       *
       * @return `true` if successful, `false` if not.
       */
		bool replace_file_extension(const char *extension);

		/**
       * @brief Clears the `path` object.
       *
       * **UNIQUE PROPERTY**: This method frees the memory 
       *                      reserve of this path, turning
       *                      this a null `path` object.
       * 
       * Frees the object memory reserve.
       */
		void clear();

		/**
       * @brief Insert a string into the path at position
       * @p pos .
       *
       * **PROPERTY 1**: If @p str is not `nullptr`or empty
       *                 it will be inserted at @p pos bytes 
       *                 from the beginning of the path.
       * **PROPERTY 2**: If @p pos is greater than path size
       *                 @p str will be inserted at the end.
       * **PROPERTY 3**: Case @p str is `nullptr` returns
       *                 `false` or case @p str is an empty
       *                 string does nothing and returns 
       *                 `true`.
       * 
       * @param str The string to be inserted.
       * @param pos Where to insert the string. Must be less 
       * than the path size.
       *
       * @return `true` if successful, `false` if not. 
       */
		bool insert(const char *str, size_t pos);

		/**
       * @brief Assigns a new path string to the object.
       *
       * **UNIQUE PROPERTY**: Assigns @p path to this path or,
       *                      if @p path is a null or empty string,
       *                      clears this path.
       * 
       * @param path A string containing the new path.
       *
       * @return `true` if successful; `false` if not.
       */
		bool assign(const char *path);

		/**
       * @brief Appends a new element to the path.
       *
       * **PROPERTY 1**: If @p element is not `nullptr`
       *                 this method appends a new entry
       *                 to the path as follows: 
       *                 - # If the last entry of the path
       *                     is a file entry (which has not
       *                     a separator at the end), it 
       *                     be turned into a directory 
       *                     entry before insert @p element
       *                     at the end of the path.
       *                 - # If the last entry is a directory
       *                     entry (which has a separator at
       *                     the end), just inserts @p element
       *                     at the end of the path.
       * **PROPERTY 2**: If @p element is absolute replace this
       *                 path with it. In this case, the path 
       *                 will be virtual if @p element begins
       *                 with a virtual delegate character (?).
       *
       * EXAMPLE
       * @code txt
       * "file" + "element" -> "file/element"
       * "folder/" + "element" -> "folder/element"
       * "folder/file" + "/element" -> "/element"
       * @endcode 
       * 
       * If the new element begins with a path separator, that is,
       * element is an absolute path, the path will be replaced by
       * @p element .
       * 
       * @param element The new element to be appended to the path.
       *
       * @return `true` if successful; `false` if not.
       */
		bool append(const char *element);

		/**
       * @brief Normalizes the path.
       *
       * **UNIQUE PROPERTY**: If this path not empty it will
       *                      be normalized in this way:
       *                      - # Removes any '.' special directory entry as
       *                          long as there's one normal entry before it.
       *                      - # Removes any '..' special directory entry as
       *                          long as there's one normal entry before it
       *                          that will be remove too.
       *                      - # Normalizes all separators, like '\' to '/', and
       *                          turns any virtual delegate that is not the first
       *                          character of the path into a null-terminator 
       *                          character.
       *                      - # Removes consecutive path separators, like
       *                          '////' to '/'.
       *
       * @return `true` if successful; `false` if not.
       */
		bool normalize();

		/**
       * @brief Makes the path virtual.
       *
       * **UNIQUE PROPERTY**: If this path is not virtual,
       *                      that is, doesn't begin with
       *                      a virtual delegate character,
       *                      makes it virtual. Otherwise, 
       *                      the contrary happens.
       * 
       * @param value A boolean that decides wheter the path will be 
       * virtualized or not.
       *
       * @return `true` if this path virtuality was not the same value than
       * @p value and `false` if not.
       */
		bool virtualize(bool value);

		/**
       * @brief Compares this and another `path` object.
       *
       * **PROPERTY 1**: If this path is equal to @p other
       *                 returns `true`; `false` otherwise.
       * **PROPERTY 2**: If both this and @p other are 
       *                 empty returns `true`. But if only
       *                 one of them is empty returns `false`.
       *
       * @param path The other object to compare.
       *
       * @return int Return a value < 0 if this path is ordered
       * before @p path ; return > 0 if @p path is ordered before
       * this path; and return 0 if the paths are the same.
       */
		bool compare(const path &other) const;

		/**
       * @brief Checks if the path has any directory name.
       *
       * **UNIQUE PROPERTY**: If this path is not empty, this
       *                      method checks whether it has
       *                      any directory entry (that is, an
       *                      ends with a separator) or not.
       *                      Special diretory entries are 
       *                      considered.
       * 
       * @warning Directories are entries ended by a `PATH_SEP`.
       *
       * @return `true` if the path has at least one direcory;
       * `false` if not. 
       */
		bool has_directories() const;

		/**
       * @brief Checks if the path has a file name.
       *
       * **UNIQUE PROPERTY**: If this path is not empty, this
       *                      method checks whether it has 
       *                      a file entry (that is, doesn't
       *                      end with a separator) or not.
       *
       * @warning The file name is the last entry of the path
       * not ended by a `PATH_SEP`.
       * 
       * @return `true` if the path has a file name;
       * `false` if not. 
       */
		bool is_file() const;

		/**
       * @brief Checks if the path is absolute.
       *
       * **UNIQUE PROPERTY**: If this path is not empty, this
       *                      method checks if it's an absolute 
       *                      path (that is, begins with the root,
       *                      directory entry, `/` or `\`) or not.
       * 
       * @return `true` if the path is absolute; `false` if not.
       */
		bool is_absolute() const;

		/**
       * @brief Checks if the path is to a directory.
       *
       * **UNIQUE PROPERTY**: If this this path is not empty,
       *                      this method checks if this path
       *                      is to a directory (that is, has 
       *                      an entry that ends with a separator
       *                      at the end of the path) or not.
       *
       * @warning Directories are entries ended by `PATH_SEP`.
       * 
       * @return `true` if the path is a directory; `false` if not.
       */
		bool is_directory() const;

		/**
       * @brief Checks if the string is empty.
       *
       * **UNIQUE PROPERTY**: Checks if this path doesn't
       *                      have any character in memory.
       * 
       * @return `true` if the path is empty;
       * `false` if not.
       */
		MEG_INLINE bool is_empty() const {
			return !size;
		}

		/**
       * @brief Checks if the path is virtual.
       *
       * **UNIQUE PROPERTY**: If this path is not empty, this
       *                      method checks whether it's virtual
       *                      (that is, the path has a virtual
       *                      delegate, `?`, as the first character)
       *                      or not. 
       * 
       * @return `true` if the path is virtual; `false` if not.
       */
		bool is_virtual() const;

		/**
       * @brief Returns an iterator to the first entry of the path.
       *
       * **UNIQUE PROPERTY**: If this path is not empty this method
       *                      returns an iterator to the beginning
       *                      of it. Otherwise, returns `end()`.
       * 
       * @return iterator An `iterator` object to the first entry of
       * path; returns `end()` if the path is empty.
       */
		inline iterator begin() const {
			return {this, 0};
		}

		/**
       * @brief Returns an iterator to one past the last entry of the 
       * path.
       *
       * **UNIQUE PROPERTY**: This method returns a past-the-end
       *                      iterator for this path. Dereferencing
       *                      the returned iterator is undefined
       *                   `  behaviour.
       *
       * @warning This iterator marks the end of the path and **must not
       * be dereferenced**. 
       * 
       * @return iterator Represeting the end of the path.
       */
		inline iterator end() const {
			return {this, size};
		}

		/**
       * @brief Cast operator to `bool` type.
       *
       * **UNIQUE PROPERTY**: This operator returns `true` if
       *                      this path is not empty; returns 
       *                      `false` otherwise.
       * 
       * @return `true` if the path is not empty; `false` if 
       * not.
       */
		MEG_INLINE explicit operator bool() const {
			return !is_empty();
		}

		/**
       * @brief Replace this path by a string.
       *
       * **UNIQUE PROPERTY**: Assigns @p path to this path or,
       *                      if @p path is a null or empty string,
       *                      clears this path.
       * 
       * @param path The new path. May not be null.
       *
       * @return This path.
       */
		path &operator=(const char *path);

		/**
       * @brief Compares this path to another for equality.
       *
       * **UNIQUE PROPERTY**: This operator compares this
       *                      path to the @p path parameter, 
       *                      in terms of string.
       * 
       * @param path The other path to compare with this.
       *
       * @return `true` if this path is equal to @p path ; `false` 
       * if not. 
       */
		MEG_INLINE bool operator==(const path &path) const {
			return compare(path);
		}

		path &operator=(const path &path) = delete;
		path &operator=(const path &&path) = delete;
	};
};	  // namespace Meg::filesystem
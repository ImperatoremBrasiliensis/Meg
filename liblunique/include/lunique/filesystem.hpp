#pragma once

#include <lunique/common.hpp>
#include <lunique/debug.hpp>

#include <cstddef>
#include <cstring>

#ifdef _WIN32
#	define LUNIQUE_PATH_SEP (char) '\\'
#	define LUNIQUE_ALT_PATH_SEP (char) '/'
#else
#	define LUNIQUE_PATH_SEP (char) '/'
#	define LUNIQUE_ALT_PATH_SEP (char) '\\'
#endif

#define LUNIQUE_VIRTUAL_MARK

namespace lunique::filesystem {
	struct path {
	private:
		size_t size{0};
		size_t capacity{0};
		char *buffer{};

		path(const char *path, size_t size) noexcept;

	public:
		/**
		 * @brief A constructor for `path` class that
		 * receives a `std::string` object.
		 * 
		 * @param path A `std::string` object to initialize
		 * the `path` object.
		 */
		path(const std::string &path);

		/**
		 * @brief A constructor for the `path` class	 
		 * that receives a C-style string.
		 * 
		 * @param path A C-style string to initialize
		 * the `path` object.
		 */
		path(const char *path) noexcept;

		/**
		 * @brief The copy constructor for  the `path`
		 * class.
		 * 
		 * @param path Another `path` object. Must not
		 * be empty.
		 */
		path(const path &path) noexcept;

		/**
		 * @brief The move constructor for the `path`
		 * class.
		 * 
		 * @param path The object from which the data
		 * will be moved.
		 */
		path(path &&path) noexcept;

		/**
		 * @brief The default constructor for the `path`
		 * class.
		 */
		path() = default;

		/**
		 * @brief The destructor for the `path` class.
		 */
		~path();

		/**
		 * @brief Gets the file name from the path object.
		 *
		 * @warning The file name is the last entry of the path
		 * but not ended by a `PATH_SEP`.
		 * 
		 * @return path A `path` object containing the file name.
		 */
		path get_file_name() const noexcept;

		/**
		 * @brief Gets the file extension in the file name.
		 * 
		 * If the path has a file name, that is, the last entry not 
		 * ended by `PATH_SEP`, this method will return the file 
		 * extension. 
		 *
		 * @warning File extension is the last dot and everything after it
		 * in the file name.
		 * 
		 * @return path A `path` object containing the file extension
		 * including the dot.
		 */
		path get_file_extension() const noexcept;

		/**
		 * @brief Gets the file name without its extension.
		 *
		 * @warning File extension is the last dot and everything after it
		 * in the file name.
		 * 
		 * @return path A `path` object containing the file name without
		 * the extension, if there's one, from the path.
		 */
		path get_pure_file_name() const noexcept;

		/**
		 * @brief Gets the last directory name.
		 *
		 * @warning Directories are entries ended by a `PATH_SEP`.
		 * 
		 * @return path A `path` object containing the directory name. 
		 */
		path get_directory_name() const noexcept;

		/**
		 * @brief Gets the parent of this path.
		 *
		 * This function returns a `path` object with out the last
		 * entry of the this path.
		 * 
		 * @return path A `path` object containing the parent path,
		 * of this path.
		 */
		path get_parent_path() const noexcept;

		/**
		 * @brief Gets a lexically resolved path version of this path.
		 * 
		 * This method will return a version of this path with all 
		 * special entries (like '.' and '..') resoleved. If this 
		 * method finds a '.' entry, removes the entry if there's at
		 * least one before it. Also, if this method finds a '..' entry
		 * it removes the entry and the entry before it.
		 *
		 * EXAMPLE 
		 * @code txt
		 * The '.' special entry:
		 * "/folder/." -> "/folder/" // The special entry was removed.
		 * "./folder/"-> "./folder" // The special entry could not be removed.
		 *
 		 * The '..' special entry.
		 * "/folder1/folder2/../.." -> "/"; // Two entries removed (there are two '..' entry).
		 * "/folder/../.." -> "/.." // Just one entry was removed, the first entry was saved.
		 * @endcode
		 *
		 * @return path A `path` object containing the normalized path.
		 */
		path get_normalized_path() const noexcept;

		/**
		 * @brief Gets the size of this `path` object path.
		 * 
		 * @return size_t The size in bytes of the path.
		 */
		in_line size_t get_path_size() const noexcept {
			return size;
		}

		in_line size_t get_path_capacity() const {
			return capacity;
		}

		size_t get_entry_count() const;

		/**
		 * @brief Gets the path as a `std::string` object.
		 * 
		 * @return std::string The `std::string` object containing the 
		 * path.
		 */
		in_line std::string get_string() const noexcept {
			return {buffer};
		}

		/**
		 * @brief Gets a pointer (C-style string) to the path string.
		 * 
		 * @return const char* The C-style string, can become a dangling
		 * pointer if the this `path` object if its destroyed.
		 */
		in_line const char *get_cstring() const noexcept {
			return buffer;
		}

		/**
		 * @brief Changes the `path` object storage capcity
		 * to @p n bytes of memory.
		 *
		 * @warning The @p n parameter is rounded to the nearest
		 * multiple of 32.
		 * 
		 * @param n Quantity of memory to reserve in bytes. If
		 * 0, the memory is freed.
		 *
		 * @return `true` if successful,`false` if not. 
		 */
		bool reserve(size_t n) noexcept;

		/**
		 * @brief Changes the `path` object memory capacity
		 * to fit to the path size.
		 * 
		 * @return `true` if successful, `false` if not.
		 */
		bool fit_capacity() noexcept;

		/**
		 * @brief Erases @p n characters from the end
		 * of the path.
		 * 
		 * @param n How many characters to erase.
		 * 
		 * @return `true` if successful, `false` if not.
		 */
		bool erase(size_t n, size_t pos);

		/**
		 * @brief Removes the last entry in the path.
		 * 
		 * @return `true` if successful, `false` if not.
		 */
		bool remove() noexcept;

		/**
		 * @brief Replaces, adds, or removes the file name of
		 * the path.
		 *
		 * If the `path` object has no file name a new name will
		 * be added to it. If @p name is `nullptr` or is an empty
		 * string (`""`) the file name will be removed, if there
		 * is one.
		 * 
		 * @param name The new file name to the path.
		 * 
		 * @return `true` if successful, `false` if not.
		 */
		bool replace_file_name(const char *name) noexcept;

		/**
		 * @brief Replaces, adds or removes the file name extension
		 * of the path.
		 *
		 * If the `path` object file name has no extension a new will
		 * be added to it. If @p name is `nullptr` or is an empty 
		 * string, the extension will be removed, if there is one.
		 * 
		 * @param extension The new file name extension, without dot.
		 *
		 * @return `true` if successful, `false` if not.
		 */
		bool replace_file_extension(const char *extension) noexcept;

		/**
		 * @brief Clears the `path` object.
		 * 
		 * Frees the object memory reserve.
		 */
		void clear() noexcept;

		/**
		 * @brief Insert a string into the path at position
		 * @p pos .
		 * 
		 * @param str The string to be inserted.
		 * @param pos Where to insert the string. *Must* be less 
		 * than the path size.
		 *
		 * @return `true` if successful, `false` if not. 
		 */
		bool insert(const char *str, size_t pos) noexcept;

		/**
		 * @brief Assigns a new path string to the object.
		 * 
		 * @param path A string containing the new path.
		 *
		 * @return `true` if successful; `false` if not.
		 */
		bool assign(const char *path) noexcept;

		/**
		 * @brief Assigns a new path string to the object.
		 * 
		 * @param path A reference to a std::string object
		 * containing the new path.
		 *
		 * @return `true` if successful; `false` if not.
		 */
		bool assign(std::string &path) noexcept;

		/**
		 * @brief Appends a new element to the path.
		 *
		 * The new element will be added respecting the path
		 * separators.
		 *
		 * EXAMPLE.
		 * `folder + element` -> folder/element.
		 * `folder/ + element` -> folder/element.
		 * 
		 * If the new element begins with a path separator, that is,
		 * element is an absolute path, the path will be replaced by
		 * @p element .
		 * 
		 * @param element The new element to be appended to the path.
		 *
		 * @return `true` if successful; `false` if not.
		 */
		bool append(const char *element) noexcept;

		/**
		 * @brief Compares this and another `path` object.
		 * 
		 * @param path The other object to compare.
		 *
		 * @return int Return a value < 0 if this path is ordered
		 * before @p path ; return > 0 if @p path is ordered before
		 * this path; and return 0 if the paths are the same.
		 */
		int compare(const path &path) const noexcept;

		/**
		 * @brief Normalizes the path.
		 *
		 * If this function finds a '.' entry, removes the entry if
		 * there's at least one entru before it. Also, if this function
		 * finds a '..' entry it removes it and the entry before it.
		 *
		 * EXAMPLE 
		 * @code txt
		 * The '.' special entry:
		 * "/folder/." -> "/folder/" // The special entry was removed.
		 * "./folder/"-> "./folder" // The special entry could not be removed.
		 *
 		 * The '..' special entry.
		 * "/folder1/folder2/../.." -> "/"; // Two entries removed (there are two '..' entry).
		 * "/folder/../.." -> "/.." // Just one entry was removed, the first entry was saved.
		 * @endcode 
		 *
		 * @return `true` if successful; `false` if not.
		 */
		bool normalize() noexcept;

		/**
		 * @brief Makes the path virtual.
		 *
		 * If the path is not virtual, that is without a '?' character
		 * in the beginning of the path, and @p value is `true` it turns
		 * it into a virtual. Otherwise, if the path is vitual and @p value
		 * is `false`, turns it into a non-virtual path.
		 * 
		 * @param value A boolean that decides wheter the path will be 
		 * virtualized or not.
		 *
		 * @return `true` if this path virtuality was not the same value than
		 * @p value and `false` if not.
		 */
		bool virtualize(bool value);

		/**
		 * @brief Checks if the path has a file name.
		 *
		 * @warning The file name is the last entry of the path
		 * not ended by a `PATH_SEP`.
		 * 
		 * @return `true` if the path has a file name;
		 * `false` if not. 
		 */
		bool has_file_name() const noexcept;

		/**
		 * @brief Checks if the path has any directory name.
		 * 
		 * @warning Directories are entries ended by a `PATH_SEP`.
		 *
		 * @return `true` if the path has at least one direcory;
		 * `false` if not. 
		 */
		bool has_directory() const noexcept;

		/**
		 * @brief Checks if the path is absolute.
		 * 
		 * @return `true` if the path is absolute; `false` if not.
		 */
		bool is_absolute() const noexcept;

		/**
		 * @brief Checks if the path is a directory.
		 *
		 * @warning Directories are entries ended by `PATH_SEP`.
		 * 
		 * @return `true` if the path is a directory; `false` if not.
		 */
		bool is_directory() const noexcept;

		/**
		 * @brief Checks if the string is empty.
		 * 
		 * @return `true` if the path is empty;
		 `false` if not.
		 */
		bool is_empty() const noexcept;

		/**
		 * @brief Checks if the path is virtual.
		 *
		 * Vrtual paths begins with a '?' character.
		 * 
		 * @return `true` if the path is virtual; `false` if not.
		 */
		bool is_virtual() const noexcept;

		/**
		 * @brief Assigns a string to this path.
		 * 
		 * @param path The C-style string to be assigned.
		 */
		path &operator=(const char *path);

		/**
		 * @brief Copies the value of @p path to this `path` object.
		 * 
		 * @param path The other path where to copy the values from.
		 */
		path &operator=(const path &path);

		/**
		 * @brief Moves another path data to this `path` object.
		 * 
		 * @param path The other `path` object where to move values
		 * from.
		 */
		path &operator=(path &&path) noexcept;

		/**
		 * @brief Appends a new entry as a string to this path.
		 * 
		 * @details See @ref append().
		 *
		 * @param str The string to append to the path.
		 */
		in_line void operator/=(const char *str) noexcept {
			append(str);
		}

		/**
		 * @brief Compares this path to another for equality.
		 * 
		 * @param path The other path to compare with this.
		 *
		 * @return `true` if this path is equal to @p path ; `false` 
		 * if not. 
		 */
		in_line bool operator==(const path &path) const noexcept {
			return compare(path) == 0;
		}

		/**
		 * @brief Compares this path to another for inequality.
		 *
		 * @details See @ref compare(). 
		 *
		 * @param path The other path to comapare with this.
		 *
		 * @return `true` if this path is inequal to @p path ; `false`
		 * if not.
		 */
		in_line bool operator!=(const path &path) const noexcept {
			return compare(path) != 0;
		}

		/**
		 * @brief Compare whether this path is greater than or equal
		 * to another path.
		 * 
		 * @param path The other path to compare.
		 *
		 * @return `true` if tnis path is greater than or equal to
		 * @p path .
		 */
		in_line bool operator>=(const path &path) const noexcept {
			return size >= path.size;
		}

		/**
		 * @brief Compares whether another path is greater than or equal
		 * to this.
		 * 
		 * @param path The other path to compare.
		 *
		 * @return `true` if @p path is greater or equal to this path.
		 */
		in_line bool operator<=(const path &path) const noexcept {
			return size <= path.size;
		}

		/**
		 * @brief Compares whether this path is greater than another.
		 * 
		 * @param path The other path to compare.
		 *
		 * @return `true` if this path is greater to @p path ; `false` if
		 * not.
		 */
		in_line bool operator>(const path &path) const noexcept {
			return size > path.size;
		}

		/**
		 * @brief Compares if another path is greater than this.
		 * 
		 * @param path The other path to compare.
		 *
		 * @return `true` if anoter path is greater than this path; 
		 * `false` if not.
		 */
		in_line bool operator<(const path &path) const noexcept {
			return size < path.size;
		}

		/**
		 * @brief Implicit cast to a C-style string.
		 * 
		 * @return const char * A poiner to the path string.
		 */
		in_line operator const char *() {
			return buffer;
		}

		/**
		 * @brief Implicit cast to a `std::string` object.
		 * 
		 * @return A `std::string` object containing the path
		 * as a string.
		 */
		in_line operator std::string() {
			return {buffer};
		}

		/**
		 * @brief Implicit cast to `bool` type.
		 * 
		 * @return `true` if the path is not empty; `false` if 
		 * not.
		 */
		in_line operator bool() {
			return !is_empty();
		}

		/**
		 * @brief Iterator over the path entries.
		 *
		 * Incrementing goes to the next entry, decrementing goes
		 * to the previous entry and dereferencing returns all the
		 * entry as a `path` object.
		 */
		class iterator {
			friend path;

			const path *p;
			size_t pos;

			iterator(const path *path, size_t pos) noexcept:
					p(path),
					pos(pos) {
			}

		public:
			/**
			 * @brief Returns the current entry.
			 * 
			 * @return path A `path` object containing the entry which
			 * the iterator is pointing/representing.
			 */
			path operator*() const noexcept;

			/**
			 * @brief Goes to the next entry in the path.
			 * 
			 * @return iterator& The iterator pointing to the next
			 * entry in the path.
			 */
			iterator &operator++() noexcept;

			/**
			 * @brief Goes to the previous entry in the path.
			 * 
			 * @return iterator& The iterator pointing to the previous
			 * entry in the path.
			 */
			iterator &operator--() noexcept;

			/**
			 * @brief Compares two iterators for equality.
			 * 
			 * @return `true` if both iterators points to the
			 * same positon; `false` if not.
			 */
			inline bool operator==(const iterator &other) const noexcept {
				return pos == other.pos || p == other.p;
			}

			/**
			 * @brief Compares two iterators for inequality.
			 * 
			 * @return `true` if both iterators does not point
			 * to the same position; `false` if not.
			 */
			inline bool operator!=(const iterator &other) const noexcept {
				return pos != other.pos || p != other.p;
			}
		};

		/**
		 * @brief Returns an iterator to the first entry of the path.
		 * 
		 * @return iterator An `iterator` object to the first entry of
		 * path; returns end() if the path is empty.
		 */
		iterator begin() const noexcept;

		/**
		 * @brief Returns an iterator to one past the last entry of the 
		 * path.
		 *
		 * @warning This iterator marks the end of the path and **must not
		 * be dereferenced**. 
		 * 
		 * @return iterator Represeting the end of the path.
		 */
		iterator end() const noexcept;
	};
};	  // namespace lunique::filesystem
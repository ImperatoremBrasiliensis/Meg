/* 
 * ======================================
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (c) 2026 Elizeu S. Souza
 * ======================================
 */

#include <Meg/filesystem.hpp>

#include <Meg/debug.hpp>

#include <cctype>
#include <cstdlib>
#include <cstring>
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
	 * First, normalize. 
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
MEG_INLINE static bool is_special_entry(const char *entry) {
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

/*
 * Return a `file` object, with a pointer to the file name and to the 
 * extension in the file name, from the `path` string parameter 
 * (including extension).
 */
template<constable_cstring type>
MEG_INLINE static file_composition<type> file_name(type path, size_t size) {
	if (!path || !size)
		return {};

	file_composition<type> ret{};

	char *p = path + size;
	do {
		if (
			p == path ||
			*(p - 1) == MEG_PATH_SEP ||
			*(p - 1) == MEG_VIRTUAL_DELEGATE
		) {
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

filesystem::path::path(const char *path, size_t n) {
	if (!path || !n)
		return;

	// Reserves enough memory for the string & the terminator.
	if (!reserve(n + 1)) {
		dbg::throw_exception("Could not reserve memory", dbg::ALLOC_FAULT);
		return;
	}

	memmove(buffer, path, n);

	buffer[n] = '\0';
	size = n;

	return;
}

filesystem::path::path(const std::string &path):
		filesystem::path(path.c_str(), path.size()) {
}

filesystem::path::path(const char *str) {
	if (!str)
		return;

	size_t len = strlen(str);
	if (!len)
		return;

	if (!reserve(++len)) {
		dbg::throw_exception("Could not reserve memory", dbg::ALLOC_FAULT);
		return;
	}

	memcpy(buffer, str, len);
	size = normalize_separators(buffer);
	return;
}

filesystem::path::path(const path &other) {
	if (other.is_empty())
		return;

	if (!reserve(other.capacity)) {
		dbg::throw_exception("Could not reserve memory", dbg::ALLOC_FAULT);
		return;
	}

	memcpy(buffer, other.buffer, other.size + 1);
	size = other.size;
	return;
}

filesystem::path filesystem::path::get_filename() const {
	if (is_empty())
		return {};

	file_composition fcomp = file_name(buffer, size);
	path ret{fcomp.name, static_cast<size_t>((buffer + size) - fcomp.name)};
	if (is_special_entry(ret.get_string()))
		return {};

	return ret;
}

filesystem::path filesystem::path::get_file_extension() const {
	if (is_empty())
		return {};

	file_composition fcomp = file_name(buffer, size);
	if (is_special_entry(fcomp.name) || fcomp.name > fcomp.extension)
		return {};

	return {fcomp.extension, static_cast<size_t>((buffer + size) - fcomp.extension)};
}

filesystem::path filesystem::path::get_pure_file_name() const {
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

filesystem::path filesystem::path::get_directory_name() const {
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

filesystem::path filesystem::path::get_parent_path() const {
	if (is_empty())
		return {};

	char *end = buffer + size;
	while (end > buffer && *(end - 1) == MEG_PATH_SEP)
		end--;

	while (end > buffer && *(end - 1) != MEG_PATH_SEP)
		end--;

	return {buffer, static_cast<size_t>(end - buffer)};
}

filesystem::path filesystem::path::get_normalized_path() const {
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

		if (
			(temp == ".." || temp == "../") &&
			!is_special_entry(ret.get_directory_name().get_string()) &&
			ret.get_entry_count() > 1
		) {
			ret.remove();
			continue;
		} else if (
			(temp == "." || temp == "./") &&
			ret.get_entry_count()
		) {
			continue;
		}

		// If `temp` is not a special path entry simply adds it to `ret`.
		ret.insert(temp.buffer, ret.size);
	}

	ret.size = normalize_separators(ret.buffer);
	return ret;
}

size_t filesystem::path::get_entry_count() const {
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

bool filesystem::path::reserve(size_t n) {
	// Round the value to the mext value multiple of 32 after `n`.
	size_t size = MEG_ALIGN(n, CAPACITY_UNITY_SIZE);
	if (size > MEG_RESERVE_LIMIT) {
		dbg::throw_exception(
			"Reservation is too large, the limit is (" MEG_MACRO_TO_STRING(
				MEG_RESERVE_LIMIT
			) ")",
			dbg::CONSTRAINT_VIOLATION
		);
	}

	if (!n) {
		free(buffer);
		buffer = nullptr;
		capacity = 0;
		return true;
	}

	if (!buffer) {	  // Allocates.
		buffer = static_cast<char *>(malloc(size));
		if (!buffer)
			return false;

		capacity = size;
		buffer[0] = '\0';
		return true;
	} else {	// or reallocates.
		if (size > capacity || size < capacity / 3) {
			void *temp = realloc(buffer, size);
			if (!temp)
				return false;

			capacity = size;
			buffer = static_cast<char *>(temp);
			buffer[size] = '\0';
		}

		return true;
	}
}

bool filesystem::path::fit_capacity() {
	void *temp = realloc(buffer, size + 1);
	if (!temp)
		return false;

	buffer = static_cast<char *>(temp);
	capacity = size + 1;

	return true;
}

bool filesystem::path::erase(size_t n, size_t pos) {
	size_t rest = size - pos;

	if (is_empty() || pos > size || !n)
		return false;

	char *p = buffer + pos;
	size_t count = n > rest ? rest : n;

	memcpy(p, p + count,
		   rest + 1 - count);	 // 1 extra for the null terminator.
	size -= count;
	return true;
}

bool filesystem::path::remove() {
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

bool filesystem::path::replace_filename(const char *name) {
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

bool filesystem::path::replace_file_extension(const char *new_ext) {
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

void filesystem::path::clear() {
	reserve(0);
	size = 0;
}

bool filesystem::path::insert(const char *str, size_t pos) {
	if (!str)
		return false;
	if (pos > size)
		pos = size;

	size_t len = strlen(str);
	if (!len)
		return true;

	size_t new_size = size + len;
	if (!reserve(new_size + 1)) {
		dbg::throw_exception("Could not reserve memory", dbg::ALLOC_FAULT);
		return false;
	}

	char *dest = buffer + pos;
	memmove(dest + len, dest, (size + 1) - pos);	// This 1 is for the '\0' character.
	memmove(dest, str, len);

	size = new_size;
	return true;
}

bool filesystem::path::assign(const char *path) {
	if (!path || !*path) {
		clear();
		return true;
	}

	size_t len = strlen(path) + 1;
	if (!reserve(len)) {
		dbg::throw_exception("Could not allocate a memory reserve", dbg::ALLOC_FAULT);
		return false;
	}

	memmove(buffer, path, len);
	size = len - 1;
	return true;
}

bool filesystem::path::virtualize(bool value) {
	if (value && !is_virtual())
		insert("?", 0);
	else if (is_virtual())
		erase(1, 0);
	else
		return false;

	return true;
}

bool filesystem::path::append(const char *element) {
	if (!element)
		return false;

	size_t len = strlen(element) + 1;	 // 1 for the null terminator.
	if (len < 2)
		return false;

	bool is_virt = element[0] == MEG_VIRTUAL_DELEGATE;
	if (
		is_empty() ||
		element[is_virt] == MEG_PATH_SEP ||
		element[is_virt] == MEG_ALT_PATH_SEP
	) {
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
	dbg::throw_exception("Could not reserve memory", dbg::ALLOC_FAULT);
	return false;
}

bool filesystem::path::compare(const path &other) const {
	if (is_empty() && other.is_empty())
		return true;
	if (is_empty() || other.is_empty())
		return false;

	return !strcmp(buffer, other.buffer);
}

bool filesystem::path::normalize() {
	return assign(get_normalized_path().get_string());
}

bool filesystem::path::has_directories() const {
	if (is_empty())
		return false;

	for (const path &entry: *this)
		if (entry.is_directory())
			return true;

	return false;
}

bool filesystem::path::is_file() const {
	if (is_empty())
		return false;

	file_composition fcomp = file_name(buffer, size);
	if (fcomp.name && !is_special_entry(fcomp.name))
		return true;

	return false;
}

bool filesystem::path::is_absolute() const {
	if (is_empty())
		return false;

	return buffer[0] == MEG_PATH_SEP ||
		(buffer[0] == MEG_VIRTUAL_DELEGATE && buffer[1] == MEG_PATH_SEP);
}

bool filesystem::path::is_directory() const {
	if (is_empty())
		return false;

	file_composition fcomp = file_name(buffer, size);
	if (!fcomp.name || is_special_entry(fcomp.name))
		return true;

	return false;
}

bool filesystem::path::is_virtual() const {
	if (is_empty())
		return false;

	return *buffer == MEG_VIRTUAL_DELEGATE;
}

filesystem::path &filesystem::path::operator=(const char *path) {
	assign(path);
	return *this;
}

filesystem::path filesystem::path::iterator::operator*() const {
	size_t end = pos;

	if (p->is_empty())
		return {};

	while (end < p->size)
		if (p->buffer[end++] == MEG_PATH_SEP)
			break;

	return {p->buffer + pos, end - pos};
}

filesystem::path::iterator &filesystem::path::iterator::operator++() {
	if (p->is_empty())
		return *this;

	while (pos < p->size)
		if (p->buffer[pos++] == MEG_PATH_SEP)
			break;

	return *this;
}

#include <lunique/filesystem.hpp>

#include <lunique/debug.hpp>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <limits.h>

using namespace lunique;
namespace Joint = filesystem;

template<typename type>
concept constable_cstring =
	std::same_as<type, char *> || std::same_as<type, const char>;

/*
 * Turns all alternative separators (like '/' on Windows) to the
 * default system separator ('\' on Windows and '/' on Unix-like systems).
 */
static size_t normalize_separators(char *path) noexcept {
	char *p = path, *pend = strlen(path) + p;

	if (!p)
		return 0;

	/*
	 * First, normalize. 
	 * Turns all '/' to '\' (Windows) or '\' to '/' (Unix-like) & 
	 * removes illegal '?' characters.
	 */
	while (*p) {
		switch (*p) {
		case LUNIQUE_ALT_PATH_SEP:
			*p = LUNIQUE_PATH_SEP;
			break;
		case '?':
			/*
			 * The Virtual Mode delegated character
			 * ('?') must be at the beginning of the
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
		case LUNIQUE_PATH_SEP:
			char *i = p;
			while (*i == LUNIQUE_PATH_SEP)
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
in_line static bool is_special_entry(const char *entry) noexcept {
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
in_line static file_composition<type>
file_name(type path, size_t size) noexcept {
	if (!path || !size)
		return {};

	file_composition<type> ret{};

	char *p = path + size;
	do {
		if (p == path || *(p - 1) == LUNIQUE_PATH_SEP || *(p - 1) == '?') {
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
		if (*(--i) == LUNIQUE_PATH_SEP)
			break;
	}

	return ret;
}

Joint::path::path(const char *path, size_t n) noexcept {
	if (!path || !n)
		return;

	// Reserves enough memory for the string & the terminator.
	if (!reserve(n + 1)) {
		dbg::throw_exception(
			"Could not reserve memory",
			dbg::ALLOC_FAULT
		);
		return;
	}

	memmove(buffer, path, n);

	buffer[n] = '\0';
	size = n;

	return;
}

Joint::path::path(const std::string &path):
		filesystem::path(path.c_str(), path.size()) {
}

Joint::path::path(const char *path) noexcept {
	size_t len;

	if (!path)
		return;

	len = strlen(path) + 1;
	if (!reserve(len))
		goto alloc_fault;

	memcpy(buffer, path, len);
	size = normalize_separators(buffer);

	return;

alloc_fault:
	dbg::throw_exception(
		dbg::exception("Could not allocate a memory reserve", dbg::ALLOC_FAULT)
	);
}

Joint::path::path(const path &path) noexcept {
	size_t len = path.size + 1;

	if (this == &path)
		return;

	if (len > capacity)
		if (!reserve(len))
			goto alloc_fault;

	memcpy(buffer, path.buffer, len);
	size = len - 1;

	return;

alloc_fault:
	dbg::throw_exception(
		dbg::exception(
			"Could not allocate a memory reserve for `filesystem::path`",
			dbg::ALLOC_FAULT
		)
	);
}

Joint::path::path(path &&path) noexcept:
		size(path.size),
		capacity(path.capacity),
		buffer(path.buffer) {
	path.buffer = nullptr;
	path.size = 0;
	path.capacity = 0;
}

Joint::path::~path() {
	clear();
}

bool Joint::path::reserve(size_t n) noexcept {
	// Round the value to the nearest multiple of 32.
	size_t size = (n + 31) & ~(31);

	if (!n) {
		free(buffer);
		buffer = nullptr;
		capacity = 0;
		return true;
	}

	// Allocates.
	if (!buffer) {
		buffer = static_cast<char *>(malloc(size));
		if (!buffer)
			return false;

		capacity = size;
		buffer[size] = '\0';
		return true;
	}
	// or reallocates.
	else {
		if (size > capacity || size <= capacity / 3) {
			void *temp = realloc(buffer, size);
			if (!temp)
				return false;

			buffer = static_cast<char *>(temp);
			capacity = size;
			buffer[size] = '\0';
		}

		return true;
	}
}

bool Joint::path::fit_capacity() noexcept {
	void *temp = realloc(buffer, size + 1);
	if (!temp)
		return false;

	buffer = static_cast<char *>(temp);
	capacity = size + 1;

	return true;
}

bool Joint::path::erase(size_t n, size_t pos) {
	if (is_empty() || !n || pos > size)
		return false;

	char *p = buffer + pos;
	size_t rest = size - pos;
	size_t count = n > rest ? rest : n;

	memcpy(p, p + count, rest + 1 - count);	   // 1 extra for the null terminator.
	size -= count;
	return true;
}

bool Joint::path::remove() noexcept {
	char *p = buffer + size - 1;
	while (p > buffer) {
		p--;
		if (*p == LUNIQUE_PATH_SEP) {
			*(++p) = '\0';
			size = p - buffer;
			return true;
		}
	}

	*buffer = '\0';
	size = 0;
	return true;
}

bool Joint::path::replace_file_name(const char *name) noexcept {
	file_composition f = file_name(buffer, size);

	if (f.name == buffer + size) {	  // If the file don't exists.
		return insert(name, size);
	} else if (remove())	// If the file exists.
		return insert(name, size);

	return false;
}

bool Joint::path::replace_file_extension(const char *new_ext) noexcept {
	if (!has_file_name())
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

void Joint::path::clear() noexcept {
	reserve(0);
	size = 0;
}

bool Joint::path::insert(const char *str, size_t pos) noexcept {
	if (!str || pos > size)
		return false;

	size_t len = strlen(str);
	if (!len)
		return false;

	size_t new_size = size + len;
	if (!reserve(new_size + 1)) {
		dbg::throw_exception("Could not reserve memory", dbg::ALLOC_FAULT);
		return false;
	}

	char *dest = buffer + pos;
	memmove(dest + len, dest, (size + 1) - pos);	// This 1 is for the '\0' character.
	memcpy(dest, str, len);

	size = new_size;
	return true;
}

bool Joint::path::assign(const char *path) noexcept {
	size_t len = strlen(path) + 1;

	if (!reserve(len)) {
		dbg::throw_exception(
			"Could not allocate a memory reserve",
			dbg::ALLOC_FAULT
		);
		return false;
	}

	memcpy(buffer, path, len);
	size = len - 1;
	return true;
}

bool Joint::path::assign(std::string &path) noexcept {
	return assign(path.c_str());
}

bool Joint::path::virtualize(bool value) {
	if (value && !is_virtual())
		insert("?", 0);
	else if (is_virtual())
		erase(1, 0);
	else
		return false;

	return true;
}

bool Joint::path::append(const char *element) noexcept {
	if (!element)
		return false;

	size_t len = strlen(element) + 1;	 // 1 for the null terminator.
	if (len < 2)
		return false;

	bool is_virt = element[0] == '?';
	if (is_empty() || element[is_virt] == LUNIQUE_PATH_SEP ||
		element[is_virt] == LUNIQUE_ALT_PATH_SEP) {
		if (!reserve(len))
			goto alloc_fault;

		memcpy(buffer, element, len);
		size = normalize_separators(buffer);
		return true;
	} else {
		if (is_virt)
			len--, element++;

		size_t offset =
			!is_directory();	// Space to insert a `LUNIQUE_PATH_SEP` at the end, if there's no.
		size_t new_cap = size + len + offset;
		if (!reserve(new_cap))
			goto alloc_fault;

		char *buf_end = buffer + size;
		memcpy(buf_end + offset, element, len);
		if (offset)
			*buf_end = LUNIQUE_PATH_SEP;
		size = normalize_separators(buffer);
		return true;
	}

alloc_fault:
	dbg::throw_exception("Could not reserve memory", dbg::ALLOC_FAULT);
	return false;
}

int Joint::path::compare(const path &path) const noexcept {
	if (!this->buffer && !path.buffer)
		return 0;
	else if (!this->buffer && path.buffer)
		return -1;
	else if (this->buffer && !path.buffer)
		return 1;

	return strcmp(buffer, path.buffer);
}

bool Joint::path::normalize() noexcept {
	return assign(get_normalized_path());
}

bool Joint::path::has_file_name() const noexcept {
	if (is_empty() || buffer[size - 1] == LUNIQUE_PATH_SEP || buffer[size - 1] == '?') {
		return false;
	}

	return true;
}

bool Joint::path::has_directory() const noexcept {
	if (is_empty())
		return false;

	for (char *p = buffer + size; p > buffer; p--) {
		if (*p == LUNIQUE_PATH_SEP)
			return true;
	}

	return false;
}

bool Joint::path::is_absolute() const noexcept {
	if (is_empty())
		return false;

	return buffer[0] == LUNIQUE_PATH_SEP || (buffer[0] == '?' && buffer[1] == LUNIQUE_PATH_SEP);
}

bool Joint::path::is_empty() const noexcept {
	return !size;
}

bool Joint::path::is_directory() const noexcept {
	if (is_empty())
		return false;

	return buffer[size - 1] == LUNIQUE_PATH_SEP;
}

bool Joint::path::is_virtual() const noexcept {
	if (is_empty())
		return false;

	return *buffer == '?';
}

Joint::path Joint::path::get_file_name() const noexcept {
	if (is_empty())
		return {};

	file_composition fcomp = file_name(buffer, size);
	path ret{fcomp.name, static_cast<size_t>((buffer + size) - fcomp.name)};
	if (is_special_entry(ret))
		return {};

	return ret;
}

Joint::path Joint::path::get_file_extension() const noexcept {
	if (is_empty())
		return {};

	file_composition fcomp = file_name(buffer, size);
	if (is_special_entry(fcomp.name))
		return {};

	return {
		fcomp.extension,
		static_cast<size_t>((buffer + size) - fcomp.extension)
	};
}

Joint::path Joint::path::get_pure_file_name() const noexcept {
	if (is_empty())
		return {};

	file_composition fcomp = file_name(buffer, size);
	if (is_special_entry(fcomp.name))
		return {};

	return {
		fcomp.name,
		static_cast<size_t>(
			(fcomp.extension ? fcomp.extension : buffer + size) - fcomp.name
		)
	};
}

Joint::path Joint::path::get_directory_name() const noexcept {
	if (is_empty())
		return {};

	char *buf = buffer + is_virtual();

	// Looks for the last path separator `LUNIQUE_PATH_SEP`.
	char *p = buffer + size;
	while (p > buf && *(p - 1) != LUNIQUE_PATH_SEP)
		p--;

	// And looks for the second last bar or the beginning of the buffer.
	char *end = p;
	while (p > buf) {
		p--;
		if (*(p - 1) == LUNIQUE_PATH_SEP)
			break;
	}

	return {p, static_cast<size_t>(end - p)};
}

Joint::path Joint::path::get_parent_path() const noexcept {
	if (is_empty())
		return {};

	char *end = buffer + size;
	while (end > buffer && *(end - 1) == LUNIQUE_PATH_SEP)
		end--;

	while (end > buffer && *(end - 1) != LUNIQUE_PATH_SEP)
		end--;

	return {buffer, static_cast<size_t>(end - buffer)};
}

Joint::path Joint::path::get_normalized_path() const noexcept {
	path ret;
	char *p1, *p2 = buffer;
	char *bend = buffer + size;

	if (is_empty())
		return {};

	while (p2 < bend) {
		// First: gets a path entry and stores it into the `temp` object.
		p1 = p2;
		while (p2 < bend && *p2 != LUNIQUE_PATH_SEP)
			p2++;
		path temp{p1, static_cast<size_t>(++p2 - p1)};

		// Second: checks if `temp` is a special path entry and try to solve it.
		if (
			(temp == path("..") || temp == path("../")) &&
			ret.get_entry_count() > 1
		) {
			// If `ret` has at least one entry removes the last entry from `ret`.
			ret.remove();
			continue;
		} else if (
			(temp == path(".") || temp == path("./")) && !ret.is_empty()
		) {
			// If `ret` has at least one entry doesn't add `temp` to it.
			continue;
		}

		// If `temp` is not a special path entry simply adds it to `ret`.
		ret.append(temp);
	}

	return ret;
}

size_t Joint::path::get_entry_count() const {
	if (is_empty())
		return 0;

	size_t count = 0;
	char *buf_end = buffer + size - 1;
	char *p = buffer + is_virtual();
	while (p <= buf_end) {
		if (*p == LUNIQUE_PATH_SEP || p == buf_end)
			count++;
		p++;
	}

	return count;
}

Joint::path &Joint::path::operator=(const path &path) {
	if (this == &path)
		return *this;

	if (!reserve(path.capacity)) {
		dbg::throw_exception(
			"Could not reserve memory",
			dbg::ALLOC_FAULT
		);
		return *this;
	}

	size = path.size;
	if (path.buffer)
		memcpy(buffer, path.buffer, size + 1);

	return *this;
}

Joint::path &Joint::path::operator=(path &&path) noexcept {
	buffer = path.buffer;
	size = path.size;
	capacity = path.capacity;

	path.buffer = nullptr;
	path.size = 0;
	path.capacity = 0;

	return *this;
}

Joint::path &Joint::path::operator=(const char *path) {
	if (!path)
		return *this;

	size_t len = strlen(path) + 1;
	if (!reserve(len)) {
		dbg::throw_exception(
			"Could not reserve memory",
			dbg::ALLOC_FAULT
		);
		return *this;
	}

	memcpy(buffer, path, len);
	size = len - 1;

	return *this;
}

Joint::path Joint::path::iterator::operator*() const noexcept {
	size_t end = pos;

	if (p->is_empty())
		return {};

	while (end < p->size)
		if (p->buffer[end++] == LUNIQUE_PATH_SEP)
			break;

	return {p->buffer + pos, end - pos};
}

Joint::path::iterator &Joint::path::iterator::operator++() noexcept {
	if (p->is_empty())
		return *this;

	while (pos < p->size)
		if (p->buffer[pos++] == LUNIQUE_PATH_SEP)
			break;

	return *this;
}

Joint::path::iterator &Joint::path::iterator::operator--() noexcept {
	if (p->is_empty())
		return *this;

	while (pos > 0 && p->buffer[pos] != LUNIQUE_PATH_SEP)
		pos--;

	return *this;
}

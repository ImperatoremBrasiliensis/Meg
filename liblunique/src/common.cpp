#include <lunique/common.hpp>

#include <cstdlib>

using namespace lunique;

common::mutex::mutex(bool locked) noexcept:
		locked(false) {
	if (locked)
		lock();
}

common::mutex::mutex() noexcept:
		locked(false) {
}

void common::mutex::lock() noexcept {
	bool expected = false;
	while (locked.compare_exchange_weak(expected, true)) {
		expected = false;
		std::this_thread::yield();
		owner = std::this_thread::get_id();
	}
}

void common::mutex::unlock() noexcept {
	if (std::this_thread::get_id() == owner)
		locked.store(false);
}
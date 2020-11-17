#pragma once

#include <Windows.h>
#include <memory>

namespace l3d::util {
struct HANDLEDeleter {
	void operator()(HANDLE handle) const
	{
		if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
	}
};

using HANDLE_unique_ptr = std::unique_ptr<void, HANDLEDeleter>;

HANDLE_unique_ptr make_HANDLE_unique_ptr(HANDLE handle)
{
	if (handle == INVALID_HANDLE_VALUE || handle == nullptr) {
		// handle error...
		return nullptr;
	}

	return HANDLE_unique_ptr(handle);
}
} // namespace l3d::util

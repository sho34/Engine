#pragma once

using namespace Concurrency;

namespace FileSystem {
	Concurrency::task<std::vector<byte>> ReadFileAsync(const std::wstring& filename);
}
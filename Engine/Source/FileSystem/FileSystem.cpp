#include "pch.h"
#include "FileSystem.h"
#include "../Common/DirectXHelper.h"

// Función que lee desde un archivo binario de forma asincrónica.
Concurrency::task<std::vector<byte>> FileSystem::ReadFileAsync(const std::wstring& filename)
{
	HANDLE fileHandle = CreateFileW(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	LARGE_INTEGER fileSize;
	GetFileSizeEx(fileHandle, &fileSize);
	std::vector<byte> returnBuffer;
	returnBuffer.resize(fileSize.QuadPart);
	DWORD NumberOfBytesRead;
	DX::ThrowIfFailed(ReadFile(fileHandle, &returnBuffer[0], (DWORD)fileSize.QuadPart, &NumberOfBytesRead, nullptr));
	CloseHandle(fileHandle);
	return Concurrency::task_from_result<std::vector<byte>>(returnBuffer);
}
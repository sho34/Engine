#pragma once

#include "../framework.h"
#include <ppltasks.h>	// Para create_task

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Configure un punto de interrupción en esta línea para detectar errores de la API Win32.
			//throw Platform::Exception::CreateException(hr);
			throw std::exception();
		}
	}

	/*
	// Función que lee desde un archivo binario de forma asincrónica.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([](StorageFile^ file)
		{
			return FileIO::ReadBufferAsync(file);
		}).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}*/

	// Función que lee desde un archivo binario de forma asincrónica.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Concurrency;
			
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

	inline Concurrency::task<std::vector<byte>> ReadShader(const std::wstring& shaderName,const std::string target) {
		using namespace Concurrency;

		const std::wstring filename = L"Shaders/" + shaderName + L".hlsl";
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		compileFlags |= D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
		ComPtr<ID3DBlob> shaderBlob;
		ComPtr<ID3DBlob> shaderError;
		DX::ThrowIfFailed(D3DCompileFromFile(filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", target.c_str(), compileFlags, 0, &shaderBlob, &shaderError));

		std::vector<byte> returnBuffer(shaderBlob->GetBufferSize());
		memcpy(&returnBuffer[0], shaderBlob->GetBufferPointer(), returnBuffer.size());
		return Concurrency::task_from_result<std::vector<byte>>(returnBuffer);
	}

	/*
	// Convierte una longitud expresada en píxeles independientes del dispositivo (PID) en una longitud expresada en píxeles físicos.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Redondear al entero más próximo.
	}
*/
	// Asignar un nombre al objeto para facilitar la depuración.
#if defined(_DEBUG)
	inline void SetName(ID3D12Object* pObject, LPCWSTR name)
	{
		pObject->SetName(name);
	}
#else
	inline void SetName(ID3D12Object*, LPCWSTR)
	{
	}
#endif

}

// Nombrar función del asistente para ComPtr<T>.
// Asigna el nombre de la variable como nombre del objeto.
#define NAME_D3D12_OBJECT(x) DX::SetName(x.Get(), L#x)

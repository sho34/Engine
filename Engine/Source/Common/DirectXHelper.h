#pragma once

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

	// Función que lee desde un archivo binario de forma asincrónica.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::string& filename)
	{
		using namespace Concurrency;
			
		HANDLE fileHandle = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		LARGE_INTEGER fileSize;
		GetFileSizeEx(fileHandle, &fileSize);
		std::vector<byte> returnBuffer;
		returnBuffer.resize(fileSize.QuadPart);
		DWORD NumberOfBytesRead;
		DX::ThrowIfFailed(ReadFile(fileHandle, &returnBuffer[0], (DWORD)fileSize.QuadPart, &NumberOfBytesRead, nullptr));
		CloseHandle(fileHandle);
		return Concurrency::task_from_result<std::vector<byte>>(returnBuffer);
	}

	inline Concurrency::task<std::vector<byte>> ReadShader(const std::string& shaderName,const std::string target) {
		using namespace Concurrency;

		const std::string filename = "Shaders/" + shaderName + ".hlsl";
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		compileFlags |= D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
		ComPtr<ID3DBlob> shaderBlob;
		ComPtr<ID3DBlob> shaderError;
		DX::ThrowIfFailed(D3DCompileFromFile(StringToWString(filename).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", target.c_str(), compileFlags, 0, &shaderBlob, &shaderError));

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

	inline void MatrixDump(DirectX::XMMATRIX& wvp)
	{
		std::string row1 = "[" +
			std::to_string(wvp.r[0].m128_f32[0]) + "," +
			std::to_string(wvp.r[0].m128_f32[1]) + "," +
			std::to_string(wvp.r[0].m128_f32[2]) + "," +
			std::to_string(wvp.r[0].m128_f32[3]) + "]";
		std::string row2 = "[" +
			std::to_string(wvp.r[1].m128_f32[0]) + "," +
			std::to_string(wvp.r[1].m128_f32[1]) + "," +
			std::to_string(wvp.r[1].m128_f32[2]) + "," +
			std::to_string(wvp.r[1].m128_f32[3]) + "]";
		std::string row3 = "[" +
			std::to_string(wvp.r[2].m128_f32[0]) + "," +
			std::to_string(wvp.r[2].m128_f32[1]) + "," +
			std::to_string(wvp.r[2].m128_f32[2]) + "," +
			std::to_string(wvp.r[2].m128_f32[3]) + "]";
		std::string row4 = "[" +
			std::to_string(wvp.r[3].m128_f32[0]) + "," +
			std::to_string(wvp.r[3].m128_f32[1]) + "," +
			std::to_string(wvp.r[3].m128_f32[2]) + "," +
			std::to_string(wvp.r[3].m128_f32[3]) + "]";
		std::string matrixDump = row1 + "\n" + row2 + "\n" + row3 + "\n" + row4 + "\n";

		OutputDebugStringA("Matrix\n");
		OutputDebugStringA(matrixDump.c_str());
	}
}

// Nombrar función del asistente para ComPtr<T>.
// Asigna el nombre de la variable como nombre del objeto.
#define NAME_D3D12_OBJECT(x) DX::SetName(x.Get(), L#x)
#define CCNAME_D3D12_OBJECT(x) x->SetName(L#x)

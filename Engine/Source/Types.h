#pragma once
#include <cstdint>
#include <map>

#include "Shaders/Compiler/ShaderCompilerOutput.h"

//notifications
struct NotificationTarget {
	bool operator()(const NotificationTarget& a, const NotificationTarget& b) const {
		return (intptr_t)a.target == (intptr_t)b.target;
	}
	bool operator < (const NotificationTarget& b) const {
		return (intptr_t)this->target == (intptr_t)b.target;
	}
	void* target;
};

struct NotificationCallbacks {
	void (*onLoadStart)(void* target) = nullptr;
	void (*onLoadComplete)(void* target, void* source) = nullptr;
	void (*onDestroy)(void* target, void* source) = nullptr;
};
typedef std::map<NotificationTarget, NotificationCallbacks> ChangesNotifications;
template<typename T>
using TemplatesNotification = std::map<T, ChangesNotifications>;

template<typename T>
void NotifyOnLoadStart(ChangesNotifications& notifications) {
	for (auto &[target, callbacks] : notifications) {
		callbacks.onLoadStart(target.target);
	}
}

template<typename T>
void NotifyOnLoadComplete(T* source, ChangesNotifications& notifications) {
	for (auto& [target, callbacks] : notifications) {
		callbacks.onLoadComplete(target.target, (void*)(source));
	}
}

template<typename T>
void NotifyOnDestroy(T* source, ChangesNotifications& notifications) {
	for (auto& [target, callbacks] : notifications) {
		callbacks.onDestroy(target.target, (void*)(source));
	}
}

enum MaterialVariablesTypes {
	MAT_VAR_BOOLEAN,
	MAT_VAR_INTEGER,
	MAT_VAR_UNSIGNED_INTEGER,
	MAT_VAR_RGB,
	MAT_VAR_RGBA,
	MAT_VAR_FLOAT,
	MAT_VAR_FLOAT2,
	MAT_VAR_FLOAT3,
	MAT_VAR_FLOAT4,
	MAT_VAR_MATRIX4X4
};

static const std::map<MaterialVariablesTypes, std::wstring> MaterialVariablesNames = {
	{MaterialVariablesTypes::MAT_VAR_BOOLEAN, L"BOOLEAN"},
	{MaterialVariablesTypes::MAT_VAR_INTEGER, L"INTEGER"},
	{MaterialVariablesTypes::MAT_VAR_UNSIGNED_INTEGER, L"UNSIGNED_INTEGER"},
	{MaterialVariablesTypes::MAT_VAR_RGB, L"RGB"},
	{MaterialVariablesTypes::MAT_VAR_RGBA, L"RGBA"},
	{MaterialVariablesTypes::MAT_VAR_FLOAT, L"FLOAT"},
	{MaterialVariablesTypes::MAT_VAR_FLOAT2, L"FLOAT2"},
	{MaterialVariablesTypes::MAT_VAR_FLOAT3, L"FLOAT3"},
	{MaterialVariablesTypes::MAT_VAR_FLOAT4, L"FLOAT4"},
	{MaterialVariablesTypes::MAT_VAR_MATRIX4X4, L"MATRIX4X4"},
};

static const std::map<MaterialVariablesTypes, size_t> MaterialVariablesSizes = {
	{MaterialVariablesTypes::MAT_VAR_BOOLEAN, sizeof(bool)},
	{MaterialVariablesTypes::MAT_VAR_INTEGER, sizeof(int)},
	{MaterialVariablesTypes::MAT_VAR_UNSIGNED_INTEGER, sizeof(unsigned int)},
	{MaterialVariablesTypes::MAT_VAR_RGB, sizeof(float[3])},
	{MaterialVariablesTypes::MAT_VAR_RGBA, sizeof(float[4])},
	{MaterialVariablesTypes::MAT_VAR_FLOAT, sizeof(float)},
	{MaterialVariablesTypes::MAT_VAR_FLOAT2, sizeof(float[2])},
	{MaterialVariablesTypes::MAT_VAR_FLOAT3, sizeof(float[3])},
	{MaterialVariablesTypes::MAT_VAR_FLOAT4, sizeof(float[4])},
	{MaterialVariablesTypes::MAT_VAR_MATRIX4X4, sizeof(float[16])},
};

struct MaterialVariableInitialValue {
	MaterialVariablesTypes variableType;
	std::any any_value;
};
typedef std::map<std::wstring, MaterialVariableInitialValue> MaterialInitialValueMap;

struct MaterialVariableMapping {
	MaterialVariablesTypes variableType;
	CBufferVariable mapping;
};
typedef std::map<std::wstring, MaterialVariableMapping> MaterialVariablesMapping;
#pragma once

namespace AudioSystem {

	void InitAudio();
	void ShutdownAudio();
	void UpdateAudio();
	void UpdateListener(XMFLOAT3 position, XMFLOAT3 forward, XMFLOAT3 up);
	std::unique_ptr<DirectX::AudioEngine>& GetAudioEngine();
}


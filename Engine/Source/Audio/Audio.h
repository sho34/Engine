#pragma once

namespace Audio {

	void InitAudio();
	void ShutdownAudio();
	void UpdateAudio();
	void UpdateListener(XMFLOAT3 position, XMFLOAT3 forward, XMFLOAT3 up);
	
	std::unique_ptr<AudioEngine>& GetAudio();
	
}


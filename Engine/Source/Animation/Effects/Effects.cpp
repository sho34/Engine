#include "pch.h"
#include "Effects.h"

namespace Animation::Effects {

	void EffectsStep(FLOAT delta)
	{

		for (auto& [fx, StepFn] : StepEffects)
		{
			StepFn(delta);
		}

	}

	void WriteEffectsConstantsBuffer(UINT backbufferIndex)
	{
		for (auto& [fx, WriteCbFn] : WriteEffectsConstantsBuffers)
		{
			WriteCbFn(backbufferIndex);
		}
	}

}
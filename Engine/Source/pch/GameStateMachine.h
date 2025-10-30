#pragma once
#include <map>

template<typename T>
struct GameStatesMachine {
	T currentState;
	std::map<T, std::function<void(T)>> onEnter;
	std::map<T, std::function<void(T)>> onLeave;
	std::map<T, std::function<void()>> onStep;
	std::map<T, std::function<void()>> onRender;
	std::map<T, std::function<void()>> onPostRender;

	void ChangeState(T newState)
	{
		T prevState = currentState;
		if (onLeave.contains(currentState)) { onLeave.at(currentState)(newState); }
		currentState = newState;
		if (onEnter.contains(currentState)) { onEnter.at(currentState)(prevState); }
	}

	void Step()
	{
		if (onStep.contains(currentState)) { onStep.at(currentState)(); }
	}

	void Render()
	{
		if (onRender.contains(currentState)) { onRender.at(currentState)(); }
	}

	void PostRender()
	{
		if (onPostRender.contains(currentState)) { onPostRender.at(currentState)(); }
	}
};
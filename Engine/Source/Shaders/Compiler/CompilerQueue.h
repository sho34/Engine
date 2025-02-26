#pragma once

//compile queue definition
struct CompilerQueue
{
	CompilerQueue() {
		queue = std::queue<std::string>();
	}

	void push(const std::string& value) {
		std::lock_guard<std::mutex> lock(mutex);
		if (queue.size() == 0 || queue.front() != value) {
			queue.push(value);
		}
	}

	void pop() {
		std::lock_guard<std::mutex> lock(mutex);
		queue.pop();
	}

	std::string front() {
		std::lock_guard<std::mutex> lock(mutex);
		return queue.front();
	}

	size_t size() {
		std::lock_guard<std::mutex> lock(mutex);
		return queue.size();
	}

	std::queue<std::string> queue;
	mutable std::mutex mutex;
};

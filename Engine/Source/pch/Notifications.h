#pragma once

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


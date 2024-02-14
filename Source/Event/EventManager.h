#ifndef EVENT_DISPATCHER
#define EVENT_DISPATCHER

#include <typeindex>
#include <functional>
#include <map>
#include <vector>

//---------------------------------------------------------------------------------
// Source: codereview.stackexchange.com/questions/252884/c-event-system-game-engine
//---------------------------------------------------------------------------------
class EventManager {
public:
	template<typename TEvent>
	void Connect(std::function<void(TEvent)> callback);

	template<typename TEvent, typename TClass>
	void Connect(TClass* instance, void (TClass::* memberFunction)(const TEvent&));

	template<typename TEvent, typename... TArgs>
	void Fire(TArgs... args);
private:
	std::map<std::type_index, std::vector<std::function<void(void*)>>> listeners;
};

template<typename TEvent>
void EventManager::Connect(std::function<void(TEvent)> callback) {
	auto wrapper = [callback](void* evt) {
		callback(*static_cast<TEvent*>(evt));
		};

	// Simplified for demonstration; actual implementation may vary
	listeners[typeid(TEvent)].push_back(wrapper);
}

template<typename TEvent, typename TClass>
void EventManager::Connect(TClass* instance, void (TClass::* memberFunction)(const TEvent&)) {
	std::function<void(TEvent)> callback = [instance, memberFunction](const TEvent& event) {
		(instance->*memberFunction)(event);
		};

	Connect<TEvent>(callback);
}

template<typename TEvent, typename... TArgs>
void EventManager::Fire(TArgs... args) {
	TEvent event{ args... };

	auto& callbacks = listeners[typeid(TEvent)];
	for (auto& callback : callbacks) {
		callback(&event);
	}
}

extern EventManager gEventManager;

#endif 

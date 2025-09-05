#pragma once
#include <functional>
#include <SceneObject.h>
#include <map>

struct Binder {
	std::multimap<std::shared_ptr<SceneObject>, std::shared_ptr<SceneObject>> binding;

	void insert(std::shared_ptr<SceneObject> soA, std::shared_ptr<SceneObject> soB)
	{
		binding.insert({ soA,soB });
		binding.insert({ soB,soA });
		soA->Bind(soB);
		soB->Bind(soA);
	}

	void erase(std::shared_ptr<SceneObject> S)
	{
		/*
		-sacamos el listado de objetos asociados al SceneObject(S) y los guardamos en un set(ST)
		-eliminamos SceneObject(S) de binding
		-recorremos el set(ST) llamando a S->Unbind(ST#)
		-recorremos el set(ST) llamando a ST(#)->Unbind(SceneObject(S))
		-recorremos el set(ST) y eliminamos binding{ ST(#),SceneObject(S) }
		*/
	}

	/*
	void insert(const std::string& s, int i) {
		forward_map[s] = i;
		reverse_map[i] = s;
	}
	*/
};

/*
#define BINDER(A,B)\
std::map<std::string,std::string> A##BindedTo##B;\
std::function<void()> Bind(std::shared_ptr<A> a, std::shared_ptr<B> b)\
{\
	std::string uuidA = a->uuid(); \
	std::string uuidB = b->uuid(); \
	a->Bind(b); \
	A##BindedTo##B.insert_or_assign(uuidA, uuidB); \
	return[a, b] {\
		a->Unbind(b);\
		A##BindedTo##B.erase(uuidA); \
	}; \
}\
*/
#pragma once
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>

using EntityID = uint32_t;

struct ComponentStorageI
{
	virtual ~ComponentStorageI() = default;
	virtual void Remove(EntityID entity) = 0;
	virtual bool Has(EntityID entity) const = 0;
};

template<typename ComponentType>
class ComponentStorage : public ComponentStorageI
{
public:
	ComponentStorage() = default;
	~ComponentStorage() override = default;
public:
	void Remove(EntityID entity) override
	{
		data.erase(entity);
	}
	bool Has(EntityID entity) const override
	{
		return data.find(entity) != data.end();
	}
	ComponentType& Set(EntityID entity, const ComponentType& component = ComponentType())
	{
		return data[entity] = component;
	}
	ComponentType& Get(EntityID entity)
	{
		return data.at(entity);
	}
	std::unordered_map<EntityID, ComponentType>& GetAll() { return data; }
private:
	std::unordered_map<EntityID, ComponentType> data;
};

class Registry
{
public:
	Registry() = default;
	~Registry() = default;

public:
	//This returns an unique EntityID
	EntityID CreateEntity()
	{
		return nextEntityID++;
	}

	//This removes all components associated with the provided entity
	void RemoveEntity(EntityID entity)
	{
		for (auto& [type, storage] : componentStorages)
		{
			if (storage->Has(entity))
			{
				storage->Remove(entity);
			}
		}
	}

	template<typename ComponentType>
	void RegisterComponentType()
	{
		std::type_index type(typeid(ComponentType));

		if (componentStorages.find(type) == componentStorages.end())
		{
			componentStorages[type] = std::make_unique<ComponentStorage<ComponentType>>();
		}
	}

	//This adds a provided component
	template<typename ComponentType>
	void SetComponent(EntityID entity, const ComponentType& component)
	{
		std::type_index type = std::type_index(typeid(ComponentType));

		RegisterComponentType<ComponentType>();

		auto* storage = static_cast<ComponentStorage<ComponentType>*>(
			componentStorages.at(type).get()
		);
		storage->Set(entity, component);
	}
	
	//This Adds a default constructed component which then can be modified or replaced
	template<typename ComponentType>
	void AddComponent(EntityID entity)
	{
		ComponentType component{};
		SetComponent<ComponentType>(entity, component);
	}

	//Removes a component from an entity
	template<typename ComponentType>
	void RemoveComponent(EntityID entity)
	{
		std::type_index type = std::type_index(typeid(ComponentType));

		auto* storage = static_cast<ComponentStorage<ComponentType>*>(
			componentStorages.at(type).get()
		);
		if (storage->Has(entity))
		{
			storage->Remove(entity);
		}
	}

	//Checks if an entity has a specific component
	template<typename ComponentType>
	ComponentType& GetComponent(EntityID entity)
	{
		std::type_index type = std::type_index(typeid(ComponentType));

		auto* storage = static_cast<ComponentStorage<ComponentType>*>(
			componentStorages.at(type).get()
		);
		if (storage->Has(entity))
		{
			return storage->Get(entity);
		}
		throw std::runtime_error("Entity does not have this component!");
	}

	template<typename ComponentType>
	std::unordered_map<EntityID, ComponentType>& GetAllComponents()
	{
		std::type_index type = std::type_index(typeid(ComponentType));
		auto* storage = static_cast<ComponentStorage<ComponentType>*>(
			componentStorages.at(type).get()
		);

		return storage->GetAll();
	}
private:
	EntityID nextEntityID = 1;
	std::unordered_map<std::type_index, std::unique_ptr<ComponentStorageI>> componentStorages;

};
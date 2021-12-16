#pragma once

#include <vector>
#include <array>
#include <functional>
#include <memory>

template<
	typename Type,
	unsigned int AttentionSpan,
	bool IsPointerType>
class _RetentiveEntityBase;

template<
	typename Type,
	unsigned int AttentionSpan>
class _RetentiveEntityBase<Type, AttentionSpan, false>
{
public:
	/**
	 * @brief Get the entity from `index` generations ago
	 *
	 * @param index Amount of generations to go backwards in time
	 * @return The entity from before `index` generations
	 */
	Type& operator[](size_t index)
	{
		return data[index];
	}

	/**
	 * @brief Get the most up-to-date entity
	 *
	 * @return The entity with generation 0
	 */
	Type& Current()
	{
		return data[0];
	}

protected:
	std::array<Type, AttentionSpan + 1> data;
};


template<
	typename Type,
	unsigned int AttentionSpan>
class _RetentiveEntityBase<Type, AttentionSpan, true>
{
public:
	/**
	 * @brief Get the entity from `index` generations ago
	 *
	 * @param index Amount of generations to go backwards in time
	 * @return The entity from before `index` generations
	 */
	Type& operator[](size_t index)
	{
		return *(data[index]);
	}

	/**
	 * @brief Get the most up-to-date entity
	 *
	 * @return The entity with generation 0
	 */
	Type& Current()
	{
		return *(data[0]);
	}

protected:
	std::array<std::shared_ptr<Type>, AttentionSpan + 1> data;	// Shared for move semantics
};

/**
 * @brief Abstract base type for all retentive things
 *
 * @tparam Type The object to give a memory to
 * @tparam AttentionSpan How many generations of the data will be remembered by the object
 */
template<
	typename Type,
	unsigned int AttentionSpan,
	bool IsPointerType = false>
class RetentiveEntity : public _RetentiveEntityBase<Type, AttentionSpan, IsPointerType>
{
public:
	RetentiveEntity() {}
	RetentiveEntity(const RetentiveEntity<Type, AttentionSpan>& other) = delete;
	RetentiveEntity(RetentiveEntity<Type, AttentionSpan>&& other) = delete;
	RetentiveEntity<Type, AttentionSpan>& operator=(const RetentiveEntity<Type, AttentionSpan>& other) = delete;
	RetentiveEntity<Type, AttentionSpan>& operator=(RetentiveEntity<Type, AttentionSpan>&& other) = delete;

	/**
	 * @brief Evolve the data in the entity
	 *
	 * Simply calls the member function pointer. The called function is then
	 * supposed to perform whatever is needed to compute the new object contents.
	 *
	 * The data is swapped BEFORE calling this function, so the evolution function should
	 * modify the data in the Current() vector (index 0)
	 */
	void Evolve()
	{
		// Evolve the object
		CycleGenerations();
		rule();
	}

	/**
	 * @brief Evolve the data in the object
	 *
	 * Simply calls the provided function pointer. The called function is then
	 * supposed to perform whatever is needed to compute the new object contents.
	 *
	 * The data is swapped BEFORE calling this function, so the evolution function should
	 * modify the data in the Current() vector (index 0)
	 *
	 * @param rule A function that evolves the data in the object
	 */
	void Evolve(std::function<void(void)> rule)
	{
		CycleGenerations();
		rule();
	}

	/**
	 * @brief Set a general rule for evolving the data
	 *
	 * @param rule The new evolution rule
	 */
	void SetEvolutionRule(std::function<void(void)> rule)
	{
		this->rule = rule;
	}

protected:
	/**
	 * @brief Swaps the objects in the array
	 */
	virtual void CycleGenerations() = 0;
};

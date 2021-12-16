#pragma once

#include "RetentiveEntity.hpp"

/**
 * @brief An object that remembers its past.
 *
 * You most likely want to use a RetentiveArray instead.
 * This structure is only really useful if you have a custom container class
 * that you need to evolve over time.
 * 
 * A retentive object is a kind of buffered object that "remembers"
 * the state of the object from the last n generations.
 *
 * This structure handles the evolution of these kinds of objects and takes
 * care of the memory allocation/freeing as well as properly swapping the
 * object pointers.
 *
 * @tparam Type The object to give a memory to
 * @tparam AttentionSpan How many generations of the data will be remembered by the object
 */
template<
	typename Type,
	unsigned int AttentionSpan,
	typename std::enable_if_t<(AttentionSpan > 0), bool> = true>
class RetentiveObject : public RetentiveEntity<Type*, AttentionSpan>
{
public:
	/**
	 * @brief Constructs a new, empty retentive object
	 */
	RetentiveObject()
	{
		// Create new vectors for every generation that needs to be remembered
		for (int n = 0; n <= AttentionSpan; n++)
		{
			data[n] = new Type();
		}
	}

	/**
	 * @brief Constructs a new, empty retentive object
	 *
	 * @param size The size of the data contained in the object
	 */
	RetentiveObject(const Type& initVal)
	{
		// Create new vectors for every generation that needs to be remembered
		for (int n = 0; n <= AttentionSpan; n++)
		{
			data[n] = new Type(initVal);
		}
	}

	/**
	 * @brief Constructs a new retentive object with the data of another object
	 *
	 * @param other The retentive object to copy from
	 */
	RetentiveObject<Type, AttentionSpan>& operator=(const RetentiveObject<Type, AttentionSpan>& other)
	{
		for (int n = 0; n <= AttentionSpan; n++)
		{
			*data[n] = *other.data[n];
		}

		return *this;
	}

	/**
	 * @brief Constructs a new retentive object pointing to the data of an rvalue
	 *
	 * @param other The retentive object to set its data to
	 */
	RetentiveObject<Type, AttentionSpan>& operator=(RetentiveObject<Type, AttentionSpan>&& other)
	{
		for (int n = 0; n <= AttentionSpan; n++)
		{
			data[n] = other.data[n];
			other.data[n] = nullptr;
		}

		return *this;
	}

	/**
	 * @brief Constructs a new retentive object with the data of another object
	 *
	 * @param other The retentive object to copy from
	 */
	RetentiveObject(const RetentiveObject<Type, AttentionSpan>& other)
	{
		*this = other;
	}

	/**
	 * @brief Constructs a new retentive object pointing to the data of an rvalue
	 *
	 * @param other The retentive object to set its data to
	 */
	RetentiveObject(RetentiveObject<Type, AttentionSpan>&& other)
	{
		*this = other;
	}

	~RetentiveObject()
	{
		for (int n = 0; n <= AttentionSpan; n++)
		{
			if (data[n] != nullptr)
				delete data[n];
		}
	}

	/**
	 * @brief Get the object from `index` generations ago
	 *
	 * @param index Amount of generations to go backwards in time
	 * @return The object from before `index` generations
	 */
	Type& operator[](size_t index) override
	{
		return *(data[index]);
	}

	/**
	 * @brief Get the most up-to-date object
	 *
	 * @return The object with generation 0
	 */
	Type& Current() override
	{
		return *(data[0]);
	}

private:
	/**
	 * @brief Swaps the objects in the array
	 */
	void CycleGenerations() override
	{
		// Cycle the object contents, so that the previous "current" object is now the 2nd. 
		// The 2nd becomes the 3rd etc, and the former last object becomes the new current
		for (int n = 1; n <= AttentionSpan; n++)
		{
			std::swap(data[0], data[n]);
		}
	}
};

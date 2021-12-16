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
	unsigned int AttentionSpan>
class RetentiveObject : public RetentiveEntity<Type, AttentionSpan, true>
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
			data[n] = std::make_shared<Type>();
		}
	}

	/**
	 * @brief Constructs a new, empty retentive object
	 *
	 * @param size The size of the data contained in the object
	 */
	RetentiveObject(const Type& initVal)
	{
		// Create new objects for every generation that needs to be remembered
		for (int n = 0; n <= AttentionSpan; n++)
		{
			data[n] = std::make_shared<Type>(initVal);
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

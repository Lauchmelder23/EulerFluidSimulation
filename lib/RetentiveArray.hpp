#pragma once

#include <vector>
#include <array>
#include <functional>

/**
* @brief An array that remembers its past.
* 
 * A retentive array is a kind of buffered array that "remembers"
 * the state of the array from the last n generations.
 * This is extremely useful for numerically solving PDEs, as the values
 * of the current array elements usually depend on the previous few states
 * of the simulation.
 * 
 * This structure handles the evolution of these kinds of arrays and takes
 * care of the memory allocation/freeing as well as properly swapping the 
 * arrays.
 * 
 * @tparam Type The type of the elements in the array
 * @tparam AttentionSpan How many generations of the data will be remembered by the array
 */
template<
	typename Type, 
	unsigned int AttentionSpan, 
	typename std::enable_if_t<(AttentionSpan > 0), bool> = true>
class RetentiveArray
{
public:
	/**
	 * @brief Constructs a new, empty retentive array 
	 */
	RetentiveArray()
	{
		// Do nothing
	}

	/**
	 * @brief Constructs a new, empty retentive array
	 * 
	 * @param size The size of the data contained in the array
	 */
	RetentiveArray(size_t size)
	{
		// Create new vectors for every generation that needs to be remembered
		for (int n = 0; n <= AttentionSpan; n++)
		{
			data[n] = std::vector<Type>(size, Type());
		}
	}

	/**
	 * @brief Constructs a new retentive array with the data of another array
	 *
	 * @param other The retentive array to copy from
	 */
	RetentiveArray<Type, AttentionSpan>& operator=(const RetentiveArray<Type, AttentionSpan>& other)
	{
		for (int n = 0; n <= AttentionSpan; n++)
		{
			data[n] = std::vector<Type>(other.data[n]);
		}

		return *this;
	}

	/**
	 * @brief Constructs a new retentive array pointing to the data of an rvalue
	 *
	 * @param other The retentive array to set its data to
	 */
	RetentiveArray<Type, AttentionSpan>& operator=(RetentiveArray<Type, AttentionSpan>&& other)
	{
		for (int n = 0; n <= AttentionSpan; n++)
		{
			data[n] = std::vector<Type>(std::move(other.data[n]));
		}

		return *this;
	}

	/**
	 * @brief Constructs a new retentive array with the data of another array
	 *
	 * @param other The retentive array to copy from
	 */
	RetentiveArray(const RetentiveArray<Type, AttentionSpan>& other)
	{
		*this = other;
	}

	/**
	 * @brief Constructs a new retentive array pointing to the data of an rvalue
	 *
	 * @param other The retentive array to set its data to
	 */
	RetentiveArray(RetentiveArray<Type, AttentionSpan>&& other)
	{
		*this = other;
	}

	~RetentiveArray()
	{
		// Do nothing
	}

	/**
	 * @brief Get the array from `index` generations ago
	 * 
	 * @param index Amount of generations to go backwards in time
	 * @return The array from before `index` generations
	 */
	std::vector<Type>& operator[](size_t index)
	{
		return data[index];
	}

	/**
	 * @brief Get the most up-to-date array
	 * 
	 * @return The array with generation 0
	 */
	std::vector<Type>& Current()
	{
		return data[0];
	}

	/**
	 * @brief Evolve the data in the array
	 * 
	 * Simply calls the member function pointer. The called function is then
	 * supposed to perform whatever is needed to compute the new array contents.
	 * 
	 * The data is swapped BEFORE calling this function, so the evolution function should
	 * modify the data in the Current() vector (index 0)
	 */
	void Evolve()
	{
		// Evolve the array
		Evolve(rule);
	}

	/**
	 * @brief Evolve the data in the array
	 *
	 * Simply calls the provided function pointer. The called function is then
	 * supposed to perform whatever is needed to compute the new array contents.
	 *
	 * The data is swapped BEFORE calling this function, so the evolution function should
	 * modify the data in the Current() vector (index 0)
	 * 
	 * @param rule A function that evolves the data in the array
	 */
	void Evolve(std::function<void(void)> rule)
	{
		// Cycle the array contents, so that the previous "current" array is now the 2nd. 
		// The 2nd becomes the 3rd etc, and the former last array becomes the new current
		for (int n = 1; n <= AttentionSpan; n++)
		{
			data[0].swap(data[1]);
		}

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


private:
	std::array<std::vector<Type>, AttentionSpan + 1> data;
	std::function<void(void)> rule = std::bind([]() {});	// Do nothing by default
};

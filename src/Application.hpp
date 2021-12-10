#pragma once 

#include <chrono>
#include "FluidField.hpp"

// Forward Declarations
struct SDL_Window;
struct SDL_Renderer;

// Should be a singleton but who cares
class Application
{
public:
	Application(int width, int height, const char* title);
	~Application();

	void Launch();

private:
	void HandleEvents();
	void Update();
	void Render();

private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	std::chrono::steady_clock::time_point before;
	bool shouldClose = false;

	FluidField* field;
};
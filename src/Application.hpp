#pragma once 

#include "VectorField.hpp"

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

	bool shouldClose = false;

	VectorField velocity;
};
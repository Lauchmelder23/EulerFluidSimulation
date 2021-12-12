#pragma once

#include <string>
#include <chrono>

struct SDL_Renderer;
struct SDL_Window;

class Window
{
public:
	void Launch();

protected:
	Window(int width, int height, const std::string& title);
	Window(const Window& other);
	Window& operator=(const Window& other);
	~Window();


	virtual void OnUpdate(double dt) {}
	virtual void OnRender(SDL_Renderer* renderer) {}

private:
	void HandleEvents();
	void Update();
	void Render();

protected:
	SDL_Window* window;

private:
	SDL_Renderer* renderer;

	bool shouldClose = false;
	std::chrono::steady_clock::time_point startOfLastFrame;
};
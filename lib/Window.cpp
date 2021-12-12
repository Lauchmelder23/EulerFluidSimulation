#include "Window.hpp"

#include <iostream>
#include <SDL.h>

void Window::Launch()
{
	SDL_ShowWindow(window);

	while (!shouldClose)
	{
		HandleEvents();
		Update();
		Render();
	}

	SDL_HideWindow(window);
}

Window::Window(int width, int height, const std::string& title)
{
	window = nullptr;
	renderer = nullptr;

	window = SDL_CreateWindow(
		title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_HIDDEN
	);
	if (window == nullptr)
	{
		char errbuf[512];
		SDL_GetErrorMsg(errbuf, 512);

		std::cerr << "Failed to create SDL Window: " << std::endl
			<< errbuf << std::endl;

		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		char errbuf[512];
		SDL_GetErrorMsg(errbuf, 512);

		std::cerr << "Failed to create SDL Renderer: " << std::endl
			<< errbuf << std::endl;

		return;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	startOfLastFrame = std::chrono::steady_clock::now();
}

Window::Window(const Window& other)
{
	*this = other;
}

Window& Window::operator=(const Window& other)
{
	const char* title = SDL_GetWindowTitle(other.window);
	int width, height;
	SDL_GetWindowSize(other.window, &width, &height);

	window = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_HIDDEN
	);

	if (window == nullptr)
	{
		char errbuf[512];
		SDL_GetErrorMsg(errbuf, 512);

		std::cerr << "Failed to create SDL Window: " << std::endl
			<< errbuf << std::endl;

		throw std::runtime_error("");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		char errbuf[512];
		SDL_GetErrorMsg(errbuf, 512);

		std::cerr << "Failed to create SDL Renderer: " << std::endl
			<< errbuf << std::endl;

		throw std::runtime_error("");
	}

	startOfLastFrame = std::chrono::steady_clock::now();

	return *this;
}

Window::~Window()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void Window::HandleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				shouldClose = true;
				break;
			}
		} break;
		}
	}
}

void Window::Update()
{
	double dt = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - startOfLastFrame).count();
	startOfLastFrame = std::chrono::steady_clock::now();

	OnUpdate(dt);
}

void Window::Render()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	OnRender(renderer);

	if (renderer == nullptr)
		throw std::runtime_error("Client window left renderer in an invalid state");
	
	SDL_RenderPresent(renderer);
}

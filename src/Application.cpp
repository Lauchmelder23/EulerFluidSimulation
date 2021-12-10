#include "Application.hpp"

#include <iostream>
#include <SDL.h>

Application::Application(int width, int height, const char* title)
{
	window = SDL_CreateWindow(
		title, 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		width, height, 
		SDL_WINDOW_SHOWN
	);
	if (window == nullptr)
	{
		char errbuf[512];
		SDL_GetErrorMsg(errbuf, 512);

		std::cerr << "Failed to create SDL Window: " << std::endl
			<< errbuf << std::endl;

		return;
	}

	// Let's just pretend we're the first application in this program to create a renderer :) what could go wrong
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

	field = new FluidField(100);
	before = std::chrono::steady_clock::now();
}

Application::~Application()
{
	delete field;

	SDL_DestroyRenderer(renderer);	// Let's just destroy this renderer regardless of other applications in this program :) what could go wrong
	SDL_DestroyWindow(window);
}

void Application::Launch()
{
	if (renderer == nullptr || window == nullptr)
	{
		throw std::runtime_error("Can't launch application. Window or Renderer is in invalid state.");
	}

	while (!shouldClose)
	{
		HandleEvents();
		Update();
		Render();
	}
}

void Application::HandleEvents()
{
	static SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_WINDOWEVENT)
		{
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				shouldClose = true;
				break;
			}
		}
	}
}

void Application::Update()
{
	double frametime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - before).count();
	before = std::chrono::steady_clock::now();

	field->DensityStep(0.001, frametime);
}

void Application::Render()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);


	field->Draw(renderer, {0, 0, 1000, 1000});

	SDL_RenderPresent(renderer);
}

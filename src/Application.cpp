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

	// Construct dummy velocity field
	// TODO: Remove eventually
	int fieldSize = 49;
	double curl1 = -1.0;
	double curl2 = 1.0;
	std::vector<double> hori(fieldSize * fieldSize);
	std::vector<double> vert(fieldSize * fieldSize);

	for (int y = 0; y < fieldSize; y++)
	{
		for (int x = 0; x < fieldSize; x++)
		{
			// map internal coords to "real world" coordinates
			double realX = -2.0 + (4.0 / (double)fieldSize) * (double)x;
			double realY = -2.0 + (4.0 / (double)fieldSize) * (double)y;

			// hori[y * fieldSize + x] = (realY + 1.0) / sqrt((realX + 1.0) * (realX + 1.0) + (realY + 1.0) * (realY + 1.0)) - (realY - 1.0) / sqrt((realX - 1.0) * (realX - 1.0) + (realY - 1.0) * (realY - 1.0)); 
			// vert[y * fieldSize + x] = -(realX + 1.0) / sqrt((realX + 1.0) * (realX + 1.0) + (realY + 1.0) * (realY + 1.0)) + (realX - 1.0) / sqrt((realX - 1.0) * (realX - 1.0) + (realY - 1.0) * (realY - 1.0));
		
			// hori[y * fieldSize + x] = realY / sqrt(realX * realX + realY * realY);
			// vert[y * fieldSize + x] = -realX / sqrt(realX * realX + realY * realY);

			hori[y * fieldSize + x] = -realY;
			vert[y * fieldSize + x] = realX;

		}
	}

	velocity = VectorField(fieldSize, fieldSize, hori, vert);
}

Application::~Application()
{
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

}

void Application::Render()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);


	velocity.Draw(renderer, {10, 10, 790, 790});

	SDL_RenderPresent(renderer);
}

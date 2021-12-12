#include "EulerFluid.hpp"

#include <iostream>
#include <SDL.h>

EulerFluid::EulerFluid(int width, int height, const char* title) :
	Window::Window(width, height, title)
{
	field = new FluidField(60);
}

EulerFluid::~EulerFluid()
{
	delete field;
}

void EulerFluid::OnUpdate(double dt)
{
	field->VelocityStep(0.002, dt);
	field->DensityStep(0.0005, dt);
}

void EulerFluid::OnRender(SDL_Renderer* renderer)
{
	field->Draw(renderer, {0, 0, 1000, 1000});
}

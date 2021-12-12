#pragma once 

#include "Window.hpp"
#include "FluidField.hpp"

class EulerFluid : public Window
{
public:
	EulerFluid(int width, int height, const char* title);
	~EulerFluid();

private:
	void OnUpdate(double dt) override;
	void OnRender(SDL_Renderer* renderer) override;

private:
	FluidField* field;
};
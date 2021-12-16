#pragma once

#include <vector>
#include "VectorField.hpp"
#include "RetentiveArray.hpp"
#include "RetentiveObject.hpp"

struct SDL_Renderer;
struct SDL_Rect;

enum class BoundaryCondition
{
	Continuous,
	InvertVertical,
	InvertHorizontal
};

class FluidField
{
public:
	FluidField(int size);
	~FluidField();

	void AddSource(int x, int y, double density, double dt);
	void AddFlow(int x, int y, double dx, double dy, double dt);
	void ApplyBoundaryConditions(BoundaryCondition condition, std::vector<double>& field);

	void Diffuse(double diff, double dt);
	void Advect(double dt);
	void DensityStep(double diff, double dt);

	void DiffuseVelocity(double visc, double dt);
	void AdvectVelocity(double dt);
	void VelocityStep(double visc, double dt);
	void Project();

	void Draw(SDL_Renderer* renderer, const SDL_Rect& target);

private:
	int size;

	RetentiveObject<VectorField, 1> velocity;
	RetentiveArray<double, 1> density;

	int lastMouseX, lastMouseY;
};
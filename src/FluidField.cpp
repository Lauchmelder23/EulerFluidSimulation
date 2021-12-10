#include "FluidField.hpp"

#include <iostream>
#include <SDL.h>
#include "VectorField.hpp"

#define VALUE(arr, x, y) ((arr)[(y) * this->size + (x)])
#define PVALUE(arr, x, y) ((*(arr))[(y) * this->size + (x)])

FluidField::FluidField(int size) :
	size(size + 2)
{
	density		= new std::vector<double>(this->size * this->size, 0.0);
	prevDensity = new std::vector<double>(this->size * this->size, 0.0);

	std::vector<double> hori(this->size * this->size);
	std::vector<double> vert(this->size * this->size);

	for (int y = 1; y < this->size - 1; y++)
	{
		for (int x = 1; x < this->size - 1; x++)
		{
			// map internal coords to "real world" coordinates
			double realX = -2.0 + (4.0 / (double)size) * (double)x;
			double realY = -2.0 + (4.0 / (double)size) * (double)y;

			hori[y * this->size + x] = 1.0 * realY;
			vert[y * this->size + x] = 1.0 * (- realX - 0.25 * realY);

			// hori[y * this->size + x] = 0.2 * realY * realY;
			// vert[y * this->size + x] = 0.2 * realX * realX;
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::InvertHorizontal, hori);
	ApplyBoundaryConditions(BoundaryCondition::InvertVertical, vert);

	vel		= new VectorField(this->size, this->size, hori, vert);	// gonna do the same inefficient calculations twice, why not
	prevVel = new VectorField(this->size, this->size, hori, vert);
}

FluidField::~FluidField()
{
	delete prevDensity;
	delete density;

	delete prevVel;
	delete vel;
}

void FluidField::AddSource(int x, int y, double dens, double dt)
{
	PVALUE(density, x, y) += dt * dens;
	PVALUE(density, x, y) = std::max(PVALUE(density, x, y), 0.0);
}

void FluidField::ApplyBoundaryConditions(BoundaryCondition condition, std::vector<double>& field)
{
	int N = this->size - 2;
	for (int i = 1; i <= N; i++)
	{

		VALUE(field, 0		, i) = (condition == BoundaryCondition::InvertHorizontal)	? -VALUE(field, 1, i) : VALUE(field, 1, i); // VALUE(field, N, i);
		VALUE(field, N + 1	, i) = (condition == BoundaryCondition::InvertHorizontal)	? -VALUE(field, N, i) : VALUE(field, N, i);	// VALUE(field, 1, i);
		VALUE(field, i		, 0) = (condition == BoundaryCondition::InvertVertical)		? -VALUE(field, i, 1) : VALUE(field, i, 1);	// VALUE(field, i, N);
		VALUE(field, i	, N + 1) = (condition == BoundaryCondition::InvertVertical)		? -VALUE(field, i, N) : VALUE(field, i, N);	// VALUE(field, i, 1);
	}

	VALUE(field, 0		, 0		) = 0.5 * (VALUE(field, 1, 0	) + VALUE(field, 0, 1	 ));
	VALUE(field, 0		, N + 1	) = 0.5 * (VALUE(field, 1, N + 1) + VALUE(field, 0, N	 ));
	VALUE(field, N + 1	, 0		) = 0.5 * (VALUE(field, N, 0	) + VALUE(field, N + 1, 1));
	VALUE(field, N + 1	, N + 1	) = 0.5 * (VALUE(field, N, N + 1) + VALUE(field, N + 1, N));
}

void FluidField::Diffuse(double diff, double dt)
{
	int N = this->size - 2;
	double a = dt * diff * N * N;

	for (int k = 0; k < 20; k++)
	{
		for (int i = 1; i <= N; i++)
		{
			for (int j = 1; j <= N; j++)
			{
				PVALUE(density, i, j) = (PVALUE(prevDensity, i, j) + a * (PVALUE(density, i - 1, j) + PVALUE(density, i + 1, j) + PVALUE(density, i, j - 1) + PVALUE(density, i, j + 1))) / (1 + 4 * a);
			}
		}

		ApplyBoundaryConditions(BoundaryCondition::Continuous, *density);
	}
}

void FluidField::Advect(double dt)
{
	int N = this->size - 2;
	double dt0 = dt * N;

	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			double x = i - dt0 * VALUE(vel->horizontal, i, j);
			double y = j - dt0 * VALUE(vel->vertical, i, j);

			if (x < 0.5)		x = 0.5;
			if (x > N + 0.5)	x = N + 0.5;
			if (y < 0.5)		y = 0.5;
			if (y > N + 0.5)	y = N + 0.5;

			int i0 = (int)x;
			int i1 = i0 + 1;
			int j0 = (int)y;
			int j1 = j0 + 1;

			double s1 = x - i0;
			double s0 = 1 - s1;
			double t1 = y - j0;
			double t0 = 1 - t1;

			PVALUE(density, i, j) = s0 * (t0 * PVALUE(prevDensity, i0, j0) + t1 * PVALUE(prevDensity, i0, j1)) +
									s1 * (t0 * PVALUE(prevDensity, i1, j0) + t1 * PVALUE(prevDensity, i1, j1));
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::Continuous, *density);
}

void FluidField::DensityStep(double diff, double dt)
{
	AddSource(this->size / 2, this->size / 2, -10000000.0, dt);
	// AddSource(50, 3, 60.0, dt);
	// AddSource(3, 50, 60.0, dt);

	int x, y;
	Uint32 buttons = SDL_GetMouseState(&x, &y);
	int dx = (double)(this->size - 2) / (double)(990 - 10) * (double)(x - 10);
	int dy = (double)(this->size - 2) / (double)(990 - 10) * (double)(y - 10);
	
	int factor = 0;

	if (buttons & SDL_BUTTON_LMASK)
		factor = 1;
	else if (buttons & SDL_BUTTON_RMASK)
		factor = -1;
	
	if(dx > 0 && dx < this->size - 1 && dy > 0 && dy < this->size - 1)
		AddSource(dx, dy, 300.0 * factor, dt);

	std::swap(prevDensity, density);	
	Diffuse(diff, dt);
	std::swap(prevDensity, density);
	Advect(dt);
}

void FluidField::Draw(SDL_Renderer* renderer, const SDL_Rect& target)
{
	double cellWidth = (double)(target.w - target.x) / (double)this->size;
	double cellHeight = (double)(target.h - target.y) / (double)this->size;

	SDL_FRect vectorCenterSquare;
	vectorCenterSquare.w = cellWidth;
	vectorCenterSquare.h = cellHeight;

	for (int y = 0; y < this->size; y++)
	{
		for (int x = 0; x < this->size; x++)
		{
			double densityVal = std::min(PVALUE(density, x, y), 1.0);
			SDL_SetRenderDrawColor(renderer, densityVal * 255, densityVal * 255, densityVal * 255, 255);

			vectorCenterSquare.x = (double)target.x + cellWidth * x;	// cellWidth * x + cellWidth / 2 - cellWidth / 10
			vectorCenterSquare.y = (double)target.y + cellHeight * y;
			SDL_RenderFillRectF(renderer, &vectorCenterSquare);
		}
	}

	vel->Draw(renderer, target);
}

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

			// hori[y * this->size + x] = 1.0 * realY;
			// vert[y * this->size + x] = 1.0 * (- realX - 0.25 * realY);

			// hori[y * this->size + x] = 0.2 * realY * realY;
			// vert[y * this->size + x] = 0.2 * realX * realX;
		}
	}

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

void FluidField::AddFlow(int x, int y, double dx, double dy, double dt)
{
	VALUE(vel->horizontal, x, y) += dt * dx;
	VALUE(vel->vertical, x, y) += dt * dy;
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

void FluidField::DiffuseVelocity(double visc, double dt)
{
	int N = this->size - 2;
	double a = dt * visc * N * N;

	for (int k = 0; k < 20; k++)
	{
		for (int i = 1; i <= N; i++)
		{
			for (int j = 1; j <= N; j++)
			{
				VALUE(vel->horizontal, i, j) = (VALUE(prevVel->horizontal, i, j) + a * (VALUE(vel->horizontal, i - 1, j) + VALUE(vel->horizontal, i + 1, j) + VALUE(vel->horizontal, i, j - 1) + VALUE(vel->horizontal, i, j + 1))) / (1 + 4 * a);
				VALUE(vel->vertical, i, j) = (VALUE(prevVel->vertical, i, j) + a * (VALUE(vel->vertical, i - 1, j) + VALUE(vel->vertical, i + 1, j) + VALUE(vel->vertical, i, j - 1) + VALUE(vel->vertical, i, j + 1))) / (1 + 4 * a);
			}
		}

		ApplyBoundaryConditions(BoundaryCondition::Continuous, vel->horizontal);
		ApplyBoundaryConditions(BoundaryCondition::Continuous, vel->vertical);
	}
}

void FluidField::AdvectVelocity(double dt)
{
	int N = this->size - 2;
	double dt0 = dt * N;

	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			double x = i - dt0 * VALUE(prevVel->horizontal, i, j);
			double y = j - dt0 * VALUE(prevVel->vertical, i, j);

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

			VALUE(vel->horizontal, i, j) = s0 * (t0 * VALUE(prevVel->horizontal, i0, j0) + t1 * VALUE(prevVel->horizontal, i0, j1)) +
				s1 * (t0 * VALUE(prevVel->horizontal, i1, j0) + t1 * VALUE(prevVel->horizontal, i1, j1));

			VALUE(vel->vertical, i, j) = s0 * (t0 * VALUE(prevVel->vertical, i0, j0) + t1 * VALUE(prevVel->vertical, i0, j1)) +
				s1 * (t0 * VALUE(prevVel->vertical, i1, j0) + t1 * VALUE(prevVel->vertical, i1, j1));
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::InvertHorizontal, vel->horizontal);
	ApplyBoundaryConditions(BoundaryCondition::InvertVertical, vel->vertical);
}

void FluidField::VelocityStep(double visc, double dt)
{
	// AddFlow(15, 30, 3000.0, 0.0, dt);
	// AddFlow(45, 30, -3000.0, 0.0, dt);
	// AddFlow(30, 15, 0.0, 3000.0, dt);

	int x, y;
	Uint32 buttons = SDL_GetMouseState(&x, &y);
	int dx = (double)(this->size - 2) / (double)(990 - 10) * (double)(x - 10);
	int dy = (double)(this->size - 2) / (double)(990 - 10) * (double)(y - 10);

	if (buttons & SDL_BUTTON_RMASK)
	{
		AddFlow(lastMouseX, lastMouseY, (dx - lastMouseX) * 500.0, (dy - lastMouseY) * 500.0, dt);
	}

	lastMouseX = dx;
	lastMouseY = dy;

	std::swap(vel, prevVel);
	DiffuseVelocity(visc, dt);
	Project();
	std::swap(vel, prevVel);
	AdvectVelocity(dt);
	Project();

	vel->RecalculateMagnitude();
}

void FluidField::Project()
{
	int N = this->size - 2;
	double h = 1.0 / (double)N;

	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			VALUE(prevVel->vertical, i, j) = -0.5 * h * (VALUE(vel->horizontal, i + 1, j) - VALUE(vel->horizontal, i - 1, j) + VALUE(vel->vertical, i, j + 1) - VALUE(vel->vertical, i, j - 1));
			VALUE(prevVel->horizontal, i, j) = 0;
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::Continuous, prevVel->horizontal);
	ApplyBoundaryConditions(BoundaryCondition::Continuous, prevVel->vertical);

	for (int k = 0; k < 20; k++)
	{
		for (int i = 1; i <= N; i++)
		{
			for (int j = 1; j <= N; j++)
			{
				VALUE(prevVel->horizontal, i, j) = (VALUE(prevVel->vertical, i, j) + VALUE(prevVel->horizontal, i - 1, j) + VALUE(prevVel->horizontal, i + 1, j) + VALUE(prevVel->vertical, i, j - 1) + VALUE(prevVel->vertical, i, j + 1)) / 4.0;
			}
		}

		ApplyBoundaryConditions(BoundaryCondition::Continuous, prevVel->horizontal);
	}

	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			VALUE(vel->horizontal, i, j) -= 0.5 * (VALUE(prevVel->horizontal, i + 1, j) - VALUE(prevVel->horizontal, i - 1, j)) / h;
			VALUE(vel->vertical, i, j) -= 0.5 * (VALUE(prevVel->horizontal, i, j + 1) - VALUE(prevVel->horizontal, i, j - 1)) / h;
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::InvertHorizontal, prevVel->horizontal);
	ApplyBoundaryConditions(BoundaryCondition::InvertVertical, prevVel->vertical);
}

void FluidField::DensityStep(double diff, double dt)
{
	// AddSource(50, 3, 60.0, dt);
	// AddSource(3, 50, 60.0, dt);

	int x, y;
	Uint32 buttons = SDL_GetMouseState(&x, &y);
	int dx = (double)(this->size - 2) / (double)(990 - 10) * (double)(x - 10);
	int dy = (double)(this->size - 2) / (double)(990 - 10) * (double)(y - 10);
	

	if (buttons & SDL_BUTTON_LMASK)
		if(dx > 0 && dx < this->size - 1 && dy > 0 && dy < this->size - 1)
			AddSource(dx, dy, 100.0, dt);

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

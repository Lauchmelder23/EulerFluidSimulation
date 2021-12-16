#include "FluidField.hpp"

#include <iostream>
#include <SDL.h>

#include "VectorField.hpp"

#define VALUE(arr, x, y) ((arr)[(y) * this->size + (x)])
#define PVALUE(arr, x, y) ((*(arr))[(y) * this->size + (x)])

#define IDX(x, y, w) ((y) * (w) + (x))

FluidField::FluidField(int size) :
	size(size + 2)
{
	density = RetentiveArray<double, 1>(this->size * this->size);

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

	velocity = RetentiveObject<VectorField, 1>(VectorField(this->size, this->size, hori, vert));
}

FluidField::~FluidField()
{
	// Do nothing
}

void FluidField::AddSource(int x, int y, double dens, double dt)
{
	density.Current()[IDX(x, y, size)] = dt * dens;
	density.Current()[IDX(x, y, size)] = std::max(density[0][IDX(x, y, size)], 0.0);
}

void FluidField::AddFlow(int x, int y, double dx, double dy, double dt)
{
	velocity.Current().horizontal[IDX(x, y, size)] += dt * dx;
	velocity.Current().vertical[IDX(x, y, size)] += dt * dy;
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
				density[0][IDX(i, j, size)] = (density[1][IDX(i, j, size)] + a * (density[0][IDX(i - 1, j, size)] + density[0][IDX(i + 1, j, size)] + density[0][IDX(i, j - 1, size)] + density[0][IDX(i, j + 1, size)])) / (1 + 4 * a);
			}
		}

		ApplyBoundaryConditions(BoundaryCondition::Continuous, density[0]);
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
			double x = i - dt0 * velocity.Current().horizontal[IDX(i, j, size)];
			double y = j - dt0 * velocity.Current().vertical[IDX(i, j, size)];

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

			density.Current()[IDX(i, j, size)] = s0 * (t0 * density[1][IDX(i0, j0, size)] + t1 * density[1][IDX(i0, j1, size)]) +
									s1 * (t0 * density[1][IDX(i1, j0, size)] + t1 * density[1][IDX(i1, j1, size)]);
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::Continuous, density[0]);
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
				velocity.Current().horizontal[IDX(i, j, size)] = (velocity[1].horizontal[IDX(i, j, size)] + a * (velocity[1].horizontal[IDX(i - 1, j, size)] + velocity[1].horizontal[IDX(i + 1, j, size)] + velocity[1].horizontal[IDX(i, j - 1, size)] + velocity[1].horizontal[IDX(i, j + 1, size)])) / (1 + 4 * a);
				velocity.Current().vertical[IDX(i, j, size)] = (velocity[1].vertical[IDX(i, j, size)] + a * (velocity[1].vertical[IDX(i - 1, j, size)] + velocity[1].vertical[IDX(i + 1, j, size)] + velocity[1].vertical[IDX(i, j - 1, size)] + velocity[1].vertical[IDX(i, j + 1, size)])) / (1 + 4 * a);
			}
		}

		ApplyBoundaryConditions(BoundaryCondition::Continuous, velocity.Current().horizontal);
		ApplyBoundaryConditions(BoundaryCondition::Continuous, velocity.Current().vertical);
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
			double x = i - dt0 * velocity[1].horizontal[IDX(i, j, size)];
			double y = j - dt0 * velocity[1].vertical[IDX(i, j, size)];

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

			velocity.Current().horizontal[IDX(i, j, size)] = s0 * (t0 * velocity[1].horizontal[IDX(i0, j0, size)] + t1 * velocity[1].horizontal[IDX(i0, j1, size)]) +
				s1 * (t0 * velocity[1].horizontal[IDX(i1, j0, size)] + t1 * velocity[1].horizontal[IDX(i1, j1, size)]);

			velocity.Current().vertical[IDX(i, j, size)] = s0 * (t0 * velocity[1].vertical[IDX(i0, j0, size)] + t1 * velocity[1].vertical[IDX(i0, j1, size)]) +
				s1 * (t0 * velocity[1].vertical[IDX(i1, j0, size)] + t1 * velocity[1].vertical[IDX(i1, j1, size)]);
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::InvertHorizontal, velocity.Current().horizontal);
	ApplyBoundaryConditions(BoundaryCondition::InvertVertical, velocity.Current().vertical);
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

	velocity.Evolve(std::bind(&FluidField::DiffuseVelocity, this, visc, dt));
	Project();
	velocity.Evolve(std::bind(&FluidField::AdvectVelocity, this, dt));
	Project();

	// vel->RecalculateMagnitude();
}

void FluidField::Project()
{
	int N = this->size - 2;
	double h = 1.0 / (double)N;

	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			velocity[1].vertical[IDX(i, j, size)] = -0.5 * h * (velocity.Current().vertical[IDX(i + 1, j, size)] - velocity.Current().vertical[IDX(i - 1, j, size)] + velocity.Current().vertical[IDX(i, j + 1, size)] - velocity.Current().vertical[IDX(i, j - 1, size)]);
			velocity[1].horizontal[IDX(i, j, size)] = 0;
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::Continuous, velocity[1].horizontal);
	ApplyBoundaryConditions(BoundaryCondition::Continuous, velocity[1].vertical);

	for (int k = 0; k < 20; k++)
	{
		for (int i = 1; i <= N; i++)
		{
			for (int j = 1; j <= N; j++)
			{
				velocity[1].horizontal[IDX(i, j, size)] = (velocity[1].vertical[IDX(i, j, size)] + velocity[1].vertical[IDX(i - 1, j, size)] + velocity[1].vertical[IDX(i + 1, j, size)] + velocity[1].vertical[IDX(i, j - 1, size)] + velocity[1].vertical[IDX(i, j + 1, size)]) / 4.0;
			}
		}

		ApplyBoundaryConditions(BoundaryCondition::Continuous, velocity[1].horizontal);
	}

	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			velocity.Current().horizontal[IDX(i, j, size)] -= 0.5 * (velocity[1].horizontal[IDX(i + 1, j, size)] - velocity[1].horizontal[IDX(i - 1, j, size)]) / h;
			velocity.Current().vertical[IDX(i, j, size)] -= 0.5 * (velocity[1].horizontal[IDX(i, j + 1, size)] - velocity[1].horizontal[IDX(i, j - 1, size)]) / h;
		}
	}

	ApplyBoundaryConditions(BoundaryCondition::InvertHorizontal, velocity[1].horizontal);
	ApplyBoundaryConditions(BoundaryCondition::InvertVertical, velocity[1].vertical);
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

	density.Evolve(std::bind(&FluidField::Diffuse, this, diff, dt));
	density.Evolve(std::bind(&FluidField::Advect, this, dt));
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
			double densityVal = std::min(density.Current()[IDX(x, y, size)], 1.0);
			SDL_SetRenderDrawColor(renderer, densityVal * 255, densityVal * 255, densityVal * 255, 255);

			vectorCenterSquare.x = (double)target.x + cellWidth * x;	// cellWidth * x + cellWidth / 2 - cellWidth / 10
			vectorCenterSquare.y = (double)target.y + cellHeight * y;
			SDL_RenderFillRectF(renderer, &vectorCenterSquare);
		}
	}

	velocity.Current().Draw(renderer, target);
}

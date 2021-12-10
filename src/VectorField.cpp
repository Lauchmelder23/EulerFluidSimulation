#include "VectorField.hpp"

#include <SDL.h>

VectorField::VectorField() :
	width(0), height(0), biggestMagnitude(1.0)
{
}

VectorField::VectorField(int width, int height) :
	width(width), height(height)
{
	horizontal	= std::vector<double>(width * height, 0.0);
	vertical	= std::vector<double>(width * height, 0.0);

	biggestMagnitude = 1.0f;
}

VectorField::VectorField(int width, int height, const std::vector<double>& hori, const std::vector<double>& vert) :
	width(width), height(height)
{
	horizontal = hori;
	vertical = vert;

	for (double u : horizontal)
	{
		for (double v : vertical)
		{
			biggestMagnitude = std::max(biggestMagnitude, u * u + v * v);
		}
	}

	if (biggestMagnitude == 0.0)	// should use an epsilon probably
		biggestMagnitude = 1.0;

	biggestMagnitude = sqrt(biggestMagnitude);
}

void VectorField::Draw(SDL_Renderer* renderer, const SDL_Rect& targetRect)
{
	double cellWidth = (double)(targetRect.w - targetRect.x) / (double)width;
	double cellHeight = (double)(targetRect.h - targetRect.y) / (double)height;

	SDL_FRect vectorCenterSquare;
	vectorCenterSquare.w = cellWidth / 5.0;
	vectorCenterSquare.h = cellHeight / 5.0;

	SDL_SetRenderDrawColor(renderer, 200, 20, 20, 60);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			vectorCenterSquare.x = (double)targetRect.x + cellWidth * (x + 0.4);	// cellWidth * x + cellWidth / 2 - cellWidth / 10
			vectorCenterSquare.y = (double)targetRect.y + cellHeight * (y + 0.4);
			SDL_RenderFillRectF(renderer, &vectorCenterSquare);
			SDL_RenderDrawLineF(renderer,
				(double)targetRect.x + cellWidth * (x + 0.5),
				(double)targetRect.y + cellHeight * (y + 0.5),
				(double)targetRect.x + cellWidth * (x + 0.5) + horizontal[y * width + x] / biggestMagnitude * cellWidth * 2.5,
				(double)targetRect.y + cellHeight * (y + 0.5) + vertical[y * width + x] / biggestMagnitude * cellHeight * 2.5
			);
		}
	}
}

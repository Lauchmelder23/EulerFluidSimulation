#pragma once

#include <vector>

struct SDL_Rect;
struct SDL_Renderer;

class VectorField
{
public:
	VectorField();
	VectorField(int width, int height);
	VectorField(int width, int height, const std::vector<double>& hori, const std::vector<double>& vert);

	void Draw(SDL_Renderer* renderer, const SDL_Rect& targetRect);
	void RecalculateMagnitude();

public:
	std::vector<double> horizontal;
	std::vector<double> vertical;

private:
	int width, height;

	double biggestMagnitude = 0.0;
};
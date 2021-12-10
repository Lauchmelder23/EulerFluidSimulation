#include "Application.hpp"

int main(int argc, char** argv)
{
	Application* app = new Application(1000, 1000, "Euler Fluid Simulation");
	app->Launch();

	delete app;
	return 0;
}
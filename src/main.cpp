#include "Application.hpp"

int main(int argc, char** argv)
{
	Application* app = new Application(800, 800, "Euler Fluid Simulation");
	app->Launch();

	delete app;
	return 0;
}
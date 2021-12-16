#include "EulerFluid.hpp"
#include "RetentiveArray.hpp"

#include <thread>

int main(int argc, char** argv)
{
	EulerFluid* app = new EulerFluid(1000, 1000, "Euler Fluid Simulation");
	app->Launch();

	delete app;
	return 0;
}
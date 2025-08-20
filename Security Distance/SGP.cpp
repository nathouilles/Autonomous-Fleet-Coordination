#include "SGP.h"

#include "Render.h"
#include "Model.h"
#include "Trace.h"
#include "KPI.h"

#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

// Include GLFW
#include <GLFW/glfw3.h>


//
// ctor: creates Model view controller
//

SGP::SGP()
	: _rndr(nullptr), _mdl(nullptr), _ic("SGP")
{
	Trace t("SGP::SGP");

	_mdl = new Model;
	_rndr = new Render;
	KPI::getKPI().startSimulation();

	_rndr->setModel(_mdl);

}

//
// dtor: détruit le MVC
//

SGP::~SGP()
{
	Trace t("SGP::~SGP");
	delete _rndr;
	delete _mdl;

}

//
// execute la simulation
//

void SGP::operator()()
{
	Trace t("SGP::()");
	const double fps = 1. / 30.;

	_rndr->draw();

	double longi = 90., //starting on the top of the box
		lati = 90.,
		speed = 45., // 45° / sec
		radius = 80.; // camera on R=3 sphere

	do {
		static double before = glfwGetTime();
		double now = glfwGetTime();
		double elapse = now - before;

		if (glfwGetKey(_rndr->window(), GLFW_KEY_UP) == GLFW_PRESS) {
			lati += elapse * speed;
		}
		if (glfwGetKey(_rndr->window(), GLFW_KEY_DOWN) == GLFW_PRESS) {
			lati -= elapse * speed;
		}
		if (glfwGetKey(_rndr->window(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
			longi -= elapse * speed;
		}
		if (glfwGetKey(_rndr->window(), GLFW_KEY_LEFT) == GLFW_PRESS) {
			longi += elapse * speed;
		}
		_rndr->setView(longi, lati);

		_mdl->tick();
		_rndr->draw();
		
		if (elapse < fps)
		{
			//comment if i want full speed calculation
			std::this_thread::sleep_for(std::chrono::milliseconds((int)((fps - elapse) * 1e3)));
		}
			

		before = now;
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(_rndr->window(), GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(_rndr->window()) == 0 && _mdl->getSimulation());
}

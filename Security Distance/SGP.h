#pragma once

#include "InstanceCount.h"

class Render;
class Model;

//
// SGP
// Simulation Gaz Parfait
//

class SGP
{
public:
	// ctor: cree le Model View Controler
	SGP();
	// dtor: détruit le MVS
	~SGP();

	// execute la simulation
	void operator()();

private:
	InstanceCount _ic;
	Render* _rndr;
	Model* _mdl;
};


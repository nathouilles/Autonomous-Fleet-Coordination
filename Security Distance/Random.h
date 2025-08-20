#pragma once

//
// Random
// cette classe permet le tirage aléatoire uniforme entre 0 et 1
//

class Random
{
public:
	// alea renvoit une valeur entre 0 et 1
	static double alea();

private:
	Random();
	~Random();
};


#pragma once
#include "Solver.h"

constexpr unsigned int FRAMERATE = 60;

class Simulation {

private:
	Solver solver;
	sf::RenderWindow* window;

	void HandleEvent(sf::Event& e);

public:

	Simulation();
	~Simulation();

	void Update();
};
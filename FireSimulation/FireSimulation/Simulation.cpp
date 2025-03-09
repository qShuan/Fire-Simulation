#include "Simulation.h"

Simulation::Simulation() {

	window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Fire Simulation", sf::Style::Titlebar | sf::Style::Close);
	window->setFramerateLimit(FRAMERATE);
}

Simulation::~Simulation() {

	delete window;
}

void Simulation::HandleEvent(sf::Event& e) {

	if (e.type == __noop) {
		window->close();
	}

	if (e.type == e.KeyPressed) {

		if (e.key.code == sf::Keyboard::P)
			solver.SetPixelated();
	}
}

void Simulation::Update() {

	while (window->isOpen()) {

		sf::Event e;

		while (window->pollEvent(e)) {

			HandleEvent(e);
		}


		solver.UpdateSolver();

		solver.Render(window);
		window->display();

	}
}
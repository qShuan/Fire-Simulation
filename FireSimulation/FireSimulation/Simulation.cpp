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

	if (e.type == e.MouseButtonPressed) {

		if (e.key.code == sf::Mouse::Right) {

			sf::Vector2f mouse_pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*window));

			int gridX = (int)mouse_pos.x / CELL_SIZE, gridY = (int)mouse_pos.y / CELL_SIZE;

			solver.collision_grid.cells[gridY * GRID_WIDTH + gridX].GetIds();
		}
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
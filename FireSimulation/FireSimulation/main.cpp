#include <iostream>
#include "Solver.h"



int main() {

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Cloth Simulation", sf::Style::Titlebar | sf::Style::Close);

	window.setFramerateLimit(60);
	Solver solver;

	for (int y = RADIUS; y < WINDOW_HEIGHT / 2.f; y += RADIUS + RADIUS) {
		for (int x = RADIUS + RADIUS; x < WINDOW_WIDTH - RADIUS - 5; x += RADIUS + RADIUS) {

			solver.AddParticle({ (float)x + rand() % 2, (float)y});
		}
	}

	while (window.isOpen()) {

		sf::Event e;

		while (window.pollEvent(e)) {

			if (e.type == e.Closed()) {
				window.close();
			}

			if (e.type == e.MouseButtonPressed) {

				if (e.key.code == sf::Mouse::Left) {

					sf::Vector2f mouse_pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

					solver.AddParticle(mouse_pos);
				}
			}
		}

		solver.UpdateSolver();

		//Clear
		window.clear();


		//Draw
		solver.Render(&window);

		window.display();

	}

	return 0;
}
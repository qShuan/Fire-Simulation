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

		if ((int)solver.GetParticles().size() < MAX_PARTICLES) {
			solver.Spawn({ WINDOW_WIDTH / 2 - 200, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 - 150, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 - 100, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 - 50, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 - 15, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 + 15, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 + 50, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 + 100, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 + 150, RENDER_RADIUS });
			solver.Spawn({ WINDOW_WIDTH / 2 + 200, RENDER_RADIUS });

			std::cout << "Number of particles: " << solver.GetParticles().size() << '/' << MAX_PARTICLES << '\n';
		}

		solver.UpdateSolver();


		solver.Render(window);
		window->display();

	}
}
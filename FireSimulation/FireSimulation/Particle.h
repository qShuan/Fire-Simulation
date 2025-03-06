#pragma once
#include "SFML/Graphics.hpp"
#include <iostream>

class Particle {

public:

	float radius = 10.f;
	float temperature = 0.f;

	sf::Vector2f position;
	sf::Vector2f last_position;
	sf::Vector2f acceleration;

	sf::Color color = sf::Color::Black;
	sf::Color initial_color = color;

	Particle(sf::Vector2f position = { 0, 0 }, float radius = 10.f)
		: radius(radius), 
		position(position), 
		last_position(position), 
		acceleration({0.f, 0.f})
	{}


	void Update(float dt) {

		sf::Vector2f velocity = position - last_position;
		last_position = position;
		position = position + velocity + acceleration * (dt * dt);

		acceleration = {};
	}
	
	void Accelerate(sf::Vector2f force) {

		acceleration += force;
	}

	void SetVelocity(sf::Vector2f velocity, float velocity_loss) {

		last_position = position - (velocity * velocity_loss);
	}

	sf::Vector2f GetVelocity() {

		return position - last_position;
	}
};
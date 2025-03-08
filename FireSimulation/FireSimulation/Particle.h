#pragma once
#include "SFML/Graphics.hpp"
#include <iostream>

constexpr float PARTICLE_RADIUS = 4.f;
constexpr float RENDER_RADIUS = PARTICLE_RADIUS * 5.f;

constexpr float MAX_TEMPERATURE = 1500.f;

class Particle {

private:
	sf::Color& LerpColor(sf::Color from, sf::Color to, float dt) {

		color.r = (sf::Uint8)std::lerp(from.r, to.r, dt);
		color.g = (sf::Uint8)std::lerp(from.g, to.g, dt);
		color.b = (sf::Uint8)std::lerp(from.b, to.b, dt);

		return color;
	}

	float& LerpTemperature(float from, float to, float dt) {

		temperature = std::lerp(from, to, dt);

		return temperature;
	}

public:

	int id;

	float radius = 10.f;
	float temperature = 0.f;

	sf::Vector2f position;
	sf::Vector2f last_position;
	sf::Vector2f acceleration;

	sf::Color color = sf::Color::White;

	Particle(sf::Vector2f position = { 0, 0 }, float radius = PARTICLE_RADIUS, int id = 0)
		: id(id),
		radius(radius),
		position(position),
		last_position(position),
		acceleration({ 0.f, 0.f })
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

	void TemperatureBehavior(const int window_height, const int window_width, float dt) {

		LerpTemperature(temperature, 0.f, 2.5f * dt);

		if (position.y + radius >= window_height - 20.f) {

			LerpTemperature(temperature, MAX_TEMPERATURE, 4.f * dt);
		}

		temperature = std::clamp(temperature, 0.f, MAX_TEMPERATURE);
		Accelerate({ 0.f, -std::pow(temperature, 2.f) * 0.01f * (temperature / MAX_TEMPERATURE) });

		const float temp_diff = 300.f;
		const float heat_temp = 500.f;
		const float intermediate_temp = heat_temp + temp_diff;
		const float intermediate_temp2 = intermediate_temp + temp_diff;

		const sf::Color heat_col = sf::Color::Red;
		const sf::Color intermediate_col = sf::Color(255, 147, 5);
		const sf::Color intermediate_col2 = sf::Color(255, 206, 92);

		if (temperature < heat_temp) {

			LerpColor(sf::Color::Black, heat_col, (temperature / heat_temp));
		}
		else if (temperature < intermediate_temp) {
			LerpColor(heat_col, intermediate_col, ((temperature - heat_temp) / temp_diff));

		}
		else if (temperature < intermediate_temp2)
			LerpColor(intermediate_col, intermediate_col2, ((temperature - intermediate_temp) / temp_diff));
		else
			LerpColor(intermediate_col2, sf::Color::White, ((temperature - intermediate_temp2) / (MAX_TEMPERATURE - intermediate_temp2)));
	}

	sf::Vector2f GetVelocity() { return position - last_position; }
};
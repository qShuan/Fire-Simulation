#pragma once
#include "Particle.h"
#include <vector>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define RADIUS 7.5f

static float LerpRadius(float from, float to, float dt) {

	float new_r = std::lerp(from, to, dt);

	return new_r;
}

static sf::Color LerpColor(sf::Color from, sf::Color to, sf::Color to2, float dt) {



	float intermediate_r = std::lerp(from.r, to.r, dt);
	float intermediate_g = std::lerp(from.g, to.g, dt);
	float intermediate_b = std::lerp(from.b, to.b, dt);

	float final_r = std::lerp(intermediate_r, to2.r, dt);
	float final_g = std::lerp(intermediate_g, to2.g, dt);
	float final_b = std::lerp(intermediate_b, to2.b, dt);



	return sf::Color(final_r, final_g, final_b);
}

static sf::Color LerpColor2(sf::Color from, sf::Color to, float dt) {


	float final_r = std::lerp(from.r, to.r, dt);
	float final_g = std::lerp(from.g, to.g, dt);
	float final_b = std::lerp(from.b, to.b, dt);



	return sf::Color(final_r, final_g, final_b);
}


class Solver {

private:

	bool paused = false;
	float m_dt = 1.f / 60.f;
	int sub_steps = 8;

	sf::Vector2f gravity = { 0.f, 1500.f };


public:

	std::vector<Particle> particles;

	Solver() = default;


	void AddParticle(sf::Vector2f position, float radius = RADIUS) {

		Particle particle(position, radius);

		particles.push_back(particle);
	}

	void UpdateSolver() {

		const float sub_dt = m_dt / static_cast<float>(sub_steps);
		for (int i = 0; i < sub_steps; i++) {
			ApplyGravity();
			ApplyCollision();
			ApplyTemperature();
			UpdateObjects(sub_dt);
			ApplyBorder();
		}
	}

	void ApplyGravity() {

		for (auto& particle : particles)
			particle.Accelerate(gravity);
	}

	void ApplyBorder() {

		for (auto& particle : particles) {

			const float velocity_loss_factor = 1.f;
			const float dampening = 0.85f;
			const sf::Vector2f position = particle.position;


			//Horizontal
			if (position.x < particle.radius || position.x + particle.radius > WINDOW_WIDTH) {

				particle.position.x = position.x < particle.radius ? particle.radius : WINDOW_WIDTH - particle.radius;
				particle.SetVelocity({ -particle.GetVelocity().x, particle.GetVelocity().y * dampening }, velocity_loss_factor);
			}

			//Vertical
			if (position.y < particle.radius || position.y + particle.radius > WINDOW_HEIGHT) {

				particle.position.y = position.y < particle.radius ? particle.radius : WINDOW_HEIGHT - particle.radius;
				particle.SetVelocity({ particle.GetVelocity().x * dampening, -particle.GetVelocity().y }, velocity_loss_factor);
			}
		}
	}

	void ApplyCollision() {

		for (int i = 0; i < particles.size(); i++) {

			for (int j = 0; j < particles.size(); j++) {

				if (i == j)
					continue;

				sf::Vector2f dir = particles[i].position - particles[j].position;
				float dst = dir.x * dir.x + dir.y * dir.y;
				float min_dst = particles[i].radius + particles[j].radius;

				if (dst < min_dst * min_dst) {

					float root_dst = std::sqrt(dir.x * dir.x + dir.y * dir.y);
					sf::Vector2f normalized_dir = dir / root_dst;
					float delta = 0.5f * (min_dst - root_dst);

					float mass_ratio1 = particles[i].radius / (particles[i].radius + particles[j].radius);
					float mass_ratio2 = particles[j].radius / (particles[i].radius + particles[j].radius);

					particles[i].position += normalized_dir * delta * mass_ratio1;
					particles[j].position -= normalized_dir * delta * mass_ratio2;

					float total_temp = particles[i].temperature + particles[j].temperature;

					total_temp /= 2.f;

					particles[i].temperature += (total_temp - particles[i].temperature) * 0.7f * m_dt;
					particles[j].temperature += (total_temp - particles[j].temperature) * 0.7f * m_dt;
				}
			}
		}
	}

	void ApplyTemperature() {

		for (auto& particle : particles) {

			if (particle.position.y + particle.radius >= WINDOW_HEIGHT - 20.f) {

				if(particle.position.x + particle.radius < WINDOW_WIDTH / 3)
					particle.temperature += (1500.f - particle.temperature) * 0.35f * m_dt;
				else if(particle.position.x + particle.radius <= WINDOW_WIDTH / 2)
					particle.temperature += (1500.f - particle.temperature) * 0.85f * m_dt;
				else
					particle.temperature += (1500.f - particle.temperature) * 0.35f * m_dt;
			}
			else if (particle.position.y - particle.radius <= WINDOW_HEIGHT / 2.f)
				particle.temperature += (0 - particle.temperature) * .6f * m_dt;

			particle.temperature = std::clamp(particle.temperature, 0.f, 1500.f);

			particle.Accelerate({ 0.f, -std::pow(particle.temperature, 2.f) * 0.01f * (particle.temperature / 1500.f)});
		}
	}

	void UpdateObjects(float dt) {

		for (auto& particle : particles)
			particle.Update(dt);
	}


	void Render(sf::RenderWindow* window) {

		sf::CircleShape circle;
		circle.setPointCount(32);

		for (auto& particle : particles) {

			if (particle.temperature < 400.f)
				continue;

			particle.radius = LerpRadius(4.f, 10.f, particle.temperature / 1500.f);

			circle.setRadius(particle.radius + 1.5f);
			circle.setOrigin(particle.radius + 1.5f, particle.radius + 1.5f);
			circle.setPosition(particle.position);


			if (particle.temperature < 500.f) {

				particle.color = LerpColor2(sf::Color::Black, sf::Color::Red, (particle.temperature / 500.f));
			}
			else if (particle.temperature < 800.f) {
				particle.color = LerpColor2(sf::Color::Red, sf::Color(255, 147, 5), ((particle.temperature - 500.f) / 300.f));

			}
			else if(particle.temperature < 1100.f)
				particle.color = LerpColor2(sf::Color(255, 147, 5), sf::Color(255, 206, 92), ((particle.temperature - 800.f) / 300.f));
			else
				particle.color = LerpColor2(sf::Color(255, 206, 92), sf::Color::White, ((particle.temperature - 1100.f) / 400.f));

			circle.setFillColor(particle.color);

			window->draw(circle);
		}
	}

	std::vector<Particle>& GetParticles() {

		return particles;
	}
};
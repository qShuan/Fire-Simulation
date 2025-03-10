#pragma once
#include "Particle.h"
#include <vector>
#include <algorithm>
#include "Collision_Grid.h"

#define FIRE 1

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr int CELL_SIZE = (int)PARTICLE_RADIUS;

constexpr int GRID_WIDTH = WINDOW_WIDTH / CELL_SIZE;
constexpr int GRID_HEIGHT = WINDOW_HEIGHT / CELL_SIZE;

constexpr int MAX_PARTICLES = 10000;

static float LerpRadius(float from, float to, float dt) {

	float new_r = std::lerp(from, to, dt);

	return new_r;
}


class Solver {

private:

	bool pixelated = false;

	float m_dt = 1.f / 60.f;
	int sub_steps = 8;
	float sub_dt = m_dt / (float)(sub_steps);

	sf::Vector2f gravity = { 0.f, 1500.f };
	sf::VertexArray va{ sf::Triangles }; // Needs to use a special texture
	sf::Texture particle_texture;
	sf::Shader brightExtract, blurH, blurV, combine, pixelate;
	sf::RenderTexture sceneTexture, brightnessTexture, blurTextureH, blurTextureV, finalTexture;
	sf::Texture sceneTextureRef;

	std::vector<std::pair<int, int>> particles_grid_positions; // Holds a particle grid position on it's ID index

	std::vector<Particle> particles;

	void AddParticle(sf::Vector2f position, float radius = PARTICLE_RADIUS) {

		// Prevent spawning particles too close together
		/*for (const auto& p : particles) {
			if (std::hypot(p.position.x - position.x, p.position.y - position.y) < (p.radius + radius)) {
				return;
			}
		}*/

		int grid_position_x = (int)position.x / CELL_SIZE, grid_position_y = (int)position.y / CELL_SIZE;

		Particle particle(position, radius, (int)particles.size());

		collision_grid.cells[grid_position_y * GRID_WIDTH + grid_position_x].particle_ids.push_back((int)particles.size());
		particles_grid_positions.push_back({ grid_position_x, grid_position_y });
		particles.push_back(particle);
	}

	void LoadTexture(const char* texture_path) {

		if (!particle_texture.loadFromFile(texture_path)) {

			std::cerr << "Failed to load texture\n";
			return;
		}

		particle_texture.setSmooth(false);
	}

	void ApplyGravity() {

		for (auto& particle : particles)
			particle.Accelerate(gravity);
	}


	void SolveBorderCollisions() {

		for (auto& particle : particles) {

			float velocity_loss_factor = 1.f;
			float dampening = 0.85f;
			sf::Vector2f position = particle.position;


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

	void ApplyTemperature(float dt) {

		for (auto& particle : particles) {

			particle.TemperatureBehavior(WINDOW_HEIGHT, WINDOW_WIDTH, dt);
		}
	}

	void SolveCells(CollisionCell& curr, CollisionCell& other, float dt) {

		for (auto& idx_1 : curr.particle_ids) {

			Particle& curr_particle = particles[idx_1];

			for (auto& idx_2 : other.particle_ids) {

				if (idx_1 == idx_2) continue;

				Particle& other_particle = particles[idx_2];

				sf::Vector2f dir = curr_particle.position - other_particle.position;
				float dst = dir.x * dir.x + dir.y * dir.y;
				float min_dst = curr_particle.radius + other_particle.radius;

				if (dst < min_dst * min_dst) {

					float root_dst = std::sqrt(dir.x * dir.x + dir.y * dir.y);
					sf::Vector2f normalized_dir = dir / root_dst;
					float delta = 0.5f * (min_dst - root_dst);


					float correction_factor = 0.2f; // Makes sure the simulation doesn't explode

					curr_particle.position += normalized_dir * delta * 0.5f * correction_factor;
					other_particle.position -= normalized_dir * delta * 0.5f * correction_factor;

					// Temperature transfer on collision
					float total_temp = curr_particle.temperature + other_particle.temperature;
					total_temp /= 2.f;

					curr_particle.temperature += (total_temp - curr_particle.temperature) * 0.5f * dt;
					other_particle.temperature += (total_temp - other_particle.temperature) * 0.5f * dt;
				}
			}
		}
	}

	void SolveGridCollisions(float dt) {

		// Only check non-redundant cells
		const std::vector<std::pair<int, int>> neighbors = {
			{-1, 0}, // Left
			{-1, -1}, // Left Up
			{0, 0}, // Current
			{0, -1}, // Up
			{1, -1} // Right up
		};

		// Iterate from bottom to top
		for (int y = GRID_HEIGHT - 1; y >= 0; y--) {

			for (int x = 0; x < GRID_WIDTH; x++) {

				auto& curr = collision_grid.GetCell(y * GRID_WIDTH + x);
				if (curr.particle_ids.empty()) continue;


				for (auto& [dx, dy] : neighbors) {

					int nx = x + dx, ny = y + dy;
					if (nx < 0 || ny < 0 || nx >= GRID_WIDTH) continue;

					auto& other = collision_grid.GetCell(ny * GRID_WIDTH + nx);

					SolveCells(curr, other, dt);
				}
			}
		}
	}

	int FindParticleIDIndex(const CollisionCell& cell, int id) {

		int index = -1;

		for (int i = 0; i < cell.particle_ids.size(); i++) {

			if (cell.particle_ids[i] == id) {

				index = i;
				break;
			}
		}

		return index;
	}

	void UpdateObjects(float dt) {

		for (auto& particle : particles) {

			int curr_grid_position_x = particles_grid_positions[particle.id].first, curr_grid_position_y = particles_grid_positions[particle.id].second;
			particle.Update(dt);

			int grid_position_x = (int)particle.position.x / CELL_SIZE;
			int grid_position_y = (int)particle.position.y / CELL_SIZE;

			particles_grid_positions[particle.id].first = grid_position_x;
			particles_grid_positions[particle.id].second = grid_position_y;

			// Check if grid indices are within bounds
			if (curr_grid_position_x < 0 || curr_grid_position_y < 0 || curr_grid_position_x >= GRID_WIDTH || curr_grid_position_y >= GRID_HEIGHT ||
				grid_position_x < 0 || grid_position_y < 0 || grid_position_x >= GRID_WIDTH || grid_position_y >= GRID_HEIGHT) {
				continue;
			}

			// If grid positions are not the same, update cells
			if (curr_grid_position_x != grid_position_x || curr_grid_position_y != grid_position_y) {

				CollisionCell& pre_update = collision_grid.cells[curr_grid_position_y * GRID_WIDTH + curr_grid_position_x];
				CollisionCell& post_update = collision_grid.cells[grid_position_y * GRID_WIDTH + grid_position_x];

				int index = FindParticleIDIndex(pre_update, particle.id);

				// Only erase if the id was found
				if (index != -1)
					pre_update.particle_ids.erase(pre_update.particle_ids.begin() + index);

				post_update.particle_ids.push_back(particle.id);
			}
		}
	}


	void UpdateVA() {

		for (int i = 0; i < particles.size(); i++) {

			int id = i * 3;
			sf::Vector2f pos = particles[i].position;
			Particle& particle = particles[i];
			float radius = particle.radius;


			// Change radius depending on temperature
			if (particles.size() >= MAX_PARTICLES) {

				if (FIRE) {

					radius = LerpRadius(0.f, RENDER_RADIUS, (particle.temperature / 1500.f));

					if (radius < RENDER_RADIUS / 4.f)
						radius = 0.f;
				}
			}

			va[id].position = pos + sf::Vector2f(-radius, -radius);
			va[id + 1].position = pos + sf::Vector2f(radius, -radius);
			va[id + 2].position = pos + sf::Vector2f(0.f, radius);

			va[id].texCoords = sf::Vector2f(0.f, 0.f);
			va[id + 1].texCoords = sf::Vector2f(400.f, 0.f);
			va[id + 2].texCoords = sf::Vector2f(200.f, 400.f);

			va[id].color = particle.color;
			va[id + 1].color = particle.color;
			va[id + 2].color = particle.color;
		}
	}


	void InitTextures() {

		sceneTexture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
		brightnessTexture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
		blurTextureH.create(WINDOW_WIDTH, WINDOW_HEIGHT);
		blurTextureV.create(WINDOW_WIDTH, WINDOW_HEIGHT);
		finalTexture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
	}

	void InitShaders() {

		brightExtract.loadFromFile("BrightnessExtraction.frag", sf::Shader::Fragment);
		blurH.loadFromFile("GaussianHorizontal.frag", sf::Shader::Fragment);
		blurV.loadFromFile("GaussianVertical.frag", sf::Shader::Fragment);
		combine.loadFromFile("CombineBlur.frag", sf::Shader::Fragment);
		pixelate.loadFromFile("Pixelation.frag", sf::Shader::Fragment);


		blurH.setUniform("resolution", (float)WINDOW_WIDTH);
		blurV.setUniform("resolution", (float)WINDOW_HEIGHT);
		pixelate.setUniform("resolution", sf::Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
		pixelate.setUniform("pixelSize", 5.f);
	}

public:

	CollisionGrid collision_grid;

	Solver() {

		collision_grid.cells.resize(GRID_HEIGHT * GRID_WIDTH);

		LoadTexture("circle.png");

		va.resize(MAX_PARTICLES * 3);

		InitTextures();
		InitShaders();
	}

	void Spawn(sf::Vector2f position) {

		if(particles.size() < MAX_PARTICLES)
			AddParticle(position + sf::Vector2f((float)(rand() % 2), 0.f));
	}

	void SetPixelated() {

		pixelated = !pixelated;
	}

	void UpdateSolver() {

		for (int i = 0; i < sub_steps; i++) {

			ApplyGravity();
			SolveGridCollisions(sub_dt);

			if(particles.size() >= MAX_PARTICLES)
				ApplyTemperature(sub_dt);

			SolveBorderCollisions();
			UpdateObjects(sub_dt);
		}
	}

	void Render(sf::RenderWindow* window) {

		UpdateVA();

		sceneTexture.clear();
		sf::RenderStates states;
		states.texture = &particle_texture;
		sceneTexture.draw(va, states);
		sceneTexture.display();


		brightnessTexture.clear();
		brightExtract.setUniform("texture", sceneTexture.getTexture());
		sf::Sprite sceneSprite(sceneTexture.getTexture());
		brightnessTexture.draw(sceneSprite, &brightExtract);
		brightnessTexture.display();


		blurTextureH.clear();
		blurH.setUniform("texture", brightnessTexture.getTexture());
		sf::Sprite brightSprite(brightnessTexture.getTexture());
		blurTextureH.draw(brightSprite, &blurH);
		blurTextureH.display();


		blurTextureV.clear();
		blurV.setUniform("texture", blurTextureH.getTexture());
		sf::Sprite blurSprite(blurTextureH.getTexture());
		blurTextureV.draw(blurSprite, &blurV);
		blurTextureV.display();


		finalTexture.clear();
		combine.setUniform("originalScene", sceneTexture.getTexture());
		combine.setUniform("blurredBloom", blurTextureV.getTexture());
		finalTexture.draw(sceneSprite, &combine);
		finalTexture.display();


		pixelate.setUniform("texture", finalTexture.getTexture());
		window->clear();
		sf::Sprite finalSprite(finalTexture.getTexture());

		if (pixelated)
			window->draw(finalSprite, &pixelate); // Final render pass
		else
			window->draw(finalSprite, &combine);
	}

	std::vector<Particle>& GetParticles() {

		return particles;
	}
};
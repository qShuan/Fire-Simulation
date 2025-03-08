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

static float LerpRadius(float from, float to, float dt) {

	float new_r = std::lerp(from, to, dt);

	return new_r;
}

class Solver {

private:

	float m_dt = 1.f / 60.f;
	int sub_steps = 8;
	float sub_dt = m_dt / (float)(sub_steps);

	sf::Vector2f gravity = { 0.f, 1500.f };
	sf::VertexArray va{ sf::Triangles }; // Needs to use a special texture
	sf::Texture particle_texture;
	sf::Shader brightExtract, blurH, blurV, combine;
	sf::RenderTexture sceneTexture, brightTexture, blurTexture1, blurTexture2;
	sf::Texture sceneTextureRef;

	std::vector<std::pair<int, int>> particles_grid_positions; // Holds a particle grid position on it's ID index

	std::vector<Particle> particles;

	void AddParticle(sf::Vector2f position, float radius = PARTICLE_RADIUS) {

		// Prevent spawning particles too close together
		for (const auto& p : particles) {
			if (std::hypot(p.position.x - position.x, p.position.y - position.y) < (p.radius + radius)) {
				return;
			}
		}

		int gridX = (int)position.x / CELL_SIZE, gridY = (int)position.y / CELL_SIZE;

		Particle particle(position, radius, (int)particles.size());

		collision_grid.cells[gridY * GRID_WIDTH + gridX].particle_ids.push_back((int)particles.size());
		particles_grid_positions.push_back({ gridX, gridY });
		particles.push_back(particle);
	}

	void SpawnParticles() {

		for (int y = (int)PARTICLE_RADIUS * 2; y < WINDOW_HEIGHT - (int)PARTICLE_RADIUS - 5; y += (int)PARTICLE_RADIUS) {
			for (int x = (int)PARTICLE_RADIUS * 2; x < WINDOW_WIDTH - (int)PARTICLE_RADIUS - 5; x += (int)PARTICLE_RADIUS) {

				AddParticle({ (float)x + (float)(rand() % 2), (float)y }, PARTICLE_RADIUS);
			}
		}
	}

	void LoadTexture(const char* texture_path) {

		if (!particle_texture.loadFromFile(texture_path)) {

			std::cerr << "Failed to load texture\n";
			exit(-1);
		}

		particle_texture.setSmooth(false);
	}

	void ApplyGravity() {

		for (auto& particle : particles)
			particle.Accelerate(gravity);
	}

	void SolveBorderCollisions() {

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


					float correction_factor = 0.1f; // Makes sure the simulation doesn't explode

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
		const int dx[] = { 1, 1, 0, 0, -1 };
		const int dy[] = { 0, 1, 0, 1, 1 };

		for (int y = 0; y < GRID_HEIGHT; y++) {

			for (int x = 0; x < GRID_WIDTH; x++) {

				auto& curr = collision_grid.GetCell(y * GRID_WIDTH + x);
				if (curr.particle_ids.empty()) continue;


				for (int k = 0; k < 5; k++) {
					int nx = x + dx[k], ny = y + dy[k];
					if (nx < 0 || ny < 0 || nx >= GRID_WIDTH || ny >= GRID_HEIGHT) continue;

					auto& other = collision_grid.GetCell(ny * GRID_WIDTH + nx);

					SolveCells(curr, other, dt);

				}
			}
		}
	}

	void UpdateObjects(float dt) {

		for (auto& particle : particles) {

			int curr_gridX = particles_grid_positions[particle.id].first, curr_gridY = particles_grid_positions[particle.id].second;
			particle.Update(dt);

			int gridX = (int)particle.position.x / CELL_SIZE;
			int gridY = (int)particle.position.y / CELL_SIZE;

			particles_grid_positions[particle.id].first = gridX;
			particles_grid_positions[particle.id].second = gridY;

			// Check if grid indices are within bounds
			if (curr_gridX < 0 || curr_gridY < 0 || curr_gridX >= GRID_WIDTH || curr_gridY >= GRID_HEIGHT ||
				gridX < 0 || gridY < 0 || gridX >= GRID_WIDTH || gridY >= GRID_HEIGHT) {
				continue;
			}

			if (curr_gridX != gridX || curr_gridY != gridY) {

				CollisionCell& pre_update = collision_grid.cells[curr_gridY * GRID_WIDTH + curr_gridX];
				CollisionCell& post_update = collision_grid.cells[gridY * GRID_WIDTH + gridX];

				auto pos = std::find(pre_update.particle_ids.begin(), pre_update.particle_ids.end(), particle.id);

				// Only erase if the id was found
				if (pos != pre_update.particle_ids.end())
					pre_update.particle_ids.erase(pos);

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

			if (FIRE)
				radius = LerpRadius(0.f, RENDER_RADIUS, (particle.temperature / 1500.f));

			if (radius < RENDER_RADIUS / 5.f)
				radius = 0.f;

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

public:

	CollisionGrid collision_grid;

	Solver() {

		collision_grid.cells.resize(GRID_HEIGHT * GRID_WIDTH);

		LoadTexture("circle.png");
		SpawnParticles();

		va.resize(particles.size() * 3);

		sceneTexture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
		brightTexture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
		blurTexture1.create(WINDOW_WIDTH, WINDOW_HEIGHT);
		blurTexture2.create(WINDOW_WIDTH, WINDOW_HEIGHT);



		brightExtract.loadFromFile("BrightnessExtraction.frag", sf::Shader::Fragment);
		blurH.loadFromFile("GaussianHorizontal.frag", sf::Shader::Fragment);
		blurV.loadFromFile("GaussianVertical.frag", sf::Shader::Fragment);
		combine.loadFromFile("CombineBlur.frag", sf::Shader::Fragment);
	}

	void UpdateSolver() {

		for (int i = 0; i < sub_steps; i++) {

			ApplyGravity();
			SolveGridCollisions(sub_dt);
			ApplyTemperature(sub_dt);
			SolveBorderCollisions();
			UpdateObjects(sub_dt);
		}
	}

	void Render(sf::RenderWindow* window) {

		UpdateVA();

		sceneTextureRef = sceneTexture.getTexture();
		brightExtract.setUniform("texture", sceneTextureRef);
		blurH.setUniform("texture", brightTexture.getTexture());
		blurH.setUniform("resolution", static_cast<float>(WINDOW_WIDTH)); // Adjust based on screen size
		blurV.setUniform("texture", blurTexture1.getTexture());
		blurV.setUniform("resolution", static_cast<float>(WINDOW_HEIGHT)); // Adjust based on screen size
		combine.setUniform("originalScene", sceneTexture.getTexture());
		combine.setUniform("blurredBloom", blurTexture2.getTexture());

		sceneTexture.clear();
		sf::RenderStates states;
		states.texture = &particle_texture;
		sceneTexture.draw(va, states);
		sceneTexture.display();

		// Step 2: Extract bright areas
		brightTexture.clear();
		sf::Sprite sceneSprite(sceneTexture.getTexture());
		brightTexture.draw(sceneSprite, &brightExtract); // Apply brightness extraction
		brightTexture.display();

		// Step 3: Apply horizontal blur
		blurTexture1.clear();
		sf::Sprite brightSprite(brightTexture.getTexture());
		brightSprite.setScale(1, -1);  // Flip vertically
		brightSprite.setPosition(0.f, (float)WINDOW_HEIGHT);  // Move it back into place
		blurTexture1.draw(brightSprite, &blurH);
		blurTexture1.display();

		// Step 4: Apply vertical blur
		blurTexture2.clear();
		sf::Sprite blurSprite(blurTexture1.getTexture());
		blurTexture2.draw(blurSprite, &blurV);
		blurTexture2.display();

		window->clear();
		combine.setUniform("originalScene", sceneTexture.getTexture());
		combine.setUniform("blurredBloom", blurTexture2.getTexture());
		window->draw(sceneSprite, &combine); // Final render pass
	}

	std::vector<Particle>& GetParticles() {

		return particles;
	}
};
#pragma once
#include <vector>

class CollisionCell {

public:
	sf::Vector2f position;
	std::vector<int> particle_ids;

	void GetIds() {

		if (particle_ids.empty()) {
			std::cout << "Cell is empty\n";
			return;
		}

		std::cout << "{";
		for (auto& id : particle_ids)
			std::cout  << id << ", ";
		std::cout << "}\n";
	}
};

class CollisionGrid {

public:
	std::vector<CollisionCell> cells;

	CollisionCell& GetCell(int index) { return cells[index]; }
};
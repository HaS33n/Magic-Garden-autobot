#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

typedef std::pair<int, int> point;
enum class side : bool {L = 0, R = 1};

class Configuration {
public:
	static point base_p, sell_p, interact_p;
	static side current_side;
	static sf::Time interval;
};
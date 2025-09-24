#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include "Execution.hpp"
#include <atomic>
#include <thread>

#define _DEBUG 0
#define _QUIT_ON_DEBUG_CFG 0

enum class AppState {init, running, halted};

class Application{
public:
	Application();
	~Application();
	void run();

private:
	void init();
	void loadCfgFromFile(const std::string path);
	void compileCommandList();
	void setDiscordHandle();

	void handleEvents();
	void update();
	void draw();

	void updateStatusIndicator();

	AppState state;
	sf::Clock clk;
	bool fresh; //ugly

	HWND discord_hwnd;

	std::vector<std::unique_ptr<Fragment>>* fragments;
	std::vector<std::unique_ptr<Fragment>>::iterator rside_beginning;
	std::list<std::variant<WORD, point>>* command_list;
	std::list<std::variant<WORD, point>>::iterator iter;

	std::vector<std::unique_ptr<sf::Drawable>> render_objs;
	sf::RenderWindow window;

	sf::Font fnt;
};
#include "../include/Application.hpp"
#include <fstream>
#include <iostream>
#include <string>

Application::Application() : window(sf::VideoMode({ 600,600 }), "Ogrodnik"), fnt("arial.ttf"){
	//window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

	fragments = nullptr;
    command_list = nullptr;
    state = AppState::init;
    fresh = true;

    sf::RectangleShape* indicator = new sf::RectangleShape({ 150,150 });
    indicator->setFillColor(sf::Color(105, 102, 101)); //grey
    indicator->setOrigin(indicator->getSize() * 0.5f);
    indicator->setPosition({ window.getSize().x * 0.5f, window.getSize().y * 0.5f });
    
    render_objs.emplace_back(indicator);

    sf::Text* txt = new sf::Text(fnt, "hai", 25);
    txt->setFillColor(sf::Color::White);
    render_objs.emplace_back(txt);

    discord_hwnd = NULL;
    setDiscordHandle();
}

Application::~Application(){
    delete fragments;
    delete command_list;
}

void Application::run(){

    while (window.isOpen()){
        handleEvents();
        update();
        draw();
    }

}

void Application::init(){
	Configuration::current_side = side::L;
	Configuration::interval = sf::seconds(5 * 60); //5 * 60

	loadCfgFromFile("config.txt");
    compileCommandList();
    if (_DEBUG && _QUIT_ON_DEBUG_CFG) {
        std::cout << "QUIT ON DEBUG CFG\n";
        exit(2);
    }

    Sleep(3000);
}

static Fragment str2frag(const char* str) {
    int data[4];
    if (sscanf(str, "%d %d %d %d", &data[0], &data[1], &data[2], &data[3]) != 4) {
        std::cerr << "wrong frag\n";
        exit(EXIT_FAILURE);
    }

    return { point(data[0],data[1]), point(data[2], data[3]) };
}

static point str2point(const char* str) {
    int data[2];
    if (sscanf(str, "%d %d", &data[0], &data[1]) != 2) {
        std::cerr << "wrong point\n";
        exit(EXIT_FAILURE);
    }

    return { data[0], data[1] };
}

void Application::loadCfgFromFile(const std::string path){
    fragments = new std::vector<std::unique_ptr<Fragment>>;
    fragments->reserve(200); //ugly as hell, but gets the job done


    //fragments->emplace_back(new Fragment({5,5}, {0,2}));

    std::fstream file(path, std::ios::in);
    if (!file.good()) {
        std::cerr << "cannot open cfg\n";
        exit(EXIT_FAILURE);
    }

    std::string line;

    std::getline(file, line);
    Configuration::interact_p = str2point(line.c_str());

    std::getline(file, line);
    Configuration::base_p = str2point(line.c_str());

    std::getline(file, line);
    Configuration::sell_p = str2point(line.c_str());

    bool rside = false;
    bool rside_exists = false;
    while (std::getline(file, line)) {
        if (line == "")
            continue;

        if (line == "RSIDE_BEGIN") {
            rside = true;
            continue;
        }
        fragments->emplace_back(new Fragment(str2frag(line.c_str()))); //????
        if (rside) {
            rside_exists = true;
            rside = false;
            rside_beginning = fragments->end() - 1; //here
        }
    }
    if (!rside_exists)
        rside_beginning = fragments->end();
}

void Application::compileCommandList(){
    command_list = new std::list<std::variant<WORD, point>>;

    //for (auto& it : *fragments) 
    for (auto it = fragments->begin(); it != fragments->end(); ++it) {

        if (it == rside_beginning)
            Configuration::current_side = side::R;

        auto& value = *it;
        auto* lst = value.get()->executeFrag();
        command_list->splice(command_list->end(), *lst);
        delete lst;
    }

    iter = command_list->begin();
    
    //dump commands for debugging
    if (_DEBUG) {
        for (auto& it : *command_list) {
            if ((std::holds_alternative<WORD>(it))) {
            
                switch (std::get<WORD>(it)) {
                case 0x41:
                    std::cout << "A\n";
                    break;
                case 0x44:
                    std::cout << "D\n";
                    break;
                case 0x57:
                    std::cout << "W\n";
                    break;
                case 0x53:
                    std::cout << "S\n";
                    break;
                default:
                    std::cout << "N/A\n";
                    break;
                }
            }
            else {
                auto p = std::get<point>(it);

                if(p == Configuration::base_p)
                    std::cout << "BASE\n";
                else if (p == Configuration::sell_p)
                    std::cout << "SELL\n";
                else if (p == Configuration::interact_p)
                    std::cout << "INTERACT\n";
                else 
                    std::cout << "N/A CLICK\n";
            }
        }
    }
}

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM target_hwnd) {
    wchar_t title[256];
   
    if (GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t))) {
        std::wstring wstr(title);
        if (wstr.rfind(L" - Discord") != std::wstring::npos) {
            *(HWND*)target_hwnd = hwnd;
            return FALSE;
        }

    }
    return TRUE;
}

void Application::setDiscordHandle(){

    EnumWindows(EnumWindowsProc, (LPARAM)&discord_hwnd);

}

void Application::handleEvents() {

    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                window.close();

            else if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
                if (state == AppState::init) {
                    state = AppState::running;
                    init();
                }
                else if(state == AppState::running)
                    state = AppState::halted;
                else {
                    state = AppState::running;
                    Sleep(3000);
                }


                updateStatusIndicator();
            }
        }
        else if (const auto* mbpressd = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mbpressd->button == sf::Mouse::Button::Left) {

            }

        }
    }
}

void Application::update() {
    sf::Text* ptr = dynamic_cast<sf::Text*>(render_objs[1].get());
    auto pos = sf::Mouse::getPosition();
    std::string str = "MOUSE COORDS: " + std::to_string(pos.x) + " , " + std::to_string(pos.y);
    ptr->setString(str);

    //safety: if discord lost focus, stop script execution
    bool pause = (1 - _DEBUG);
    if (discord_hwnd != NULL) {
       pause = GetForegroundWindow() != discord_hwnd;
    }


    if ((clk.getElapsedTime() >= Configuration::interval && state == AppState::running ) || (fresh && state == AppState::running)) {
        if (iter == command_list->end()) {
            clk.restart();
            iter = command_list->begin();

            fresh = false;
        }

        else if(!pause){
            sendINP(*iter);
            iter++;
        }
    }
    
}

void Application::draw() {
    window.clear(sf::Color::Black);
    for (auto& it : render_objs)
        window.draw(*it);

    window.display();
}

void Application::updateStatusIndicator(){
    sf::RectangleShape* ptr = dynamic_cast<sf::RectangleShape*>(render_objs[0].get());

    if (state == AppState::running)
        ptr->setFillColor(sf::Color::Green);
    else
        ptr->setFillColor(sf::Color::Red);
}

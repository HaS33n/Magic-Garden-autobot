#include "../include/Execution.hpp"


#define goUP inputs->emplace_back(WORD(0x57))
#define goDOWN inputs->emplace_back(WORD(0x53))
#define goBACK inputs->emplace_back(WORD(0x44 - 3 * (bool)Configuration::current_side))
#define goFRWD inputs->emplace_back(WORD(0x41 + 3 * (bool)Configuration::current_side))


#define INTERACT inputs->emplace_back(point(Configuration::interact_p))
#define BASE inputs->emplace_back(point(Configuration::base_p))
#define SHOP inputs->emplace_back(point(Configuration::sell_p))

void sendFakeKey(WORD vk, bool shift) {
    std::vector<INPUT> inputs;

    if (shift) {
        INPUT s = {};
        s.type = INPUT_KEYBOARD;
        s.ki.wVk = VK_SHIFT;
        inputs.push_back(s);
    }
    
    INPUT k = {};
    k.type = INPUT_KEYBOARD;
    k.ki.wVk = vk;
    inputs.push_back(k);

    INPUT k2 = {};
    k2.type = INPUT_KEYBOARD;
    k2.ki.wVk = vk;
    k2.ki.dwFlags = KEYEVENTF_KEYUP;
    inputs.push_back(k2);

    if (shift) {
        INPUT s = {};
        s.type = INPUT_KEYBOARD;
        s.ki.wVk = VK_SHIFT;
        s.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(s);
    }

    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

void clickAt(point p){
    SetCursorPos(p.first, p.second);

    INPUT input[2] = {};

    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(2, input, sizeof(INPUT));
}

void sendINP(std::variant<WORD, point> param){
    if ((std::holds_alternative<WORD>(param)))
        sendFakeKey(std::get<WORD>(param));

    else
        clickAt(std::get<point>(param));
}

std::list<std::variant<WORD, point>>* setStartingPos(){
    std::list<std::variant<WORD, point>>* inputs = new std::list<std::variant<WORD, point>>;

    BASE;
    goUP;
    goFRWD;

    return inputs;
}

Fragment::Fragment(point uf, point lb) : upper_fwd(uf), lower_bcwd(lb) {

}

std::list<std::variant<WORD, point>>* Fragment::executeFrag() const{
    std::list<std::variant<WORD, point>>* inputs = setStartingPos();

    const point destination(lower_bcwd.first, upper_fwd.second);
    for (int i = 0; i < destination.first; i++) {
        goFRWD;
    }
    for (int i = 0; i < destination.second; i++) {
        goUP;
    }

    const int width = upper_fwd.first - lower_bcwd.first;
    const int height = upper_fwd.second - lower_bcwd.second;

    for (int i = 0; i <= height; i++) { //modified <= check if not =
        bool direction = i % 2 == 0; //if even go fwd, back otherwise

        for (int j = 0; j < width; j++) {
            INTERACT;
            INTERACT;
            if (direction) {
                goFRWD;
            }
            else
                goBACK;
        }
        INTERACT;
        INTERACT;
        goDOWN;
    }

    SHOP;
    INTERACT;

    return inputs;
}

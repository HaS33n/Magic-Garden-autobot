#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <list>
#include "Configuration.hpp"

void sendFakeKey(WORD vk, bool shift = false);
void clickAt(point p);
void sendINP(std::variant<WORD,point> param);

std::list<std::variant<WORD, point>>* setStartingPos();

class Fragment {
public:
    Fragment(point uf, point lb);
    std::list<std::variant<WORD, point>>* executeFrag() const;
private:
    point upper_fwd, lower_bcwd;

};


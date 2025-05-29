#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Test");
    std::cout << "SFML includes work!" << std::endl;
    window.close();
    return 0;
}

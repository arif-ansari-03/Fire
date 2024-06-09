#include <SFML/Graphics.hpp>
#include <verlet.hpp>
#include <cstdlib> 
#include <iostream> 
#include <time.h> 
using namespace std;

int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    sf::RenderWindow window(sf::VideoMode(900, 900), "SFML works!", sf::Style::Default, settings);

    srand(time(0));

    Solver solver(0);
    solver.add_particle(sf::Vector2f(450.f, 850.f));

    int i = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        window.clear();

        if (++i <= 3000000 && !(i%100)) { solver.P[0].temperature += 5.f; }

        solver.update(0.01f);

        for (auto &particle: solver.P)
        {
            window.draw(particle.shape);
        }
        window.display();
    }

    return 0;
}
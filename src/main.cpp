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

    sf::RectangleShape rectangle;
    rectangle.setFillColor(sf::Color::White);
    rectangle.setSize(sf::Vector2f(900.f, 900.f));

    srand(time(0));

    Solver solver(0);

    for (float x = 22.f; x < 340.f; x += 7.f)
    for (float y = 12.f; y < 540.f; y += 7.f)
    {
        solver.add_particle(sf::Vector2f(x, y));
        solver.P.back().update_velocity(sf::Vector2f(1.f, 0.f), 0.01f);
    }

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

        

        solver.update(0.04f);
        window.draw(rectangle);

        // if (solver.n >= 700)
        for (auto &particle: solver.P)
        {
            window.draw(particle.shape);
        }

        window.display();
    }

    return 0;
}
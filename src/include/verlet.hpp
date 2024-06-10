#include <SFML/Graphics.hpp>
#include <vector>
#include <math.h>
#include <random>

using namespace std;

sf::Color temp_to_col(float T)
{
	float R = 5000.f, G = 20000.f, B = 30000.f;
	float r = min(255.f, T / R * 255.f);
	float g = max(0.f, min(255.f, (T-R)/(G-R) * 255.f));
	float b = max(0.f, min(255.f, (T-G)/(B-G) * 255.f));

    return sf::Color(r, g, b);
}

float length(sf::Vector2f a)
{
    return sqrt(a.x*a.x + a.y*a.y);
}

struct particle
{
    sf::Vector2f position;
    sf::Vector2f old_position;
    sf::Vector2f acceleration;
    float radius;
    sf::CircleShape shape;
    sf::Vector2f rr;

    float temperature;
    float k = 800.f;

    particle()
    {
        position = {400.f, 400.f};
        old_position = position;
        radius = 10.f;
        rr = {radius, radius};
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(position-rr);
    }

    particle(sf::Vector2f pos)
    {
        position = pos;
        temperature = 100.f;
        old_position = position;
        radius = 5.f;
        rr = {radius, radius};
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(position-rr);
    }

    void update(float dt)
    {
        sf::Vector2f velocity = position - old_position;

        old_position = position;

        position = position + velocity + acceleration * dt * dt;

        acceleration = {};

        temperature -= k * dt;
        if (temperature < 0) temperature = 0;

        shape.setPosition(position - rr);
        shape.setFillColor(temp_to_col(temperature));
    }

    void update_velocity(sf::Vector2f v, float dt)
    {
        old_position = position - v * dt;
    }

    void accelerate(sf::Vector2f acc)
    {
        acceleration += acc;
    }
};


struct Solver
{
    sf::Vector2f gravity = {0.f, 12.f};
    vector<particle> P;
    float k = 0.6f;
    int n;
    float border_x = 900.f, border_y = 900.f;

    Solver(int a)
    {
        P = vector<particle>(a);
        n = a;
    }

    void add_particle(sf::Vector2f pos)
    {
        P.emplace_back(particle(pos));
        n++;
    }

    void apply_gravity()
    {
        for (auto &particle : P)
        particle.accelerate(gravity);
    }

    void apply_bouyant_force()
    {
        for (auto &particle : P)
        {
            sf::Vector2f F = {0.f, -0.01f * (min(20000.f, particle.temperature))};
            particle.accelerate(F);
        }
    }

    void update_position(float dt)
    {
        for (auto &particle: P)
        particle.update(dt);
    }

    void apply_constraint()
    {
        // sf::Vector2f center = {450.f, 450.f};
        // float radius = 200.f;

        // for (auto &particle: P)
        // {
        //     sf::Vector2f to_obj = particle.position - center;
        //     float dist = length(to_obj);

        //     if (dist > radius - particle.radius)
        //     {
        //         sf::Vector2f n = to_obj / length(to_obj) * (radius - particle.radius);
        //         particle.position = center + n;
        //     }
        // }
        
        for (auto &particle: P)
        {
            particle.position.x = min(particle.position.x, border_x - particle.radius);
            particle.position.x = max(particle.position.x, particle.radius);
            particle.position.y = min(particle.position.y, border_y - particle.radius);
            particle.position.y = max(particle.position.y, particle.radius);
        }
    }

    void solve_collisions(float dt)
    {
        for (int i = 0; i < n-1; i++)
        {
            // temp update
            if (P[i].position.y+P[i].radius>=border_y-40.f)
                P[i].temperature += 0.3f * (20000 - P[i].temperature) * dt;

            for (int j = i+1; j < n; j++)
            {
                if (P[j].position.x - P[i].position.x > 20.f) break;

                sf::Vector2f col_axis = P[i].position - P[j].position;
                float dist = length(col_axis);

                if (dist < P[i].radius+P[j].radius)
                {
                    col_axis = col_axis / dist;
                    dist = P[i].radius+P[j].radius - dist;
                    P[i].position = P[i].position + col_axis * .5f * dist;
                    P[j].position = P[j].position - col_axis * .5f * dist;

                    // update temp
                    float delta_T = P[i].temperature-P[j].temperature;
                    
                    P[i].temperature = P[i].temperature - 0.5f * delta_T * dt;
                    P[j].temperature = P[j].temperature + 0.5f * delta_T * dt;

                    P[i].shape.setFillColor(temp_to_col(P[i].temperature));
                    P[j].shape.setFillColor(temp_to_col(P[i].temperature));
                }
            }
        }
    }

    void udpate_temperature(float dt)
    {
        for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++)
        {
            if (P[j].position.x - P[i].position.x > 21.f) break;

            sf::Vector2f col_axis = P[i].position - P[j].position;
            float dist = length(col_axis)-0.03f;

            if (dist < P[i].radius+P[j].radius)
            {
                float delta_T = P[i].temperature-P[j].temperature;
                
                P[i].temperature = P[i].temperature - k * delta_T * dt;
                P[j].temperature = P[j].temperature + k * delta_T * dt;

                P[i].shape.setFillColor(temp_to_col(P[i].temperature));
                P[j].shape.setFillColor(temp_to_col(P[i].temperature));
            }
        }

        for (auto &particle : P)
        {
            if (particle.position.y+particle.radius>=border_y-1.f)
                particle.temperature += 0.3f * k * (20000 - particle.temperature) * dt;
        }
    }

    void update(float dt)
    {
        for (int i = 0; i < 1; i++)
        {
            sort(P.begin(), P.end(), [](particle &a, particle &b){return a.position.x < b.position.x;});
            apply_gravity();
            apply_bouyant_force();
            apply_constraint();
            solve_collisions(dt);
            update_position(dt);
        }
    }
};
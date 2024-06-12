#include <SFML/Graphics.hpp>
#include <vector>
#include <math.h>
#include <random>

using namespace std;

sf::Color temp_to_col(float T)
{
	float R = 700.f, G = 40000.f, B = 70000.f;
	float r = min(255.f, T / R * 255.f);
	float g = max(0.f, min(255.f, (T-R)/(G-R) * 255.f));
	float b = max(0.f, min(255.f, (T-G)/(B-G) * 255.f));

    float a = 255.f - 1.f / T;
    // a *= 255.f;
    a /= 2.f;
    if (T < 6000.f) a = 0.f;
    a = 255.f;
    return sf::Color(r, g, b, a);
}

float radius_prop(float T)
{
    return (90000.f - T) / 90000.f;
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
    float k = .6f;

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

    particle(sf::Vector2f pos, float rad)
    {
        position = pos;
        temperature = 100.f;
        old_position = position;
        radius = rad;
        rr = {radius, radius};
        shape.setRadius(radius+0.f);
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(position-rr);
    }

    void update(float dt)
    {
        sf::Vector2f velocity = position - old_position;

        old_position = position;

        position = position + velocity + acceleration * dt * dt;

        acceleration = {};

        temperature -= k * temperature * dt;
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

vector<int> grid[2000][1000];

struct Solver
{
    sf::Vector2f gravity = {0.f, 100.f};
    vector<particle> P;
    float k = 0.6f;
    float radius = 3.f;
    int n;
    float border_x = 350.f, border_y = 700.f;

    Solver(int a)
    {
        P = vector<particle>(a);
        n = a;
    }

    void add_particle(sf::Vector2f pos)
    {
        P.emplace_back(particle(pos, radius));
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
            sf::Vector2f F = {0.f, -0.03f * particle.temperature};
            // if (particle.temperature > 20000.f) F.y -= 14000.f;
            // if (particle.temperature > 25000.f) F.y -= 14000.f;
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

    void solve_cell_and_particle(vector<int> &_grid, particle &particle, float dt)
    {
        for (int &i : _grid)
        {
            sf::Vector2f col_axis = P[i].position - particle.position;
            float dist = length(col_axis);
            if (dist < P[i].radius + particle.radius)
            {
                col_axis = col_axis / dist;
                dist = P[i].radius + particle.radius - dist;
                P[i].position = P[i].position + 0.5f * dist * col_axis;
                particle.position = particle.position - 0.5f * dist * col_axis;

                float delta_T = P[i].temperature-particle.temperature;
                    
                P[i].temperature = P[i].temperature - 7.f * delta_T * dt;
                particle.temperature = particle.temperature + 7.f * delta_T * dt;

                P[i].shape.setFillColor(temp_to_col(P[i].temperature));
                // P[i].shape.setRadius(2.f * radius_prop(P[i].temperature) * P[i].radius);
                particle.shape.setFillColor(temp_to_col(particle.temperature));
                // particle.shape.setRadius(2.f * radius_prop(particle.temperature) * particle.radius);
            }
        }
    }

    float left_border = 30.f, right_border = 450.f;
    void solve_collisions(float dt)
    {
        int grid_size = 2 * radius;

        vector<pair<int, int>> temp;

        for (int m = 0; m < n; m++)
        {
            if (P[m].position.y+P[m].radius>=border_y-10.f)
            {
                P[m].temperature += 0.95f * (30000 - P[m].temperature) * dt;
                // P[m].shape.setRadius(2.f * radius_prop(P[m].temperature) * P[m].radius);
            }
            particle &particle = P[m];
            int x = 1+particle.position.x, y = 1+particle.position.y;
            int i = x / grid_size, j = y / grid_size;
            ++i, ++j;
            ++i, ++j;

            if (i-1<0 ||j-1<0) continue;
            solve_cell_and_particle(grid[i-1][j-1], particle, dt);
            solve_cell_and_particle(grid[i-1][j], particle, dt);
            solve_cell_and_particle(grid[i-1][j+1], particle, dt);
            solve_cell_and_particle(grid[i][j-1], particle, dt);
            solve_cell_and_particle(grid[i][j], particle, dt);
            solve_cell_and_particle(grid[i][j+1], particle, dt);
            solve_cell_and_particle(grid[i+1][j-1], particle, dt);
            solve_cell_and_particle(grid[i+1][j], particle, dt);
            solve_cell_and_particle(grid[i+1][j+1], particle, dt);
            grid[i][j].emplace_back(m);
            temp.emplace_back(make_pair(i, j));
        }

        for (auto &[i, j] : temp) if (!grid[i][j].empty()) grid[i][j].clear();
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
                particle.temperature += 0.2f * k * (30000 - particle.temperature) * dt;
        }
    }

    void update(float dt)
    {
        for (int i = 0; i < 4; i++)
        {
            apply_gravity();
            apply_bouyant_force();
            apply_constraint();
            solve_collisions(dt/4.f);
            update_position(dt/4.f);
        }
    }
};
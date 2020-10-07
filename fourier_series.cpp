// Author: Eviatar Mor, 07/10/2020
// https://en.wikipedia.org/wiki/Fourier_series

#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include <array>

#define _USE_MATH_DEFINES
#include <cmath>

#if !defined(__MINGW32__)
#include <random>
#else
#include <ctime>
#endif
// --------------------------------------------------------------------------

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// --------------------------------------------------------------------------

static constexpr unsigned int Width  = 800;
static constexpr unsigned int Height = 800;

static constexpr float Radius = 100.f;
static constexpr unsigned int Arms = 1;
static constexpr float Speed = 20.f;
// --------------------------------------------------------------------------

// Simply generate a random number between min and max.
template<typename T>
static T random(T min, T max) 
{
    // There is a problem with the MinGW compiler.
#if !defined(__MINGW32__)
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(min, max);
	return static_cast<T>(dis(gen));
#else
    float scale = rand() / static_cast<float>(RAND_MAX);
    return min + scale * (max - min);
#endif
}
// --------------------------------------------------------------------------

static inline double rad_to_deg(double radians) {
    return radians * (180.0 / M_PI);
}
static inline double deg_to_rad(double degrees) {
    return degrees * (M_PI / 180.0);
}
// --------------------------------------------------------------------------

struct Circle
{
public:
    Circle(const float radius)
    {
        circle.setRadius(radius);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineColor(sf::Color(random(0, 255), random(0, 255), random(0, 255)));
        circle.setOutlineThickness(3);

        segment.setPosition(circle.getPosition());
        segment.setFillColor(circle.getOutlineColor());
        segment.setSize(sf::Vector2f(radius, 3.f));
    }

    // Angle in radians
    void rotate(const float angle)
    {
        circle.rotate(rad_to_deg(angle));
        segment.rotate(rad_to_deg(angle));
    }

    void set_position(const sf::Vector2f vec)
    {
        circle.setPosition(vec);
        segment.setPosition(vec);
    }

    sf::Vector2f get_end_point() const 
    {
        const sf::Vector2f p1 = segment.getPosition();

        float theta = deg_to_rad(circle.getRotation());

        // Finding the point from the segment on the circle
        sf::Vector2f vec;
        vec.x = p1.x + circle.getRadius() * cosf(theta);
        vec.y = p1.y + circle.getRadius() * sinf(theta);

        return vec;
    }

    void draw_on(sf::RenderWindow& window)
    {
        window.draw(circle);
        window.draw(segment);
    }

private:
    sf::CircleShape circle;
    sf::RectangleShape segment;

    friend class CircleArms;
};
// --------------------------------------------------------------------------

class CircleArms
{
public: 
    CircleArms(unsigned int arms) 
    {
        if(arms >= 1)
        {
            Circle circle(Radius);
            circle.set_position(sf::Vector2f((Width / 5) + (Radius / 2), Height / 2));
            circles.push_back(circle);

            // Setting up the next circle
            for(unsigned int i = 1; i < arms; i++)
            {
                const auto& last_circle = circles[i - 1];

                float radius = last_circle.circle.getRadius() / 2;
                
                // if(i == 1)
                //     radius /= 1.5;
                
                Circle circle(radius);
                circle.set_position(sf::Vector2f(
                    last_circle.segment.getPosition().x + last_circle.segment.getSize().x,
                    last_circle.segment.getPosition().y + (last_circle.segment.getSize().y / 2)));

                circles.push_back(circle);
            }

            segment[0].color = circles.back().circle.getOutlineColor();
            segment[1].color = circles.back().circle.getOutlineColor();
        }
    }

    void update(const sf::Time& dt)
    {
        if(!circles.empty()) 
        {
            float angle = 1.f;
            float multi = 1.f;
            for(unsigned int i = 0; i < circles.size(); i++)
            {
                auto& circle = circles[i];

                // Nothing special can be any number
                // circle.rotate((4 / M_PI) * sinf(deg_to_rad(angle) * dt.asSeconds()));
                circle.rotate(sinf(angle * dt.asSeconds()));

                if(i >= 1)
                {
                    const auto& last_circle = circles[i - 1];
                    circle.set_position(last_circle.get_end_point());
                }

                angle += 20.f;
                multi += 2.f;
            }

            segment[0].position = sf::Vector2f(circles.back().get_end_point());
            segment[1].position = sf::Vector2f(XStartOfWave, circles.back().get_end_point().y);

            points.push_back(segment[1]);
            for(size_t i = 0; i < points.size(); i++)
            {
                auto& p = points[i];

                // Moving the segment
                p.position.x += Speed * dt.asSeconds();

                // Delete points when out of screen
                if(p.position.x >= Width)
                    points.erase(points.begin() + i);
            }
        }
    }

    void draw_on(sf::RenderWindow& window)
    {
        for(auto& circle : circles)
            circle.draw_on(window);

        window.draw(segment.data(), 2, sf::Lines);
        window.draw(points.data(), points.size(), sf::LinesStrip);
    }

private:
    std::vector<Circle> circles;
    std::vector<sf::Vertex> points;
    std::array<sf::Vertex, 2> segment;

    static constexpr float XStartOfWave = Width / 2.f;
};
// --------------------------------------------------------------------------

int main()
{
#ifdef __MINGW32__
    srand(time(nullptr));
#endif

    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;
    sf::RenderWindow window(sf::VideoMode(Width, Height), "Fourier Series Visualization", sf::Style::Default, settings);

    CircleArms circlelist(Arms);

    sf::Clock clock;
    while(window.isOpen())
    {
        sf::Time dt = clock.restart();

        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        
        circlelist.update(dt);
        circlelist.draw_on(window);

        window.display();
    }
}
 

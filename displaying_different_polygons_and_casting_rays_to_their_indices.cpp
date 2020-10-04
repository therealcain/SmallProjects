// Author: Eviatar Mor, 10/01/2020

#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <memory>
#include <cmath>
// ------------------------------------------------------------------------

constexpr float Width  = 800;
constexpr float Height = 800;
// ------------------------------------------------------------------------

// Converting from degrees to radians
template<typename T>
static constexpr float toRad(T deg) { return deg * (3.1459 / 180); }
// ------------------------------------------------------------------------

// Computing the distance between two points
static inline float distance(const sf::Vector2f p1, const sf::Vector2f p2) {
    return sqrtf(powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2));
}
// ------------------------------------------------------------------------

// Simply generate a random number between min and max.
template<typename T>
static T random(T min, T max) 
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(min, max);

	return static_cast<T>(dis(gen));
}
// ------------------------------------------------------------------------

// Converting a vector from one type to another.
// For example: Vector2<float> ---> Vector2<unsigned int>
template<typename T, typename U>
static inline sf::Vector2<T> vector_cast(const sf::Vector2<U> vec) {
    return sf::Vector2<T>(static_cast<T>(vec.x), static_cast<T>(vec.y));
}
// ------------------------------------------------------------------------

// A ray made up of two vectors aka points.
struct Ray { sf::Vector2f p1, p2; };
// ------------------------------------------------------------------------

// Rotating the ray by it's first point.
static inline Ray rotateRay(Ray ray, float angle) 
{
    // Rotation Matrix:
    // x' = x * cos(θ) - y * sin(θ)
    // y' = x * sin(θ) + y * cos(θ)
    ray.p2.x = ((ray.p2.x - ray.p1.x) * cosf(angle) - (ray.p2.y - ray.p1.y) * sinf(angle)) + ray.p1.x;
    ray.p2.y = ((ray.p2.x - ray.p1.x) * sinf(angle) + (ray.p2.y - ray.p1.y) * cosf(angle)) + ray.p1.y;

    return ray;
}
// ------------------------------------------------------------------------

// Setting a new length for the ray
static inline void setRayLength(Ray& ray, unsigned int length)
{
    const float dist = distance(ray.p1, ray.p2);
    const float alpha = atan2f(ray.p2.y - ray.p1.y, ray.p2.x - ray.p1.x);

    ray.p2.x = ray.p1.x + length * cos(alpha);
    ray.p2.y = ray.p1.y + length * sin(alpha);
}
// ------------------------------------------------------------------------

// Get point of intersection
// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
static sf::Vector2f pointIntersection(const sf::Vector2f A, const sf::Vector2f B, const sf::Vector2f C, const sf::Vector2f D)
{
    // First Line
    const float a1 = B.y - A.y;
    const float b1 = A.x - B.x;
    const float c1 = a1 * (A.x) + b1 * (A.y);

    // Second Line
    const float a2 = D.y - C.y;
    const float b2 = C.x - D.x;
    const float c2 = a2 * (C.x) + b2 * (C.y);

    // Determinant 
    const float det = a1 * b2 - a2 * b1;

    float x, y;

    if(det == 0.f)
    {
        // Parallel
    }
    else 
    {
        // Point of touch
        x = (b2 * c1 - b1 * c2) / det;
        y = (a1 * c2 - a2 * c1) / det;
    }

    return sf::Vector2f(x, y);
}

// Check if two lines are intersecting
static inline bool intersection(const sf::Vector2f A, const sf::Vector2f B, const sf::Vector2f C, const sf::Vector2f D)
{
    // Determinant
    auto on_segment = [](const sf::Vector2f A, const sf::Vector2f B, const sf::Vector2f C) {
        return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
    };  

    return on_segment(A,C,D) != on_segment(B,C,D) and on_segment(A,B,C) != on_segment(A,B,D);
}
// ------------------------------------------------------------------------

// Creates a shape with random points and size at the mouse position
static sf::CircleShape createShape(const sf::Vector2i mousepos)
{
    sf::CircleShape shape;

    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(sf::Color::Black);
    shape.setOutlineThickness(1);
    shape.setPointCount(random(3, 10));
    shape.setRadius(random(30, 120));
    shape.setOrigin(sf::Vector2f(shape.getRadius(), shape.getRadius()));
    shape.setPosition(vector_cast<float>(mousepos));

    return shape;
}
// ------------------------------------------------------------------------

int main()
{
    // Creates a window with AA
    sf::ContextSettings settings;
    settings.antialiasingLevel = 32;
    sf::RenderWindow window(sf::VideoMode(Width, Height), "Raycast 2D Test", sf::Style::Default, settings);

    // The shapes that will appear on the screen
    std::vector<sf::CircleShape> shapes;

    while(window.isOpen())
    {
        const auto MousePos = sf::Mouse::getPosition(window);

        // Events handling
        sf::Event event;
        while(window.pollEvent(event)) 
        {
            // When exit button pressed then actually exit.
            if(event.type == sf::Event::Closed)
                window.close();

            // When mouse pressed create a shape
            if(event.type == sf::Event::MouseButtonPressed)
                shapes.push_back(createShape(MousePos));
        }

        // All of the rays that have been projected every frame
        std::vector<Ray> rays;

        // Creates rays for the window coordinates
        Ray windowPoints[4];
        // Top Left Corner
        windowPoints[0].p1 = vector_cast<float>(MousePos);
        windowPoints[0].p2 = sf::Vector2f(0, 0);
        // Top Right Corner
        windowPoints[1].p1 = vector_cast<float>(MousePos);
        windowPoints[1].p2 = sf::Vector2f(Width, 0);
        // Bottom Left Corner
        windowPoints[2].p1 = vector_cast<float>(MousePos);
        windowPoints[2].p2 = sf::Vector2f(0, Height);
        // Bottom Right Corner
        windowPoints[3].p1 = vector_cast<float>(MousePos);
        windowPoints[3].p2 = sf::Vector2f(Width, Height);

        // Move all of the rays to the vector
        rays.push_back(std::move(windowPoints[0]));
        rays.push_back(std::move(windowPoints[1]));
        rays.push_back(std::move(windowPoints[2]));
        rays.push_back(std::move(windowPoints[3]));

        // Looping over all of the points in all shapes
        for(auto& shape : shapes)
        {
            for(size_t i = 0; i < shape.getPointCount(); i++)
            {
                Ray ray;

                // Starting position of the Ray is where the mouse is
                ray.p1 = vector_cast<float>(MousePos);

                // Ending position of the ray is where the shapes point are
                const auto point = shape.getTransform().transformPoint(shape.getPoint(i));
                ray.p2 = point;

                rays.push_back(ray);
            }
        }

        // Looping over all of the points in all shapes to check for intersection
        for(auto& shape : shapes)
        {
            for(size_t i = 0; i < shape.getPointCount(); i++)
            {
                // Current Point
                const auto current = shape.getTransform().transformPoint(shape.getPoint(i));

                // Next Point
                const auto next = shape.getTransform().transformPoint(shape.getPoint((i + 1) % shape.getPointCount()));

                for(auto& ray : rays)
                {
                    // Check intersection between two lines
                    if(intersection(current, next, ray.p1, ray.p2))
                    {
                        // Get the point of intersection
                        const auto point = pointIntersection(current, next, ray.p1, ray.p2);

                        // Calculates the line length to the point
                        const float dist = distance(point, ray.p1);

                        // Update the length to point of intersection
                        setRayLength(ray, dist);
                    }     
                }
            }
        }

        // Clears GPU Buffer
        window.clear(sf::Color::White);

        // Shape Draws
        for(auto& shape : shapes)
            window.draw(shape);

        // Ray Draws
        for(auto& ray : rays)
        {
            sf::Vertex vertex[2] { ray.p1, ray.p2 };
            vertex->color = sf::Color::Red;
            window.draw(vertex, 2, sf::Lines);
        }

        // Swap GPU Buffers
        window.display();
    }
}

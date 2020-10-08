// Author: Eviatar Mor, 08/10

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
// -----------------------------------------------------------------------------

static constexpr unsigned int Width  = 800;
static constexpr unsigned int Height = 800;

static constexpr float Gravity = 9.81;
// -----------------------------------------------------------------------------

static inline double distance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
    return sqrtf(powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2));
}
// -----------------------------------------------------------------------------

static inline double get_angle(const sf::Vector2f& p1, const sf::Vector2f& p2) {
    return atan2f(p2.y - p1.y, p2.x - p1.x);
}
// -----------------------------------------------------------------------------

static inline void set_line_length(const sf::Vector2f& p1, sf::Vector2f& p2, unsigned int length)
{
    const float dist  = distance(p1, p2);
    const float alpha = get_angle(p1, p2);

    p2.x = p1.x + length * cosf(alpha);
    p2.y = p1.y + length * sinf(alpha);
}
// -----------------------------------------------------------------------------

class Bullet
{
public:
    Bullet(const sf::Vector2f position, const float velocity, const float angle)
        : m_velocity(velocity), m_angle(angle)
    {
        bullet.setRadius(Radius);
        bullet.setPosition(position);
        bullet.setFillColor(sf::Color::Blue);
    }

    void update(sf::RenderWindow& window, const sf::Time& dt)
    {
        const auto pos = bullet.getPosition();

        const float x = pos.x + m_velocity * cosf(m_angle);
        const float y = pos.y + m_velocity * sinf(m_angle);

        printf("x:%f y:%f\n", x, y);

        bullet.setPosition(x, y);

        window.draw(bullet);
    }

    auto get_position() const {
        return bullet.getPosition();
    }

private:
    sf::CircleShape bullet;
    float m_velocity, m_angle;

    static constexpr float Radius = 15;
};
// -----------------------------------------------------------------------------

class Player
{
public:
    Player()
    {
        shape.setFillColor(sf::Color::Black);
        shape.setSize(sf::Vector2f(40, 40));
        shape.setPosition(Width / 2, Height / 2);
        shape.setOrigin(shape.getSize().x / 2, shape.getSize().y / 2);

        acceleration.x = Accelerating;
        acceleration.y = Accelerating;
    }

    void update(sf::RenderWindow& window, const sf::Time& dt)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            velocity.x += acceleration.x;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            velocity.x -= acceleration.x;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            velocity.y -= acceleration.y;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            velocity.y += acceleration.y;

        shape.move(velocity.x * dt.asSeconds(), velocity.y * dt.asSeconds());

        const auto MousePos = sf::Mouse::getPosition(window);
        sf::Vertex aim[2];
        aim[0].color = sf::Color::Blue;
        aim[1].color = sf::Color::Blue;
        aim[0].position = shape.getPosition();
        aim[1].position = sf::Vector2f(MousePos.x, MousePos.y);

        set_line_length(aim[0].position, aim[1].position, RayLength);

        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            bullets.emplace_back(aim[0].position, 10, get_angle(aim[0].position, aim[1].position));

        for(size_t i = 0; i < bullets.size(); i++)
        {
            const auto pos = bullets[i].get_position();
            if(pos.x >= Width || pos.x <= 0.f || 
               pos.y >= Height || pos.y <= 0.f)
            {
                bullets.erase(bullets.begin() + i);
            }
            
            bullets[i].update(window, dt);
        }

        window.draw(shape);
        window.draw(aim, 2, sf::Lines);
    }

private:
    sf::RectangleShape shape;
    sf::Vector2f velocity, acceleration;
    std::vector<Bullet> bullets;

    static constexpr float Accelerating = 5.f;
    static constexpr unsigned int RayLength = 50;
};
// -----------------------------------------------------------------------------

int main()
{
    sf::RenderWindow window(sf::VideoMode(Width, Height), "Projectile Motion");
    window.setVerticalSyncEnabled(true);

    Player player;

    sf::Clock delta_clock;
    while(window.isOpen())
    {
        sf::Time dt = delta_clock.restart();

        sf::Event event;
        while(window.pollEvent(event))
        {
            // When window closed
            if(event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);
        player.update(window, dt);
        window.display();
    }
}

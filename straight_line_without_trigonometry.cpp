// Author: Eviatar Mor, 13/10/2020

#include <SFML/Graphics.hpp>
// -----------------------------------------------------

static constexpr unsigned int Width  = 800;
static constexpr unsigned int Height = 800;
// -----------------------------------------------------

inline static 
void to_cartesian(sf::Vector2f& scrn) 
{
    scrn.x = scrn.x - Width / 2;
    scrn.y = -scrn.y + Height / 2;
}

inline static 
void to_screen(sf::Vector2f& cart) 
{
    cart.x = cart.x + Width / 2;
    cart.y = -(cart.y + Height / 2);
}
// -----------------------------------------------------


static 
sf::Vector2f calc_length(const sf::Vector2i& mpos)
{
    const float MaxLength = Width * Height;
    
    sf::Vector2f endpos;
    endpos.x = mpos.x;
    endpos.y = mpos.y;
    
    to_cartesian(endpos);

    endpos.x *= MaxLength;
    endpos.y *= MaxLength;

    to_screen(endpos);

    return endpos;
}
// -----------------------------------------------------

int main()
{
    sf::RenderWindow window(sf::VideoMode(Width, Height), "Straight Line without Trigo");

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

       const auto MousePos = sf::Mouse::getPosition(window);

        sf::Vertex vert[] {
            sf::Vector2f(Width / 2, Height / 2),
            calc_length(MousePos)
        };

        window.draw(vert, 2, sf::Lines);

        window.display();
    }
}

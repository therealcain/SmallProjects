// Author: Eviatar Mor, 08/10/2000

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
// -------------------------------------------------------------------------------

static constexpr unsigned int Width  = 800;
static constexpr unsigned int Height = 800;
// -------------------------------------------------------------------------------

void continue_line(sf::Vector2f& vec) 
{
  // Find angle of mouse position
	const float theta = atan2f(vec.y - (Height / 2), vec.x - (Width / 2));
  
  // Setting a new length with the same direction
	vec.x += (Width * Height) * cosf(theta);
	vec.y += (Width * Height) * sinf(theta);
}
// -------------------------------------------------------------------------------

int main()
{
	sf::RenderWindow window(sf::VideoMode(Width, Height), "Straight Line");


	sf::Vertex line[2];
	line[0].color = sf::Color::White;
	line[1].color = sf::Color::White;

	line[0].position = sf::Vector2f(Width / 2, Height / 2);

	while(window.isOpen())
	{
		sf::Event event;
		while(window.pollEvent(event)) 
		{
			if(event.type == sf::Event::Closed)
				window.close();
		}

    // getting the current mouse position
		const auto MousePos = sf::Mouse::getPosition(window);
		
    // Finishing line
		sf::Vector2f end_pos(MousePos.x, MousePos.y);
		continue_line(end_pos);

		line[1].position = end_pos;

		window.clear();
		window.draw(line, 2, sf::Lines);
		window.display();
	}
}

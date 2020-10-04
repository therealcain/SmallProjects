// Author: Eviatar Mor, 10/04/2020
// The Mandelbrot set was invented in 1980 by the mathematician Benoit Mandelbrot.
// It's an example of a fractal in mathematics and can be explained by the formula:
//           2
// Z     =  Z  + c
//  n+1      n
// C and Z are complex numbers, n is zero or a positive integer.
// Wiki: https://en.wikipedia.org/wiki/Mandelbrot_set
// --------------------------------------------------------------

#include <SFML/Graphics.hpp>

#include <iostream> 
#include <complex>
// --------------------------------------------------------------

constexpr unsigned int Width  = 800;
constexpr unsigned int Height = 800;

// Max Iterations to allow for the mandelbrot set
constexpr unsigned int MaxIterations = 1000;
// --------------------------------------------------------------

// Mapping a numeric range onto another numeric range.
template<typename T>
static inline T numeric_map(const T value, const T in_min, const T in_max, const T out_min, const T out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
// --------------------------------------------------------------

static sf::Color pick_color(unsigned int iter)
{
    switch(iter)
    {
        case 0: // brown 3
            return sf::Color(66, 30, 15);
        case 1: // dark violett
            return sf::Color(25, 7, 26);
        case 2: // darkest blue
            return sf::Color(9, 1, 47);
        case 3: // blue 5
            return sf::Color(4, 4, 73);
        case 4: // blue 4
            return sf::Color(0, 7, 73);
        case 5: // blue 3
            return sf::Color(12, 44, 138);
        case 6: // blue 2
            return sf::Color(24, 82, 177);
        case 7: // blue 1
            return sf::Color(57, 125, 209);
        case 8: // blue 0
            return sf::Color(134, 181, 229);
        case 9: // lightest blue
            return sf::Color(211, 236, 248);
        case 10: // lightest yellow
            return sf::Color(241, 233, 191);
        case 11: // light yellow
            return sf::Color(248, 201, 95);
        case 12: // dirty yellow
            return sf::Color(255, 170, 0);
        case 13: // brown 0
            return sf::Color(204, 128, 0);
        case 14: // brown 1
            return sf::Color(153, 87, 0);
        case 15: // brown 2
            return sf::Color(106, 52, 3);
    };

    return sf::Color::Black;
}
// --------------------------------------------------------------

static sf::Image mandelbrot_set(double min, double max)
{
    sf::Image image;
    image.create(Width, Height, sf::Color::White);

    // Start of the mandelbrot set
    for(size_t y = 0; y < Height; y++)
    {
        for(size_t x = 0; x < Width; x++)
        {
            // Mandelbrot Zoom
            const double x_scaled = numeric_map(static_cast<double>(x), 0.0, static_cast<double>(Width), min, max);
            const double y_scaled = numeric_map(static_cast<double>(y), 0.0, static_cast<double>(Height), min, max);

            // The formula complex variables
            //           2
            // Z     =  Z  + c
            //  n+1      n
            std::complex<double> c(x_scaled, y_scaled);
            std::complex<double> z(0.0, 0.0);
            int n = 0;

            while(std::abs(z) < 2.0 && n < MaxIterations)
            {
                z = (z * z) + c;

                n++;
            }

            // Make sure the iterations are inbounds
            if(n < MaxIterations && n > 0)
                n %= 16;

            image.setPixel(x, y, pick_color(n));
        }
    }

    return image;
}
// --------------------------------------------------------------

int main()
{
    sf::RenderWindow window(sf::VideoMode(Width, Height), "Mandelbrot Set");

    // Making sf::Image drawable
    sf::Texture tex;
    tex.loadFromImage(mandelbrot_set(-2, 2));
    sf::Sprite output(tex);

    while(window.isOpen())
    {
        // Window Events Handling
        sf::Event event;
        while(window.pollEvent(event))
        {
            // Close window on exit button
            if(event.type == sf::Event::Closed)
                window.close(); 
        }

        window.clear();
        window.draw(output);
        window.display();
    }
}

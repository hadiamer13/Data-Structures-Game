#include <SFML/Graphics.hpp>

int main() {
    // Create a 200x200 window
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");

    // Create a green circle shape
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    // Main game/application loop
    while (window.isOpen()) {
        // Process system events
        sf::Event event;
        while (window.pollEvent(event)) {
            // "Close requested" event: we close the window
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Clear screen with a default color (black)
        window.clear();

        // Draw your green circle
        window.draw(shape);

        // Update the window display
        window.display();
    }

    return 0;
}

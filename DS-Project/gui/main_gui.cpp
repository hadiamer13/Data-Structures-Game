#include "contracts/Types.h"
#include "contracts/Event.h"
#include "contracts/ViewSnapshot.h"
#include <iostream>
#include <SFML/Graphics.hpp>

// Reason for error: if include file show error. go to Project > Properties > C/C++ > General > Additional dependencies and add $(ProjectDir) to path.
static void contractSmokeTest()
{
    ViewSnapshot snap;
    snap.kind = TreeKind::BST;
    snap.rootId = 0;
    snap.nodes.resize(3);

    // Node 0 = root (50), left = 1 (30), right = 2 (70)
    snap.nodes[0].id = 0; snap.nodes[0].key = 50;
    snap.nodes[0].left = 1; snap.nodes[0].right = 2;

    snap.nodes[1].id = 1; snap.nodes[1].key = 30;
    snap.nodes[1].parent = 0;

    snap.nodes[2].id = 2; snap.nodes[2].key = 70;
    snap.nodes[2].parent = 0;

    VhEvent e;
    e.type = EventType::VISIT;
    e.id = 0;
    e.alarmCost = 1;

    VhPoint p(10.0f, 20.0f);

    std::cout << "Nodes: " << snap.nodes.size()
        << ", root key: " << snap.nodes[snap.rootId].key
        << ", event cost: " << e.alarmCost
        << ", point: " << p.x << "," << p.y << "\n";
}

int main() {
    contractSmokeTest();
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

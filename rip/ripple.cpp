#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(500, 500), "SFML works!");
    window.setFramerateLimit(60);

    sf::RenderTexture target1;
    target1.create(500, 500);

    sf::Shader shockwave;
    shockwave.loadFromFile("shockwave.glsl", sf::Shader::Fragment);

    sf::Texture tex;
    tex.loadFromFile("texture.png");
    sf::Sprite sprite;
    sprite.setTexture(tex);

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();

            if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape)
                window.close();
        }


        shockwave.setUniform("iMouse", sf::Vector2f(-1.f, -1.f));
        // if mouse button is pressed add new ripples
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
            if(mousePosition.x < 500 && mousePosition.y < 500)
            {
                sf::Vector2f mouse(mousePosition);

                mouse.x /= 500.f;
                mouse.y /= 500.f;

                mouse.y = 1 - mouse.y;


                std::cout << mouse.x << " " << mouse.y << std::endl;

                shockwave.setUniform("iMouse", mouse);
            }
        }

        shockwave.setUniform("iTime", clock.getElapsedTime().asSeconds());

        shockwave.setUniform("buffer", sf::Shader::CurrentTexture);

        target1.clear(sf::Color::Red);
        sprite.setPosition(0, 0);
        target1.draw(sprite);
        sprite.move(100, 100);
        target1.draw(sprite);
        sprite.move(100, 100);
        target1.draw(sprite);
        sprite.move(100, 100);
        target1.draw(sprite);
        target1.display();

        window.clear();
        // window.draw(sf::Sprite(target1.getTexture()), &shockwave);
        window.draw(sf::Sprite(target1.getTexture()), &shockwave);
        window.display();

        // swap the buffers around, first becomes second, second becomes third and third becomes first
    }

    return 0;
}
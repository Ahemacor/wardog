#include <SFML/Graphics.hpp>
#include <tinyxml.h>
#include <tinystr.h>
#include <string>

struct WindowSettings
{
    WindowSettings() = default;

    WindowSettings(int w, int h, const std::string& name)
    : w(w)
    , h(h)
    , name(name)
    {}

    int w = 400;
    int h = 400;
    std::string name = "Untitled";
};



int main()
{
    WindowSettings windowSettings;

    TiXmlDocument doc("config\\window.xml");
    doc.LoadFile();
	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

    pElem = hDoc.FirstChildElement().Element();
    hRoot = TiXmlHandle(pElem);

	TiXmlElement* pWindowNode = hRoot.FirstChild("Windows").FirstChild().Element();
    windowSettings.name = pWindowNode->Attribute("name");
    pWindowNode->QueryIntAttribute("w", &windowSettings.w);
    pWindowNode->QueryIntAttribute("h", &windowSettings.h);

    sf::RenderWindow window(sf::VideoMode(windowSettings.w, windowSettings.h), windowSettings.name);
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}
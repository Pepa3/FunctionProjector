#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
/*
* TODO: show mouse coordinates and intersections with curves
*/
sol::state lua;
std::vector<std::function<double(double)>> f;
std::vector<sf::VertexArray> curve;
sf::VertexArray pts(sf::Lines, 8*8);
sf::Text text;
size_t resolution = 200;
float zoomY = 0.2f;
float zoomX = 10.f;

double nullf(double x){ return x; }

void reload(){
    std::ifstream file;
    file.open("function.lua", std::ios::in);
    std::string func;
    text.setString("");

    f.clear();
    while(!(file.eof())){
        std::getline(file, func);
        if(func == "")continue;
        text.setString(text.getString() + func + "\n");
        sol::protected_function_result pfr = lua.safe_script("return function(x) return " + func + " end", [](lua_State*, sol::protected_function_result pfr){
            sol::error err = pfr;
            text.setString(text.getString() + "\n" + "An error occurred: " + std::string(err.what()) + "\n");
            return pfr;
        });
        if(!pfr.valid()){ continue; }

        sol::protected_function_result x = ((sol::protected_function) pfr)(0);

        if(!x.valid()){
            sol::error err = x;
            text.setString(text.getString() + "\n" + "An error occured: " + std::string(err.what()) + "\n");
        }else{
            f.push_back(pfr);
        }

    }
    file.close();

}

void appendX_Nth(float n){
    pts.append(sf::Vertex(sf::Vector2f(1600 / 2.f + n*(800 / zoomX), (900 / 2.f) + 10), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f(1600 / 2.f + n*(800 / zoomX), (900 / 2.f) - 10), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f(1600 / 2.f - n*(800 / zoomX), (900 / 2.f) + 10), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f(1600 / 2.f - n*(800 / zoomX), (900 / 2.f) - 10), sf::Color::White));
}
void appendY_Nth(float n){
    pts.append(sf::Vertex(sf::Vector2f((1600 / 2.f) + 10, 900 / 2.f + n*(450 * zoomY)), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f((1600 / 2.f) - 10, 900 / 2.f + n*(450 * zoomY)), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f((1600 / 2.f) + 10, 900 / 2.f - n*(450 * zoomY)), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f((1600 / 2.f) - 10, 900 / 2.f - n*(450 * zoomY)), sf::Color::White));
}

void redraw(){
    curve.clear();
    for(auto& fun : f){
        curve.push_back(sf::VertexArray(sf::LineStrip, resolution));
        for(size_t x = 0; x <= resolution; x++){
            double norm = (double) x / resolution;      //   0..1
            double real = (norm - 0.5) * 2;             //  -1..1
            double r_result = fun(real * zoomX) * zoomY;      //f(-1..1), scaled by zoomX => -1..1, scaled by zoomY
            double n_result = (r_result + 1) / 2;       //f(-1..1) =>  0..1

            curve.back().append(sf::Vertex(sf::Vector2f(
                (float) x / resolution * 1600,
                900 - (float) n_result * 900),
                sf::Color::Green));
        }
    }
    pts.clear();
    for(float i = 1; i <= zoomX; i++){
        appendX_Nth(i);
    }
    for(float i = 1; i <= 1/zoomY; i++){
        appendY_Nth(i);
    }
}

int main(int argc, char* argv[]){
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math);
    sf::Font font;
    if(!font.loadFromFile("arial.ttf")){
        std::cerr << "Could not load font 'arial.ttf'" << std::endl;
        exit(-2);
    }
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Plotter");
    text = sf::Text("",font,20);
    text.setFillColor(sf::Color::White);
    text.setPosition(0,0);
    reload();
    redraw();
    sf::VertexArray triangle(sf::LineStrip, resolution);
    sf::VertexArray axis(sf::Lines, 4);
    axis.append(sf::Vertex(sf::Vector2f(0, 900/2), sf::Color::White));
    axis.append(sf::Vertex(sf::Vector2f(1600, 900 / 2), sf::Color::White));
    axis.append(sf::Vertex(sf::Vector2f(1600/2, 0), sf::Color::White));
    axis.append(sf::Vertex(sf::Vector2f(1600/2, 900), sf::Color::White));

    while(window.isOpen()){
        sf::Event event;
        while(window.pollEvent(event)){
            if(event.type == sf::Event::Closed){
                window.close();
            }else if(event.type == sf::Event::KeyPressed){
                char key = event.key.code;
                if(key == sf::Keyboard::R) {
                    reload();
                    redraw();
                }else if(key == sf::Keyboard::Left){
                    zoomX *= 2;
                    redraw();
                }else if(key == sf::Keyboard::Right){
                    zoomX /= 2;
                    redraw();
                }else if(key == sf::Keyboard::Up){
                    zoomY *= 2;
                    redraw();
                }else if(key == sf::Keyboard::Down){
                    zoomY /= 2;
                    redraw();
                }else if(key == sf::Keyboard::Add){
                    resolution *= 2;
                    redraw();
                }else if(key == sf::Keyboard::Subtract){
                    resolution /= 2;
                    redraw();
                } else if(key == sf::Keyboard::Q){
                    window.close();
                }
            }
        }

        window.clear(sf::Color::Black);
        window.draw(axis);
        window.draw(text);
        for(auto& curv : curve){
            window.draw(curv);
        }
        window.draw(pts);
        window.display();
    }

    return 0;
}
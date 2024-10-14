#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
/*
* TODO: show mouse intersections with curves
*/
sol::state lua;
std::vector<std::function<double(double)>> f;
std::vector<sf::VertexArray> curve;
sf::VertexArray pts(sf::Lines, 8*8);
sf::VertexArray mouseAxis(sf::Lines, 4);
sf::VertexArray axis(sf::Lines, 4);
sf::Text text;
sf::Text mousePos;
float wwidth = 1000, wheight = 600;
size_t resolution = 200;//number of curve vertex points
float zoomY = 3.f;
float zoomX = 6.f;
int mx = 800, my = 450;

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
    pts.append(sf::Vertex(sf::Vector2f(wwidth / 2.f + n*(wwidth/2 / zoomX), (wheight / 2.f) + 10), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f(wwidth / 2.f + n*(wwidth/2 / zoomX), (wheight / 2.f) - 10), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f(wwidth / 2.f - n*(wwidth/2 / zoomX), (wheight / 2.f) + 10), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f(wwidth / 2.f - n*(wwidth/2 / zoomX), (wheight / 2.f) - 10), sf::Color::White));
}
void appendY_Nth(float n){
    pts.append(sf::Vertex(sf::Vector2f((wwidth / 2.f) + 10, wheight / 2.f + n*(wheight/2 / zoomY)), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f((wwidth / 2.f) - 10, wheight / 2.f + n*(wheight/2 / zoomY)), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f((wwidth / 2.f) + 10, wheight / 2.f - n*(wheight/2 / zoomY)), sf::Color::White));
    pts.append(sf::Vertex(sf::Vector2f((wwidth / 2.f) - 10, wheight / 2.f - n*(wheight/2 / zoomY)), sf::Color::White));
}

void redraw(){
    curve.clear();
    for(auto& fun : f){
        curve.push_back(sf::VertexArray(sf::LineStrip, resolution));
        for(size_t x = 0; x <= resolution; x++){
            double norm = (double) x / resolution;      //   0..1
            double real = (norm - 0.5) * 2;             //  -1..1
            double r_result = fun(real * zoomX) / zoomY;      //f(-1..1), scaled by zoomX => -1..1, scaled by zoomY
            double n_result = (r_result + 1) / 2;       //f(-1..1) =>  0..1

            curve.back().append(sf::Vertex(sf::Vector2f(
                (float) x / resolution * wwidth,
                wheight - (float) n_result * wheight),
                sf::Color::Green));
        }
    }
    pts.clear();
    for(float i = 1; i <= zoomX; i++){
        appendX_Nth(i);
    }
    for(float i = 1; i <= zoomY; i++){
        appendY_Nth(i);
    }
    mouseAxis[0].position.x = wwidth/2;
    mouseAxis[2].position.y = wheight/2;
    axis[0].position.y = wheight/2;
    axis[1].position.y = wheight/2;
    axis[1].position.x = wwidth;
    axis[2].position.x = wwidth/2;
    axis[3].position.x = wwidth/2;
    axis[3].position.y = wheight;
}

int main(int argc, char* argv[]){
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math);
    lua.script("sin,cos,tg,pi,pow,abs = math.sin, math.cos, math.tan, math.pi, math.pow, math.abs");
    //lua.script("sin = function(d) return math.sin(math.rad(d)) end");
    sf::Font font;
    if(!font.loadFromFile("font.ttf")){
        std::cerr << "Could not load font 'font.ttf'" << std::endl;
        exit(-2);
    }
    sf::RenderWindow window(sf::VideoMode(wwidth, wheight), "Plotter");
    text = sf::Text("",font,20);
    text.setFillColor(sf::Color::White);
    text.setPosition(0,0);
    mousePos = sf::Text("", font, 20);
    mousePos.setFillColor(sf::Color::Red);
    mousePos.setPosition(0, 0);
    mouseAxis[0].position.x = wwidth / 2;
    mouseAxis[2].position.y = wheight / 2;
    reload();
    redraw();
    axis[0] = sf::Vertex(sf::Vector2f(0, wheight/2), sf::Color::White);
    axis[1] = sf::Vertex(sf::Vector2f(wwidth, wheight / 2), sf::Color::White);
    axis[2] = sf::Vertex(sf::Vector2f(wwidth/2, 0), sf::Color::White);
    axis[3] = sf::Vertex(sf::Vector2f(wwidth/2, wheight), sf::Color::White);

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
                    zoomY /= 2;
                    redraw();
                }else if(key == sf::Keyboard::Down){
                    zoomY *= 2;
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
            }else if(event.type == sf::Event::MouseMoved){
                int mx = event.mouseMove.x, my = event.mouseMove.y;
                mousePos.setPosition(mx+10, my-25);//TODO: Maybe not at the cursor but at some corner of the screen
                std::stringstream str;
		        str.precision(3);
		        str << (mx - wwidth / 2)/wwidth*zoomX*2 << " " << (my - wheight / 2)/wheight*zoomY*-2;
		        mousePos.setString(str.str());
		        //mousePos.setString(std::format("{:.3} {:.3}", (mx - wwidth / 2)/wwidth*zoomX*2, (my - wheight / 2)/wheight*zoomY*-2));
                mouseAxis[0].position.y = my;
                mouseAxis[1].position.y = my;
                mouseAxis[1].position.x = mx;
                mouseAxis[2].position.x = mx;
                mouseAxis[3].position.x = mx;
                mouseAxis[3].position.y = my;
            }else if(event.type == sf::Event::Resized){
		window.setView(sf::View(sf::FloatRect(0,0,event.size.width,event.size.height)));
		wwidth = event.size.width;
		wheight = event.size.height;
		redraw();
	    }
        }

        window.clear(sf::Color::Black);
        window.draw(axis);
        window.draw(text);
        for(auto& curv : curve){
            window.draw(curv);
        }
        window.draw(pts);
        window.draw(mouseAxis);
        window.draw(mousePos);
        window.display();
    }

    return 0;
}

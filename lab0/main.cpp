#include "stream_utils.hpp"

#include <iostream>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

int main() {
    int columns, rows;

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    columns = w.ws_col;
    rows = w.ws_row;
#endif

    StreamHandler sh(std::cout, rows, 0.5f);
    // StreamHandler sh(std::cout, columns, rows);

    auto size = sh.size();
    int sizex = size.first;
    int sizey = size.second;
    std::cout << sizex << " " << sizey << std::endl;

    
    float t = 0;
    float t1 = 0;
    float pi = 3.1415;
    float k = 1;
    while (1) {
        sh.clear();

        
        {
            // внешний треугольник
            float x0, y0, x1, y1, x2, y2;
            x0 = sizex/2 + k*std::cos(t)*sizex/2;
            y0 = sizey/2 + k*std::sin(t)*sizey/2;
            
            x1 = sizex/2 + k*std::cos(t + 2*pi/3)*sizex/2;
            y1 = sizey/2 + k*std::sin(t + 2*pi/3)*sizey/2;
            
            x2 = sizex/2 + k*std::cos(t + 2*pi*2/3)*sizex/2;
            y2 = sizey/2 + k*std::sin(t + 2*pi*2/3)*sizey/2;
            
            sh.draw_circle(x0, y0, 5, 'o');
            sh.draw_triangle(x0, y0, x1, y1, x2, y2, 'A');
        }
        {
            float x0, y0, x1, y1, x2, y2;

            x0 = sizex/2 + (3.0/2.0 - 1.0/2.0)/std::sqrt(4)*k*std::cos(-t1)*sizex/2;
            y0 = sizey/2 + (3.0/2.0 - 1.0/2.0)/std::sqrt(4)*k*std::sin(-t1)*sizey/2;
            
            x1 = sizex/2 + (3.0/2.0 - 1.0/2.0)/std::sqrt(4)*k*std::cos(-t1 + 2*pi/3)*sizex/2;
            y1 = sizey/2 + (3.0/2.0 - 1.0/2.0)/std::sqrt(4)*k*std::sin(-t1 + 2*pi/3)*sizey/2;
            
            x2 = sizex/2 + (3.0/2.0 - 1.0/2.0)/std::sqrt(4)*k*std::cos(-t1 + 2*pi*2/3)*sizex/2;
            y2 = sizey/2 + (3.0/2.0 - 1.0/2.0)/std::sqrt(4)*k*std::sin(-t1 + 2*pi*2/3)*sizey/2;

            sh.draw_triangle(x0, y0, x1, y1, x2, y2, 'B');
        }
        
        
        sh.draw_line(0, 0, 0, sizey, '@');
        sh.draw_line(0, 0, sizex, 0, '@');
        sh.draw_line(sizex, 0, sizex, sizey, '@');
        sh.draw_line(0, sizey, sizex, sizey, '@');

        sh.flush_ansi();
        // std::cout << sizex << " " << sizey << std::endl;
        usleep(100000);
        t += 0.15;
        t1 += 0.4;
    }

    return 0;
}
#include <functional>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#define min(a, b) (a)>(b) ? (b) : (a) 
#define max(a, b) (a)<(b) ? (b) : (a) 

class StreamHandler {
    int width, height;
    float Wh_koef = 1;
    std::reference_wrapper< std::ostream > stream;
    std::vector< std::string > buffer; 
public:
    StreamHandler(std::ostream& stream, int width, int height): width(width), height(height), stream(stream), buffer(height, std::string(width, ' ')) {
        clear(' ');
    }

    StreamHandler(std::ostream& stream, int height, float CharWidthOverHeight_koef = 1): stream(stream) {
        this->height = height;
        this->Wh_koef = CharWidthOverHeight_koef;
        this->width = (int) std::roundf((float)height / Wh_koef);

        buffer = std::vector(height, std::string(width, ' '));

        clear(' ');
    }

    ~StreamHandler() {}

    std::pair<int,int> size() const { return {width, height}; }
    float koef() const { return Wh_koef; }
    
    void clear(char color = ' ') {
        for (auto& str: buffer) {
            for (auto& c: str) {
                c = color;
            }
        }
    }

    void flush() {
        for (auto& str: buffer) {
            stream.get() << str << std::endl;
        }
    }
    void flush_ansi() {
        stream.get() << "\x1b[3" << std::flush;
        for (auto& str: buffer) {
            stream.get() << str << std::endl;
        }
    }

    char& at(int x, int y) {
        // std::cout << "Write(" << x << "," << y << ")" << std::endl;
        return buffer[y][x];
    }
    const char& at(int x, int y) const {
        return buffer[y][x];
    }

    void put(float xf, float yf, char color) {
        int x = std::clamp(std::round(xf), 0.0f, (float)width-1);
        int y = std::clamp(std::round(yf), 0.0f, (float)height-1);
        at(x,y) = color;
    }

    void draw_line(float x0, float y0, float x1, float y1, char color, bool flipped=false) {
        if (!flipped) {
            x0 = std::clamp(x0, 0.0f, (float)width-1);
            x1 = std::clamp(x1, 0.0f, (float)width-1);
            y0 = std::clamp(y0, 0.0f, (float)height-1);
            y1 = std::clamp(y1, 0.0f, (float)height-1);
        }

        if (x0 > x1) {
            float temp;

            temp = x0; x0 = x1; x1 = temp;
            temp = y0; y0 = y1; y1 = temp;
        }

        int xfrom = x0;
        int xto = x1;
        int yfrom = y0;
        int yto = y1;
        
        // std::cout << "Line from " << x0 << " " << y0 << " to " << x1 << " " << y1 << std::endl;
        
        int diry = y1-y0;
        if (diry > 0) diry = 1;
        else if (diry < 0) diry = -1;
        
        int dirx = 1;
        // int dirx = x1-x0;
        // if (dirx > 0) dirx = 1;
        // else if (dirx < 0) dirx = -1;
        
        if (x1 == x0) {
            // std::cout << "x0==x1" << std::endl;
            for (int y=yfrom; y != yto+diry; y += diry) {
                this->put(xfrom, y ,color);
            }
            return;
        }
        if (y1 == y0) {
            // std::cout << "y0==y1" << std::endl;
            for (int x=xfrom; x != xto+dirx; x += dirx) {
                this->put(x, yfrom, color);
            }
            return;
        }
        
        float dxf = x1 - x0;
        float dyf = y1 - y0;
        
        
        float steep = dyf/dxf;
        // std::cout << "steep = " << steep << std::endl;
        float error = 0;
        float de = std::abs(steep);
        
        if (!flipped && de > 1) {
            // std::cout << "flip" << std::endl;
            draw_line(y0, x0, y1, x1, color, true);
            return;
        }

        
        int y = y0;
        float yf = y0;

        for (int x=xfrom; x!= xto+dirx; x += dirx) {
            if (flipped) this->put(y,x,color);
            else this->put(x,y,color);
            error = error + de;
            while (error > 1) {
                yf += diry;
                error -= 1;
            }
            y = std::roundf(yf);
        }
    }


    void draw_triangle(
        float x0, float y0,
        float x1, float y1,
        float x2, float y2,
        char color
    ) {
        draw_line(x0, y0, x1, y1, color);
        draw_line(x1, y1, x2, y2, color);
        draw_line(x2, y2, x0, y0, color);
    }

    void draw_circle(
        float x0, float y0,
        float R,
        char color
    ) {
        // std::cout << "Circle(" << x0 << "," << y0 << ",R=" << R << ")" << std::endl;

        float x = 0;
        float y = R;

        float delta = 1 - 2*R;
        float error = 0;

        while (y >= x) {
            this->put(x0 + x, y0 + y, color);
            this->put(x0 + x, y0 - y, color);
            this->put(x0 - x, y0 + y, color);
            this->put(x0 - x, y0 - y, color);
            this->put(x0 + y, y0 + x, color);
            this->put(x0 + y, y0 - x, color);
            this->put(x0 - y, y0 + x, color);
            this->put(x0 - y, y0 - x, color);

            error = 2 * (delta + y) - 1;

            if ((delta < 0) && (error <= 0)) {
                x += 1;
                delta += 2*x + 1;
                continue;
            }

            if ((delta > 0) && (error > 0)) {
                y -= 1;
                delta -= 2 * y + 1;
                continue;
            }

            x += 1; y -= 1;
            delta += 2 * (x - y);
        }
        
    }
};
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <algorithm>

#include <memory>

namespace GL {
    #include <GL/glew.h>
    #include <GL/freeglut.h>
    #include <GL/gl.h>
};

double drand() {
    return (double)rand()/(double)RAND_MAX;
}

struct Point {
    double x, y;
    Point() = default;
    Point(double x, double y): x(x), y(y) {}
    Point(const Point& other) = default;

    Point& operator=(const Point& other) = default;

    Point operator+(Point p) {
        return Point(x+p.x, y+p.y);
    }
    Point operator-(Point p) {
        return Point(x-p.x, y-p.y);
    }
    Point operator-() {
        return Point(-x, -y);
    }
    Point operator*(double k) {
        return Point(k*x, k*y);
    }
};

struct Color {
    double r, g, b;
    double a = 0;

    Color operator+(Color c) {
        return Color{r+c.r, g+c.g, b+c.b, a};
    }
    Color operator*(double k) {
        return Color{r*k, g*k, b*k, a};
    }
};

double FPS = 60.0f;
double delaytime = 1000000.0f/FPS;

void setFPS(double nFPS) {
    if (nFPS <= 0) return;
    FPS = nFPS;
    delaytime = 1000000/nFPS;
}

int WINX = 1200;
int WINY = 700;

GL::GLuint vertexBuffer = 0;
GL::GLuint colorBuffer = 0;
std::vector<GL::GLsizei> vertexCounts;
std::vector<Point> allPoints;
std::vector<Color> allColors;

void reshape(int width, int height) {
    GL::glutReshapeWindow(WINX, WINY);
}

void force_reshape(int width, int height) {
    GL::glutReshapeWindow(width, height);
    WINX = width; WINY = height;
}

// Функция для генерации вершин эллипса (TRIANGLE_FAN)
std::pair<std::vector<Point>, std::vector<Color>> 
draw_filled_ellipse(Point center, Point radii, Color color, int shapeness) {
    std::pair<std::vector<Point>, std::vector<Color>> result;
    
    double koef = (2 * M_PI) / shapeness;
    
    // Центр для TRIANGLE_FAN
    result.first.push_back(center);
    result.second.push_back(color);
    
    // Точки окружности
    for (int i = 0; i <= shapeness; i++) {
        double angle = i * koef;
        result.first.push_back(Point(
            center.x + cos(angle) * radii.x,
            center.y + sin(angle) * radii.y
        ));
        result.second.push_back(color);
    }
    
    return result;
}

// Функция для генерации вершин прямоугольника (TRIANGLE_FAN)
std::pair<std::vector<Point>, std::vector<Color>> 
draw_filled_rect(Point point, double width, double height, Color color) {
    std::pair<std::vector<Point>, std::vector<Color>> result;
    
    // Центр прямоугольника для TRIANGLE_FAN
    Point center(point.x + width/2, point.y + height/2);
    result.first.push_back(center);
    result.second.push_back(color);
    
    // Вершины прямоугольника по порядку
    result.first.push_back(Point(point.x, point.y));
    result.second.push_back(color);
    result.first.push_back(Point(point.x + width, point.y));
    result.second.push_back(color);
    result.first.push_back(Point(point.x + width, point.y + height));
    result.second.push_back(color);
    result.first.push_back(Point(point.x, point.y + height));
    result.second.push_back(color);
    result.first.push_back(Point(point.x, point.y)); // замыкаем
    result.second.push_back(color);
    
    return result;
}

// Функция для генерации вершин полигона (TRIANGLE_FAN)
std::pair<std::vector<Point>, std::vector<Color>> 
draw_filled_poly(std::vector<Point> &points, Color color) {
    std::pair<std::vector<Point>, std::vector<Color>> result;
    
    if (points.size() < 3) return result;
    
    // Вычисляем центр полигона для TRIANGLE_FAN
    Point center(0, 0);
    for (const auto& p : points) {
        center.x += p.x;
        center.y += p.y;
    }
    center.x /= points.size();
    center.y /= points.size();
    
    result.first.push_back(center);
    result.second.push_back(color);
    
    // Добавляем все точки полигона
    for (const auto& p : points) {
        result.first.push_back(p);
        result.second.push_back(color);
    }
    // Замыкаем первой точкой
    result.first.push_back(points[0]);
    result.second.push_back(color);
    
    return result;
}

struct State {
    double t;
    double dt;
    bool stop;
};

#define min(a, b) (a) > (b) ? (b) : (a)

struct DrawingContext {
    Point lefttop;
    Point size;

    Point transform(Point p) {
        return Point(lefttop.x + p.x*size.x, lefttop.y + p.y*size.y);
    }
    
    Point scale(Point p) {
        return Point(p.x*size.x, p.y*size.y);
    }

    double transform(double k) {
        return min(size.x, size.y)*k;
    }

    double transform_x(double k) {
        return size.x*k;
    }
    double transform_y(double k) {
        return size.y*k;
    }
};

class UpdatableObject {
public:
    virtual bool update(std::shared_ptr<State> state) { return true; }
};

class DrawableObject {
public:
    virtual std::pair<std::vector<Point>, std::vector<Color>> 
    draw(std::shared_ptr<DrawingContext> context) = 0;
};

class ComplexObject: public UpdatableObject, public DrawableObject {};

class Stage: public ComplexObject {
public:
    std::vector<std::shared_ptr<UpdatableObject>> updaties;
    std::vector<std::shared_ptr<DrawableObject>> drawies;

    void add_updatie(std::shared_ptr<UpdatableObject> obj) {
        updaties.push_back(obj);
    }

    void add_drawie(std::shared_ptr<DrawableObject> obj) {
        drawies.push_back(obj);
    }

    bool update(std::shared_ptr<State> state) override {
        std::vector<void*> to_remove;
        for (auto& obj: updaties) {
            if (!obj->update(state)) {
                to_remove.push_back(obj.get());
            }
        }
        for (void* ptr: to_remove) {
            auto it = std::find_if(updaties.begin(), updaties.end(),
                [ptr](const std::shared_ptr<UpdatableObject>& sp) {
                    return sp.get() == ptr;
                });
            if (it != updaties.end()) {
                updaties.erase(it);
            }
        }
        return true;
    }

    std::pair<std::vector<Point>, std::vector<Color>> 
    draw(std::shared_ptr<DrawingContext> context) override {
        std::pair<std::vector<Point>, std::vector<Color>> result;
        
        for (auto& obj: drawies) {
            auto current = obj->draw(context);

            // Сохраняем количество вершин каждого объекта для отдельной отрисовки
            vertexCounts.push_back(current.first.size());

            result.first.insert(
                result.first.end(),
                current.first.begin(),
                current.first.end()
            );

            result.second.insert(
                result.second.end(),
                current.second.begin(),
                current.second.end()
            );
        }
        
        return result;
    }
};

class Polygon: public ComplexObject {
public:
    std::vector<Point> points;
    Color color;

    Polygon() = default;
    Polygon(std::vector<Point> &points, Color color): points(points), color(color) {}

    std::pair<std::vector<Point>, std::vector<Color>> 
    draw(std::shared_ptr<DrawingContext> context) override {
        std::vector<Point> temp_points = points;
        for (auto &point: temp_points) {
            point = context->transform(point);
        }

        return draw_filled_poly(temp_points, color);
    }
};

class Rectangle: public ComplexObject {
public:
    Point lefttop;
    Point size;
    Color color;

    Rectangle() = default;
    Rectangle(Point lefttop, Point size, Color color): lefttop(lefttop), size(size), color(color) {}

    std::pair<std::vector<Point>, std::vector<Color>> 
    draw(std::shared_ptr<DrawingContext> context) override {
        return draw_filled_rect(
            context->transform(lefttop), 
            context->transform_x(size.x), 
            context->transform_y(size.y), 
            color
        );
    }
};

class Circle: public ComplexObject {
public:
    Point center;
    double radius;
    Color color;

    Circle() = default;
    Circle(Point center, double radius, Color color): center(center), radius(radius), color(color) {}

    std::pair<std::vector<Point>, std::vector<Color>> 
    draw(std::shared_ptr<DrawingContext> context) override {
        return draw_filled_ellipse(
            context->transform(center), 
            Point(context->transform_x(radius), context->transform_y(radius)), 
            color, 
            100
        );
    }
};

class Meteor: public ComplexObject {
public:
    std::shared_ptr<Circle> circle;
    Point direction;
    double speed;
    bool finished = false;
    
    Meteor(std::shared_ptr<Circle> circle, Point direction, double speed): 
        circle(circle), direction(direction), speed(speed) {}

    bool update(std::shared_ptr<State> state) override {
        if (finished) return false;

        circle->center = circle->center + direction * state->dt * speed;
        circle->radius *= 1 + 0.1*(drand()*2-1);
        if (circle->center.y + circle->radius < 0) {
            finished = true; 
            return false;
        }

        return true;
    }

    std::pair<std::vector<Point>, std::vector<Color>> 
    draw(std::shared_ptr<DrawingContext> context) override {
        return circle->draw(context);
    }
};

class RotatingAnimation: public ComplexObject {
public:
    std::shared_ptr<DrawableObject> object;

    Point center;
    double radius;
    double offset;
    double speed;
    
    RotatingAnimation(
        std::shared_ptr<DrawableObject> object,
        Point center, double radius,
        double offset,
        double speed
    ): object(object), center(center), radius(radius), offset(offset), speed(speed)  {}

    bool update(std::shared_ptr<State> state) override {
        offset += state->dt * speed;
        while (offset > 2*M_PI) offset -= 2*M_PI;
        return true;
    }

    std::pair<std::vector<Point>, std::vector<Color>> 
    draw(std::shared_ptr<DrawingContext> context) override {
        std::shared_ptr<DrawingContext> newCtx;
        newCtx.reset(new DrawingContext(*context));

        Point newLeftTop = context->lefttop + context->scale(center) + 
                          Point(std::cos(offset), std::sin(offset)) * context->transform(radius);

        newCtx->lefttop = newLeftTop;

        return object->draw(newCtx);
    }
};

template<class T>
class ColorChangingAnimation: public ComplexObject {
public:
    std::shared_ptr<T> object;

    Color color1;
    Color color2;
    double speed;
    double offset;
    double dir; // направление изменения цвета

    ColorChangingAnimation(std::shared_ptr<T> object, Color color1, Color color2, double speed, double offset=0.0): 
        object(object), color1(color1), color2(color2), speed(speed), offset(offset), dir(1.0) {}

    bool update(std::shared_ptr<State> state) override {
        offset += state->dt * speed * dir;
        if (offset >= 1.0) {
            offset = 1.0;
            dir = -1.0;
        }
        else if (offset <= 0.0) {
            offset = 0.0;
            dir = 1.0;
        }
        return true;
    }

    std::pair<std::vector<Point>, std::vector<Color>> 
    draw(std::shared_ptr<DrawingContext> context) override {
        Color color = color1*(1.0 - offset) + color2*offset; 
        object->color = color;
        return object->draw(context);
    }
};

std::shared_ptr<State> state;
std::shared_ptr<DrawingContext> context;
std::shared_ptr<Stage> main_stage;

void keyboardKeys(unsigned char key, int x, int y) {
    switch (key) {
        case ' ':
            if (state->stop) {
                state->dt = delaytime/1000000.0f;
                state->stop = false;
            }
            else {
                state->dt = 0;
                state->stop = true;
            }
            break;
        case 'M': case 'm': 
        {
            Point pos = Point(0.15 + drand()*0.7, 1 + drand()*0.2);
            Point direction = Point((drand()*2-1)*0.1, -0.1 - drand()*0.5);
            Color color = {0.8 + 0.2*drand(), 0.08*drand(), 0.08*drand(), 1};
            double speed = 7 + 3*drand();
            double radius = 0.01 + drand()*0.01;

            std::shared_ptr<Circle> circle(
                new Circle(pos, radius, color)
            );
            std::shared_ptr<Meteor> meteor(
                new Meteor(circle, direction, speed)
            );

            main_stage->add_updatie(meteor);
            main_stage->add_drawie(meteor);
        } break;
        default:
            break;
    }
}

void prepare() {
    state.reset(new State);
    context.reset(new DrawingContext);
    main_stage.reset(new Stage);

    state->dt = delaytime/1000000.0f;
    state->t = 0;
    
    double speed = 1.25;
    
    {// Background
        std::shared_ptr<Rectangle> back(
            new Rectangle(Point(0,0), Point(1,1), {0,0,0,1})
        );
        std::shared_ptr<ColorChangingAnimation<Rectangle>> anim(
            new ColorChangingAnimation<Rectangle>(
                back, 
                {0.55, 0.64, 1},
                {0.17, 0.196, 0.3},
                speed/M_PI
            )
        );

        main_stage->add_updatie(anim);
        main_stage->add_drawie(anim);
    }
    
    {// Sun & Moon
        std::shared_ptr<RotatingAnimation> sun(
            new RotatingAnimation( 
                std::shared_ptr<Circle>(new Circle(Point(0,0), 0.08, {0.75, 0.3, 0.4, 1})),
                Point(0.5, 0), 0.1, M_PI/4, -speed
            )
        );

        main_stage->add_updatie(sun);
        main_stage->add_drawie(sun);

        std::shared_ptr<RotatingAnimation> moon(
            new RotatingAnimation( 
                std::shared_ptr<Circle>(new Circle(Point(0,0), 0.08, {0.2, 0.3, 0.8, 1})),
                Point(0.5, 0), 0.1, M_PI/4 + M_PI, -speed
            )
        );

        main_stage->add_updatie(moon);
        main_stage->add_drawie(moon);
    }

    {// Ground
        std::shared_ptr<Circle> ground( 
            new Circle(Point(0.5, -0.75), 1.25, {0, 1, 0, 1})
        );
        std::shared_ptr<ColorChangingAnimation<Circle>> anim(
            new ColorChangingAnimation<Circle>(
                ground,
                {0.082, 0.7176, 0.086, 1},
                {0.05, 0.349, 0.05, 1},
                speed/M_PI
            )
        );

        main_stage->add_updatie(anim);
        main_stage->add_drawie(anim);
    }

    {// House
        std::shared_ptr<Rectangle> back(
            new Rectangle(Point(0.3,0.3), Point(0.08,0.15), {0,0,0,1})
        );

        std::shared_ptr<ColorChangingAnimation<Rectangle>> anim(
            new ColorChangingAnimation<Rectangle>(
                back, 
                {0.278, 0.04, 0.027},
                {0.121, 0.043, 0.035},
                speed/M_PI
            )
        );

        main_stage->add_updatie(anim);
        main_stage->add_drawie(anim);
    }

    {// House roof
        std::vector<Point> points = {Point(0.3, 0.45), Point(0.34, 0.55), Point(0.38, 0.45)};
        std::shared_ptr<Polygon> back(
            new Polygon(points, {0,0,0,1})
        );

        std::shared_ptr<ColorChangingAnimation<Polygon>> anim(
            new ColorChangingAnimation<Polygon>(
                back, 
                {0.807, 0.533, 0.101},
                {0.325, 0.196, 0},
                speed/M_PI
            )
        );

        main_stage->add_updatie(anim);
        main_stage->add_drawie(anim);
    }
}

void init_buffers() {
    GL::glGenBuffers(1, &vertexBuffer);
    GL::glGenBuffers(1, &colorBuffer);
}

void draw() {
    GL::glClear(GL_COLOR_BUFFER_BIT);

    context->lefttop = {0,0};
    context->size = {(double)WINX, (double)WINY};
    
    vertexCounts.clear();
    allPoints.clear();
    allColors.clear();
    
    auto buffers = main_stage->draw(context);
    allPoints = buffers.first;
    allColors = buffers.second;
    
    if (!allPoints.empty() && !allColors.empty()) {
        // Включаем массивы вершин и цветов
        GL::glEnableClientState(GL_VERTEX_ARRAY);
        GL::glEnableClientState(GL_COLOR_ARRAY);
        
        // Загружаем данные вершин
        GL::glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        GL::glBufferData(GL_ARRAY_BUFFER, 
                        allPoints.size() * sizeof(Point), 
                        allPoints.data(), 
                        GL_DYNAMIC_DRAW);
        GL::glVertexPointer(2, GL_DOUBLE, 0, 0);
        
        // Загружаем данные цветов
        GL::glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        GL::glBufferData(GL_ARRAY_BUFFER, 
                        allColors.size() * sizeof(Color), 
                        allColors.data(), 
                        GL_DYNAMIC_DRAW);
        GL::glColorPointer(4, GL_DOUBLE, 0, 0);
        
        // Рисуем каждый объект отдельно
        size_t offset = 0;
        for (GL::GLsizei count : vertexCounts) {
            GL::glDrawArrays(GL_TRIANGLE_FAN, offset, count);
            offset += count;
        }
        
        // Отключаем массивы
        GL::glDisableClientState(GL_VERTEX_ARRAY);
        GL::glDisableClientState(GL_COLOR_ARRAY);
        
        // Разбиндовываем буфер
        GL::glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GL::glutSwapBuffers();
    GL::glutPostRedisplay();
}

void update() {
    using namespace std::chrono;
    
    auto start = steady_clock::now();
    main_stage->update(state);
    draw();
    auto end = steady_clock::now();
    
    double dur = duration_cast<microseconds>(end-start).count();
    if (dur < delaytime)
        usleep(delaytime - dur);
}

int main(int argc, char** argv) {
    srand(time(0));
    
    // glut
    GL::glutInit(&argc, argv);
    GL::glutInitDisplayMode(GLUT_DOUBLE);
    GL::glutInitWindowSize(WINX, WINY);
    GL::glutInitWindowPosition(100, 100);
    GL::glutCreateWindow("Laba");
    
    // Инициализируем GLEW
    GL::glewInit();
    
    GL::glClearColor(1, 1, 1, 0);
    GL::glMatrixMode(GL_PROJECTION);
    GL::glLoadIdentity();
    GL::glOrtho(0, WINX, 0, WINY, 0, 1);
    
    // Инициализируем буферы
    init_buffers();
    
    // Подготавливаем сцену
    prepare();
    
    // Устанавливаем функции обратного вызова
    GL::glutDisplayFunc(update);
    GL::glutReshapeFunc(reshape);
    GL::glutKeyboardFunc(keyboardKeys);
    
    GL::glutMainLoop();

    return 0;
}
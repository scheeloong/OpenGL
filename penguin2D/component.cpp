#include "component.h"
#include <math.h>
#include <vector>

// Inline function 
// Convert the given value from degrees to radians.
inline float deg2rad(float degrees) { return degrees * M_PI / 180; }

// Do nothing for destructor 
Component::~Component() {}

// -------------------- Rect ---------------------

Rectangle::~Rectangle() {}

Rectangle::Rectangle(float x1, float y1, float x2, float y2)
  : x1_(x1), y1_(y1), x2_(x2), y2_(y2) 
{
  mode_ = GL_POLYGON;
}

// Position it around the origin, thus -width/2 and width/2
Rectangle::Rectangle(float width, float height)
  : x1_(-width/2), y1_(-height/2), x2_(width/2), y2_(height/2) 
{
  mode_ = GL_POLYGON;
}


void Rectangle::Update() 
{
  // Draw the rectangle
	glBegin(mode_); // set the mode (e.g.: GL_POLYGON) 
    glVertex2f(x1_, y1_);
    glVertex2f(x1_, y2_);
    glVertex2f(x2_, y2_);
    glVertex2f(x2_, y1_);
  glEnd();
}

// Set the mode 
void Rectangle::set_mode(GLenum mode) 
{
  mode_ = mode;
}

GLenum Rectangle::mode() const 
{
  return mode_;
}

// -------------------- Entity ---------------------

// use default constructor from components
Entity::Entity() : components_() {}

// Destroy the entire iterator 
Entity::~Entity() 
{
  std::vector<Component*>::iterator it;
  for (it = components_.begin(); it != components_.end(); it++)
    delete *it;
}

void Entity::AddComponent(Component *component) 
{
  components_.push_back(component);
}


void Entity::Update() 
{
  // Duplicate the top of current matrix 
  glPushMatrix();

  std::vector<Component*>::iterator it;
  // Note: Polymorphism occurs here 
  for (it = components_.begin(); it != components_.end(); it++) 
    (*it)->Update();

  // Remove the top of current matrix 
  glPopMatrix(); 
}

// -------------------- Joint ---------------------

Joint::~Joint() {}

Joint::Joint(float x, float y, float a, Component *component, bool mark)
  : x_(x), y_(y), a_(a), mark_(mark) 
{
  component_  = component;
}

float *Joint::a_ptr() 
{
  return &a_;
}

void Joint::set_a(float a) 
{
  a_ = a;
}

float *Joint::x_ptr() 
{
  return &x_;
}

float *Joint::y_ptr() 
{
  return &y_;
}

void Joint::set_y(float y) 
{
  y_ = y;
}


// Note: This function is not part of any class, it is sort of a global function 
// Creates and returns an entity that draws a joint marker at origin.
// The entity is actually created only the first time this is called and
// re-used in subsequent calls.
// Therefore, declare entity as static 
Entity *joint_marker() 
{
  static Entity circle; // create a new Entity called circle  
  static bool flag = false;
  if (!flag) 
  {
    Circle *c = new Circle(0, 0, 2); // Create a new circle with radius 2 centered at orign 
    c->set_mode(GL_LINE_LOOP);       // Set line as loop (not filled) 
    c->set_smoothness(12);					 // Set smoothness as 12
    circle.AddComponent(new Color(0.2, 0.2, 0.2));
    circle.AddComponent(c);  // Set the color, and add the Circle Component to it 
    flag = true; // only call this circle settling statement once 
  }
  return &circle;
}

void Joint::Update() 
{
  // Duplicate top matrix 
  glPushMatrix();
    // Translate the matrix by X and Y 
    glTranslatef(x_, y_, 0);

    // Duplicate top matrix 
    glPushMatrix();
      // We only want the translation to be preserved 
      // after the component is done drawing itself.
        
      glRotatef(a_, 0, 0, 1);
      component_->Update(); // draw the resulting matrix 
    glPopMatrix(); // remove the rotated matrix 

  // If this joint is attached to a component,
  if (mark_) 
  {
    // Push the bits that contains color data 
    glPushAttrib(GL_COLOR_BUFFER_BIT);
      joint_marker()->Update(); // Draw the circle that was returned by calling its update function 
    glPopAttrib(); // Remove bits that contains color data 
  }

  // Remove translated matrix 
  glPopMatrix();
}

// -------------------- Color ---------------------

Color::~Color() {}

Color::Color(float r, float g, float b, float a)
  : r_(r), g_(g), b_(b), a_(a) {}

void Color::Update() 
{
  glColor4f(r_, g_, b_, a_);
}

// -------------------- Circle ---------------------

Circle::~Circle() {}

Circle::Circle(float x, float y, float r)
  : x_(x), y_(y), r_(r), smoothness_(90), mode_(GL_POLYGON) {}

int Circle::smoothness() const 
{
  return smoothness_;
}

void Circle::set_smoothness(int smoothness) 
{
  smoothness_ = smoothness;
}

void Circle::set_mode(GLenum mode) 
{
  mode_ = mode;
}

GLenum Circle::mode() const 
{
  return mode_;
}

void Circle::Update() 
{
  // Circle is basically a polygon but with close to infinitely many points 
  float step = 360.0f / static_cast<float>(smoothness_);
  glBegin(mode_);
    for (float i = 0; i < 360.0f; i += step)
      glVertex2f(x_ + r_ * cos(deg2rad(i))         ,            y_ + r_ * sin(deg2rad(i)));
  glEnd();
}


// -------------------- Zoomer ---------------------

Zoomer::~Zoomer() {}

Zoomer::Zoomer(float f) : f_(f) {}

void Zoomer::Update() 
{
  // Scale x and y by same amount, don't scale z => scale z by factor of 1 
  glScalef(f_, f_, 1);
}

float *Zoomer::f_ptr() 
{
  return &f_; // pass value by reference 
}

void Zoomer::set_f(float f) 
{
  f_ = f;
}


// -------------------- Translator ---------------------


Translator::~Translator() {}

Translator::Translator(float x, float y) : x_(x), y_(y) {}

void Translator::Update() 
{
  //Translate by x and y 
  glTranslatef(x_, y_, 0);
}

float *Translator::x_ptr() 
{
  return &x_;
}

float *Translator::y_ptr() 
{
  return &y_;
}

void Translator::set_x(float x) 
{
  x_ = x;
}

void Translator::set_y(float y) 
{
  y_ = y;
}






// vim:set sw=2 ts=2:

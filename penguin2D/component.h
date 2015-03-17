#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>
#include "gl.h" // for glEnum

// Component -> Entity -> Component1
//		       -> Component2
//           -> Rectangle 
//	     -> Circle 
//	     -> Color 
// 	     -> Zoomer 
//           -> Translator 

// A Component is the core unit in the system.
// Component is a virtual base class
class Component  //  A parent class 
{
  public:
    // Update() => Draw, Transform Matrix, or Input/Output drawing something,
    // This function is expected to be called every frame.
    virtual void Update() = 0;
    virtual ~Component() = 0; // Destructor   private:
};

// An Entity is a component that can contain other components. 
// An Entity preserves the current matrix. 
// Other components that are not Entity have no such requirement.
class Entity : public Component
{

  private:
    std::vector<Component*> components_; // Contains a vector of components 

  public:
    Entity();
    virtual ~Entity();

    // Adds a component to the entity. 
    // Note: Order of insertion matters because that's the order in which the child components' @Update@ method will be called.
    // Note that the Entity takes ownership of the component and assumes responsibility of freeing it.
    void AddComponent(Component *component);

    // Saves the current matrix, update all child components and restore the matrix.
    virtual void Update();
};

// Parent Component -> Joint -> Child Component 
// => Can move child relative to parent 

// A Joint is a connection between a parent component irtual ~Circle();and its child that
// allows moving or rotating the child relative to the parent.
// This is done so that a transformation matrix (translation or rotation) can be made before child component is drawn 

// The parameters of a Joint are mutable between frames.
// mutable keyword => mutable variables can be modified inside a const function. 
class Joint : public Component 
{
  private:
    float x_; // X position (for this robot project, x doesn't change), but for penguin, y position doesn't change 
    float y_;	     // Y position 
    float a_;	     // angle 
    // Constructs a new Joint at position (x_, y_) with angle (a_) relative to
    // the parent that will hold this joint.

    Component *component_; // points to a component 
    const bool mark_; 
    // If @mark@ is true, then a marker will be drawn around this joint
    // *after* the child has finished drawing.
  public:
    // Note: Changes made to the matrix by the wrapped child component will be reverted after it is done.

    // Note: The (component) will never be deleted by the Joint. Deleting the component is the caller's responsibility.
    Joint(float x, float y, float a, Component *component, bool mark = false);
    virtual ~Joint();
    virtual void Update();

    // Returns a pointer to the joint angle, needed for GLUI integration.
    float *a_ptr();

    // Returns a pointer to the y position, needed for GLUI integration.
    float *y_ptr();

    // Returns a pointer to the x position, needed for GLUI integration.
    float *x_ptr();
    // Changes the angle of rotation.
    void set_a(float a);

    // Changes the y position of the joint.
    void set_y(float y);

    // Note: X position is never changed and only set at the beginning of the call. 
};

// A Rect is a component that draws a rectangle of fixed size.

// Whether the rectangle is a polygon or a set of lines can be changed by
// changing the value of @mode@ with @set_mode@.
class Rectangle : public Component
{
  private:
    // Upper left and lower right positions of rectangle 
    const float x1_; 
    const float y1_; 
    const float x2_;
    const float y2_;
    GLenum mode_; // mode at which rectangle is drawn

  public:
    // Constructs a Rect that will draw a rectangle with opposite corners
    // @(x1, y1)@ and @(x2, y2)@.
    Rectangle(float x1, float y1, float x2, float y2);

    // Constructs a Rect that will draw a rectangle of the given width and height centered at the origin.
    Rectangle(float width, float height);
    virtual ~Rectangle();
    virtual void Update();

    // Changes the mode in which the rectangle is drawn. This defaults to @GL_POLYGON@.
    void set_mode(GLenum mode);   

    // Get the mode in which the rectangle will be drawn.
    GLenum mode() const;
};

// A component that draws a circle.
//
// By default, the circle is drawn by using:
// 90 points = the smoothness of the circle and is configurable using @set_smoothness@.
// Drawing mode can be changed using set_mode() 
class Circle : public Component
{
  private:
    // X and Y position of center of circle
    const float x_;
    const float y_;
    // Radius of circle
    const float r_;
    // smoothness of circle
    int smoothness_;
    // Mode used to draw circle 
    GLenum mode_;
  public:
    // Constructs a Circle that will draw a circle at @(x, y)@ with radius
    // @r@.
    // By default this is a unit circle centered at origin.
    Circle(float x=0, float y=0, float r=1);
    virtual void Update();
    virtual ~Circle();

    // Gets the current smoothness level.
    int smoothness() const;

    // Changes the smoothness level of this circle.
    void set_smoothness(int smoothness);

    // Changes the mode in which the circle is drawn. This defaults to
    // @GL_POLYGON@.
    void set_mode(GLenum mode);

    // Get the mode in which the circle will be drawn.
    GLenum mode() const;
};

// A Color  changes the current color.
class Color : public Component 
{
  private:
    const float r_;
    const float g_;
    const float b_;
    const float a_;

  public:
    // Constructs a new Color that will set the color value to the given (r,g,b,a)
    // Default (0,0,0,1) 
    Color(float r = 0, float g = 0, float b = 0, float a = 1);
    virtual void Update();
    virtual ~Color();

};

// Zoomer 
// Note: X and Y must have the same scale factor for the glScalef() function for it to zoom properly 
class Zoomer : public Component 
{

  private:
    float f_; // zoom factor 

  public:
    // Creates a Zoomer that will zoom by the given scale factor.
    //
    // Default is 1.
    Zoomer(float f = 1);
    virtual void Update();
    virtual ~Zoomer();

    // Returns a pointer to the scale factor, needed for GLUI integration.
    float *f_ptr();

    // Set scale factor.
    void set_f(float f);

};

// Translator 
// A class that does translation  
class Translator : public Component 
{

  private:
    float x_; // x translation
    float y_; // y translation
  public:
    // Creates a Translator that will translate by the given x and y.
    //
    // Default is 1.
    Translator(float x_= 0, float y_=0);
    virtual void Update();
    virtual ~Translator();

    // Returns a pointer for x translation, needed for GLUI integration.
    float *x_ptr();
    // Returns a pointer for x translation, needed for GLUI integration.
    float *y_ptr();

    // Set scale factor.
    void set_x(float x);
    void set_y(float y);

};

// vim:set sw=2 ts=2:

#endif /* end of include guard: COMPONENT_H */


#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>
#include "gl.h"

// A Supplier is an object that supplies a value to components.
template <typename T>
class Supplier {
    public:
        // Returns the value.
        //
        // The idea is that components will be able to use Suppliers to get
        // values that are mutable and there's no simple way of informing the
        // component that the value has changed. For example, if we only have
        // a pointer to the value and we don't know when it will change.
        virtual T Get() = 0;
        virtual ~Supplier() {}

        // Returns a Supplier that always returns the given value.
        static Supplier<T>* constant(T value);

        // Returns a Supplier that returns the value to which the given
        // pointer points.
        static Supplier<T>* pointer(T* ptr);

        // Returns a Supplier that returns the value returned by the given
        // function.
        static Supplier<T>* function(T (*f)());
};

class Wrapper; // Forward declaration.

// A Component is the core unit in the system.
class Component {
  public:
    // "Update" the component -- whatever that means for the component.
    //
    // This could mean drawing something, transforming the matrix, or maybe
    // doing some form of I/O.
    //
    // This function is expected to be called every frame.
    virtual void Update() = 0;

    virtual ~Component() = 0;

    // A @Wrapper@ around the current component to add components to be
    // updated before or after @this@.
    Wrapper &wrap();

    // A Component that attaches @this@ to the parent at @(x, y, z)@. The
    // returned component essentially translates to the given coordinates
    // and updates @this@ there.
    //
    // If @draw_joint@ is true, a circular marker will be drawn at the given
    // coordinates after updating this component.
    Component *attach(float x = 0, float y = 0, float z = 0,
                      bool draw_joint = true);

    // A wrapper around this component that enables the given OpenGL
    // functionality before updating @this@ and then disables it back.
    Component *enableDisable(GLenum cap);

    // A wrapper around this component that executes only if the given
    // supplier returns true.
    Component *onlyWhen(Supplier<bool> *condition);

    // A wrapper around this component that executes only if the given pointer
    // points to a true value.
    Component *onlyWhen(bool *condition);

    // Similar to Component::polygonOffset except that the returned component
    // automatically enables GL_POLYGON_OFFSET_FILL, updates @this@, disables
    // GL_POLYGON_OFFSET_FILL. Further, everything but updating @this@ is
    // disabled if the given bool pointer points to a false value.
    //
    // This is specific to the use-case in robot.cpp.
    Component *polyOffset(float factor, float units, bool *cond);

    // A Component that will push the given attribute to the stack before
    // updating @this@ and then pop it back.
    Component *pushPopAttribute(GLbitfield mask);

    // A Component that will draw a circle around the given point with the
    // given radius. The circle will be parallel to the XY plane.
    static Component *circle(float x, float y, float z, float r);
        
    // A Component that will change the current color.
    static Component *color(float r, float g, float b, float a = 1.0);

    // A Cuboid with opposite corners @(x1, y1, z1)@ and @(x2, y2, z2).
    static Component *cuboid(float x1, float y1, float z1,
                             float x2, float y2, float z2);

    // A Cuboid at origin with the given width, height and depth. The origin
    // will be the center of the new cuboid.
    static Component *cuboid(float width, float height, float depth);

    // A Component that will disable the given OpenGL functionality.
    static Component *disable(GLenum cap);

    // A Component that will enable the given OpenGL functionality.
    static Component *enable(GLenum cap);

    // A Component that will call the given function.
    static Component *function(void (*f)());

    // A Component that will apply the given light properties using
    // @glLightfv@.
    static Component *light(GLenum light, GLenum pname,
                            const GLfloat *params);

    // A Component that will apply the given material properties using
    // @glMaterialfv@.
    static Component *material(GLenum face, GLenum pname,
                               const GLfloat *params);

    // A Component that will apply the given material properties using
    // @glMaterialf@.
    static Component *material(GLenum face, GLenum pname, GLfloat param);

    // A Component that does nothing.
    static Component *nil();

    // A Component that will apply @glPolygonMode@ with the given arguments.
    static Component *polygonMode(GLenum face, GLenum mode);

    // A Component that will pop an attribute off the stack.
    static Component *popAttrib();

    // A Component that will pop a matrix off the stack.
    static Component *popMatrix();

    // A Component that will apply a @glPolygonOffset@ with the given values.
    static Component *polygonOffset(float factor = 0.0, float units = 1.0);

    // A Component that will push the given attribute to the stack.
    static Component *pushAttrib(GLbitfield attrib);

    // A Component that will push the current matrix to the stack.
    static Component *pushMatrix();

    // A Component that will rotate about the three axes by the given values.
    static Component *rotate(float x = 0, float y = 0, float z = 0);

    // A Component that will rotate about the three axes by the values
    // supplied by the given suppliers.
    //
    // Null suppliers will cause no rotation about their corresponding axes.
    //
    // Rotation about the Z-axis is applied first.
    static Component *rotatable(Supplier<float> *x = 0,
                                Supplier<float> *y = 0,
                                Supplier<float> *z = 0);

    // A Component that will scale on the three axes by the given values.
    // Provide the same supplier for all three values for a uniform scale.
    //
    // Null suppliers have no effect.
    static Component *scalable(Supplier<float> *x = 0,
                               Supplier<float> *y = 0,
                               Supplier<float> *z = 0);

    // A Component that will translate on the three axes by the specified
    // values.
    static Component *translate(float x, float y, float z);

    // A Component that will translate on the three axes by the values
    // supplied by the given suppliers. 
    static Component *translatable(Supplier<float> *x,
                                   Supplier<float> *y,
                                   Supplier<float> *z);
};

// An Entity is a component that can contain other components. An Entity
// preserves the current matrix. Other components have no such requirement.
class Entity : public Component {
  public:
    Entity();
    virtual ~Entity();

    // Adds a component to the entity. Order of insertion matters because
    // that's the order in which the child components' @Update@ method will be
    // called.
    //
    // Note that the Entity takes ownership of the component and assumes
    // responsibility of freeing it.
    void AddComponent(Component *component);

    // Saves the current matrix, update all child components and restore the
    // matrix.
    virtual void Update();

    // Convenience shorthand for @AddComponent@. Returns a reference to @this@
    // entity, allowing chaining calls.
    //
    // Allows writing:
    //    myEntity << componentA
    //             << componentB
    //             << ...
    //             ;
//    Entity &operator <<(Component *component);

  private:
    std::vector<Component*> components_;
};

// A Wrap is a component that can wrap a component and update other components
// before and after updating that component. The current matrix is NOT
// preserved.
//
// The Wrapper takes ownership of components added before or after the wrapped
// component but not of the wrapped component.
class Wrapper : public Component {
    public:
        // Constructs a Wrapper for the given component.
        Wrapper(Component *component);
        virtual ~Wrapper();

        // Updates all components added before the wrapped component, updates
        // the wrapped component, and then all components added after it.
        virtual void Update();

        // Add a component to be updated before the wrapped component.
        void AddPrev(Component *component);

        // Add a component to be executed after the wrapped component.
        void AddNext(Component *component);

        // Alias for @AddBefore@.
        Wrapper &operator <<(Component *component);

        // Alias for @AddAfter@.
        Wrapper &operator >>(Component *component);
    private:
        Component *component_;
        std::vector<Component*> before_;
        std::vector<Component*> after_;
};

#endif /* end of include guard: COMPONENT_H */


#include "component.h"
#include "gl.h"
#include <math.h>

inline float deg2rad(float deg) { return deg * M_PI / 180; }

//////////////////////////////////////////////////////////////////////////////
// Supplier
//////////////////////////////////////////////////////////////////////////////

// A supplier that always returns the same value.
template<typename T>
class ConstantSupplier : public Supplier<T> {
    public:
        ConstantSupplier(T value) { value_ = value; }
        virtual ~ConstantSupplier() { }
        virtual T Get() { return value_; }
    private:
        T value_;
};

// A Supplier that returns a value from a pointer.
template <typename T>
class PointerSupplier : public Supplier<T> {
    public:
        // Constructs a Supplier that will supply the value that the given
        // pointer points to.
        PointerSupplier(T *pointer) { pointer_ = pointer; }
        virtual ~PointerSupplier() { }
        virtual T Get() { return *pointer_; }
    private:
        T* pointer_;
};

template <typename T>
class FunctionSupplier : public Supplier<T> {
    public:
        FunctionSupplier(T (*f)()) {
            f_ = f;
        }
        virtual ~FunctionSupplier() {}
        virtual T Get() { return (*f_)(); }
    private:
        T (*f_)();
};

template <typename T>
Supplier<T> *Supplier<T>::constant(T value) {
    return new ConstantSupplier<T>(value);
}

template <typename T>
Supplier<T> *Supplier<T>::pointer(T *ptr) {
    return new PointerSupplier<T>(ptr);
}

template <typename T>
Supplier<T> *Supplier<T>::function(T (*f)()) {
    return new FunctionSupplier<T>(f);
}

//////////////////////////////////////////////////////////////////////////////
// Component implementations
//////////////////////////////////////////////////////////////////////////////

// A cuboid.
class Cuboid : public Component {
    public:
        Cuboid(float x1, float y1, float z1,
               float x2, float y2, float z2,
               GLenum mode = GL_QUADS) {
            x1_ = x1; y1_ = y1; z1_ = z1;
            x2_ = x2; y2_ = y2; z2_ = z2;
            mode_ = mode;
        }


        Cuboid(float width, float height, float depth,
               GLenum mode = GL_QUADS) {
            x1_ = -width  / 2; x2_ =  width / 2;
            y1_ = -height / 2; y2_ = height / 2;
            z1_ = -depth  / 2; z2_ =  depth / 2;
            mode_ = mode;
        }

        virtual ~Cuboid() {}

        virtual void Update() {
            glBegin(mode_);

            // draw front face
            glNormal3f(x1_, y1_, z2_); glVertex3f(x1_, y1_, z2_);
            glNormal3f(x2_, y1_, z2_); glVertex3f(x2_, y1_, z2_);
            glNormal3f(x2_, y2_, z2_); glVertex3f(x2_, y2_, z2_);
            glNormal3f(x1_, y2_, z2_); glVertex3f(x1_, y2_, z2_);

            // draw back face
            glNormal3f(x2_, y1_, z1_); glVertex3f(x2_, y1_, z1_);
            glNormal3f(x1_, y1_, z1_); glVertex3f(x1_, y1_, z1_);
            glNormal3f(x1_, y2_, z1_); glVertex3f(x1_, y2_, z1_);
            glNormal3f(x2_, y2_, z1_); glVertex3f(x2_, y2_, z1_);

            // draw left face
            glNormal3f(x1_, y1_, z1_); glVertex3f(x1_, y1_, z1_);
            glNormal3f(x1_, y1_, z2_); glVertex3f(x1_, y1_, z2_);
            glNormal3f(x1_, y2_, z2_); glVertex3f(x1_, y2_, z2_);
            glNormal3f(x1_, y2_, z1_); glVertex3f(x1_, y2_, z1_);

            // draw right face
            glNormal3f(x2_, y1_, z2_); glVertex3f(x2_, y1_, z2_);
            glNormal3f(x2_, y1_, z1_); glVertex3f(x2_, y1_, z1_);
            glNormal3f(x2_, y2_, z1_); glVertex3f(x2_, y2_, z1_);
            glNormal3f(x2_, y2_, z2_); glVertex3f(x2_, y2_, z2_);

            // draw top
            glNormal3f(x1_, y2_, z2_); glVertex3f(x1_, y2_, z2_);
            glNormal3f(x2_, y2_, z2_); glVertex3f(x2_, y2_, z2_);
            glNormal3f(x2_, y2_, z1_); glVertex3f(x2_, y2_, z1_);
            glNormal3f(x1_, y2_, z1_); glVertex3f(x1_, y2_, z1_);

            // draw bottom
            glNormal3f(x1_, y1_, z1_); glVertex3f(x1_, y1_, z1_);
            glNormal3f(x2_, y1_, z1_); glVertex3f(x2_, y1_, z1_);
            glNormal3f(x2_, y1_, z2_); glVertex3f(x2_, y1_, z2_);
            glNormal3f(x1_, y1_, z2_); glVertex3f(x1_, y1_, z2_);

            glEnd();
        }

    private:
        GLenum mode_;
        float x1_, y1_, z1_,
              x2_, y2_, z2_;
};

// Translates the Model View Matrix.
class Translatable : public Component {
    public:
        Translatable(Supplier<float> *x,
                     Supplier<float> *y,
                     Supplier<float> *z) {
            x_ = x; y_ = y; z_ = z;
        }

        virtual ~Translatable() {
            delete x_; delete y_; delete z_;
        }

        virtual void Update() {
            glTranslatef(x_->Get(), y_->Get(), z_->Get());
        }
    private:
        Supplier<float> *x_;
        Supplier<float> *y_;
        Supplier<float> *z_;
};

// A Component that will scale the model view matrix.
class Scalable : public Component {
    public:
        Scalable(Supplier<float> *x = 0,
                 Supplier<float> *y = 0,
                 Supplier<float> *z = 0) {
            x_ = x != 0 ? x : Supplier<float>::constant(1.0);
            y_ = y != 0 ? y : Supplier<float>::constant(1.0);
            z_ = z != 0 ? z : Supplier<float>::constant(1.0);
        }

        virtual ~Scalable() {
            delete x_; delete y_; delete z_;
        }

        virtual void Update() {
            glScalef(x_->Get(), y_->Get(), z_->Get());
        }

    private:
        Supplier<float> *x_;
        Supplier<float> *y_;
        Supplier<float> *z_;
};

// Rotates the model view matrix.
class Rotatable : public Component {
    public:
        Rotatable(Supplier<float> *angle_x = 0,
                  Supplier<float> *angle_y = 0,
                  Supplier<float> *angle_z = 0) {
            angle_x_ = angle_x;
            angle_y_ = angle_y;
            angle_z_ = angle_z;
        }

        virtual ~Rotatable() {
            if (angle_x_ != 0)
                delete angle_x_;
            if (angle_y_ != 0)
                delete angle_y_;
            if (angle_z_ != 0)
                delete angle_z_;
        }

        virtual void Update() {
            if (angle_z_ != 0)
                glRotatef(angle_z_->Get(), 0, 0, 1);
            if (angle_x_ != 0)
                glRotatef(angle_x_->Get(), 1, 0, 0);
            if (angle_y_ != 0)
                glRotatef(angle_y_->Get(), 0, 1, 0);
        }

    private:
        Supplier<float> *angle_x_;
        Supplier<float> *angle_y_;
        Supplier<float> *angle_z_;
};

// A Component that does not do anything.
class NilComponent : public Component {
    public:
        NilComponent() {}
        virtual ~NilComponent() {}
        virtual void Update() {}
};

// A Component that calls the given function.
class FunctionComponent : public Component {
    public:
        FunctionComponent(void (*f)()) {
            f_ = f;
        }
        virtual ~FunctionComponent() {}
        virtual void Update() { (*f_)(); }
    private:
        void (*f_)();
};

class ColorComponent : public Component {
    public:
        ColorComponent(float r, float g, float b, float a) {
            r_ = r; g_ = g; b_ = b; a_ = a;
        }
        virtual ~ColorComponent() {}
        virtual void Update() {
            glColor4f(r_, g_, b_, a_);
        }
    private:
        float r_, g_, b_, a_;
};

class ConditionalComponent : public Component {
    public:
        ConditionalComponent(Component *component, Supplier<bool> *cond) {
            component_ = component;
            cond_ = cond;
        }
        virtual ~ConditionalComponent() {
            delete cond_;
            delete component_;
        }
        virtual void Update() {
            if (cond_->Get()) {
                component_->Update();
            }
        }
    private:
        Component *component_;
        Supplier<bool> *cond_;
};

class PolygonOffsetComponent : public Component {
    public:
        PolygonOffsetComponent(float factor, float units) {
            factor_ = factor;
            units_ = units;
        }
        virtual ~PolygonOffsetComponent() {}
        virtual void Update() {
            glPolygonOffset(factor_, units_);
        }
    private:
        float factor_, units_;
};

class CapabilityComponent : public Component {
    public:
        CapabilityComponent(GLenum cap, bool enable) {
            cap_ = cap;
            enable_ = enable;
        }
        virtual ~CapabilityComponent() { }
        virtual void Update() {
            if (enable_) {
                glEnable(cap_);
            } else {
                glDisable(cap_);
            }
        }
    private:
        GLenum cap_;
        bool enable_;
};

class PolygonModeComponent : public Component {
    public:
        PolygonModeComponent(GLenum face, GLenum mode) {
            face_ = face;
            mode_ = mode;
        }
        virtual ~PolygonModeComponent() {}
        virtual void Update() {
            glPolygonMode(face_, mode_);
        }
    private:
        GLenum face_, mode_;
};

class PushAttributeComponent : public Component {
    public:
        PushAttributeComponent(GLbitfield mask) {
            mask_ = mask;
        }
        virtual ~PushAttributeComponent() {}
        virtual void Update() {
            glPushAttrib(mask_);
        }
    private:
        GLbitfield mask_;
};

class LightComponent : public Component {
    public:
        LightComponent(GLenum light, GLenum pname, const GLfloat *params) {
            light_ = light;
            pname_ = pname;
            params_ = params;
        }
        virtual ~LightComponent() {}
        virtual void Update() {
            glLightfv(light_, pname_, params_);
        }
    private:
        GLenum light_, pname_;
        const GLfloat *params_;
};

class MaterialfvComponent: public Component {
    public:
        MaterialfvComponent(GLenum face, GLenum pname, const GLfloat *params) {
            face_ = face;
            pname_ = pname;
            params_ = params;
        }
        virtual ~MaterialfvComponent() {}
        virtual void Update() {
            glMaterialfv(face_, pname_, params_);
        }
    private:
        GLenum face_, pname_;
        const GLfloat *params_;
};

class MaterialfComponent: public Component {
    public:
        MaterialfComponent(GLenum face, GLenum pname, GLfloat param) {
            face_ = face;
            pname_ = pname;
            param_ = param;
        }
        virtual ~MaterialfComponent() {}
        virtual void Update() {
            glMaterialf(face_, pname_, param_);
        }
    private:
        GLenum face_, pname_;
        GLfloat param_;
};

class CircleComponent : public Component {
    public:
        CircleComponent(float x, float y, float z, float r) {
            x_ = x; y_ = y; z_ = z; r_ = r;
        }
        virtual ~CircleComponent() {}
        virtual void Update() {
            glBegin(GL_POLYGON);
            for (float i = 0; i < 360.0f; i += 4.0) {
                glNormal3f(0, 0, 1);
                glVertex3f(x_ + r_ * cos(deg2rad(i)),
                           y_ + r_ * sin(deg2rad(i)), z_);
            }
            glEnd();
        }
    private:
        float x_, y_, z_, r_;
};

//////////////////////////////////////////////////////////////////////////////
// Component
//////////////////////////////////////////////////////////////////////////////

Component::~Component() { }

Wrapper &Component::wrap() {
    return *(new Wrapper(this));
}

Component *Component::enableDisable(GLenum cap) {
    return &(wrap()
            << Component::enable(cap)
            >> Component::disable(cap));
}

static Component *markJoint(bool draw_joint) {
    if (draw_joint) {
        return
            Component::function([]{
                    glColor4f(0, 0, 0, 1.0);
                    glutWireSphere(0.05, 10, 10); })
            ->pushPopAttribute(GL_COLOR_BUFFER_BIT);
    } else {
        return Component::nil();
    }
}

Component *Component::polyOffset(float factor, float units, bool *cond) {
    return (wrap()
            << Component::polygonOffset(factor, units)->onlyWhen(cond))
        .enableDisable(GL_POLYGON_OFFSET_FILL);
}

Component *Component::polygonMode(GLenum face, GLenum mode) {
    return new PolygonModeComponent(face, mode);
}

Component *Component::function(void (*f)()) {
    // Preferable to use this for components that are single-line.
    return new FunctionComponent(f);
}

Component *Component::color(float r, float g, float b, float a) {
    return new ColorComponent(r, g, b, a);
}

Component *Component::cuboid(float x1, float y1, float z1,
                             float x2, float y2, float z2) {
    return new Cuboid(x1, y1, z1, x2, y2, z2);
}

Component *Component::cuboid(float width, float height, float depth) {
    return new Cuboid(width, height, depth);
}

Component *Component::pushMatrix() {
    return Component::function([] { glPushMatrix(); });
}

Component *Component::popMatrix() {
    return Component::function([] { glPopMatrix(); });
}

Component *Component::polygonOffset(float factor, float units) {
    return new PolygonOffsetComponent(factor, units);
};

Component *Component::enable(GLenum cap) {
    return new CapabilityComponent(cap, true);
}

Component *Component::disable(GLenum cap) {
    return new CapabilityComponent(cap, false);
}

Component *Component::pushPopAttribute(GLbitfield mask) {
    return &(wrap()
            << Component::pushAttrib(mask)
            >> Component::popAttrib());
}

Component *Component::pushAttrib(GLbitfield mask) {
    return new PushAttributeComponent(mask);
}

Component *Component::scalable(Supplier<float> *x,
                               Supplier<float> *y,
                               Supplier<float> *z) {
    return new Scalable(x, y, z);
}

Component *Component::popAttrib() {
    return Component::function([]{ glPopAttrib(); });
}

Component *Component::attach(float x, float y, float z, bool draw_joint) {
    return &(wrap()
            << Component::pushMatrix()
            << Component::translate(x, y, z)
            >> markJoint(draw_joint)
            >> Component::popMatrix());
}

Component *Component::onlyWhen(Supplier<bool> *condition) {
    return new ConditionalComponent(this, condition);
}

Component *Component::onlyWhen(bool *condition) {
    return onlyWhen(Supplier<bool>::pointer(condition));
}

Component *Component::translate(float x, float y, float z) {
    return new Translatable(Supplier<float>::constant(x),
                            Supplier<float>::constant(y),
                            Supplier<float>::constant(z));
}

Component *Component::translatable(Supplier<float> *x,
                                   Supplier<float> *y,
                                   Supplier<float> *z) {
    return new Translatable(x, y, z);
}

Component *Component::rotate(float x, float y, float z) {
    return new Rotatable(x != 0 ? Supplier<float>::constant(x) : 0,
                         y != 0 ? Supplier<float>::constant(y) : 0,
                         z != 0 ? Supplier<float>::constant(z) : 0);
}

Component *Component::rotatable(Supplier<float> *x,
                                Supplier<float> *y,
                                Supplier<float> *z) {
    return new Rotatable(x, y, z);
}

Component *Component::nil() {
    return new NilComponent();
}

Component *Component::light(GLenum light, GLenum pname, const GLfloat *params) {
    return new LightComponent(light, pname, params);
}

Component *Component::material(GLenum face, GLenum pname, const GLfloat *params) {
    return new MaterialfvComponent(face, pname, params);
}

Component *Component::material(GLenum face, GLenum pname, GLfloat param) {
    return new MaterialfComponent(face, pname, param);
}

Component *Component::circle(float x, float y, float z, float r) {
    return new CircleComponent(x, y, z, r);
}

//////////////////////////////////////////////////////////////////////////////
// Entity
//////////////////////////////////////////////////////////////////////////////

Entity::Entity() : components_() { }

Entity::~Entity() {
    std::vector<Component*>::iterator it = components_.begin();
    for (; it != components_.end(); it++)
        delete *it;
}

void Entity::AddComponent(Component *component) {
  components_.push_back(component);
}

//Entity & Entity::operator<<(Component *component) {
 // AddComponent(component);
 // return *this;
//}

void Entity::Update() {
  glPushMatrix();

  std::vector<Component*>::iterator it;
  for (it = components_.begin(); it != components_.end(); it++)
    (*it)->Update();

  glPopMatrix();
}

//////////////////////////////////////////////////////////////////////////////
// Wrapper
//////////////////////////////////////////////////////////////////////////////

Wrapper::Wrapper(Component *component) : before_(), after_() {
    component_ = component;
}

Wrapper::~Wrapper() {
    std::vector<Component*>::iterator it;

    for (it = before_.begin(); it != before_.end(); it++)
        delete *it;

    for (it = after_.begin(); it != after_.end(); it++)
        delete *it;
}

void Wrapper::AddPrev(Component *component) {
    before_.push_back(component);
}

void Wrapper::AddNext(Component *component) {
    after_.push_back(component);
}

Wrapper & Wrapper::operator<<(Component *component) {
    AddPrev(component);
    return *this;
}

Wrapper & Wrapper::operator>>(Component *component) {
    AddNext(component);
    return *this;
}

void Wrapper::Update() {
    std::vector<Component*>::iterator it;

    for (it = before_.begin(); it != before_.end(); it++)
        (*it)->Update();

    component_->Update();

    for (it = after_.begin(); it != after_.end(); it++)
        (*it)->Update();
}

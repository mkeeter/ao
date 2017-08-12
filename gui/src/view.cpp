#include <QMouseEvent>

#include "gui/view.hpp"
#include "gui/shader.hpp"

View::View(QWidget* parent)
    : QOpenGLWidget(parent), camera(size()),
      settings({-10, -10, -10}, {10, 10, 10}, 10)
{
    setMouseTracking(true);
    connect(this, &View::settingsChanged,
            this, &View::onSettingsChanged);
}

void View::setShapes(QList<Shape*> new_shapes)
{
    // Erase all existing shapes
    for (auto s : shapes)
    {
        disconnect(s, &Shape::gotMesh, this, &View::update);
        s->deleteLater();
    }
    shapes.clear();

    // Connect all new shapes
    for (auto s : new_shapes)
    {
        connect(s, &Shape::gotMesh, this, &View::update);
        connect(s, &Shape::gotMesh, this, &View::checkMeshes);
        connect(this, &View::settingsChanged,
                s, &Shape::startRender);
        s->startRender(settings);
        s->setParent(this);

        shapes.push_back(s);
    }
    update();
}

void View::openSettings(bool)
{
    if (pane.isNull())
    {
        pane = new SettingsPane(settings);
        connect(pane, &SettingsPane::changed,
                this, &View::settingsChanged);
        pane->show();
        pane->setFixedSize(pane->size());
    }
    else
    {
        pane->setFocus();
    }
}

void View::onSettingsChanged(Settings s)
{
    settings = s;
}

void View::initializeGL()
{
    Shader::initializeGL();
    axes.initializeGL();
    background.initializeGL();
}

void View::paintGL()
{
    background.draw();

    auto m = camera.M();
    for (auto& s : shapes)
    {
        s->draw(m);
    }

    if (show_axes)
    {
        axes.drawSolid(m);
        axes.drawWire(m);
    }
}

void View::resizeGL(int width, int height)
{
    camera.resize({width, height});
}

void View::mouseMoveEvent(QMouseEvent* event)
{
    QOpenGLWidget::mouseMoveEvent(event);

    if (mouse.state == mouse.DRAG_ROT)
    {
        camera.rotateIncremental(event->pos() - mouse.pos);
        update();
    }
    else if (mouse.state == mouse.DRAG_PAN)
    {
        camera.panIncremental(event->pos() - mouse.pos);
        update();
    }
    mouse.pos = event->pos();
}

void View::mousePressEvent(QMouseEvent* event)
{
    QOpenGLWidget::mousePressEvent(event);

    if (mouse.state == mouse.RELEASED)
    {
        if (event->button() == Qt::LeftButton)
        {
            mouse.state = mouse.DRAG_ROT;
        }
        else if (event->button() == Qt::RightButton)
        {
            mouse.state = mouse.DRAG_PAN;
        }
    }
}

void View::mouseReleaseEvent(QMouseEvent* event)
{
    QOpenGLWidget::mouseReleaseEvent(event);
    mouse.state = mouse.RELEASED;
}

void View::wheelEvent(QWheelEvent *event)
{
    QOpenGLWidget::wheelEvent(event);
    camera.zoomIncremental(event->angleDelta().y(), mouse.pos);
    update();
}

void View::showAxes(bool a)
{
    show_axes = a;
    update();
}

void View::checkMeshes() const
{
    bool all_done = true;
    QList<const Kernel::Mesh*> meshes;
    for (auto s : shapes)
    {
        if (s->done())
        {
            meshes.push_back(s->getMesh());
        }
        else
        {
            all_done = false;
        }
    }
    if (all_done)
    {
        emit(meshesReady(meshes));
    }
}

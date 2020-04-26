#include "AnnotationGraphicsView.h"
#include "GraphicsAnnotationItem.h"
#include "AnnotationEdit.h"
#include <QDebug>

namespace codetextedit {

AnnotationGraphicsView::AnnotationGraphicsView(QGraphicsScene *scene, QWidget* parent)
: QGraphicsView(scene, parent)
{
}

void AnnotationGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    QPoint globalPt = mapToGlobal(event->pos());
    QPoint mousePt = mapFromGlobal(globalPt);
    QPointF scenePt = mapToScene(mousePt);
    QPointF startPt(0,scenePt.y());
    QGraphicsItem* item = scene()->itemAt(startPt,QTransform());

    if (item != nullptr)
    {
        GraphicsAnnotationItem *textItem = dynamic_cast<GraphicsAnnotationItem*>(item);
        if (textItem != nullptr)
        {
            textItem->hover();
            emit mouseMove(textItem);
        }
    }
    else
    {
        emit mouseMove(nullptr);
    }
}

} // namespace

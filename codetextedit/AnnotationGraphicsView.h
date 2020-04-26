#ifndef ANNOTATIONGRAPHICSVIEW_H
#define ANNOTATIONGRAPHICSVIEW_H

#include <QGraphicsView>

namespace codetextedit
{
class GraphicsAnnotationItem;

class AnnotationEdit;

class AnnotationGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    AnnotationGraphicsView(QGraphicsScene *, QWidget *parent = nullptr);
    void mouseMoveEvent(QMouseEvent *event) override;
signals:
    void mouseMove(GraphicsAnnotationItem*);
};

}
#endif // ANNOTATIONGRAPHICSVIEW_H

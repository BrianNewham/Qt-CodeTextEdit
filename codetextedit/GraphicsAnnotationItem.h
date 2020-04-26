/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#ifndef GRAPHICSANNOTATIONITEM_H
#define GRAPHICSANNOTATIONITEM_H

#include <QGraphicsTextItem>
#include <QPainter>
#include <QPaintEvent>
#include <QPointF>
#include <QObject>
#include "Annotation.h"

namespace codetextedit
{
    class GraphicsAnnotationItem;
    class AnnotationEdit;

    class AnnotationButton
    {
    public:
        struct PaintingStyle
        {
            enum {style1,style2} style = style1;
            int diameter = 16;
            int dotDiameter = 12;
            QColor blankColor = QColor("#f0f0f0");

        };
        AnnotationButton(GraphicsAnnotationItem* item, int index, const QColor& color)
          : m_item(item), m_index(index), m_color(color) {}
        virtual ~AnnotationButton() = default;
        void paint(QPainter* painter);

        int index() const {return m_index;}
        GraphicsAnnotationItem* item() {return m_item;}
        void setChecked(bool checked) {m_checked = checked;}
        bool isChecked() const {return m_checked;}
        QRectF boundingRect() const;
        static void setStyle(PaintingStyle style) {paintingStyle = style;}
        static PaintingStyle style() {return paintingStyle;}
    private:
        GraphicsAnnotationItem* m_item;
        int m_index;
        QColor m_color;
        bool m_checked = false;
        static PaintingStyle paintingStyle;

        void paintStyle1(QPainter*);
        void paintStyle2(QPainter*);
    };

    class GraphicsAnnotationItem : public QGraphicsItem
    {
    public:
        GraphicsAnnotationItem(QGraphicsItem* parent = nullptr);
        GraphicsAnnotationItem(const QString&, const AnnotationContainer&, int, QGraphicsItem* parent = nullptr);
        void paint(QPainter*, const QStyleOptionGraphicsItem* =nullptr,  QWidget* =nullptr) override;
        void setButtonTab(int tab) {m_buttonTab = tab;}
        void setLineAscentDescent(int ascent, int descent) {m_ascent=ascent; m_descent=descent;}
        void setFont(QFont font) {m_font = font;}
        void setPlainText(QString text) {m_message = text;}
        void setDefaultTextColor(QColor color) {m_color=color;}
        int buttonTab() {return m_buttonTab;}
        int capture(QPointF);
        bool isCaptured() const {return m_captured;}
        void setChecked(int);
        QRectF textRect() const;
        QRectF boundingRect() const override;
        void reset();
        int buttonsCount() {return m_buttonList.count();}
        static void setHighlight(GraphicsAnnotationItem* item);
        void hover();
        QString message() const {return m_message;}

        static const int buttonGap = 16;
    private:
        QList<AnnotationButton*> m_buttonList;
        QStringList m_messageList;
        QString m_message;
        QFont m_font;
        QColor m_color;
        int m_ascent = 0;
        int m_descent = 0;
        int m_buttonTab = -1;
        int m_defaultButton=-1;
        bool m_captured=false;
        bool m_highlight = false;
        static GraphicsAnnotationItem *currentHighlight;

        friend class AnnotationButton;
    };

} // namespace codetextedit
#endif // GRAPHICSANNOTATIONITEM_H

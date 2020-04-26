/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#include "GraphicsAnnotationItem.h"
#include "AnnotationEdit.h"

#include <QFontMetrics>
#include <QDebug>

namespace codetextedit {

AnnotationButton::PaintingStyle AnnotationButton::paintingStyle;
GraphicsAnnotationItem* GraphicsAnnotationItem::currentHighlight = nullptr;

QColor interpolateColor(float weight, const QColor& color1, const QColor& color2)
{
    return QColor(
        color1.red  () * (1-weight) + color2.red  () * weight,
        color1.green() * (1-weight) + color2.green() * weight,
        color1.blue () * (1-weight) + color2.blue () * weight,
        255);
}

void AnnotationButton::paintStyle1(QPainter *painter)
{
    //QColor fillColor = m_checked ? m_color : interpolateColor(0.7, m_color, paintingStyle.blankColor);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing,true);
    QRectF rect = boundingRect();
    QBrush brush("white",Qt::SolidPattern);
    painter->fillRect(rect,brush);
    QPen pen(m_color,2);
    painter->setPen(pen);
    QRectF circleRect(rect.x()+1,rect.y()+1,rect.width()-2,rect.height()-2);
    painter->drawEllipse(circleRect);
    if (m_checked)
    {
        QBrush brush(m_color,Qt::SolidPattern);
        painter->setBrush(brush);
        int indent = paintingStyle.diameter - paintingStyle.dotDiameter;
        QRectF indicatorRect(rect.x()+indent,rect.y()+indent,rect.width()-2*indent,rect.height()-2*indent);
        painter->drawEllipse(indicatorRect);
    }
    painter->restore();
}

void AnnotationButton::paintStyle2(QPainter *painter)
{
    QColor color = m_checked ? m_color : interpolateColor(0.7, m_color, paintingStyle.blankColor);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing,true);
    QRectF rect = boundingRect();
    QBrush eraseBrush("white",Qt::SolidPattern);
    painter->fillRect(rect,eraseBrush);
    QColor fillColor(color);
    QBrush fillBrush(fillColor,Qt::SolidPattern);
    painter->setBrush(fillBrush);
    painter->setPen(Qt::NoPen);
    QRectF circleRect(rect.x()+1,rect.y()+1,rect.width()-2,rect.height()-2);
    painter->drawEllipse(circleRect);
    painter->restore();
}

void AnnotationButton::paint(QPainter *painter)
{
    if (paintingStyle.style == AnnotationButton::PaintingStyle::style1)
        paintStyle1(painter);
    else if (paintingStyle.style == AnnotationButton::PaintingStyle::style2)
        paintStyle2(painter);
}

QRectF AnnotationButton::boundingRect() const
{
    int diameter = paintingStyle.diameter;
    int radius = diameter / 2;
    int lineheight = m_item->m_ascent + m_item->m_descent;
    int x = m_item->buttonTab() + m_index * (diameter + m_item->buttonGap);
    int y = -m_item->m_ascent + lineheight/2;
    return QRectF(x - radius, y - radius, diameter, diameter);
}

GraphicsAnnotationItem::GraphicsAnnotationItem(QGraphicsItem* parent)
    : QGraphicsItem(parent), m_message(""), m_defaultButton(-1)
{
    setPlainText(m_message);
}

GraphicsAnnotationItem::GraphicsAnnotationItem
    (const QString& message, const AnnotationContainer& container, int defaultButton, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_message(message), m_defaultButton(defaultButton)
{
    setPlainText(message);
    if (container.count() > 1)
    {
        for (int i=0; i<container.count(); i++)
        {
            AnnotationButton* button = new AnnotationButton(this,i,container[i]->alertColor());
            m_buttonList.append(button);
            m_messageList.append(container[i]->message());
        }
    }
    setAcceptHoverEvents(true);
}

void GraphicsAnnotationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*,  QWidget*)
{
    painter->save();
    QRectF rect = boundingRect();
    QBrush brush(QColor("white"),Qt::SolidPattern);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);
    if (m_highlight)
    {
        QBrush brush(QColor(0xD8,0xD8,0xD8),Qt::SolidPattern);
        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);
        QRectF highlightRect = rect;
        highlightRect.setWidth(m_buttonTab-4);
        painter->drawRect(highlightRect);
    }
    painter->setBrush(Qt::NoBrush);
    painter->setPen(m_color);
    painter->setFont(m_font);
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, m_message);
    if (m_buttonTab != -1)
    {
        for (int i=0; i<m_buttonList.count(); i++)
            m_buttonList[i]->paint(painter);
    }
    painter->restore();
}

QRectF GraphicsAnnotationItem::boundingRect() const
{
    int width = m_buttonTab + m_buttonList.count()*(buttonGap + AnnotationButton::style().diameter);
    //qDebug() << QRectF(0, -m_ascent, width, m_ascent + m_descent);
    return QRectF(0, -m_ascent, width, m_ascent + m_descent);
}

int GraphicsAnnotationItem::capture(QPointF pt)
{
    m_captured = false;
    if (pt.x() >= textRect().x() && pt.x() <= textRect().right())
        m_captured = true;
    int index = -1;
    if (!m_captured)
    {
        for (int i=0; i < m_buttonList.count(); i++)
        {
            QRectF rect = m_buttonList[i]->boundingRect();
            //qDebug() << i << rect << buttonGap << pt.x() << (pt.x() >= rect.x()-buttonGap/2 && pt.x() <= rect.right()+buttonGap/2);
            if (pt.x() >= rect.x()-buttonGap/2 && pt.x() <= rect.right()+buttonGap/2)
            {
                m_buttonList[i]->setChecked(true);
                setPlainText(m_messageList[i]);
                m_highlight = true;
                index = i;
            }
            else
            {
                m_buttonList[i]->setChecked(false);
            }
        }

        QRectF rect = boundingRect();
        update(QRectF(m_buttonTab - AnnotationButton::style().diameter/2, -m_ascent, rect.width() - m_buttonTab, rect.height()));
    }
    return index;
}

void GraphicsAnnotationItem::setChecked(int index)
{
    for (int i=0; i< m_buttonList.count(); i++)
    {
        m_buttonList[i]->setChecked(i==index);
    }
}

void GraphicsAnnotationItem::setHighlight(GraphicsAnnotationItem *item)
{
   if (currentHighlight != nullptr)
   {
       currentHighlight->m_highlight = false;
       currentHighlight->update();
   }
   currentHighlight = item;
   if (item != nullptr)
   {
       item->m_highlight = true;
       item->update();
   }
}

void GraphicsAnnotationItem::hover()
{
    if (currentHighlight != this)
    {
        if (currentHighlight != nullptr )
        {
            currentHighlight->m_highlight = false;
            currentHighlight->update();
        }
        m_highlight = true;
        update();
        currentHighlight = this;
    }
}

QRectF GraphicsAnnotationItem::textRect() const
{
    QFontMetrics metrics(m_font);
    int pixelsWide = metrics.horizontalAdvance(m_message);
    //int pixelsHigh = metrics.height();

    return QRectF(0,-m_ascent, pixelsWide, m_ascent + m_descent);
}

void GraphicsAnnotationItem::reset()
{
    setPlainText(m_message);
    for (int i=0; i< m_buttonList.count(); i++)
    {
        m_buttonList[i]->setChecked(i==m_defaultButton);
    }
}

} // namespace

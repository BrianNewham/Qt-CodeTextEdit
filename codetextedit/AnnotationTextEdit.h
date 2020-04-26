/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#ifndef ANNOTATIONTEXTEDIT_H
#define ANNOTATIONTEXTEDIT_H

#include <QTextEdit>

#include <QTextEdit>
#include <QObject>
#include <QTextLine>

 class QPaintEvent;
 class QResizeEvent;
 class QSize;
 class QWidget;

 class LineNumberArea;


 class AnnotationTextEdit : public QTextEdit
 {
     Q_OBJECT

 public:
     AnnotationTextEdit(QWidget *parent = nullptr);
     void clearHighlight();
     void highlightCurrentLine(const QTextBlock&, bool highlight);
     void setAscentDescent(int ascent, int descent) {m_ascent=ascent; m_descent=descent;}
     int lineSpacing() const {return m_ascent+m_descent;}
 private slots:
     void mouseMoveEvent(QMouseEvent *) override;
 private:
     static int currentBlockNumber;
     int m_ascent, m_descent;
 signals:
     void blockHighlighted(int);
 };

#endif // ANNOTATIONTEXTEDIT_H

/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#include "AnnotationTextEdit.h"
#include <QMouseEvent>
#include <QDebug>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>
#include <QFontMetrics>

int AnnotationTextEdit::currentBlockNumber = -1;

AnnotationTextEdit::AnnotationTextEdit(QWidget *parent) : QTextEdit(parent)
{
}

void AnnotationTextEdit::clearHighlight()
{
    if (currentBlockNumber != -1)
    {
        for (QTextBlock block=document()->begin(); block!=document()->end(); block = block.next())
        {
            if (block.blockNumber()==currentBlockNumber)
                highlightCurrentLine(block,false);
        }
    }
    currentBlockNumber = -1;
}

void AnnotationTextEdit::highlightCurrentLine(const QTextBlock& block, bool highlight)
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;

    QColor lineColor = highlight ? "#D8D8D8" : "white";

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = QTextCursor(block);
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);

    if (highlight)
        currentBlockNumber = block.blockNumber();
}

void AnnotationTextEdit::mouseMoveEvent(QMouseEvent* event)
{
    int eventPtY = event->localPos().y();
    QTextCursor firstVisibleCursor = cursorForPosition(QPoint(0,0));
    QRect firstRect = cursorRect(firstVisibleCursor);
    QTextBlock firstBlock = firstVisibleCursor.block();
    int firstY = firstBlock.layout()->position().y() - firstRect.y();
    bool highlighted = false;
    for (QTextBlock block = document()->begin(); block != document()->end(); block = block.next())
    {
        int blockPtY = block.layout()->position().y() - firstY;
        if (blockPtY <= eventPtY && blockPtY + lineSpacing() >= eventPtY)
        {
            highlightCurrentLine(block,true);
            emit blockHighlighted(block.blockNumber());
            highlighted = true;
            break;
        }
    }
    if (!highlighted)
    {
        emit blockHighlighted(-1);
        clearHighlight();
    }
}






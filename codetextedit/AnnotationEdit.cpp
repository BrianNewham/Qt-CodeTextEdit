/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#include "AnnotationEdit.h"
#include "AnnotationWorker.h"
#include "GraphicsAnnotationItem.h"

#include <QTextDocument>
#include <QTextBlock>
#include <QDebug>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QGraphicsProxyWidget>
#include <QGroupBox>
#include <QFontMetrics>

namespace codetextedit
{

static const int fontSize = 10;
//static const QString fontFamilyEditor = "Arial";
//static const QString fontFamilyAnnotation = "Arial";
static const QString fontFamilyEditor = "Source Code Pro";
static const QString fontFamilyAnnotation = "Source Code Pro";

AnnotationDialog::AnnotationDialog(const AnnotationContainer& container, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Popup);
    QVBoxLayout *layout = new QVBoxLayout(this);
    for (Annotation* annotation : container)
    {
        QColor color = annotation->alertColor();
        auto hexComponent = [](int color)->QString {return QString("%1").arg(color,2,16,QLatin1Char('0'));};
        QString hexString = '#' + hexComponent(color.red()) + hexComponent(color.green()) + hexComponent(color.blue());
        QString colorStr = "color: " + hexString;

        QLabel *label = new QLabel(annotation->message(),this);
        label->setStyleSheet(colorStr);
        layout->addWidget(label);

        label = new QLabel(annotation->solutionHelp(),this);
        label->setStyleSheet(colorStr);
        layout->addWidget(label);

        layout->addWidget(new QLabel("==============="));
    }
    setLayout(layout);
}

AnnotationEdit::AnnotationEdit(Annotator* annotator, CodeTextHighlighter *highlighter, QWidget* parent)
    : QWidget(parent)
    , m_highlighter(highlighter)
    , m_annotator(annotator)
{
    m_splitter = new QSplitter(this);
    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsView = new AnnotationGraphicsView(m_graphicsScene, m_splitter);
    m_textEdit = new AnnotationTextEdit(m_splitter);
    m_empty = new QWidget(this);
    m_gvContainer = new QWidget(this);

    m_textEdit->setGeometry(0,0,width()*0.6,height());
    m_textEdit->setFont(QFont(fontFamilyEditor,fontSize));
    m_textEdit->setWordWrapMode(QTextOption::NoWrap);
    m_textEdit->setTabStopDistance(QFontMetricsF(m_textEdit->font()).horizontalAdvance(' ') * 8);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_graphicsScene->setSceneRect(0,0,width()*0.4,height()-2);
    m_graphicsView->setGeometry(width()*0.6,0,width()*0.4,height());
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_graphicsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* hbox = new QVBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);
    hbox->addWidget(m_graphicsView);
    hbox->addWidget(m_empty);
    m_empty->setMaximumHeight(m_graphicsView->height() - m_graphicsView->viewport()->height());
    m_gvContainer->setLayout(hbox);

    m_splitter->addWidget(m_textEdit);
    m_splitter->addWidget(m_gvContainer);

    connect(m_textEdit->document()->documentLayout(), &QAbstractTextDocumentLayout::update, [this](const QRectF&) { this->updateAnnotations(); });
    connect(m_textEdit, &QTextEdit::textChanged, this, &AnnotationEdit::textChanged);
    connect(m_textEdit, &QTextEdit::textChanged, this, &AnnotationEdit::updateAnnotations);
    connect(m_textEdit, &QTextEdit::cursorPositionChanged, [this]() {
        m_textEdit->clearHighlight();
        GraphicsAnnotationItem::setHighlight(nullptr);
    });
    connect(m_textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &AnnotationEdit::textEditScrollBarChanged);
    connect(m_graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &AnnotationEdit::graphicsViewScrollBarChanged);
    connect(m_splitter, &QSplitter::splitterMoved, this, [this]() {
       synchronizeSceneWithDocument();
    });
    connect(m_textEdit, &AnnotationTextEdit::blockHighlighted, [this](int blockNumber) {
       GraphicsAnnotationItem *item = m_blockToItemMap[blockNumber];
       GraphicsAnnotationItem::setHighlight(item);
    });
    connect(m_graphicsView, &AnnotationGraphicsView::mouseMove, this, &AnnotationEdit::highlightLine);
    m_textEdit->setFocus();

    m_highlighter->setDocument(m_textEdit->document());

    m_annotationRefreshTimer.setInterval(400);
    m_annotationRefreshTimer.setSingleShot(true);
    connect(&m_annotationRefreshTimer, &QTimer::timeout, this, &AnnotationEdit::refreshAnnotations);

    m_annotationWorker = new AnnotationWorker(m_annotator, this);
    connect(m_annotationWorker, &AnnotationWorker::analyzed, this, &AnnotationEdit::rebuildAnnotations);

    setMouseTracking(true);

    AnnotationButton::PaintingStyle style;
    style.diameter = 16;
    style.dotDiameter = 14;
    style.style = AnnotationButton::PaintingStyle::style1;
    AnnotationButton::setStyle(style);
}

AnnotationEdit::~AnnotationEdit()
{
    m_annotationWorker->kill();

    while(!m_annotationWorker->isFinished())
        QThread::msleep(100);

    deleteAll();
}

void AnnotationEdit::loadFile(QString filePath)
{
    QFile file(filePath);
    if(! file.open(QFile::ReadOnly))
        return;

    auto content = file.readAll();

    m_textEdit->setPlainText(content);

    updateAnnotations();
    synchronizeSceneWithDocument();
}

void AnnotationEdit::setContents(QString contents)
{
    m_textEdit->setPlainText(contents);

    updateAnnotations();
    synchronizeSceneWithDocument();
}

QString AnnotationEdit::toPlainText()
{
    return m_textEdit->toPlainText();
}

void AnnotationEdit::setPlainText(QString text)
{
    m_textEdit->setPlainText(text);
}

void AnnotationEdit::resizeEvent(QResizeEvent *event)
{
    //int viewportHeight = m_textEdit->viewport()->height();
    int emptyHeight    = m_textEdit->height() - m_textEdit->viewport()->height();
    m_empty->setMaximumHeight(emptyHeight);
    m_empty->setMinimumHeight(emptyHeight);

    //qDebug() << m_textEdit->height() << m_textEdit->viewport()->height()  << "  EM" << m_empty->height()    << " Y: " << m_textEdit->y() << m_textEdit->viewport()->y();
    m_splitter->setGeometry(0,0,width(),height());
    //m_graphicsScene->setSceneRect(0,0,m_graphicsView->width(),viewportHeight);
    QWidget::resizeEvent(event);
    synchronizeSceneWithDocument();
}

void AnnotationEdit::mousePressEvent(QMouseEvent *event)
{
    if (m_currentItem != nullptr)
    {
        m_currentItem->setFont(QFont(fontFamilyAnnotation,fontSize,QFont::Normal));
        m_currentItem->update();
        m_currentItem = nullptr;
    }
    GraphicsAnnotationItem::setHighlight(nullptr);
    QPoint globalPt = mapToGlobal(event->pos());
    QPoint mousePt = m_graphicsView->mapFromGlobal(globalPt);
    QPointF scenePt = m_graphicsView->mapToScene(mousePt);
    QPointF startPt(0,scenePt.y());
    QGraphicsItem* item = m_graphicsScene->itemAt(startPt,QTransform());

    if (item != nullptr)
    {
        GraphicsAnnotationItem *textItem = dynamic_cast<GraphicsAnnotationItem*>(item);
        if (textItem != nullptr)
        {
           int index = textItem->capture(scenePt);
           if (textItem->isCaptured())
           {
               showPopup(textItem);
           }
           else if (index != -1)
           {
               m_currentItem = textItem;
               QColor color = m_itemMap[m_currentItem][index]->alertColor();
               m_currentItem->setDefaultTextColor(color);
               m_currentItem->setFont(QFont(fontFamilyAnnotation,fontSize,QFont::Bold));
               m_currentItem->update();
           }
        }
    }
}

void AnnotationEdit::updateAnnotations()
{
    m_annotationRefreshTimer.start();
    synchronizeSceneWithDocument();
}

void AnnotationEdit::refreshAnnotations()
{
    deleteAll();
    m_currentItem = nullptr;
    GraphicsAnnotationItem::setHighlight(nullptr);

    QStringList lines;
    bool allBlank = extractLines(m_textEdit->document(), lines);
    if(allBlank)
        return;

    m_annotationWorker->analyze(lines);
}

void AnnotationEdit::rebuildAnnotations(AnnotationMap annotations)
{
    QTextDocument *document = m_textEdit->document();
    m_annotationMap = annotations;

    int buttonTab = 0;
    LineNumber lineNum = 0;
    int ascent = 0, descent = 0;

    m_itemMap.clear();
    m_blockToItemMap.clear();
    m_itemToBlockMap.clear();

    for (QTextBlock block = document->begin(); block != document->end(); block = block.next())
    {
        QTextLine line = block.layout()->lineAt(0);
        if(! line.isValid()) {
            qWarning() << "Invalid line found: Probably too early to function.";
            return;
        }

        ascent = line.ascent();
        descent = line.descent();

        m_textEdit->setAscentDescent(ascent,descent);
        break;
    }

    for (QTextBlock block = document->begin(); block != document->end(); block = block.next())
    {
        //QTextLine line = block.layout()->lineAt(0);

        int yBlock = int(block.layout()->position().y());
        int yBaseline = yBlock + ascent;
        //qDebug() << lineNum << "Y: " << yBlock << " ASC, DESC: " << line.ascent() << line.descent();

        if (!block.text().isEmpty())
        {
            AnnotationContainer container = m_annotationMap[lineNum];
            if (!container.isEmpty())
            {
                int buttonIndex = -1;
                QString bracketString;
                if (container.count() > 1)
                    bracketString = " (" + QString::number(container.count()) + ')';
                QString priorityString = priorityMessage(container,buttonIndex) + bracketString;

                int maxWidth = longestWidth(container);
                int width = textWidth(priorityString);
                if (width > maxWidth)
                    maxWidth = width;
                if (buttonTab < maxWidth)
                    buttonTab = maxWidth;

                GraphicsAnnotationItem* item = new GraphicsAnnotationItem(priorityString,container,buttonIndex);
                //qDebug() << "rebuild " << block.blockNumber();
                m_blockToItemMap.insert(block.blockNumber(),item);
                m_itemToBlockMap.insert(item,block.blockNumber());
                m_itemMap.insert(item, container);
                item->setPos(0,yBaseline);
                item->setLineAscentDescent(ascent, descent);
                m_graphicsScene->addItem(item);
                item->setFont(QFont(fontFamilyAnnotation,fontSize,QFont::Normal));
                item->setPlainText(priorityString);
                if (buttonIndex != -1 && item->buttonsCount()>0)
                {
                    m_currentItem = item;
                    item->reset();
                    item->setDefaultTextColor(container[buttonIndex]->alertColor());
                }
                else if (container.count()==1)
                {
                    item->setDefaultTextColor(container[0]->alertColor());
                }
            }
        }
        ++ lineNum;
    }
    int gap = textWidth("   ");
    for (GraphicsAnnotationItem* item : m_itemMap.keys())
        item->setButtonTab(buttonTab+gap);

    synchronizeSceneWithDocument();
}

void AnnotationEdit::synchronizeSceneWithDocument()
{
    QTextDocument *document = m_textEdit->document();
    //qDebug() << "height " << height() << " gv height " << m_graphicsView->height() << "gvc " << m_gvContainer->height() << " empyt " << m_empty->height();
    int size = int(document->documentLayout()->documentSize().height());
    if (size <= m_textEdit->height() - 1)
        size = m_graphicsView->height() - 2;

    //qDebug() << "TextDoc: " << size << "  Canvas: " << rect;

    m_graphicsScene->setSceneRect(QRectF(0,0,m_graphicsView->width(),size));
//    qDebug() << m_textEdit->viewport()->size() << m_graphicsView->size();
}

void AnnotationEdit::textEditScrollBarChanged(int value)
{
    // I had trouble with event feedback so I had to squelch the cross signalling while updating.
    //  They scrollbars where keeping each other busy.

    double ratio = (double)value/m_textEdit->verticalScrollBar()->maximum();

//    qDebug() << "TE->GV: "
//             << "  GV: " << m_graphicsView->verticalScrollBar()->minimum() << m_graphicsView->verticalScrollBar()->value() << m_graphicsView->verticalScrollBar()->maximum()
//             << "  TE: " << m_textEdit->verticalScrollBar()->minimum() << m_textEdit->verticalScrollBar()->value() << m_textEdit->verticalScrollBar()->maximum()
//             << "  ==> " << value << ratio;

    disconnect(m_graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &AnnotationEdit::graphicsViewScrollBarChanged);
    m_graphicsView->verticalScrollBar()->setValue(ratio * m_graphicsView->verticalScrollBar()->maximum());
    connect(m_graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &AnnotationEdit::graphicsViewScrollBarChanged);
}

void AnnotationEdit::graphicsViewScrollBarChanged(int value)
{
    // I had trouble with event feedback so I had to squelch the cross signalling while updating.
    //  They scrollbars where keeping each other busy.

    double ratio = (double)value/m_graphicsView->verticalScrollBar()->maximum();

//    qDebug() << "GV->TE: "
//             << "  GV: " << m_graphicsView->verticalScrollBar()->minimum() << m_graphicsView->verticalScrollBar()->value() << m_graphicsView->verticalScrollBar()->maximum()
//             << "  TE: " << m_textEdit->verticalScrollBar()->minimum() << m_textEdit->verticalScrollBar()->value() << m_textEdit->verticalScrollBar()->maximum()
//             << "  ==> " << value << ratio;

    disconnect(m_textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &AnnotationEdit::textEditScrollBarChanged);
    m_textEdit->verticalScrollBar()->setValue(ratio * m_textEdit->verticalScrollBar()->maximum());
    connect(m_textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &AnnotationEdit::textEditScrollBarChanged);
}

void AnnotationEdit::showPopup(GraphicsAnnotationItem *item)
{
    AnnotationContainer container = m_itemMap[item];
    if (!container.isEmpty())
    {
        item->setFont(QFont(fontFamilyAnnotation,fontSize,QFont::Bold));
        item->update();
        AnnotationDialog dialog(container,this);
        dialog.exec();
        item->setFont(QFont(fontFamilyAnnotation,fontSize,QFont::Normal));
        item->update();
    }
}

QString AnnotationEdit::priorityMessage(const AnnotationContainer& container, int& buttonIndex)
{
    buttonIndex = -1;
    if (!container.isEmpty())
    {
        for (int i=0; i<container.count()  && buttonIndex==-1; i++)
        {
            if (container[i]->category()==Annotation::CATEGORY_Error)
                buttonIndex = i;
        }
        for (int i=0; i<container.count() && buttonIndex==-1; i++)
        {
            if (container[i]->category()==Annotation::CATEGORY_Warning)
                buttonIndex = i;
        }
        for (int i=0; i<container.count() && buttonIndex==-1; i++)
        {
            if (container[i]->category()==Annotation::CATEGORY_Hint)
                buttonIndex = i;
        }
        if (buttonIndex==-1)
            buttonIndex = 0;
    }
    else
    {
        return "";
    }
    return container[buttonIndex]->message();
}

int AnnotationEdit::textWidth(const QString &string) const
{
    QFont font(fontFamilyAnnotation,fontSize,QFont::Bold);
    QFontMetrics metrics(font);
    return metrics.horizontalAdvance(string);
}

int AnnotationEdit::longestWidth(const AnnotationContainer& container) const
{
    QString message;
    int maxWidth = 0;
    for (Annotation* annotation : container)
    {
        int messageWidth = textWidth(annotation->message());
        if (maxWidth < messageWidth)
            maxWidth = messageWidth;
    }
    return maxWidth;
}

void AnnotationEdit::deleteAll()
{
    m_currentItem = nullptr;
    GraphicsAnnotationItem::setHighlight(nullptr);
    m_graphicsScene->clear();
    m_itemMap.clear();

    for(auto container: m_annotationMap.values())
        qDeleteAll(container);

    m_annotationMap.clear();
}

bool AnnotationEdit::extractLines(QTextDocument *document, QStringList& lines)
{
    QTextBlock block = document->firstBlock();

    bool allBlank = true;

    while(block.isValid())
    {
        auto text = block.text();
        if(! text.trimmed().isEmpty())
            allBlank = false;

        lines.append( block.text() );
        block = block.next();
    }

    if(allBlank) {
        return true;
    }

    return false;
}

void AnnotationEdit::highlightLine(GraphicsAnnotationItem *item)
{
    bool highlighted = false;
    if (item != nullptr)
    {
       int blockNumber = m_itemToBlockMap[item];
       QTextDocument *document = m_textEdit->document();
       for (QTextBlock block = document->begin(); block != document->end(); block = block.next())
       {
           if (block.blockNumber() == blockNumber)
           {
               m_textEdit->highlightCurrentLine(block,true);
               highlighted = true;
               break;
           }
       }
    }
   if (!highlighted)
   {
       m_textEdit->clearHighlight();
       GraphicsAnnotationItem::setHighlight(nullptr);
   }}

} // namespace codetextedit

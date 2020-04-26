/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#ifndef AnnotationEdit_H
#define AnnotationEdit_H

#include <QString>
#include <QStringList>
#include <QFont>
#include <QWidget>
#include <QDialog>
#include <QRadioButton>
#include <QSplitter>
#include <QTextEdit>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTimer>

#include "CodeTextHighlighter.h"
#include "Annotation.h"
#include "AnnotationTextEdit.h"
#include "AnnotationGraphicsView.h"

namespace codetextedit
{
    class AnnotationWorker;
    class GraphicsAnnotationItem;
    class AnnotationGraphicsView;
    ///
    /// \brief The AnnotationDialog class
    ///
    class AnnotationDialog : public QDialog
    {
        Q_OBJECT
    public:
        AnnotationDialog(const AnnotationContainer&, QWidget* parent);
    };

    ///
    /// \brief The AnnotationEdit class
    ///
    class AnnotationEdit : public QWidget
    {
        Q_OBJECT

    public:
        AnnotationEdit(Annotator* annotator, CodeTextHighlighter* highlighter, QWidget *parent = nullptr);
        virtual ~AnnotationEdit();

        void loadFile(QString filePath);
        void setContents(QString contents);
        QString toPlainText();
        void setPlainText(QString text);
    signals:
        void textChanged();

    protected:
        void resizeEvent(QResizeEvent *) override;
        void mousePressEvent(QMouseEvent *) override;

    private slots:
        void updateAnnotations();
        void refreshAnnotations();
        void rebuildAnnotations(AnnotationMap annotations);
        void synchronizeSceneWithDocument();

        void textEditScrollBarChanged(int);
        void graphicsViewScrollBarChanged(int);
        void highlightLine(GraphicsAnnotationItem*);
    private:
        void showPopup(GraphicsAnnotationItem*);
        QString priorityMessage(const AnnotationContainer&, int&);
        int textWidth(const QString&) const;
        int longestWidth(const AnnotationContainer&) const;
        void deleteAll();
        bool extractLines(QTextDocument* document, QStringList& lines);

        CodeTextHighlighter*    m_highlighter = nullptr;
        QTimer                  m_annotationRefreshTimer;
        Annotator*              m_annotator = nullptr;
        AnnotationMap           m_annotationMap;
        AnnotationWorker*       m_annotationWorker = nullptr;

        QSplitter*              m_splitter;
        QGraphicsScene*         m_graphicsScene;
        AnnotationGraphicsView*          m_graphicsView;
        AnnotationTextEdit*     m_textEdit;
        QWidget*                m_empty;
        QWidget*                m_gvContainer;

        GraphicsAnnotationItem* m_currentItem = nullptr;
        QMap<GraphicsAnnotationItem*, AnnotationContainer> m_itemMap;
        QMap<int,GraphicsAnnotationItem*> m_blockToItemMap;
        QMap<GraphicsAnnotationItem*,int> m_itemToBlockMap;

        friend GraphicsAnnotationItem;
    };



} // namespace codetextedit

#endif // AnnotationEdit_H

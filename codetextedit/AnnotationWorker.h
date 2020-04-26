/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#ifndef ANNOTATIONWORKER_H
#define ANNOTATIONWORKER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "Annotation.h"


namespace codetextedit {

///
/// \brief Worker class which handles annotation updates
///
class AnnotationWorker : public QThread
{
    Q_OBJECT

public:
    AnnotationWorker(Annotator* annotator, QObject *parent = nullptr);
    ~AnnotationWorker() override;

    void kill();
    void analyze(QStringList lines);

signals:
    void analyzed(AnnotationMap annotations);

protected:
    void run() override;

private:
    Annotator*      annotator;

    QMutex          mutex;
    QWaitCondition  condition;
    bool            restart = false;
    bool            abort = false;
    bool            killLoop = false;

    QStringList     lines;
    AnnotationMap   result;
};

} // namespace codetextedit

#endif // ANNOTATIONWORKER_H

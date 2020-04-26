/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#include <QTextDocument>
#include <QTextBlock>
#include <QDebug>

#include "AnnotationWorker.h"

namespace codetextedit {

AnnotationWorker::AnnotationWorker(Annotator *annotator, QObject *parent)
    : QThread(parent)
    , annotator(annotator)
{
    qRegisterMetaType<AnnotationMap>("AnnotationMap");
}

AnnotationWorker::~AnnotationWorker()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void AnnotationWorker::kill()
{
    mutex.lock();
    killLoop = true;
    condition.wakeOne();
    mutex.unlock();
}

void AnnotationWorker::analyze(QStringList lines)
{
    QMutexLocker locker(&mutex);

    this->lines = lines;

    if (!isRunning()) {
        start(LowPriority);
    }
    else {
        restart = true;
        condition.wakeOne();
    }

}

void AnnotationWorker::run()
{
    forever {
        mutex.lock();
        QStringList lines = this->lines;
        mutex.unlock();

        if(lines.size() > 0) {

            annotator->prepareAnalysis(lines);
            while(annotator->analyzeStep()) {
                if(restart) {
                    break;
                }
            }

            if(! restart) {
                emit analyzed(annotator->analysisResult());
            }
        }

        mutex.lock();
        if (!restart)
            condition.wait(&mutex);

        if(killLoop) {
            mutex.unlock();

            exit();
            return;
        }

        restart = false;
        mutex.unlock();
    }
}


} // namespace codetextedit

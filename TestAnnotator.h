#ifndef TESTANNOTATOR_H
#define TESTANNOTATOR_H

#include <QList>
#include <QMap>
#include <QTextDocument>
#include "codetextedit/Annotation.h"


using namespace codetextedit;



class TestAnnotator : public QObject, public codetextedit::Annotator
{
public:
    explicit TestAnnotator(QObject *parent = nullptr);
    virtual ~TestAnnotator() = default;

    void prepareAnalysis(QStringList lines) override;
    bool analyzeStep() override;
    AnnotationMap analysisResult() override;

    void scanLine(int lineNum, QString line);


private:
    QStringList m_lines;
    int m_currentLine;
    AnnotationMap m_annotationMap;
};

#endif // TESTANNOTATOR_H

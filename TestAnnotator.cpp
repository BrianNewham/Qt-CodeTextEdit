#include <QTextDocument>
#include <QTextBlock>

#include "TestAnnotator.h"

using codetextedit::Annotation;

TestAnnotator::TestAnnotator(QObject* parent) : QObject(parent)
{
}

void TestAnnotator::prepareAnalysis(QStringList lines)
{
    m_lines = lines;
    m_currentLine = 0;
}

bool TestAnnotator::analyzeStep()
{
    QString line = m_lines[m_currentLine];
    scanLine(m_currentLine, line);

    ++ m_currentLine;

    bool hasMore = m_currentLine < m_lines.size();
    return hasMore;
}

AnnotationMap TestAnnotator::analysisResult()
{
    auto copy = m_annotationMap;
    m_annotationMap.clear();
    return copy;
}

void TestAnnotator::scanLine(int lineNum, QString line)
{
    QStringList list = line.split(',');
    AnnotationContainer container;

    if (list[0]=="XX")
    {
        Annotation *annotation = new Annotation;
        annotation->setCategory(Annotation::CATEGORY_Unspecified);
        annotation->setSolutionHelp("This is an XX Error 1");
        annotation->setMessage("This is an XX message 1");
        annotation->setAlertColor("green");
        container.append(annotation);

        annotation = new Annotation;
        annotation->setCategory(Annotation::CATEGORY_Hint);
        annotation->setSolutionHelp("This is an XX command 2");
        annotation->setMessage("This is an XX message 2  to be or not to be");
        annotation->setAlertColor("#FF00FF");
        container.append(annotation);

        annotation = new Annotation;
        annotation->setCategory(Annotation::CATEGORY_Warning);
        annotation->setSolutionHelp("This is an XX command 3");
        annotation->setMessage("This is an XX message 3");
        annotation->setAlertColor("blue");
        container.append(annotation);

        annotation = new Annotation;
        annotation->setCategory(Annotation::CATEGORY_Error);
        annotation->setSolutionHelp("This is an XX command 4");
        annotation->setMessage("This is an XX message 4");
        annotation->setAlertColor("red");
        container.append(annotation);
    }

    if (list[0]=="ZZ")
    {
        Annotation *annotation = new Annotation;
        annotation->setCategory(Annotation::CATEGORY_Unspecified);
        annotation->setSolutionHelp("This is an ZZ Error 1");
        annotation->setMessage("This is an ZZ message 1");
        annotation->setAlertColor("yellow");
        container.append(annotation);

        annotation = new Annotation;
        annotation->setCategory(Annotation::CATEGORY_Hint);
        annotation->setSolutionHelp("This is an ZZ command 2");
        annotation->setMessage("This is an ZZ message 2  to be or not to be");
        annotation->setAlertColor("magenta");
        container.append(annotation);

        annotation = new Annotation;
        annotation->setCategory(Annotation::CATEGORY_Warning);
        annotation->setSolutionHelp("This is an ZZ command 3");
        annotation->setMessage("This is an ZZ message 3");
        annotation->setAlertColor("#00FF00");
        container.append(annotation);

        annotation = new Annotation;
        annotation->setCategory(Annotation::CATEGORY_Error);
        annotation->setSolutionHelp("This is an ZZ command 4");
        annotation->setMessage("This is an ZZ message 4");
        annotation->setAlertColor("red");
        container.append(annotation);
    }

    else if (list[0]=="YY")
    {
        Annotation *annotation = new Annotation;

        annotation->setCategory(Annotation::CATEGORY_Hint);
        annotation->setSolutionHelp("This is a YY command");
        annotation->setMessage("This is a YY message");
        annotation->setAlertColor("blue");
        container.append(annotation);
    }
    m_annotationMap[ lineNum ] = container;
}

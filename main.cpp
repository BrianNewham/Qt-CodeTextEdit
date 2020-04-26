#include "codetextedit/AnnotationEdit.h"
#include "TestAnnotator.h"

#include <QApplication>
#include <QMainWindow>

using namespace codetextedit;

Keywords TestKeywords_0 = {
"1.0",
{ "XX", "YY", },
{ "AA" , "BB" , "CC", "DD", "EE" },
{ "LABEL" }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/SourceCodePro-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/SourceCodePro-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/SourceCodePro-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/SourceCodePro-BoldItalic.ttf");

    TestAnnotator *annotator = new TestAnnotator;
    CodeTextHighlighter *highlighter = new CodeTextHighlighter;
    highlighter->setKeywords(&TestKeywords_0);

    AnnotationEdit *editor = new AnnotationEdit(annotator, highlighter);
    editor->setContents(
        "XX,1   AA,1,2,4\n"
        "YY,4   BB,1,2,8\n");
    editor->show();
    editor->setGeometry(50, 50, 800, 500);

    auto status = app.exec();

    delete editor;
    delete annotator;
    delete highlighter;

    return status;
}


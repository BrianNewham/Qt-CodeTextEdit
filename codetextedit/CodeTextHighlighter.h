/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#ifndef CODETEXTHIGHLIGHTER_H
#define CODETEXTHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextDocument>

namespace codetextedit {

struct Keywords
{
    QString version;
    QStringList controlCommands;
    QStringList deviceCommands;
    QStringList goodLabelNames;
};

class CodeTextHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CodeTextHighlighter(QTextDocument *parent = nullptr);

    void setKeywords(Keywords* keywords);

protected:
    void highlightBlock(const QString &line) override;

    void highlightPart(const QRegularExpressionMatch& match, int cap, QTextCharFormat format);

    void highlightControlCommand(const QString& line, int start, int length);
    void highlightControlCommand(const QRegularExpressionMatch& match, int cap);
    void highlightControlParams(const QString& line, int start, int length);
    void highlightControlParams(const QRegularExpressionMatch& match, int cap);
    void highlightDeviceCommand(const QString& line, int start, int length);
    void highlightDeviceCommand(const QRegularExpressionMatch& match, int cap);
    void highlightDeviceParams(const QString& line, int start, int length);
    void highlightDeviceParams(const QRegularExpressionMatch& match, int cap);
    void highlightLabelTag(const QRegularExpressionMatch& match, int cap);

    bool matchCommandFullMulti(const QString& line);
    bool matchCommandFull(const QString& line);
    bool matchCommandPartial(const QString& line);
    bool matchDeclaration(const QString& line);
    bool matchLabelTag(const QString& line);

private:
    QTextCharFormat formatBad;

    QTextCharFormat formatDeclarationKey;
    QTextCharFormat formatDeclarationValue;

    QTextCharFormat formatControlCommandOk;
    QTextCharFormat formatControlParams;
    QTextCharFormat formatDeviceCommandOk;
    QTextCharFormat formatDeviceParams;

    QTextCharFormat formatLabelTag;

protected:
    Keywords* languageKeywords = nullptr;
};

} // namespace codetextedit

#endif // CODETEXTHIGHLIGHTER_H

/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#include "CodeTextHighlighter.h"

#include <QDebug>

namespace codetextedit {

CodeTextHighlighter::CodeTextHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    formatDeclarationKey.setFontWeight(QFont::Bold);
    formatDeclarationKey.setForeground(QColor(0xB390C2));

    formatDeclarationValue.setFontWeight(QFont::Bold);
    formatDeclarationValue.setForeground(QColor(0xE0A145));

    formatBad.setFontWeight(QFont::Bold);
    formatBad.setForeground(QColor(0xD9402A));

    formatControlCommandOk.setFontWeight(QFont::Bold);
    formatControlCommandOk.setForeground(QColor(0x5E9BBF));

    formatControlParams.setForeground(QColor(0xB7A7AD));

    formatDeviceCommandOk.setFontWeight(QFont::Bold);
    formatDeviceCommandOk.setForeground(QColor(0x99CC6E));

    formatDeviceParams.setForeground(QColor(0x6B5A60));

    formatLabelTag.setFontWeight(QFont::Bold);
    formatLabelTag.setForeground(QColor(0xC67BD6));
    formatLabelTag.setBackground(QColor(0xC67BD6).lighter());
}

void CodeTextHighlighter::setKeywords(Keywords *keywords)
{
    languageKeywords = keywords;
}

void CodeTextHighlighter::highlightBlock(const QString &line)
{
    if(matchLabelTag(line)) return;
    if(matchCommandFullMulti(line)) return;
    if(matchCommandFull(line)) return;
    if(matchCommandPartial(line)) return;
    if(matchDeclaration(line)) return;
}

void CodeTextHighlighter::highlightPart(const QRegularExpressionMatch &match, int cap, QTextCharFormat format)
{
    setFormat(match.capturedStart(cap),
              match.capturedLength(cap),
              format);
}

void CodeTextHighlighter::highlightControlCommand(const QString &line, int start, int length)
{
    if(length == 0)
        return;

    auto controlCmd = line.mid(start, length);
    bool goodCommand = languageKeywords && languageKeywords->controlCommands.contains(controlCmd);

    setFormat(start,
              length,
              goodCommand  ?  formatControlCommandOk : formatBad);
}

void CodeTextHighlighter::highlightControlCommand(const QRegularExpressionMatch &match, int cap)
{
    if(match.capturedLength(cap) == 0)
        return;

    auto controlCmd = match.captured(cap);
    bool goodCommand = languageKeywords && languageKeywords->controlCommands.contains(controlCmd);

    setFormat(match.capturedStart(cap),
              match.capturedLength(cap),
              goodCommand  ?  formatControlCommandOk : formatBad);
}

void CodeTextHighlighter::highlightControlParams(const QString &line, int start, int length)
{
    setFormat(start,
              length,
              formatControlParams);
}

void CodeTextHighlighter::highlightControlParams(const QRegularExpressionMatch &match, int cap)
{
    setFormat(match.capturedStart(cap),
              match.capturedLength(cap),
              formatControlParams);

}

void CodeTextHighlighter::highlightDeviceCommand(const QString &line, int start, int length)
{
    if(length == 0)
        return;

    auto controlCmd = line.mid(start, length);
    bool goodCommand = languageKeywords && languageKeywords->deviceCommands.contains(controlCmd);

    setFormat(start,
              length,
              goodCommand  ?  formatDeviceCommandOk : formatBad);
}

void CodeTextHighlighter::highlightDeviceCommand(const QRegularExpressionMatch &match, int cap)
{
    if(match.capturedLength(cap) == 0)
        return;

    auto deviceCmd = match.captured(cap);
    bool goodCommand = languageKeywords && languageKeywords->deviceCommands.contains(deviceCmd);

    setFormat(match.capturedStart(cap),
              match.capturedLength(cap),
              goodCommand  ?  formatDeviceCommandOk : formatBad);
}

void CodeTextHighlighter::highlightDeviceParams(const QString &, int start, int length)
{
    setFormat(start,
              length,
              formatDeviceParams);
}

void CodeTextHighlighter::highlightDeviceParams(const QRegularExpressionMatch &match, int cap)
{
    setFormat(match.capturedStart(cap),
              match.capturedLength(cap),
              formatDeviceParams);
}

void CodeTextHighlighter::highlightLabelTag(const QRegularExpressionMatch &match, int cap)
{
    auto labelName = match.captured(cap);

    setFormat(match.capturedStart(cap),
              match.capturedLength(cap),
              formatLabelTag);
}

bool CodeTextHighlighter::matchCommandFullMulti(const QString &line)
{
    // Matches a control code plus an sequence of device commands separated by a separator...

    auto pattern = QRegularExpression(R"____(^(\w+)(,\d+)?\s+([\w,;0-9]+)?(\s+//.+)?)____", QRegularExpression::CaseInsensitiveOption);
    auto match = pattern.match(line);

    if(match.hasMatch() && match.capturedStart() == 0) {

        highlightControlCommand(match, 1);
        highlightControlParams(match, 2);

        auto cpbRow = match.captured(3);
        if(! cpbRow.contains(';'))
            return false;

        int cpbStart = match.capturedStart(3);
        int lastEnd = 0;

        auto reCPB = QRegularExpression(R"____((\w+)([,0-9MPX]+)?)____", QRegularExpression::CaseInsensitiveOption);

        QRegularExpressionMatchIterator i = reCPB.globalMatch(cpbRow);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();

            int pos = match.capturedStart();
            int len = match.capturedLength();
            int cmdPos = match.capturedStart(1);
            int cmdLen = match.capturedLength(1);
            int paramPos = match.capturedStart(2);
            int paramLen = match.capturedLength(2);

            if(match.capturedStart() > 0) {

                if(lastEnd + 1  !=  pos) { } // Syntax error: Separator is not a ;
                if(cpbRow[pos - 1] != ';') { } // Syntax error: Separator is not ;
            }

            highlightDeviceCommand(line, cpbStart + cmdPos, cmdLen);
            highlightDeviceParams(line, cpbStart + paramPos, paramLen);

            lastEnd = pos + len;
        }

        return true;
    }

    return false;
}

bool CodeTextHighlighter::matchCommandFull(const QString &line)
{
    // Matches a control code plus an optional device code ...

    auto pattern = QRegularExpression(R"____(^(\w+)(,\d+)?\s+(\w+)([,0-9MPX]+)?(\s+//.+)?)____", QRegularExpression::CaseInsensitiveOption);
    auto match = pattern.match(line);
    if(match.hasMatch() && match.capturedStart() == 0) {

        highlightControlCommand(match, 1);
        highlightControlParams(match, 2);
        highlightDeviceCommand(match, 3);
        highlightDeviceParams(match, 4);

        return true;
    }
    else {
        return false;
    }
}

bool CodeTextHighlighter::matchCommandPartial(const QString &line)
{
    // Matches a control code only ...

    auto pattern = QRegularExpression(R"____((\w+)(,\d+)?)____", QRegularExpression::CaseInsensitiveOption);
    auto match = pattern.match(line);
    if(match.hasMatch() && match.capturedStart() == 0) {

        highlightControlCommand(match, 1);
        highlightControlParams(match, 2);

        return true;
    }
    else {
        return false;
    }
}

bool CodeTextHighlighter::matchDeclaration(const QString &line)
{
    auto pattern = QRegularExpression(R"____(#\s*(\w+)(\s*=\s*(\w+))?)____", QRegularExpression::CaseInsensitiveOption);
    auto match = pattern.match(line);
    if(match.hasMatch() && match.capturedStart() == 0) {

        highlightPart(match, 1, formatDeclarationKey);
        highlightPart(match, 3, formatDeclarationValue);

        return true;
    }
    else {
        return false;
    }
}

bool CodeTextHighlighter::matchLabelTag(const QString &line)
{
    // Matches a label,number ...

    auto pattern = QRegularExpression(R"____((\w+),\d+)____", QRegularExpression::CaseInsensitiveOption);
    auto match = pattern.match(line);
    if(match.hasMatch() && match.capturedStart() == 0) {

        if(languageKeywords && ! languageKeywords->goodLabelNames.contains(match.captured(1)))
            return false;

        highlightLabelTag(match, 0);

        return true;
    }
    else {
        return false;
    }
}

} // namespace codetextedit


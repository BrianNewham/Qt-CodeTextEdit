/***********************************************************************
The CodeTextEditor provides an editor showing live hints and annotations.

Copyright (c) 2020 Brian Newham <seaweedsolutionsltd@gmail.com>.

CodeTextEdit is free software dual licensed under the GNU LGPL or MIT License.
***********************************************************************/

#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QTextDocument>

namespace codetextedit
{
    class Annotation
    {
    public:
        enum Category
        {
            CATEGORY_Unspecified                  = 0,
            CATEGORY_Hint                         = 1,
            CATEGORY_Warning                      = 2,
            CATEGORY_Error                        = 3,
        };

        // Content data
    private:
        Category    m_category = CATEGORY_Unspecified;
        QColor      m_alertColor;
        QString     m_message;
        QString     m_solutionHelp;

    public:
        Category    category() const {return m_category;}
        QColor      alertColor() const {return m_alertColor;}
        QString     message() const {return m_message;}
        QString     solutionHelp() const {return m_solutionHelp;}

        void setCategory(Category category) {m_category=category;}
        void setAlertColor(const QColor& color) {m_alertColor = color;}
        void setMessage(const QString& message) {m_message = message;}
        void setSolutionHelp(const QString& help) {m_solutionHelp=help;}

        Annotation() = default;
        virtual ~Annotation() = default;
    };

    using LineNumber = int;
    using AnnotationContainer = QList<Annotation*>;
    using AnnotationMap = QMap<LineNumber, AnnotationContainer>;

    class Annotator
    {
    public:
        explicit Annotator() {}
        virtual ~Annotator() {}

        /// Copies the source to a local variable
        virtual void prepareAnalysis(QStringList lines) = 0;

        /// True if finished
        virtual bool analyzeStep() = 0;

        /// Retrieve the result. Transfers ownership of all heap allocated objects to caller.
        virtual AnnotationMap analysisResult() = 0;

    protected:
    };

} // namespace codetextedit

#endif // ANNOTATION_H

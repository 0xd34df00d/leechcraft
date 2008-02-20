#ifndef DOCUMENTGENERATOR_H
#define DOCUMENTGENERATOR_H
#include <QDomDocument>
#include <QDomElement>
#include <QVariantList>

class QString;

class DocumentGenerator
{
    static QDomDocument Document_;
public:
    enum InputType
    {
        TypeText
        , TypeFile
    };
    static QString GetStylesheet ();
    static QDomDocument GetDocument ();

    static void CreateDocument ();
    static void CreateHead (const QString&);
    static void AddAutorefresh (int);
    static QDomElement CreateLink (const QString&, const QString&);
    static QDomElement CreateText (const QString& = QString ());
    static QDomElement CreateInlineText (const QString&);
    static QDomElement CreateHeading (const QString&, int);
    static QDomElement CreateDefaultHeading ();
    static QDomElement CreateTable ();
    static QDomElement CreateRow (const QVariantList&);
    static QDomElement CreateForm (const QString&, bool);
    static QDomElement CreateInputField (InputType, const QString&);
    static QDomElement CreateSubmitButton (const QString&);
    static void ApplyStyle (QDomElement& node, const QString&);
    static void ApplyClass (QDomElement& node, const QString&);
};

#endif


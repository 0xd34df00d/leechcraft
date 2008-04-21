#include <QFile>
#include <QString>
#include <QtDebug>
#include "documentgenerator.h"

QDomDocument DocumentGenerator::Document_;

QString DocumentGenerator::GetStylesheet ()
{
    QFile sheet (":/defaultstylesheet.css");
    sheet.open (QIODevice::ReadOnly);
    return sheet.readAll ();
}

QDomDocument DocumentGenerator::GetDocument ()
{
    return Document_;
}

void DocumentGenerator::CreateDocument ()
{
    Document_ = QDomDocument ("html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\"");
    QDomElement html = Document_.createElement ("html");
    html.setAttribute ("xmlns", "http://www.w3.org/1999/xhtml");
    html.setAttribute ("xml:lang", "en");
    Document_.appendChild (html);
    Document_.documentElement ().appendChild (Document_.createElement ("head"));
    Document_.documentElement ().appendChild (Document_.createElement ("body"));
}

void DocumentGenerator::CreateHead (const QString& t)
{
    QDomNode head = Document_.elementsByTagName ("head").at (0);
    QDomElement title = Document_.createElement ("title"),
                cssRel = Document_.createElement ("link");
    head.appendChild (title);
    head.appendChild (cssRel);
    cssRel.setAttribute ("rel", "stylesheet");
    cssRel.setAttribute ("type", "text/css");
    cssRel.setAttribute ("href", "/resources/stylesheet");
    title.appendChild (Document_.createTextNode (t));
}

void DocumentGenerator::AddAutorefresh (int value)
{
    QDomElement body = Document_.elementsByTagName ("body").at (0).toElement ();
    body.setAttribute ("onLoad", QString ("setTimeout ('window.location.reload ()', %1)").arg (value * 1000));
}

QDomElement DocumentGenerator::CreateLink (const QString& where, const QString& text)
{
    QDomElement result = Document_.createElement ("a");
    result.appendChild (Document_.createTextNode (text));
    result.setAttribute ("href", where);
    return result;
}

QDomElement DocumentGenerator::CreateText (const QString& text)
{
    QDomElement result = Document_.createElement ("div");
    result.appendChild (Document_.createTextNode (text));
    return result;
}

QDomElement DocumentGenerator::CreateInlineText (const QString& text)
{
    QDomElement result = Document_.createElement ("span");
    result.appendChild (Document_.createTextNode (text));
    return result;
}

QDomElement DocumentGenerator::CreateHeading (const QString& text, int level)
{
    QDomElement result = Document_.createElement ("h" + QString::number (level));
    result.appendChild (Document_.createTextNode (text));
    return result;
}

QDomElement DocumentGenerator::CreateDefaultHeading ()
{
    QDomElement result = Document_.createElement ("div"),
                header = CreateHeading ("LeechCraft Remoter deep alpha", 2),
                mainlink = CreateLink ("/", "Main page"),
                image = Document_.createElement ("img");
    result.appendChild (header);
    result.appendChild (mainlink);
    return result;
}

QDomElement DocumentGenerator::CreateTable ()
{
    QDomElement result = Document_.createElement ("table");
    return result;
}

QDomElement DocumentGenerator::CreateRow (const QVariantList& list)
{
    QDomElement tr = Document_.createElement ("tr");
    for (int i = 0; i < list.size (); ++i)
    {
        QDomElement td = Document_.createElement ("td");
        td.appendChild (Document_.createTextNode (list.at (i).toString ()));
        tr.appendChild (td);
    }
    return tr;
}

QDomElement DocumentGenerator::CreateForm (const QString& where)
{
    QDomElement form = Document_.createElement ("form");
    form.setAttribute ("action", where);
    form.setAttribute ("method", "post");
    form.setAttribute ("enctype", "multipart/form-data");
    return form;
}

QDomElement DocumentGenerator::CreateInputField (DocumentGenerator::InputType type, const QString& name)
{
    QDomElement result = Document_.createElement ("input");
    QString t;
    switch (type)
    {
        case TypeText:
            t = "text";
            break;
        case TypeFile:
            t = "file";
            break;
        case TypeTextbox:
            result = Document_.createElement ("textarea");
            result.appendChild (Document_.createTextNode (""));
            result.setAttribute ("cols", QString::number (50));
            result.setAttribute ("rows", QString::number (10));
            result.setAttribute ("name", name);
            return result;
    }
    result.setAttribute ("type", t);
    result.setAttribute ("name", name);
    return result;
}

QDomElement DocumentGenerator::CreateSubmitButton (const QString& text)
{
    QDomElement elem = Document_.createElement ("input");
    elem.setAttribute ("name", text);
    elem.setAttribute ("type", "submit");
    return elem;
}

void DocumentGenerator::ApplyStyle (QDomElement& node, const QString& style)
{
    node.setAttribute ("style", style);
}

void DocumentGenerator::ApplyClass (QDomElement& node, const QString& cl)
{
    node.setAttribute ("class", cl);
}


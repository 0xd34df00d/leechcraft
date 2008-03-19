#include "parser.h"

// Via
// http://www.theukwebdesigncompany.com/articles/entity-escape-characters.php
QString Parser::UnescapeHTML (const QString& escaped) const
{
    QString result = escaped;
    result.replace ("&euro;", "€");
    result.replace ("&quot;", "\"");
    result.replace ("&amp;", "&");
    result.replace ("&nbsp;", " ");
    result.replace ("&lt;", "<");
    result.replace ("&gt;", ">");
    return result;
}


#ifndef STYLES_P_H
#define STYLES_P_H

#endif // STYLES_P_H

#include <QSharedData>
#include <QHash>


namespace Calligra{
namespace Sheets{


/////////////////////////////////////////////////////////////////////////////
//
// Style::Private
//
/////////////////////////////////////////////////////////////////////////////

class StylePrivate : public QSharedData
{
public:
    QHash<Key, SharedSubStyle> subStyles;
};


/////////////////////////////////////////////////////////////////////////////
//
// CustomStyle::Private
//
/////////////////////////////////////////////////////////////////////////////

class CustomStylePrivate : public QSharedData
{
public:
    QString name;
    StyleType type;
};
}
}

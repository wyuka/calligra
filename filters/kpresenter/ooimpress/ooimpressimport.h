/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef OoImpress_IMPORT_H__
#define OoImpress_IMPORT_H__

#include <koFilter.h>

#include <qdom.h>
#include <qdict.h>
#include <qptrlist.h>

/**
 *  This class implements a stack for the different styles of an object.
 *
 *  There can be several styles that are valid for one object. For example
 *  a page has a style 'dp1' and a textobject on that page has styles 'pr3'
 *  and 'P7' and a paragraph in that textobject has styles 'P1' and 'T3'.
 *
 *  If you want to know if there is, for example,  the attribute 'fo:font-family'
 *  for this paragraph, you have to look into style 'T3', 'P1', 'P7' and 'dp1'.
 *  When you find this attribute in one style you have to stop processing the list
 *  and take the found attribute for this object.
 *
 *  This is what this class does. You can push styles on the stack while walking
 *  through the xml-tree to your object and then ask the stack if any of the styles
 *  provides a certain attribute. The stack will search from top to bottom, i.e.
 *  in our example from 'T3' to 'dp1' and return the first occurrence of the wanted
 *  attribute. If it cannot find the attribute in the stack then it will look for
 *  attribute in the default-style (if set) and return it.
 *
 *  So this is some sort of inheritance where the styles on top of the stack overwrite
 *  the same attribute of a lower style on the stack.
 */
class StyleStack
{
public:
    StyleStack();
    virtual ~StyleStack();

    /**
     * Clears the complete stack.
     */
    void clear();

    /**
     * Remove only the styles 'above' the mark. If no mark is set, clear the whole stack.
     * This is used to keep the styles of an object on the stack and remove only the
     * style of that objects child-objects.
     */
    void clearMark();

    /**
     * Set the mark (stores the index of the object on top of the stack).
     */
    void setMark();

    /**
     * Removes the style on top of the stack.
     */
    void pop();

    /**
     * Pushs the new style onto the stack.
     */
    void push(const QDomElement* style);

    /**
     * Check if any of the styles on the stack has an attribute called 'name'.
     */
    bool hasAttribute(const QString& name);

    /**
     * Search for the attribute called 'name', starting on top of the stack,
     * and return it.
     */
    QString attribute(const QString& name);

private:
    uint m_mark;

    // We use QPtrList instead of QPtrStack because we need access to all styles
    // not only the top one.
    QPtrList<QDomElement> m_stack;
};

class OoImpressImport : public KoFilter
{
    Q_OBJECT
public:
    OoImpressImport( KoFilter * parent, const char * name, const QStringList & );
    virtual ~OoImpressImport();

    virtual KoFilter::ConversionStatus convert( QCString const & from, QCString const & to );

private:
    void createDocumentInfo( QDomDocument &docinfo );
    void createDocumentContent( QDomDocument &doccontent );
    void createStyleMap( QDomDocument &docstyles );
    void insertStyles( const QDomElement& styles );
    void fillStyleStack( const QDomElement& object );
    QDomElement parseObject( QDomDocument& doc, const QDomElement& object, int offset );
    QDomElement parseLineObject( QDomDocument& doc, const QDomElement& object, int offset );
    QDomElement parseTextBox( QDomDocument& doc, const QDomElement& textBox );
    QDomElement parseList( QDomDocument& doc, const QDomElement& paragraph );
    QDomElement parseParagraph( QDomDocument& doc, const QDomElement& list );
    KoFilter::ConversionStatus openFile();

    QDomDocument    m_content;
    QDomDocument    m_meta;
    QDomDocument    m_settings;
    QDict<QDomElement> m_styles;
    StyleStack m_styleStack;
};

#endif


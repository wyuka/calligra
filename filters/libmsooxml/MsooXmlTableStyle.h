/* This file is part of the KDE project
 * Copyright (C) 2010-2011 Carlos Licea <carlos@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef MSOOXMLTABLESTYLE_H
#define MSOOXMLTABLESTYLE_H

#include "msooxml_export.h"

#include <KoCellStyle.h>
#include <KoBorder.h>

#include <QColor>
#include <QFlags>
#include <QMap>

namespace MSOOXML {

/// Reading and storage

struct MSOOXML_EXPORT TableStyleProperties
{
    enum Property {
        BottomBorder = 1,
        InsideHBorder = 2,
        InsideVBorder = 4,
        LeftBorder = 8,
        RightBorder = 16,
        Tl2brBorder = 32,
        TopBorder = 64,
        Tr2blBorder = 128,
        BackgroundColor = 256,
        TopMargin = 512,
        RightMargin = 1024,
        BottomMargin = 2048,
        LeftMargin = 4096
    };
    Q_DECLARE_FLAGS(Properties, Property)
    Properties setProperties;

    KoBorder::BorderData bottom;
    KoBorder::BorderData insideH;
    KoBorder::BorderData insideV;
    KoBorder::BorderData left;
    KoBorder::BorderData right;
    KoBorder::BorderData tl2br;
    KoBorder::BorderData top;
    KoBorder::BorderData tr2bl;

    QColor backgroundColor;

    qreal topMargin;
    qreal rightMargin;
    qreal bottomMargin;
    qreal leftMargin;
};

class MSOOXML_EXPORT TableStyle
{
public:

    TableStyle();
    virtual ~TableStyle();

    void setId(const QString& id);
    QString id() const;

private:
    QString m_id;
};

/// Instantiation classes

class MSOOXML_EXPORT LocalTableStyles
{
public:
    LocalTableStyles();
    ~LocalTableStyles();

    TableStyleProperties* localStyle(int row, int column);
    void setLocalStyle(MSOOXML::TableStyleProperties* properties, int row, int column);

private:
    QMap<QPair<int,int>, TableStyleProperties*> m_properties;
};

class MSOOXML_EXPORT TableStyleConverterProperties
{
public:
    TableStyleConverterProperties();
    virtual ~TableStyleConverterProperties();

    void setRowCount(int rowCount);
    int rowCount() const;

    void setColumnCount(int columnCount);
    int columnCount() const;

    void setRowBandSize(int size);
    int rowBandSize() const;

    void setColumnBandSize(int size);
    int columnBandSize() const;

    ///LocalStyles is a collection of cell<->style relationships
    void setLocalStyles(const LocalTableStyles& localStyles);
    LocalTableStyles localStyles() const;

    ///LocalTableStyle is a style defined to be the default style of a table. Defined locally.
    void setLocalDefaulCelltStyle(MSOOXML::TableStyleProperties* properties);
    TableStyleProperties* localDefaultCellStyle() const;

private:
    int m_rowCount;
    int m_columnCount;
    int m_rowBandSize;
    int m_columnBandSize;
    LocalTableStyles m_localStyles;
    MSOOXML::TableStyleProperties* m_localDefaultCellStyle;
};

class MSOOXML_EXPORT TableStyleConverter
{
public:
    TableStyleConverter(int row, int column);
    virtual ~TableStyleConverter();

    virtual KoCellStyle::Ptr style(int row, int column) = 0;

protected:
    void applyStyle(MSOOXML::TableStyleProperties* styleProperties, KoCellStyle::Ptr& style, int row, int column);

private:
    void applyBordersStyle(MSOOXML::TableStyleProperties* styleProperties, KoCellStyle::Ptr& style, int row, int column);
    void applyBackground(MSOOXML::TableStyleProperties* styleProperties, KoCellStyle::Ptr& style, int row, int column);

    int m_row;
    int m_column;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(MSOOXML::TableStyleProperties::Properties)

#endif
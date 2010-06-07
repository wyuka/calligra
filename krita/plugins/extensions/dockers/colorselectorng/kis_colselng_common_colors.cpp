/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_colselng_common_colors.h"
#include <cmath>
#include <QPixmap>
#include <QHash>
#include <QImage>
#include <QList>
#include <QColor>
#include <QPainter>
#include <QResizeEvent>

#include <QDebug>
#include <iostream>


KisColSelNgCommonColors::KisColSelNgCommonColors(QWidget *parent) :
    KisColSelNgColorPatches(parent), m_numColors(30), m_patchWidth(25), m_patchHeight(25)
{
    m_extractedColors = extractColors();
    setColors(m_extractedColors);
    setPatchLayout(Horizontal, false, 2);
//    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

//void KisColSelNgCommonColors::paintEvent(QPaintEvent *)
//{
//    QPainter painter(this);
//
//    int widgetWidth = width();
//    int numPatchesInARow = widgetWidth/m_patchWidth;
//
//    for(int i=0; i<m_extractedColors.size(); i++) {
//        int row = i/numPatchesInARow;
//        int col = i%numPatchesInARow;
//        painter.fillRect(col*m_patchWidth, row*m_patchHeight, m_patchWidth, m_patchHeight, m_extractedColors.at(i));
//    }
//}
//


enum ColorAxis {RedAxis=0, GreenAxis, BlueAxis};

class Color
{
public:
    Color(QRgb rgb) : r(qRed(rgb)), g(qGreen(rgb)), b(qBlue(rgb)) {}
    unsigned char r;
    unsigned char g;
    unsigned char b;
    inline unsigned char operator[](ColorAxis i) const
    {
        if(i==RedAxis) return r;
        if(i==GreenAxis) return g;
        return b;
    }
};

class VBox
{
    QList<Color> m_colors;
public:
    VBox(QList<QRgb> rgbList)
    {
        QList<Color> colorList;
        for(int i=0; i<rgbList.size(); i++) {
            colorList.append(Color(rgbList.at(i)));
        }
        m_colors = colorList;
    }

    VBox(QList<Color> colorList) : m_colors(colorList) {}

    int population() const { return m_colors.size(); }

    VBox divide()
    {
        ColorAxis axis = biggestAxis();
        Q_ASSERT(axisSize(axis)>=3);
        unsigned char divpos = divPos(axis);
        QList<Color> newVBoxColors;
        for(int i=m_colors.size()-1; i>=0; i--) {
            Color c = m_colors.at(i);
            if(c[axis]>divpos) {
                m_colors.removeAt(i);
                newVBoxColors.append(c);
            }
        }

        return VBox(newVBoxColors);
    }

    QRgb mean() const
    {
        int r=0;
        int g=0;
        int b=0;
        for(int i=0;i<m_colors.size(); i++) {
            r+=(int) m_colors.at(i)[RedAxis];
            g+=(int) m_colors.at(i)[GreenAxis];
            b+=(int) m_colors.at(i)[BlueAxis];
        }
        int size = m_colors.size();
        Q_ASSERT(size>0);
//        qDebug()<<size;

        return qRgb(r/size, g/size, b/size);
    }

    unsigned char axisSize(ColorAxis axis) const
    {
        unsigned char valMin = 255;
        unsigned char valMax = 0;
        for(int i=0; i<m_colors.size(); i++) {
            if(m_colors.at(i)[axis]>valMax)
                valMax=m_colors.at(i)[axis];
            if(m_colors.at(i)[axis]<valMin)
                valMin=m_colors.at(i)[axis];
        }
        return valMax-valMin;
    }

    ColorAxis biggestAxis() const
    {
        unsigned char sR = axisSize(RedAxis);
        unsigned char sG = axisSize(GreenAxis);
        unsigned char sB = axisSize(BlueAxis);
        if(sR>sG && sR>sB) return RedAxis;
        if(sG>sR && sG>sB) return GreenAxis;
        return BlueAxis;
    }

private:
//    unsigned char divPos(ColorAxis axis) const
//    {
//        QList<unsigned char> values;
//        for(int i=0;i<m_colors.size(); i++) {
//            values.append(m_colors.at(i)[axis]);
//        }
//        qSort(values);
//        return values.at(values.size()*4/5);
//    }
    unsigned char divPos(ColorAxis axis) const
    {
        short min=m_colors.at(0)[axis];
        short max=m_colors.at(0)[axis];
        for(int i=0;i<m_colors.size(); i++) {
            if(min>m_colors.at(i)[axis]) min=m_colors.at(i)[axis];
            if(max<m_colors.at(i)[axis]) max=m_colors.at(i)[axis];
        }

        return (min+max)/2;
    }
};


QList<QColor> KisColSelNgCommonColors::extractColors()
{
    QList<QRgb> colors = getColors();
    if(colors.size()<m_numColors) return QList<QColor>();

    VBox startBox(colors);
    QList<VBox> boxes;
    boxes.append(startBox);

    while (boxes.size()<m_numColors*3/5) {
        int biggestBox=-1;
        int biggestBoxPopulation=-1;

        for(int i=0; i<boxes.size(); i++) {
            if(boxes.at(i).population()>biggestBoxPopulation) {
                if(boxes.at(i).axisSize(boxes.at(i).biggestAxis())>10) {
                    biggestBox=i;
                    biggestBoxPopulation=boxes.at(i).population();
                }
            }
        }

        VBox newBox = boxes[biggestBox].divide();
        boxes.append(newBox);
    }


    while (boxes.size()<m_numColors) {
        int biggestBox=-1;
        int biggestBoxAxisSize=-1;

        for(int i=0; i<boxes.size(); i++) {
            if(boxes.at(i).axisSize(boxes.at(i).biggestAxis())>biggestBoxAxisSize) {
                biggestBox=i;
                biggestBoxAxisSize=boxes.at(i).axisSize(boxes.at(i).biggestAxis());
            }
        }

        VBox newBox = boxes[biggestBox].divide();
        boxes.append(newBox);
    }

    QList<QColor> colorList;
    for(int i=0; i<boxes.size(); i++) {
        colorList.append(QColor(boxes.at(i).mean()));
    }

    return colorList;
}

QList<QRgb> KisColSelNgCommonColors::getColors()
{
    QPixmap pixmap("/home/damdam/Pictures/backgrounds/mare.jpg");
//    QPixmap pixmap("/home/damdam/progn/kde/ImageColorsExtractor/testimgs/sanduhr.jpg");
    int width = pixmap.width();
    int height = pixmap.height();

    int pixelCount = height*width;
    if(pixelCount> (1<<16)) {
        qreal factor = sqrt((1<<16)/(qreal) pixelCount);
        pixmap=pixmap.scaledToWidth(width*factor);
        width=pixmap.width();
        height=pixmap.height();
    }

    QImage image = pixmap.toImage();
    QSet<QRgb> colorList;

    for (int i=0; i<width; i++) {
        for (int j=0; j<height; j++) {
//            if(!colorList.contains(image.pixel(i, j)))
                colorList.insert(image.pixel(i, j));
        }
    }

    return colorList.toList();
}
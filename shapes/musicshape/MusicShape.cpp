/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <m.kruiselbrink@student.tue.nl>
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
#include "MusicShape.h"
#include <QPainter>
#include <kdebug.h>
#include <KoViewConverter.h>

#include "core/Sheet.h"
#include "core/Part.h"
#include "core/Voice.h"
#include "core/Staff.h"
#include "core/VoiceBar.h"
#include "core/Chord.h"
#include "core/Note.h"
#include "core/Clef.h"
#include "core/Bar.h"

#include "MusicStyle.h"
#include "Engraver.h"

using namespace MusicCore;

// helper method, used by the constructor to easily create a short piece of music
static Chord* mkNote(Chord::Duration duration, Staff* staff, int pitch)
{
    Chord* c = new Chord(duration);
    c->addNote(staff, pitch);
    return c;
}

MusicShape::MusicShape() : m_engraver(new Engraver())
{
    m_sheet = new Sheet();
    Part* part = m_sheet->addPart("Piano");
    Staff* staff = part->addStaff();
    Staff* staff2 = part->addStaff();
    Voice* voice = part->addVoice();
    Voice* voice2 = part->addVoice();
    Bar* b1 = m_sheet->addBar();
    Bar* b2 = m_sheet->addBar();
    Bar* b3 = m_sheet->addBar();

    voice->bar(b1)->addElement(new Clef(staff, Clef::Trebble, 2, 0));
    voice->bar(b1)->addElement(mkNote(Chord::Quarter, staff, 0));
    voice->bar(b1)->addElement(mkNote(Chord::Quarter, staff, 1));
    voice->bar(b1)->addElement(mkNote(Chord::Quarter, staff, 2));
    voice->bar(b1)->addElement(mkNote(Chord::Quarter, staff, 0));
    voice->bar(b2)->addElement(mkNote(Chord::Quarter, staff, 0));
    voice->bar(b2)->addElement(mkNote(Chord::Quarter, staff, 1));
    voice->bar(b2)->addElement(mkNote(Chord::Quarter, staff, 2));
    voice->bar(b2)->addElement(mkNote(Chord::Quarter, staff, 0));
    voice->bar(b3)->addElement(mkNote(Chord::Quarter, staff, 2));
    voice->bar(b3)->addElement(mkNote(Chord::Quarter, staff, 3));
    voice->bar(b3)->addElement(mkNote(Chord::Half, staff, 4));
    voice2->bar(b1)->addElement(new Clef(staff2, Clef::Bass, 3, 0));
    voice2->bar(b1)->addElement(new Chord(staff2, Chord::Whole));
    voice2->bar(b2)->addElement(new Chord(staff2, Chord::Half));
    voice2->bar(b2)->addElement(new Chord(staff2, Chord::Quarter));
    voice2->bar(b2)->addElement(new Chord(staff2, Chord::Eighth));
    voice2->bar(b2)->addElement(new Chord(staff2, Chord::Sixteenth));
    voice2->bar(b2)->addElement(new Chord(staff2, Chord::Sixteenth));
    voice2->bar(b3)->addElement(mkNote(Chord::Quarter, staff2, 0));
    voice2->bar(b3)->addElement(mkNote(Chord::Quarter, staff2, 1));
    voice2->bar(b3)->addElement(mkNote(Chord::Quarter, staff2, 2));
    voice2->bar(b3)->addElement(mkNote(Chord::Quarter, staff2, 0));
    m_engraver->engraveSheet(m_sheet);

    m_style = new MusicStyle();
}

MusicShape::~MusicShape()
{
    delete m_sheet;
    delete m_style;
}

void MusicShape::resize( const QSizeF &newSize )
{
//  kDebug()<<" MusicShape::resize( const QSizeF &newSize ) " << newSize << endl;
    KoShape::resize(newSize);
}

static void paintStaff( QPainter& painter, MusicStyle* style, Staff *staff )
{
    double dy = staff->lineSpacing();
    double y = staff->top();
    painter.setPen(style->staffLinePen());
    for (int i = 0; i < staff->lineCount(); i++) {
        painter.drawLine(QPointF(0.0, y + i * dy), QPointF(1000.0, y + i * dy));
    }
}

static void paintChord( QPainter& painter, MusicStyle* style, Chord* chord, double x, Clef* clef )
{
    x = x + chord->x();
    if (chord->noteCount() == 0) { // a rest
        Staff *s = chord->staff();
        style->renderRest( painter, x, s->top() + (2 - (chord->duration() == Chord::Whole)) * s->lineSpacing(), chord->duration() );
        return;
    }
    Note *n = chord->note(0);
    Staff * s = n->staff();
    int line = 14;
    if (clef && clef->shape() == Clef::FClef) line = 4;
    if (clef) {
        line -= 2*clef->line();
    } else {
        line -= 4;
    }
    line = line - n->pitch();
    if (line > 9) { // lines under the bar
        painter.setPen(style->staffLinePen());
        for (int i = 10; i <= line; i+= 2) {
            double y = s->top() + i * s->lineSpacing() / 2;
            painter.drawLine(QPointF(x - 4, y), QPointF(x + 15, y));
        }
    } else if (line < -1) { // lines above the bar
        painter.setPen(style->staffLinePen());
        for (int i = -2; i >= line; i-= 2) {
            double y = s->top() + i * s->lineSpacing() / 2;
            painter.drawLine(QPointF(x - 4, y), QPointF(x + 15, y));
        }
    }

    double stemLen = -7;
    double stemX = x + 11;
    if (line < 4) { stemLen = 7; stemX = x; }
    painter.setPen(style->stemPen());
    painter.drawLine(QPointF(stemX, chord->y() + s->top() + line * s->lineSpacing() / 2),
                     QPointF(stemX, chord->y() + s->top() + (line + stemLen) * s->lineSpacing() / 2));
    style->renderNoteHead( painter, x, chord->y() + s->top() + line * s->lineSpacing() / 2, chord->duration() );
    x += 30;
}

static void paintClef( QPainter& painter, MusicStyle* style, Clef *c, double x )
{
    Staff* s = c->staff();
    style->renderClef( painter, x + c->x(), s->top() + (s->lineCount() - c->line()) * s->lineSpacing(), c->shape());
}

static void paintVoice( QPainter& painter, MusicStyle* style, Voice *voice )
{
    Clef* curClef = 0;
    double x = 0;
    for (int b = 0; b < voice->part()->sheet()->barCount(); b++) {
        VoiceBar* vb = voice->bar(voice->part()->sheet()->bar(b));
        for (int e = 0; e < vb->elementCount(); e++) {
            MusicElement *me = vb->element(e);
            Chord *c = dynamic_cast<Chord*>(me);
            if (c) paintChord( painter, style, c, x, curClef );
            Clef *cl = dynamic_cast<Clef*>(me);
            if (cl) {
                paintClef( painter, style, cl, x );
                curClef = cl;
            }
        }
        x += voice->part()->sheet()->bar(b)->size();
    }
}

static void paintPart( QPainter& painter, MusicStyle* style, Part *part )
{
    for (int i = 0; i < part->staffCount(); i++) {
        paintStaff(painter, style, part->staff(i));
    }
    double firstStaff = part->staff(0)->top();
    int c = part->staffCount()-1;
    double lastStaff = part->staff(c)->top() + part->staff(c)->lineSpacing() * (part->staff(c)->lineCount()-1);
    double x = 0;
    for (int b = 0; b < part->sheet()->barCount(); b++) {
        x += part->sheet()->bar(b)->size();
        painter.drawLine(QPointF(x, firstStaff), QPointF(x, lastStaff));
    }
    for (int i = 0; i < part->voiceCount(); i++) {
        paintVoice(painter, style, part->voice(i));
    }
}

static void paintSheet( QPainter& painter, MusicStyle* style, Sheet *sheet )
{
    for (int i = 0; i < sheet->partCount(); i++) {
        paintPart(painter, style, sheet->part(i));
    }
}

void MusicShape::paint( QPainter& painter, const KoViewConverter& converter )
{
//  kDebug()<<" MusicShape::paint( QPainter& painter, const KoViewConverter& converter )\n";

    applyConversion( painter, converter );

    painter.setClipping(true);
    painter.setClipRect(QRectF(0, 0, size().width(), size().height()));

    paintSheet( painter, m_style, m_sheet );
}


void MusicShape::saveOdf( KoShapeSavingContext * context )
{
    // TODO
}

/* This file is part of the KDE project
 * Copyright (C) 2007-2009,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008-2010 Thorsten Zachmann <zachmann@kde.org>
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

#include "StyleDocker.h"
#include "StylePreview.h"
#include "StyleButtonBox.h"

#include <KoFlake.h>
#include <KoGradientBackground.h>
#include <KoPageApp.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceManager.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoShapeStroke.h>
#include <KoShapeStrokeCommand.h>
#include <KoShapeBackgroundCommand.h>
#include <KoPathFillRuleCommand.h>
#include <KoShapeTransparencyCommand.h>
#include <KoPathShape.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorBackground.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>
#include <KoShapeController.h>
#include <KoResourceSelector.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>
#include <KoColorPopupAction.h>
#include <KoSliderCombo.h>

#include <klocale.h>

#include <QGridLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QLabel>

const int MsecsThresholdForMergingCommands = 2000;

StyleButtonBox::StyleButtons StrokeButtons = StyleButtonBox::None|StyleButtonBox::Solid|StyleButtonBox::Gradient;

StyleButtonBox::StyleButtons FillButtons = StyleButtonBox::None|StyleButtonBox::Solid|StyleButtonBox::Gradient|StyleButtonBox::Pattern;

StyleButtonBox::StyleButtons FillRuleButtons = StyleButtonBox::EvenOdd|StyleButtonBox::Winding;

StyleDocker::StyleDocker(QWidget * parent)
    : QDockWidget(parent)
    , m_canvas(0)
    , m_lastFillCommand(0)
    , m_lastStrokeCommand(0)
    , m_lastColorFill(0)
{
    setWindowTitle(i18n("Stroke and Fill"));

    QWidget *mainWidget = new QWidget(this);

    m_preview = new StylePreview(mainWidget);

    m_buttons = new StyleButtonBox(mainWidget, 2, 3);
    m_buttons->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_stack = new QStackedWidget(mainWidget);
    m_stack->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_opacity = new KoSliderCombo(mainWidget);
    m_opacity->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_opacity->setMinimum(0);
    m_opacity->setMaximum(100);
    m_opacity->setValue(100);
    m_opacity->setDecimals(0);

    m_spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_layout = new QGridLayout(mainWidget);
    m_layout->addWidget(m_preview, 0, 0, 2, 1);
    m_layout->addWidget(m_buttons, 0, 1, 2, 1);
    m_layout->addWidget(m_stack, 0, 2, 1, 2);
    m_layout->addWidget(new QLabel(i18n("Opacity:")), 1, 2);
    m_layout->addWidget(m_opacity, 1, 3);
    m_layout->addItem(m_spacer, 2, 2);
    m_layout->setMargin(0);
    m_layout->setVerticalSpacing(0);
    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    m_colorSelector = new QToolButton(m_stack);
    m_actionColor = new KoColorPopupAction(m_stack);
    m_colorSelector->setDefaultAction(m_actionColor);

    KoResourceServerProvider * serverProvider = KoResourceServerProvider::instance();
    KoAbstractResourceServerAdapter * gradientResourceAdapter = new KoResourceServerAdapter<KoAbstractGradient>(serverProvider->gradientServer(), this);
    KoResourceSelector * gradientSelector = new KoResourceSelector(gradientResourceAdapter, m_stack);
    gradientSelector->setColumnCount(1);
    gradientSelector->setRowHeight(20);
    gradientSelector->setMinimumWidth(100);

    KoAbstractResourceServerAdapter * patternResourceAdapter = new KoResourceServerAdapter<KoPattern>(serverProvider->patternServer(), this);
    KoResourceSelector * patternSelector = new KoResourceSelector(patternResourceAdapter, m_stack);
    patternSelector->setColumnCount(5);
    patternSelector->setRowHeight(30);
    patternSelector->setMinimumWidth(100);

    m_stack->addWidget(m_colorSelector);
    m_stack->addWidget(gradientSelector);
    m_stack->addWidget(patternSelector);
    m_stack->setContentsMargins(0, 0, 0, 0);
    m_stack->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_stack->setMinimumWidth(100);

    connect(m_preview, SIGNAL(fillSelected()), this, SLOT(fillSelected()));
    connect(m_preview, SIGNAL(strokeSelected()), this, SLOT(strokeSelected()));
    connect(m_buttons, SIGNAL(buttonPressed(int)), this, SLOT(styleButtonPressed(int)));
    connect(m_actionColor, SIGNAL(colorChanged(const KoColor &)),
             this, SLOT(updateColor(const KoColor &)));
    connect(gradientSelector, SIGNAL(resourceSelected(KoResource*)),
             this, SLOT(updateGradient(KoResource*)));
    connect(gradientSelector, SIGNAL(resourceApplied(KoResource*)),
             this, SLOT(updateGradient(KoResource*)));
    connect(patternSelector, SIGNAL(resourceSelected(KoResource*)),
             this, SLOT(updatePattern(KoResource*)));
    connect(patternSelector, SIGNAL(resourceApplied(KoResource*)),
             this, SLOT(updatePattern(KoResource*)));
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
             this, SLOT(locationChanged(Qt::DockWidgetArea)));
    connect(m_opacity, SIGNAL(valueChanged(qreal, bool)), this, SLOT(updateOpacity(qreal)));

    setWidget(mainWidget);
}

StyleDocker::~StyleDocker()
{
}

void StyleDocker::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this); // "Every connection you make emits a signal, so duplicate connections emit two signals"
    }

    resetColorCommands();

    m_canvas = canvas;
    if (! m_canvas) {
        return;
    }

    connect(m_canvas->shapeManager(), SIGNAL(selectionChanged()),
            this, SLOT(selectionChanged()));
    connect(m_canvas->shapeManager(), SIGNAL(selectionContentChanged()),
            this, SLOT(selectionContentChanged()));
    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
             this, SLOT(resourceChanged(int, const QVariant&)));

    KoShape * shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    if (shape)
        updateStyle(shape->stroke(), shape->background());
    else {
        KoShape* page = m_canvas->resourceManager()->koShapeResource(KoPageApp::CurrentPage);
        if (page) {
            updateStyle(page->stroke(), page->background());
        }
        else {
            updateStyle(0, 0);
        }
    }
}

void StyleDocker::selectionChanged()
{
    resetColorCommands();
    updateStyle();
}

void StyleDocker::resetColorCommands()
{
    m_lastFillCommand = 0;
    m_lastStrokeCommand = 0;
    m_lastColorFill = 0;
    m_lastColorStrokes.clear();
}

void StyleDocker::selectionContentChanged()
{
    updateStyle();
}

void StyleDocker::updateStyle()
{
    if (! m_canvas)
        return;

    m_opacity->blockSignals(true);
    KoShape * shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    if (shape) {
        updateStyle(shape->stroke(), shape->background());
        m_opacity->setValue(100-shape->transparency()*100);
    }
    else {
        updateStyle(0, 0);
        m_opacity->setValue(100);
    }
    m_opacity->blockSignals(false);
}

void StyleDocker::updateStyle(KoShapeStrokeModel * stroke, KoShapeBackground * fill)
{
    if (! m_canvas)
        return;

    KoCanvasResourceManager * provider = m_canvas->resourceManager();
    int activeStyle = provider->resource(KoCanvasResourceManager::ActiveStyleType).toInt();

    QColor qColor;
    if (activeStyle == KoFlake::Foreground) {
        KoShapeStroke * stroke2 = dynamic_cast<KoShapeStroke*>(stroke);
        if (stroke)
            qColor = stroke2->color();
        else
            qColor = m_canvas->resourceManager()->foregroundColor().toQColor();
    }
    else {
        KoColorBackground * background = dynamic_cast<KoColorBackground*>(fill);
        if (background)
            qColor = background->color();
        else
            qColor = m_canvas->resourceManager()->backgroundColor().toQColor();
    }
    m_actionColor->setCurrentColor(qColor);
    updateStyleButtons(activeStyle);
    m_preview->update(stroke, fill);
}

void StyleDocker::fillSelected()
{
    if (! m_canvas)
        return;

    m_canvas->resourceManager()->setResource(KoCanvasResourceManager::ActiveStyleType, KoFlake::Background);
    updateStyleButtons(KoFlake::Background);
}

void StyleDocker::strokeSelected()
{
    if (! m_canvas)
        return;

    m_canvas->resourceManager()->setResource(KoCanvasResourceManager::ActiveStyleType, KoFlake::Foreground);
    updateStyleButtons(KoFlake::Foreground);
}

void StyleDocker::resourceChanged(int key, const QVariant&)
{
    switch (key) {
        case KoCanvasResourceManager::ForegroundColor:
        case KoCanvasResourceManager::BackgroundColor:
            updateStyle();
            break;
    }
}

void StyleDocker::styleButtonPressed(int buttonId)
{
    if (! m_canvas)
        return;

    switch (buttonId) {
        case StyleButtonBox::None:
        {
            resetColorCommands();

            KoCanvasResourceManager * provider = m_canvas->resourceManager();
            KoSelection *selection = m_canvas->shapeManager()->selection();
            if (! selection || ! selection->count())
                break;

            if (provider->resource(KoCanvasResourceManager::ActiveStyleType).toInt() == KoFlake::Background)
                m_canvas->addCommand(new KoShapeBackgroundCommand(selection->selectedShapes(), 0));
            else
                m_canvas->addCommand(new KoShapeStrokeCommand(selection->selectedShapes(), 0));
            m_stack->setCurrentIndex(0);
            updateStyle();
            break;
        }
        case StyleButtonBox::Solid:
            m_stack->setCurrentIndex(0);
            break;
        case StyleButtonBox::Gradient:
            m_stack->setCurrentIndex(1);
            break;
        case StyleButtonBox::Pattern:
            m_stack->setCurrentIndex(2);
            break;
        case StyleButtonBox::EvenOdd:
            updateFillRule(Qt::OddEvenFill);
            break;
        case StyleButtonBox::Winding:
            updateFillRule(Qt::WindingFill);
            break;
    }
}

void StyleDocker::updateColor(const KoColor &c)
{
    if (! m_canvas)
        return;

    KoSelection *selection = m_canvas->shapeManager()->selection();
    if (! selection || ! selection->count()) {
        KoShape* page = m_canvas->resourceManager()->koShapeResource(KoPageApp::CurrentPage);
        if (page) {
            QList<KoShape*> shapes;
            shapes.append(page);
            updateColor(c.toQColor(), shapes);
        }
        else {
            KoCanvasResourceManager * provider = m_canvas->resourceManager();
            int activeStyle = provider->resource(KoCanvasResourceManager::ActiveStyleType).toInt();

            if (activeStyle == KoFlake::Foreground)
                m_canvas->resourceManager()->setForegroundColor(c);
            else
                m_canvas->resourceManager()->setBackgroundColor(c);
        }
    }
    else {
        updateColor(c.toQColor(), selection->selectedShapes());
        updateStyle();
    }
}

void StyleDocker::updateColor(const QColor &c, const QList<KoShape*> & selectedShapes)
{
    if (! m_canvas)
        return;

    Q_ASSERT(! selectedShapes.isEmpty() );

    KoColor kocolor(c, KoColorSpaceRegistry::instance()->rgb8());

    KoCanvasResourceManager * provider = m_canvas->resourceManager();
    int activeStyle = provider->resource(KoCanvasResourceManager::ActiveStyleType).toInt();

    // check which color to set foreground == border, background == fill
    if (activeStyle == KoFlake::Foreground) {
        if (m_lastColorChange.msecsTo(QTime::currentTime()) > MsecsThresholdForMergingCommands) {
            m_lastColorStrokes.clear();
            m_lastStrokeCommand = 0;
        }
        if (m_lastColorStrokes.count() && m_lastStrokeCommand) {
            foreach(KoShapeStrokeModel * stroke, m_lastColorStrokes) {
                KoShapeStroke * lineStroke = dynamic_cast<KoShapeStroke*>(stroke);
                if (lineStroke)
                    lineStroke->setColor(c);
            }
            m_lastStrokeCommand->redo();
        }
        else {
            m_lastColorStrokes.clear();
            QList<KoShape *>::const_iterator it(selectedShapes.begin());
            for (;it != selectedShapes.end(); ++it) {
                // get the stroke of the first selected shape and check if it is a line stroke
                KoShapeStroke * oldStroke = dynamic_cast<KoShapeStroke*>((*it)->stroke());
                KoShapeStroke * newStroke = 0;
                if (oldStroke) {
                    // preserve the properties of the old stroke if it is a line stroke
                    newStroke = new KoShapeStroke(*oldStroke);
                    newStroke->setLineBrush(QBrush());
                    newStroke->setColor(c);
                }
                else {
                    newStroke = new KoShapeStroke(1.0, c);
                }
                m_lastColorStrokes.append(newStroke);
            }

            m_lastStrokeCommand = new KoShapeStrokeCommand(selectedShapes, m_lastColorStrokes);
            m_canvas->addCommand(m_lastStrokeCommand);
        }
        m_lastColorChange = QTime::currentTime();
        m_canvas->resourceManager()->setForegroundColor(kocolor);
    }
    else {
        if (m_lastColorChange.msecsTo(QTime::currentTime()) > MsecsThresholdForMergingCommands) {
            m_lastColorFill = 0;
            m_lastFillCommand = 0;
        }
        if (m_lastColorFill && m_lastFillCommand) {
            m_lastColorFill->setColor(c);
            m_lastFillCommand->redo();
        }
        else {
            m_lastColorFill = new KoColorBackground(c);
            m_lastFillCommand = new KoShapeBackgroundCommand(selectedShapes, m_lastColorFill);
            m_canvas->addCommand(m_lastFillCommand);
        }
        m_lastColorChange = QTime::currentTime();
        m_canvas->resourceManager()->setBackgroundColor(kocolor);
    }
}

void StyleDocker::updateGradient(KoResource * item)
{
    if (! m_canvas)
        return;

    resetColorCommands();

    KoAbstractGradient * gradient = dynamic_cast<KoAbstractGradient*>(item);
    if (! gradient)
        return;

    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes();
    if (selectedShapes.isEmpty()) {
        KoShape* page = m_canvas->resourceManager()->koShapeResource(KoPageApp::CurrentPage);
        if (page) {
            selectedShapes.append(page);
        }
        else {
            return;
        }
    }

    QGradient * newGradient = gradient->toQGradient();
    if (! newGradient)
        return;

    QGradientStops newStops = newGradient->stops();
    delete newGradient;

    KoCanvasResourceManager * provider = m_canvas->resourceManager();
    int activeStyle = provider->resource(KoCanvasResourceManager::ActiveStyleType).toInt();

    // check which color to set foreground == stroke, background == fill
    if (activeStyle == KoFlake::Background) {
        KUndo2Command * firstCommand = 0;
        foreach (KoShape * shape, selectedShapes) {
            KoShapeBackground * fill = applyFillGradientStops(shape, newStops);
            if (! fill)
                continue;
            if (! firstCommand)
                firstCommand = new KoShapeBackgroundCommand(shape, fill);
            else
                new KoShapeBackgroundCommand(shape, fill, firstCommand);
        }
        m_canvas->addCommand(firstCommand);
    }
    else {
        QList<KoShapeStrokeModel*> newStrokes;
        foreach (KoShape * shape, selectedShapes) {
            QBrush brush = applyStrokeGradientStops(shape, newStops);
            if (brush.style() == Qt::NoBrush)
                continue;

            KoShapeStroke * stroke = dynamic_cast<KoShapeStroke*>(shape->stroke());
            KoShapeStroke * newStroke = 0;
            if (stroke)
                newStroke = new KoShapeStroke(*stroke);
            else
                newStroke = new KoShapeStroke(1.0);
            newStroke->setLineBrush(brush);
            newStrokes.append(newStroke);
        }
        m_canvas->addCommand(new KoShapeStrokeCommand(selectedShapes, newStrokes));
    }
    updateStyle();
}

void StyleDocker::updatePattern(KoResource * item)
{
    if (! m_canvas)
        return;

    resetColorCommands();

    KoPattern * pattern = dynamic_cast<KoPattern*>(item);
    if (! pattern)
        return;

    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes();
    if (selectedShapes.isEmpty()) {
        KoShape* page = m_canvas->resourceManager()->koShapeResource(KoPageApp::CurrentPage);
        if (page) {
            selectedShapes.append(page);
        }
        else {
            return;
        }
    }

    KoImageCollection *imageCollection = m_canvas->shapeController()->resourceManager()->imageCollection();
    if (imageCollection) {
        KoPatternBackground * fill = new KoPatternBackground(imageCollection);
        fill->setPattern(pattern->image());
        m_canvas->addCommand(new KoShapeBackgroundCommand(selectedShapes, fill ));
        updateStyle();
    }
}

void StyleDocker::updateFillRule(Qt::FillRule fillRule)
{
    if (! m_canvas)
        return;

    KoSelection *selection = m_canvas->shapeManager()->selection();
    if (! selection || ! selection->count())
        return;

    QList<KoPathShape*> selectedPaths = selectedPathShapes();
    QList<KoPathShape*> pathsToChange;
    foreach (KoPathShape * path, selectedPaths) {
        if (path->fillRule() != fillRule)
            pathsToChange.append(path);
    }
    if (pathsToChange.count())
        m_canvas->addCommand(new KoPathFillRuleCommand(pathsToChange, fillRule));
}

void StyleDocker::updateOpacity(qreal opacity)
{
    if (! m_canvas)
        return;

    KoSelection *selection = m_canvas->shapeManager()->selection();
    if (! selection || ! selection->count())
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes(KoFlake::TopLevelSelection);
    if (!selectedShapes.count())
        return;

    m_canvas->addCommand(new KoShapeTransparencyCommand(selectedShapes, 1.0-opacity/100));
}

QList<KoPathShape*> StyleDocker::selectedPathShapes()
{
    QList<KoPathShape*> pathShapes;

    if (! m_canvas)
        return pathShapes;

    KoSelection *selection = m_canvas->shapeManager()->selection();
    if (! selection || ! selection->count())
        return pathShapes;

    foreach (KoShape * shape, selection->selectedShapes()) {
        KoPathShape * path = dynamic_cast<KoPathShape*>(shape);
        if (path)
            pathShapes.append(path);
    }

    return pathShapes;
}

void StyleDocker::updateStyleButtons(int activeStyle)
{
    if (activeStyle == KoFlake::Background) {
        if (selectedPathShapes().count())
            m_buttons->showButtons(FillButtons|FillRuleButtons);
        else
            m_buttons->showButtons(FillButtons);
    }
    else {
        m_buttons->showButtons(StrokeButtons);
        if (m_stack->currentIndex() == 2)
            m_stack->setCurrentIndex(0);
    }
}

void StyleDocker::locationChanged(Qt::DockWidgetArea area)
{
    switch (area) {
        case Qt::TopDockWidgetArea:
        case Qt::BottomDockWidgetArea:
            m_spacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
            break;
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
            m_spacer->changeSize(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            break;
        default:
            break;
    }
    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_layout->invalidate();
}

KoShapeBackground *StyleDocker::applyFillGradientStops(KoShape *shape, const QGradientStops &stops)
{
    if (! shape || ! stops.count())
        return 0;

    KoGradientBackground *newGradient = 0;
    KoGradientBackground *oldGradient = dynamic_cast<KoGradientBackground*>(shape->background());
    if (oldGradient) {
        // just copy the gradient and set the new stops
        QGradient *g = KoFlake::cloneGradient(oldGradient->gradient());
        g->setStops(stops);
        newGradient = new KoGradientBackground(g);
        newGradient->setTransform(oldGradient->transform());
    }
    else {
        // no gradient yet, so create a new one
        QLinearGradient *g = new QLinearGradient(QPointF(0, 0), QPointF(1, 1));
        g->setCoordinateMode(QGradient::ObjectBoundingMode);
        g->setStops(stops);
        newGradient = new KoGradientBackground(g);
    }
    return newGradient;
}

QBrush StyleDocker::applyStrokeGradientStops(KoShape *shape, const QGradientStops &stops)
{
    if (! shape || ! stops.count())
        return QBrush();

    QBrush gradientBrush;
    KoShapeStroke *stroke = dynamic_cast<KoShapeStroke*>(shape->stroke());
    if (stroke)
        gradientBrush = stroke->lineBrush();

    QGradient *newGradient = 0;
    const QGradient *oldGradient = gradientBrush.gradient();
    if (oldGradient) {
        // just copy the new gradient stops
        newGradient = KoFlake::cloneGradient(oldGradient);
        newGradient->setStops(stops);
    }
    else {
        // no gradient yet, so create a new one
        QLinearGradient *g = new QLinearGradient(QPointF(0, 0), QPointF(1, 1));
        g->setCoordinateMode(QGradient::ObjectBoundingMode);
        g->setStops(stops);
        newGradient = g;
    }

    QBrush brush(*newGradient);
    delete newGradient;

    return brush;
}

#include <StyleDocker.moc>

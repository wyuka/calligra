/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PAINTOP_H_
#define KIS_PAINTOP_H_

#include <qstring.h>

#include <ksharedptr.h>
#include <klocale.h>

#include "kis_types.h"
#include "kis_id.h"
#include "kis_vec.h"

#include <koffice_export.h>

class KisPoint;
class KisAlphaMask;
class KisPainter;
class KisPaintBox;

/**
 * This class keeps information that can be used in the painting process, for example by
 * bruses.
 **/
class KRITACORE_EXPORT KisPaintInformation {
public:
    KisPaintInformation(double pressure = PRESSURE_DEFAULT,
                        double xTilt = 0.0, double yTilt = 0.0,
                        KisVector2D movement = KisVector2D())
        : pressure(pressure), xTilt(xTilt), yTilt(yTilt), movement(movement) {}
    double pressure;
    double xTilt;
    double yTilt;
    KisVector2D movement;
};

class KRITACORE_EXPORT KisPaintOp : public KShared
{

public:

        KisPaintOp(KisPainter * painter);
    virtual ~KisPaintOp();

    virtual void paintAt(const KisPoint &pos, const KisPaintInformation& info) = 0;
    void setSource(KisPaintDeviceImplSP p);

    /**
     * Whether this paintop wants to deposit paint even when not moving, i.e. the
     * tool needs to activate its timer.
     */
    virtual bool incremental() { return false; }
    
protected:

    virtual KisLayerSP computeDab(KisAlphaMaskSP mask);


    /**
     * Split the coordinate into whole + fraction, where fraction is always >= 0.
     */
    virtual void splitCoordinate(double coordinate, Q_INT32 *whole, double *fraction);

    KisPainter * m_painter;
    KisPaintDeviceImplSP m_source; // use this layer as source layer for the operation
};

class KisPaintOpFactory  : public KShared
{

public:
    KisPaintOpFactory() {};
    virtual ~KisPaintOpFactory() {};

    virtual KisPaintOp * createOp(KisPainter * painter) = 0;
    virtual KisID id() { return KisID("abstractpaintop", i18n("Abstract PaintOp")); }

    /**
     * The filename of the pixmap we can use to represent this paintop in the ui.
     */
    virtual QString pixmap() { return ""; };
    
    /**
     * Whether this paintop is internal to a certain tool or can be used
     * in various tools. If false, it won't show up in the toolchest.
     */
    virtual bool userVisible() { return true; }

    /**
     * Slot the paint op into the relevant toolbox, if so desired. It's
     * up to the paintop to decide whether it want to so something with this
     */
    virtual void slot(KisPaintBox * box) { Q_UNUSED(box); };
};
#endif // KIS_PAINTOP_H_

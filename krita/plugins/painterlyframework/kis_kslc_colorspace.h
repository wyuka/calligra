/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#ifndef KIS_KSLC_COLORSPACE_H_
#define KIS_KSLC_COLORSPACE_H_

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace.h"

#include "kis_rgb_to_kslc_color_conversion_transformation.h"
#include "kis_ks_to_rgb_color_conversion_transformation.h"

template< typename _TYPE_, quint32 _N_ >
class KisKSLCColorSpace : public KisKSColorSpace< _TYPE_,_N_ >
{
    typedef KisKSColorSpace< _TYPE_,_N_ > parent;

    public:

        KisKSLCColorSpace(KoColorProfile *p) : parent(p, ColorSpaceId().id(), ColorSpaceId().name()) {}
        ~KisKSLCColorSpace() {}

        KoColorSpace *clone() const
        {
            return new KisKSLCColorSpace<_TYPE_,_N_>(parent::profile()->clone());
        }

        KoID colorModelId() const
        {
            return ColorModelId();
        }

    public: // static

        static KoID ColorSpaceId()
        {
            return KoID(ColorModelId().id()+parent::ColorDepthId().id(),
                        ColorModelId().name()+" ("+parent::ColorDepthId().name()+")");
        }

        static KoID ColorModelId()
        {
            QByteArray name = QString("Painterly Color Space LC, precision %1").arg(_N_).toUtf8();
            return KoID("KSLC"+QString::number(_N_), i18n(name.data()));
        }

};

template< typename _TYPE_, quint32 _N_ >
class KisKSLCColorSpaceFactory : public KoColorSpaceFactory
{
    public:
        QString id() const { return KisKSLCColorSpace<_TYPE_,_N_>::ColorSpaceId().id(); }
        QString name() const { return KisKSLCColorSpace<_TYPE_,_N_>::ColorSpaceId().name(); }
        KoID colorModelId() const { return KisKSLCColorSpace<_TYPE_,_N_>::ColorModelId(); }
        KoID colorDepthId() const { return KisKSColorSpace<_TYPE_,_N_>::ColorDepthId(); }
        bool userVisible() const { return _N_>=9; }

        int referenceDepth() const { return sizeof(_TYPE_)*8; }
        bool isIcc() const { return false; }
        bool isHdr() const { return false; }

        QList<KoColorConversionTransformationFactory*> colorConversionLinks() const
        {
            QList<KoColorConversionTransformationFactory*> list;

            // RGB to KS
            list.append(new KisRGBToKSLCColorConversionTransformationFactory<_TYPE_,_N_>);
            // KS to RGB
            list.append(new KisKSToRGBColorConversionTransformationFactory<_TYPE_,_N_>("LC"));

            return list;
        }

        KoColorConversionTransformationFactory *createICCColorConversionTransformationFactory(QString _colorModelId, QString _colorDepthId) const
        {
            Q_UNUSED(_colorModelId);
            Q_UNUSED(_colorDepthId);
            return 0;
        }

        KoColorSpace *createColorSpace(const KoColorProfile *p) const
        {
            return new KisKSLCColorSpace<_TYPE_,_N_>(p->clone());
        }

        bool profileIsCompatible(const KoColorProfile *profile) const
        {
            const KisIlluminantProfile *p = dynamic_cast<const KisIlluminantProfile *>(profile);
            if ((!p) || (p->wavelengths() != _N_)) {
                return false;
            }
            return true;
        }

        QString defaultProfile() const
        {
            return QString("D-65 Illuminant Profile - " + QString::number(_N_) + " wavelengths - Black [11.0,0.35]");
        }
};

#endif // KIS_KSLC_COLORSPACE_H_

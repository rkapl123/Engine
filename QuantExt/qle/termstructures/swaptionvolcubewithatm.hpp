/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file qle/termstructures/swaptionvolcubewithatm.hpp
    \brief Wrapper class for a SwaptionVolatilityCube that easily and efficiently exposes ATM vols.
    \ingroup termstructures
*/

#ifndef quantext_swaptionvolcubewithatm_hpp
#define quantext_swaptionvolcubewithatm_hpp

#include <ql/termstructures/volatility/swaption/swaptionvolcube.hpp>

#include <ql/shared_ptr.hpp>

namespace QuantExt {
using namespace QuantLib;

//! Wrapper class for a SwaptionVolatilityCube that easily and efficiently exposes ATM vols.
/*! This class implements SwaptionVolatilityStructure and takes a cube as an input. If asked for
    a volatility with strike=Null<Real>() it will return the ATM vol by asking the ATM surface directly.
    If asked for any other strike it will pass it on to the cube.

    There is no calculation of ATM in this class.

    \ingroup termstructures
 */
class SwaptionVolCubeWithATM : public SwaptionVolatilityStructure {
public:
    //! Constructor. This is a floating term structure (settlement days is zero) to match
    //! QuantLib::SwaptionVolatilityCube
    SwaptionVolCubeWithATM(const QuantLib::ext::shared_ptr<SwaptionVolatilityCube>& cube,
                           const bool useAtmSubStructure = true)
        : SwaptionVolatilityStructure(0, cube->calendar(), cube->businessDayConvention(), cube->dayCounter()),
          cube_(cube), useAtmSubStructure_(useAtmSubStructure) {
        enableExtrapolation(cube_->allowsExtrapolation());
        registerWith(cube);
    }

    //! \name TermStructure interface
    //@{
    DayCounter dayCounter() const override { return cube_->dayCounter(); }
    Date maxDate() const override { return cube_->maxDate(); }
    Time maxTime() const override { return cube_->maxTime(); }
    const Date& referenceDate() const override { return cube_->referenceDate(); }
    Calendar calendar() const override { return cube_->calendar(); }
    Natural settlementDays() const override { return cube_->settlementDays(); }
    //! \name VolatilityTermStructure interface
    //@{
    Rate minStrike() const override { return cube_->minStrike(); }
    Rate maxStrike() const override { return cube_->maxStrike(); }
    //@}
    //! \name SwaptionVolatilityStructure interface
    //@{
    const Period& maxSwapTenor() const override { return cube_->maxSwapTenor(); }
    VolatilityType volatilityType() const override { return cube_->volatilityType(); }
    //@}

    QuantLib::ext::shared_ptr<SwaptionVolatilityCube> cube() const { return cube_; }

protected:
    // Nothing to do here, just ask the cube
    QuantLib::ext::shared_ptr<SmileSection> smileSectionImpl(Time optionTime, Time swapLength) const override {
        return cube_->smileSection(optionTime, swapLength);
    }

    // Here we check if strike is Null<Real>() and ask the ATM surface if so.
    Volatility volatilityImpl(Time optionTime, Time swapLength, Rate strike) const override {
        if (strike == Null<Real>() && useAtmSubStructure_) {
            return cube_->atmVol()->volatility(optionTime, swapLength, 0.0);
        } else {
            return cube_->volatility(optionTime, swapLength, strike);
        }
    }

    Real shiftImpl(Time optionTime, Time swapLength) const override { return cube_->shift(optionTime, swapLength); }

private:
    QuantLib::ext::shared_ptr<SwaptionVolatilityCube> cube_;
    bool useAtmSubStructure_ = false;
};

} // namespace QuantExt

#endif

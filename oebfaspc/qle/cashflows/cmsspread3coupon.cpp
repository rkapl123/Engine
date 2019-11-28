/*
 Copyright (C) 2014 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#include <../oebfaspc/qle/cashflows/cmsspread3coupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>

namespace QuantExt {
    CmsSpread3Coupon::CmsSpread3Coupon(
        const Date &paymentDate, Real nominal, const Date &startDate,
        const Date &endDate, Natural fixingDays,
        const boost::shared_ptr<SwapSpread3Index> &index, Real gearing,
        Spread spread, const Date &refPeriodStart,
        const Date &refPeriodEnd,
        const DayCounter &dayCounter, bool isInArrears)
        : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                             fixingDays, index, gearing, spread,
                             refPeriodStart, refPeriodEnd, dayCounter,
                             isInArrears),
          index_(index) {}

    void CmsSpread3Coupon::accept(AcyclicVisitor &v) {
        Visitor<CmsSpread3Coupon> *v1 = dynamic_cast<Visitor<CmsSpread3Coupon> *>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    CmsSpread3Leg::CmsSpread3Leg(const Schedule &schedule,
                               const boost::shared_ptr<SwapSpread3Index> &index)
        : schedule_(schedule), swapSpread3Index_(index),
          paymentAdjustment_(Following), inArrears_(false),
          zeroPayments_(false) {}

    CmsSpread3Leg &CmsSpread3Leg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1, notional);
        return *this;
    }

    CmsSpread3Leg &
    CmsSpread3Leg::withNotionals(const std::vector<Real> &notionals) {
        notionals_ = notionals;
        return *this;
    }

    CmsSpread3Leg &
    CmsSpread3Leg::withPaymentDayCounter(const DayCounter &dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    CmsSpread3Leg &
    CmsSpread3Leg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    CmsSpread3Leg &CmsSpread3Leg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1, fixingDays);
        return *this;
    }

    CmsSpread3Leg &
    CmsSpread3Leg::withFixingDays(const std::vector<Natural> &fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    CmsSpread3Leg &CmsSpread3Leg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1, gearing);
        return *this;
    }

    CmsSpread3Leg &
    CmsSpread3Leg::withGearings(const std::vector<Real> &gearings) {
        gearings_ = gearings;
        return *this;
    }

    CmsSpread3Leg &CmsSpread3Leg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1, spread);
        return *this;
    }

    CmsSpread3Leg &
    CmsSpread3Leg::withSpreads(const std::vector<Spread> &spreads) {
        spreads_ = spreads;
        return *this;
    }

    CmsSpread3Leg &CmsSpread3Leg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    CmsSpread3Leg &CmsSpread3Leg::withZeroPayments(bool flag) {
        zeroPayments_ = flag;
        return *this;
    }

    CmsSpread3Leg::operator Leg() const {
        return FloatingLeg<SwapSpread3Index, CmsSpread3Coupon,
            CappedFlooredCmsSpread3Coupon>(
            schedule_, notionals_, swapSpread3Index_, paymentDayCounter_,
            paymentAdjustment_, fixingDays_, gearings_, spreads_, std::vector<Real>(1,Null<Real>()),
            std::vector<Real>(1, Null<Real>()), inArrears_, zeroPayments_);
    }
}

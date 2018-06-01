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
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file cmsspreadcoupon.hpp
    \brief CMS spread coupon
*/

#pragma once

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <qle/indexes/swapspread3index.hpp>
#include <ql/time/schedule.hpp>

using namespace QuantLib;

namespace QuantLib {
    class SwapIndex;
}

namespace QuantExt {

    //! CMS spread coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class CmsSpread3Coupon : public FloatingRateCoupon {
      public:
        CmsSpread3Coupon(const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  Natural fixingDays,
                  const boost::shared_ptr<SwapSpread3Index>& index,
                  Real gearing = 1.0,
                  Spread spread = 0.0,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false);
        //! \name Inspectors
        //@{
        const boost::shared_ptr<SwapSpread3Index>& swapSpreadIndex() const {
            return index_;
        }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::shared_ptr<SwapSpread3Index> index_;
    };

    class CappedFlooredCmsSpread3Coupon : public CappedFlooredCoupon {
    public:
        CappedFlooredCmsSpread3Coupon(
            const Date& paymentDate,
            Real nominal,
            const Date& startDate,
            const Date& endDate,
            Natural fixingDays,
            const boost::shared_ptr<SwapSpread3Index>& index,
            Real gearing = 1.0,
            Spread spread = 0.0,
            const Rate cap = Null<Rate>(),
            const Rate floor = Null<Rate>(),
            const Date& refPeriodStart = Date(),
            const Date& refPeriodEnd = Date(),
            const DayCounter& dayCounter = DayCounter(),
            bool isInArrears = false)
            : CappedFlooredCoupon(boost::shared_ptr<FloatingRateCoupon>(new
                CmsSpread3Coupon(paymentDate, nominal, startDate, endDate, fixingDays,
                    index, gearing, spread, refPeriodStart, refPeriodEnd,
                    dayCounter, isInArrears)), cap, floor) {}

        virtual void accept(AcyclicVisitor& v) {
            Visitor<CappedFlooredCmsSpread3Coupon>* v1 =
                dynamic_cast<Visitor<CappedFlooredCmsSpread3Coupon>*>(&v);
            if (v1 != 0)
                v1->visit(*this);
            else
                CappedFlooredCoupon::accept(v);
        }
    };

    //! helper class building a sequence of capped/floored cms-spread-rate coupons
    class CmsSpread3Leg {
      public:
        CmsSpread3Leg(const Schedule& schedule,
               const boost::shared_ptr<SwapSpread3Index>& swapSpreadIndex);
        CmsSpread3Leg& withNotionals(Real notional);
        CmsSpread3Leg& withNotionals(const std::vector<Real>& notionals);
        CmsSpread3Leg& withPaymentDayCounter(const DayCounter&);
        CmsSpread3Leg& withPaymentAdjustment(BusinessDayConvention);
        CmsSpread3Leg& withFixingDays(Natural fixingDays);
        CmsSpread3Leg& withFixingDays(const std::vector<Natural>& fixingDays);
        CmsSpread3Leg& withGearings(Real gearing);
        CmsSpread3Leg& withGearings(const std::vector<Real>& gearings);
        CmsSpread3Leg& withSpreads(Spread spread);
        CmsSpread3Leg& withSpreads(const std::vector<Spread>& spreads);
        CmsSpread3Leg& inArrears(bool flag = true);
        CmsSpread3Leg& withZeroPayments(bool flag = true);
        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<SwapSpread3Index> swapSpread3Index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        bool inArrears_, zeroPayments_;
    };


    //! base pricer for vanilla CMS spread coupons
    class CmsSpread3CouponPricer : public FloatingRateCouponPricer {
      public:
        explicit CmsSpread3CouponPricer(
                           const Handle<Quote> &correlation = Handle<Quote>())
        : correlation_(correlation) {
            registerWith(correlation_);
        }

        Handle<Quote> correlation() const{
            return correlation_;
        }

        void setCorrelation(
                         const Handle<Quote> &correlation = Handle<Quote>()) {
            unregisterWith(correlation_);
            correlation_ = correlation;
            registerWith(correlation_);
            update();
        }
      private:
        Handle<Quote> correlation_;
    };

}

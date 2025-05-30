/*
 Copyright (C) 2018 Quaternion Risk Management Ltd
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

/*! \file qle/indexes/equityindex.cpp
\brief equity index class for holding equity fixing histories and forwarding.
\ingroup indexes
*/

#include <qle/indexes/equityindex.hpp>

#include <boost/make_shared.hpp>

namespace QuantExt {

EquityIndex2::EquityIndex2(const std::string& familyName, const Calendar& fixingCalendar, const Currency& currency,
                         const Handle<Quote> spotQuote, const Handle<YieldTermStructure>& rate,
                         const Handle<YieldTermStructure>& dividend, const Handle<EquityAnnouncedDividendCurve>& announcedDividend)
    : familyName_(familyName), currency_(currency), rate_(rate), dividend_(dividend), announcedDividend_(announcedDividend),
      spotQuote_(spotQuote), fixingCalendar_(fixingCalendar) {

    name_ = familyName;
    registerWith(spotQuote_);
    registerWith(rate_);
    registerWith(dividend_);
    registerWith(announcedDividend_);
    registerWith(Settings::instance().evaluationDate());
    QL_DEPRECATED_DISABLE_WARNING
    registerWith(IndexManager::instance().notifier(name()));
    QL_DEPRECATED_ENABLE_WARNING
}

Real EquityIndex2::fixing(const Date& fixingDate, bool forecastTodaysFixing) const {
    return fixing(fixingDate, forecastTodaysFixing, false);
}

Real EquityIndex2::fixing(const Date& fixingDate, bool forecastTodaysFixing, bool incDividend) const {

    Date today = Settings::instance().evaluationDate();

    if (fixingDate > today || (fixingDate == today && forecastTodaysFixing))
        return forecastFixing(fixingDate, incDividend);

    QL_REQUIRE(isValidFixingDate(fixingDate),
               "Fixing date " << fixingDate << " is not valid for equity index '" << name_ << "'");

    Real result = Null<Decimal>();

    if (fixingDate < today || Settings::instance().enforcesTodaysHistoricFixings()) {
        // must have been fixed
        // do not catch exceptions
        result = pastFixing(fixingDate);
        QL_REQUIRE(result != Null<Real>(), "Missing equity index fixing " << name() << " fixing for " << fixingDate);
    } else {
        try {
            // might have been fixed
            result = pastFixing(fixingDate);
        } catch (Error&) {
            ; // fall through and forecast
        }
        if (result == Null<Real>())
            return forecastFixing(fixingDate, incDividend);
    }

    return result;
}

Real EquityIndex2::forecastFixing(const Date& fixingDate) const { return forecastFixing(fixingDate, false); }

Real EquityIndex2::forecastFixing(const Date& fixingDate, bool incDividend) const {
    QL_REQUIRE(!rate_.empty(), "null term structure set to this instance of " << name());
    return forecastFixing(rate_->timeFromReference(fixingDate), incDividend);
}

Real EquityIndex2::forecastFixing(const Time& fixingTime) const { return forecastFixing(fixingTime, false); }

Real EquityIndex2::forecastFixing(const Time& fixingTime, bool incDividend) const {
    QL_REQUIRE(!spotQuote_.empty(), "null spot quote set to this instance of " << name());
    QL_REQUIRE(!rate_.empty() && !dividend_.empty(), "null term structure set to this instance of " << name());

    // we base the forecast always on the spot quote (and not on today's fixing)
    Real price = spotQuote_->value();

    // compute the forecast applying the usual no arbitrage principle
    Real forward;
    if (incDividend) {
        forward = price / rate_->discount(fixingTime);
    } else {
        Real announcedDividendT0 = 0.0;
        Real announcedDividend = 0.0;
        if (!announcedDividend_.empty()) {
            announcedDividendT0 = announcedDividend_->discountedFutureDividends(0.0);
            announcedDividend = announcedDividend_->discountedFutureDividends(fixingTime);
        }
        forward = (price - announcedDividendT0) * dividend_->discount(fixingTime) / rate_->discount(fixingTime) +
                  announcedDividend;
    }
    return forward;
}

void EquityIndex2::addDividend(const Dividend& dividend, bool forceOverwrite) {
    DividendManager::instance().addDividend(name(), dividend, forceOverwrite);
}

Real EquityIndex2::dividendsBetweenDates(const Date& startDate, const Date& endDate, const bool historicalOnly) const {
    const Date& today = Settings::instance().evaluationDate();

    Date end = historicalOnly ? std::min(endDate, today) : endDate;

    const std::set<Dividend>& history = dividendFixings();
    Real dividends = 0.0;

    if (!history.empty()) {
        for (std::set<Dividend>::const_iterator fd = history.begin(); fd != history.end() && fd->exDate <= end; ++fd) {
            if (fd->exDate >= startDate)
                dividends += fd->rate;
        }
    }
    return dividends;
}

QuantLib::ext::shared_ptr<EquityIndex2> EquityIndex2::clone(const Handle<Quote> spotQuote, const Handle<YieldTermStructure>& rate,
                    const Handle<YieldTermStructure>& dividend,
                    const Handle<EquityAnnouncedDividendCurve>& discreteDividend) const {
    return QuantLib::ext::make_shared<EquityIndex2>(familyName(), fixingCalendar(), currency(), spotQuote, rate, dividend, discreteDividend);
}

} // namespace QuantExt

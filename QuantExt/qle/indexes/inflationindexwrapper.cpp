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

#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <qle/indexes/inflationindexwrapper.hpp>

namespace QuantExt {


ZeroInflationIndexWrapper::ZeroInflationIndexWrapper(const QuantLib::ext::shared_ptr<ZeroInflationIndex> source)
    : ZeroInflationIndex(source->familyName(), source->region(), source->revised(), source->frequency(), source->availabilityLag(), source->currency(),
                         source->zeroInflationTermStructure()),
      source_(source) {}

Rate ZeroInflationIndexWrapper::fixing(const Date& fixingDate, bool /*forecastTodaysFixing*/) const {
    std::pair<Date, Date> dd = inflationPeriod(fixingDate, frequency());
    return source_->fixing(dd.first);
}

YoYInflationIndexWrapper::YoYInflationIndexWrapper(const QuantLib::ext::shared_ptr<ZeroInflationIndex> zeroIndex,
                                                   const Handle<YoYInflationTermStructure>& ts)
    : YoYInflationIndex(zeroIndex, ts), zeroIndex_(zeroIndex) {
    registerWith(zeroIndex_);
}

YoYInflationIndexWrapper::YoYInflationIndexWrapper(const QuantLib::ext::shared_ptr<ZeroInflationIndex> zeroIndex,
                                                   const bool interpolated, const Handle<YoYInflationTermStructure>& ts)
    : YoYInflationIndex(zeroIndex, interpolated, ts),
      zeroIndex_(zeroIndex) {
    registerWith(zeroIndex_);
}

Rate YoYInflationIndexWrapper::fixing(const Date& fixingDate, bool /*forecastTodaysFixing*/) const {

    // duplicated logic from YoYInflationIndex, this would not be necessary, if forecastFixing
    // was defined virtual in InflationIndex
    if (needsForecast(fixingDate)) {
        return forecastFixing(fixingDate);
    }
    // historical fixing
    return YoYInflationIndex::fixing(fixingDate);
}

Real YoYInflationIndexWrapper::forecastFixing(const Date& fixingDate) const {
    if (!yoyInflationTermStructure().empty())
        return YoYInflationIndex::fixing(fixingDate);
    auto interpolation = YoYInflationIndex::interpolated() ? CPI::Linear : CPI::Flat;
    Real f1 = CPI::laggedFixing(zeroIndex_, fixingDate, 0 * Days, interpolation);
    Real f0 = CPI::laggedFixing(zeroIndex_, fixingDate - 1 * Years, 0 * Days, interpolation);
    return (f1 - f0) / f0;
}

} // namespace QuantExt

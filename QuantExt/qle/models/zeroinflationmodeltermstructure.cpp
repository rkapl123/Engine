/*
 Copyright (C) 2020 Quaternion Risk Management Ltd
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

#include <qle/models/zeroinflationmodeltermstructure.hpp>

using QuantLib::Array;
using QuantLib::Date;
using QuantLib::Real;
using QuantLib::Size;
using QuantLib::Time;

namespace QuantExt {

ZeroInflationModelTermStructure::ZeroInflationModelTermStructure(
    const QuantLib::ext::shared_ptr<CrossAssetModel>& model, Size index)
    : ZeroInflationTermStructure(inflationTermStructure(model, index)->baseDate(),
                                 inflationTermStructure(model, index)->observationLag(),
                                 inflationTermStructure(model, index)->frequency(),
                                 inflationTermStructure(model, index)->dayCounter()),
      model_(model), index_(index), referenceDate_(inflationTermStructure(model_, index_)->referenceDate()),
      relativeTime_(0.0) {
    registerWith(model_);
    update();
}

void ZeroInflationModelTermStructure::update() {
    notifyObservers();
}

Date ZeroInflationModelTermStructure::maxDate() const {
    // we don't care. Let the underlying classes throw exceptions if applicable
    return Date::maxDate();
}

Time ZeroInflationModelTermStructure::maxTime() const {
    // see maxDate
    return QL_MAX_REAL;
}

const Date& ZeroInflationModelTermStructure::referenceDate() const {
    return referenceDate_;
}

Date ZeroInflationModelTermStructure::baseDate() const {
    return inflationPeriod(referenceDate_ - observationLag_, frequency()).first;
}

void ZeroInflationModelTermStructure::referenceDate(const Date& d) {
    referenceDate_ = d;
    relativeTime_ = dayCounter().yearFraction(inflationTermStructure(model_, index_)->referenceDate(), referenceDate_);
    update();
}

void ZeroInflationModelTermStructure::state(const Array& s) {
    state_ = s;
    checkState();
    notifyObservers();
}

void ZeroInflationModelTermStructure::move(const Date& d, const Array& s) {
    state(s);
    referenceDate(d);
}

}

/*
 Copyright (C) 2021 Quaternion Risk Management Ltd
 Copyright (C) 2021 Skandinaviska Enskilda Banken AB (publ)
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

#include <ored/portfolio/trademonetary.hpp>
#include <ored/utilities/parsers.hpp>
#include <ored/utilities/to_string.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/export.hpp>

namespace ore {
namespace data {


void TradeMonetary::fromXML(XMLNode* node) { 
	currency_ = XMLUtils::getChildValue(node, "Currency", false);
    value_ = XMLUtils::getChildValueAsDouble(node, "Value", true);
}

XMLNode* TradeMonetary::toXML(XMLDocument& doc) {
    XMLNode* node = doc.allocNode("Data"); //Need to fix this
    XMLUtils::addChild(doc, node, "Value", value_);
    XMLUtils::addChild(doc, node, "Currency", currency_);
    return node;
}

std::string TradeMonetary::currency() const { 
    if (!currency_.empty())
        return parseCurrencyWithMinors(currency_).code();
    else
        QL_FAIL("No currency provided in TradeMonetary class");
}

QuantLib::Real TradeMonetary::value() const { return convertMinorToMajorCurrency(currency_, value_); }

} // namespace data
} // namespace ore
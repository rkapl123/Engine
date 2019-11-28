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

/*! \file ored/portfolio/legbuilders.hpp
    \brief Leg Builders
    \ingroup portfolio
*/

#pragma once

#include <ored/portfolio/enginefactory.hpp>

namespace ore {
namespace data {

class CMSSpread3LegBuilder : public LegBuilder {
public:
    CMSSpread3LegBuilder() : LegBuilder("CMSSpread3") {}
    Leg buildLeg(const LegData& data, const boost::shared_ptr<EngineFactory>& engineFactory,
        const string& configuration) const override;
};

} // namespace data
} // namespace ore

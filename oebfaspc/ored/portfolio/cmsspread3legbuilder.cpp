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

#include <ored/portfolio/legbuilders.hpp>
#include <ored/portfolio/legdata.hpp>
#include <../oebfaspc/qle/indexes/swapspread3index.hpp>
#include <../oebfaspc/ored/portfolio/cmsspread3legbuilder.hpp>

namespace ore {
namespace data {

Leg CMSSpread3LegBuilder::buildLeg(const LegData& data, const boost::shared_ptr<EngineFactory>& engineFactory,
    const string& configuration) const {
    auto cmsSpreadData = boost::dynamic_pointer_cast<CMSSpread3LegData>(data.concreteLegData());
    QL_REQUIRE(cmsSpreadData, "Wrong LegType, expected CMSSpread");
    auto index1 = *engineFactory->market()->swapIndex(cmsSpreadData->swapIndex1(), configuration);
    auto index2 = *engineFactory->market()->swapIndex(cmsSpreadData->swapIndex2(), configuration);
    auto index3 = *engineFactory->market()->swapIndex(cmsSpreadData->swapIndex3(), configuration);
    const vector<double>& spreads = cmsSpreadData->spreads();
    const vector<double>& gearings = cmsSpreadData->gearings();
    double spread = (spreads.size() > 0 ? spreads[0] : 0);
    double gearing1 = (gearings.size() > 0 ? gearings[0] : 0);
    double gearing2 = (gearings.size() > 1 ? gearings[1] : 0);
    double gearing3 = (gearings.size() > 2 ? gearings[2] : 0);
    return makeCMSSpread3Leg(data,
        boost::make_shared<QuantExt::SwapSpread3Index>("CMSSpread3_" + index1->familyName() + "_" + index2->familyName() + "_" + index3->familyName(), 
            index1, index2, index3, gearing1, gearing2, gearing3, spread, 
            cmsSpreadData->strike(), cmsSpreadData->cap(), cmsSpreadData->floor(), cmsSpreadData->elseconstant(), cmsSpreadData->structchoice()),
        engineFactory);
}

} // namespace data
} // namespace ore

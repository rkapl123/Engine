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

/*! \file ored/portfolio/optiondata.hpp
    \brief trade option data model and serialization
    \ingroup portfolio
*/

#pragma once

#include <ored/portfolio/schedule.hpp>
#include <ored/portfolio/tradebarrier.hpp>

namespace ore {
namespace data {

//! Serializable obejct holding barrier data
/*!
  \ingroup tradedata
*/
class BarrierData : public ore::data::XMLSerializable {
public:
    //! Default constructor
    BarrierData() : initialized_(false), rebate_(0.0) {}
    //! Constructor
    BarrierData(const std::string& barrierType, const std::vector<double>& levels, const double rebate,
                const std::vector<ore::data::TradeBarrier>& tradeBarriers, const std::string& style = std::string(),
                const std::optional<string>& strictComparison = std::nullopt, const std::optional<bool>& overrideTriggered = std::nullopt)
        : initialized_(true), type_(barrierType), levels_(levels), rebate_(rebate), tradeBarriers_(tradeBarriers),
          style_(style), strictComparison_(strictComparison), overrideTriggered_(overrideTriggered) {}

    //! \name Inspectors
    //@{
    const std::string& type() const { return type_; }
    double rebate() const { return rebate_; }
    const std::string& rebateCurrency() const { return rebateCurrency_; }
    const std::string& rebatePayTime() const { return rebatePayTime_; }
    const std::vector<ore::data::TradeBarrier>& levels() const { return tradeBarriers_; }
    const std::string& style() const { return style_; }
    bool initialized() const { return initialized_; }
    const std::optional<string>& strictComparison() const { return strictComparison_; }
    const std::optional<bool> overrideTriggered() const { return overrideTriggered_; }
    //@}

    //! \name Serialisation
    //@{
    virtual void fromXML(ore::data::XMLNode* node) override;
    virtual ore::data::XMLNode* toXML(ore::data::XMLDocument& doc) const override;
    //@}

private:
    bool initialized_;
    std::string type_;
    std::vector<double> levels_;
    double rebate_;
    std::vector<ore::data::TradeBarrier> tradeBarriers_;
    std::string rebateCurrency_;
    std::string rebatePayTime_;
    std::string style_;
    std::optional<string> strictComparison_;
    std::optional<bool> overrideTriggered_;
};
} // namespace data
} // namespace oreplus

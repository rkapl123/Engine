/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
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

/*! \file qle/instruments/equityforward.hpp
\brief equityforward instrument

\ingroup instruments
*/

#ifndef quantext_equity_forward_hpp
#define quantext_equity_forward_hpp

#include <ql/currency.hpp>
#include <ql/exchangerate.hpp>
#include <ql/instrument.hpp>
#include <ql/money.hpp>
#include <ql/position.hpp>
#include <ql/quote.hpp>
#include <qle/indexes/fxindex.hpp>
namespace QuantExt {
using namespace QuantLib;

/*! This class holds the term sheet data for an Equity Forward instrument.

\ingroup instruments
*/
class EquityForward : public Instrument {
public:
    class arguments;
    class engine;
    //! \name Constructors
    //@{
    EquityForward( //! Equity Name
        const std::string& name,
        //! Currency of the underlying
        const Currency& currency,
        //! if true, we are long the forward
        const Position::Type& longShort,
        //! Quantity (number of lots \f$times\f$ lot size)
        const Real& quantity,
        //! Maturity date
        const Date& maturityDate,
        //! Strike
        const Real& strike,
        // Payment Date
        const Date& payDate = Date(),
        // Payment Currency
        const Currency payCcy = Currency(),
        // FX Index used for fx conversion
        const QuantLib::ext::shared_ptr<QuantExt::FxIndex>& fxIndex = nullptr,
        // Fixing Date for a fx conversion if paymentCurrency != currency
        const Date& fixingDate = Date());
    //! \name Instrument interface
    //@{
    bool isExpired() const override;
    void setupArguments(PricingEngine::arguments*) const override;
    //@}

    //! \name Additional interface
    //@{
    const std::string& name() const { return name_; }
    Currency currency() const { return currency_; }
    Position::Type longShort() const { return longShort_; }
    Real quantity() const { return quantity_; }
    Date maturityDate() const { return maturityDate_; }
    Real strike() const { return strike_; }
    Date payDate() const { return payDate_; }
    QuantLib::ext::shared_ptr<QuantExt::FxIndex> fxIndex() const { return fxIndex_; }
    //@}
private:
    // data members
    std::string name_;
    Currency currency_;
    Position::Type longShort_;
    Real quantity_;
    Date maturityDate_;
    Real strike_;
    Date payDate_;
    Currency payCcy_;
    QuantLib::ext::shared_ptr<FxIndex> fxIndex_;
    Date fixingDate_;
};

//! \ingroup instruments
class EquityForward::arguments : public virtual PricingEngine::arguments {
public:
    std::string name;
    Currency currency;
    Position::Type longShort;
    Real quantity;
    Date maturityDate;
    Real strike;
    Date payDate;
    Currency payCurrency;
    QuantLib::ext::shared_ptr<FxIndex> fxIndex;
    Date fixingDate;
    void validate() const override;
};

//! \ingroup instruments
class EquityForward::engine : public GenericEngine<EquityForward::arguments, Instrument::results> {};
} // namespace QuantExt

#endif

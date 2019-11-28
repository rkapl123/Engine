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

/*! \file swapspreadindex.hpp
    \brief swap-rate spread indexes
*/

#pragma once

#include <ql/indexes/swapindex.hpp>
using namespace QuantLib;

namespace QuantExt {

    //! class for swap-rate spread indexes
    class SwapSpread3Index : public InterestRateIndex {
      public:
        SwapSpread3Index(const std::string& familyName,
                        const boost::shared_ptr<SwapIndex>& swapIndex1,
                        const boost::shared_ptr<SwapIndex>& swapIndex2,
                        const boost::shared_ptr<SwapIndex>& swapIndex3,
                        const Real gearing1 = 1.0,
                        const Real gearing2 = -1.0,
                        const Real gearing3 = 1.0, 
                        const Real spread = 0.0,
                        const Real strike = 0.005,
                        const Real cap = 0.01, 
                        const Real floor = 0.0, 
                        const Real elseconstant = 0.01, 
                        const int structchoice = 1);

        //! \name InterestRateIndex interface
        //@{
        Date maturityDate(const Date& valueDate) const {
            QL_FAIL("SwapSpreadIndex does not provide a single maturity date");
        }
        Rate forecastFixing(const Date& fixingDate) const;
        Rate pastFixing(const Date& fixingDate) const;
        Rate calculateFixing(Real f1, Real f2, Real f3) const;
        bool allowsNativeFixings() { return false; }
        //@}

        //! \name Inspectors
        //@{
        boost::shared_ptr<SwapIndex> swapIndex1() { return swapIndex1_; }
        boost::shared_ptr<SwapIndex> swapIndex2() { return swapIndex2_; }
        boost::shared_ptr<SwapIndex> swapIndex3() { return swapIndex3_; }
        Real gearing1() { return gearing1_; }
        Real gearing2() { return gearing2_; }
        Real gearing3() { return gearing3_; }
        int structchoice() { return structchoice_; }
        //@}


    private:
        int structchoice_;
        boost::shared_ptr<SwapIndex> swapIndex1_, swapIndex2_, swapIndex3_;
        Real gearing1_, gearing2_, gearing3_, strike_, elseconstant_, cap_, floor_, spread_;
    };


    inline Rate SwapSpread3Index::forecastFixing(const Date& fixingDate) const {
        // this also handles the case when one of indices has
        // a historic fixing on the evaluation date
        Real f1 = swapIndex1_->fixing(fixingDate, false);
        Real f2 = swapIndex2_->fixing(fixingDate, false);
        Real f3 = swapIndex3_->fixing(fixingDate, false);
        return calculateFixing(f1, f2, f3);
    }

    inline Rate SwapSpread3Index::pastFixing(const Date& fixingDate) const {

        Real f1 = swapIndex1_->pastFixing(fixingDate);
        Real f2 = swapIndex2_->pastFixing(fixingDate);
        Real f3 = swapIndex3_->pastFixing(fixingDate);
        // if one of the fixings is missing we return null, indicating
        // a missing fixing for the spread index
        return calculateFixing(f1, f2, f3);
    }

    inline Rate SwapSpread3Index::calculateFixing(Real f1, Real f2, Real f3) const {
        // if one of the fixings is missing we return null, indicating
        // a missing fixing for the spread index
        if (structchoice_ == 1)
            return (f1 - f2 > strike_ ? std::max(gearing3_ * f3, floor_) : elseconstant_);
        else if (structchoice_ == 2)
            return (f1 - f2 > strike_ ? f3 + spread_ : elseconstant_); // praktisch egal
        else if (structchoice_ == 3)
            return (gearing1_ * (f1 - f2) > f3 ? std::max(f3 + spread_, floor_) : elseconstant_); // TODO: add Agg IA 40%
        else if (structchoice_ == 4)
            return std::max(std::min(f3 + spread_, gearing1_ * (f1 - f2)), floor_); // praktisch egal
        else if (structchoice_ == 5)
            return f1 + spread_; // TODO: add Agg IA 60%
        else {
            QL_FAIL("Unknown structtype: " << structchoice_);
            return Null<Real>();
        }
    }
}

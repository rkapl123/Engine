/*
 Copyright (C) 2021 Quaternion Risk Management Ltd
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

#include <orea/app/analytics/analyticfactory.hpp>
#include <orea/app/analyticsmanager.hpp>
#include <orea/app/reportwriter.hpp>
#include <orea/app/structuredanalyticserror.hpp>

#include <ored/utilities/log.hpp>
#include <ored/utilities/to_string.hpp>

#include <ql/errors.hpp>

using namespace std;
using namespace boost::filesystem;
using ore::data::InMemoryReport;

namespace ore {
namespace analytics {
    
void AnalyticsManager::initialise() {
    for (const auto& a : inputs_->analytics()) 
        auto ap = AnalyticFactory::instance().build(a, inputs_, shared_from_this(), true);
    initialised_ = true;
}

void AnalyticsManager::clear() {
    LOG("AnalyticsManager: Remove all analytics currently registered");
    analytics_.clear();
    validAnalytics_.clear();
}
    
void AnalyticsManager::addAnalytic(const std::string& label, const QuantLib::ext::shared_ptr<Analytic>& analytic) {
    // Label is not necessarily a valid analytics type
    // Get the latter via analytic->analyticTypes()
    LOG("register analytic with label '" << label << "' and sub-analytics " << to_string(analytic->analyticTypes()));
    analytics_.push_back(make_pair(label, analytic));
    // This forces an update of valid analytics vector with the next call to validAnalytics()
    validAnalytics_.clear();
}

const std::set<std::string>& AnalyticsManager::validAnalytics() {
    if (validAnalytics_.size() == 0) {
        for (auto a : analytics_) {
            const std::set<std::string>& types = a.second->analyticTypes();
            validAnalytics_.insert(types.begin(), types.end());
        }
    }
    return validAnalytics_;
}

const std::set<std::string>& AnalyticsManager::requestedAnalytics() {
    return inputs_->analytics();
}
    
bool AnalyticsManager::hasAnalytic(const std::string& type) {
    const std::set<std::string>& va = validAnalytics();
    return va.find(type) != va.end();
}

const QuantLib::ext::shared_ptr<Analytic>& AnalyticsManager::getAnalytic(const std::string& type) const {
    for (const auto& a : analytics_) {
        const std::set<std::string>& types = a.second->analyticTypes();
        if (types.find(type) != types.end())
            return a.second;
    }
    QL_FAIL("analytic type " << type << " not found, check validAnalytics()");
}

std::vector<QuantLib::ext::shared_ptr<ore::data::TodaysMarketParameters>> AnalyticsManager::todaysMarketParams() {
    std::vector<QuantLib::ext::shared_ptr<ore::data::TodaysMarketParameters>> tmps;
    for (const auto& a : analytics_) {
        std::vector<QuantLib::ext::shared_ptr<ore::data::TodaysMarketParameters>> atmps = a.second->todaysMarketParams();
        tmps.insert(end(tmps), begin(atmps), end(atmps));
    }
    return tmps;
}

void AnalyticsManager::runAnalytics(
    const QuantLib::ext::shared_ptr<MarketCalibrationReportBase>& marketCalibrationReport) {
    QL_REQUIRE(initialised_, "AnalyticsManager has not been initialised");
    if (analytics_.size() == 0)
        return;

    std::vector<QuantLib::ext::shared_ptr<ore::data::TodaysMarketParameters>> tmps = todaysMarketParams();

    // load the market data if at least one analytic requires that and we have non-empty tmps
    if (std::any_of(analytics_.begin(), analytics_.end(),
                    [](const std::pair<std::string, QuantLib::ext::shared_ptr<Analytic>>& a) {
                        return a.second->requiresMarketData();
                    }) &&
        std::any_of(
            tmps.begin(), tmps.end(),
            [](const QuantLib::ext::shared_ptr<ore::data::TodaysMarketParameters>& tmp) { return !tmp->empty(); })) {

        std::set<Date> marketDates;
        for (const auto& a : analytics_) {
            auto mdates = a.second->marketDates();
            marketDates.insert(mdates.begin(), mdates.end());
        }

        LOG("AnalyticsManager::runAnalytics: populate loader for dates: " << to_string(marketDates));

        marketDataLoader_->populateLoader(tmps, marketDates);

        QuantLib::ext::shared_ptr<InMemoryReport> mdReport =
            QuantLib::ext::make_shared<InMemoryReport>(inputs_->reportBufferSize());
        QuantLib::ext::shared_ptr<InMemoryReport> fixingReport =
            QuantLib::ext::make_shared<InMemoryReport>(inputs_->reportBufferSize());
        QuantLib::ext::shared_ptr<InMemoryReport> dividendReport =
            QuantLib::ext::make_shared<InMemoryReport>(inputs_->reportBufferSize());

        ore::analytics::ReportWriter(inputs_->reportNaString())
            .writeMarketData(*mdReport, marketDataLoader_->loader(), inputs_->asof(),
                             marketDataLoader_->quotes()[inputs_->asof()], !inputs_->entireMarket());
        ore::analytics::ReportWriter(inputs_->reportNaString())
            .writeFixings(*fixingReport, marketDataLoader_->loader());
        ore::analytics::ReportWriter(inputs_->reportNaString())
            .writeDividends(*dividendReport, marketDataLoader_->loader());

        reports_["MARKETDATA"]["marketdata"] = mdReport;
        reports_["FIXINGS"]["fixings"] = fixingReport;
        reports_["DIVIDENDS"]["dividends"] = dividendReport;
    }

    // run requested analytics
    for (auto a : analytics_) {
        LOG("run analytic with label '" << a.first << "'");
        a.second->startTimer("Run " + a.second->label() + "Analytic");
        try {
            a.second->runAnalytic(marketDataLoader_->loader(), inputs_->analytics());
        } catch (const exception& e) {
            failedAnalytics_.push_back(a.first);
            StructuredAnalyticsErrorMessage(a.first, "Failed Analytic", e.what());
        }
        a.second->stopTimer("Run " + a.second->label() + "Analytic");
        LOG("run analytic with label '" << a.first << "' finished.");
        // then populate the market calibration report if required
        if (marketCalibrationReport)
            a.second->marketCalibration(marketCalibrationReport);
    }

    if (inputs_->portfolio()) {
        auto pricingStatsReport = QuantLib::ext::make_shared<InMemoryReport>(inputs_->reportBufferSize());
        ReportWriter(inputs_->reportNaString()).writePricingStats(*pricingStatsReport, inputs_->portfolio());
        reports_["STATS"]["pricingstats"] = pricingStatsReport;
    }

    Timer timer;
    for (auto a : analytics_) {
        Timer analyticTimer = a.second->getTimer();
        if (!analyticTimer.empty()) {
            timer.addTimer(a.first, analyticTimer);
        }
    }
    if (!timer.empty()) {
        auto runTimesReport = QuantLib::ext::make_shared<InMemoryReport>();
        ReportWriter(inputs_->reportNaString()).writeRunTimes(*runTimesReport, timer);
        reports_["STATS"]["runtimes"] = runTimesReport;
    }

    if (marketCalibrationReport) {
        auto report = marketCalibrationReport->outputCalibrationReport();
        if (report) {
            if (auto rpt = QuantLib::ext::dynamic_pointer_cast<InMemoryReport>(report))
                reports_["MARKET"]["todaysmarketcalibration"] = rpt;
        }
    }

    inputs_->writeOutParameters();
}

Analytic::analytic_reports const AnalyticsManager::reports() {
    Analytic::analytic_reports reports = reports_;
    for (auto a : analytics_) {
        auto rs = a.second->reports();
        reports.insert(rs.begin(), rs.end());
    }
    return reports;
}

Analytic::analytic_npvcubes const AnalyticsManager::npvCubes() {
    Analytic::analytic_npvcubes results;
    for (auto a : analytics_) {
        auto rs = a.second->npvCubes();
        results.insert(rs.begin(), rs.end());
    }
    return results;
}

Analytic::analytic_mktcubes const AnalyticsManager::mktCubes() {
    Analytic::analytic_mktcubes results;
    for (auto a : analytics_) {
        auto rs = a.second->mktCubes();
        results.insert(rs.begin(), rs.end());
    }
    return results;
}

Analytic::analytic_stresstests const AnalyticsManager::stressTests() {
    Analytic::analytic_stresstests results;
    for (auto a : analytics_) {
        auto rs = a.second->stressTests();
        results.insert(rs.begin(), rs.end());
    }
    return results;
}

std::map<std::string, Size> checkReportNames(const ore::analytics::Analytic::analytic_reports& rpts) {                                     
    std::map<std::string, Size> m;
    for (const auto& rep : rpts) {
        for (auto b : rep.second) {
            string reportName = b.first;
            auto it = m.find(reportName);
            if (it == m.end())
                m[reportName] = 1;
            else
                m[reportName] ++;
        }
    }
    for (auto r : m) {
        LOG("report name " << r.first << " occurs " << r.second << " times");
    }
    return m;
}

bool endsWith(const std::string& name, const std::string& suffix) {
    if (suffix.size() > name.size())
        return false;
    else
        return std::equal(suffix.rbegin(), suffix.rend(), name.rbegin());
}

void AnalyticsManager::toFile(const ore::analytics::Analytic::analytic_reports& rpts, const std::string& outputPath,
                              const std::map<std::string, std::string>& reportNames, const char sep,
                              const bool commentCharacter, char quoteChar, const string& nullString,
                              const std::set<std::string>& lowerHeaderReportNames) {
    std::map<std::string, Size> hits = checkReportNames(rpts);    
    for (const auto& rep : rpts) {
        string analytic = rep.first;
        for (auto b : rep.second) {
            string reportName = b.first;
            QuantLib::ext::shared_ptr<InMemoryReport> report = b.second;
            string fileName;
            auto it = hits.find(reportName);
            QL_REQUIRE(it != hits.end(), "something wrong here");
            if (it->second == 1) {
                // The report name is unique, check whether we want to rename it or use the standard name
                auto it2 = reportNames.find(reportName);
                fileName = (it2 != reportNames.end() && !it2->second.empty()) ? it2->second : reportName;
            }
            else {
                ALOG("Report " << reportName << " occurs " << it->second << " times, fix report naming");
                fileName = analytic + "_" + reportName + "_" + to_string(hits[fileName]);
            }

            // attach a suffix only if it does not have one already
            string suffix = "";
            if (!endsWith(fileName,".csv") && !endsWith(fileName, ".txt"))
                suffix = ".csv";
            std::string fullFileName = outputPath + "/" + fileName + suffix;

            report->toFile(fullFileName, sep, commentCharacter, quoteChar, nullString,
                           lowerHeaderReportNames.find(reportName) != lowerHeaderReportNames.end());
            LOG("report " << reportName << " written to " << fullFileName); 
        }
    }
}

QuantLib::ext::shared_ptr<ore::data::InMemoryReport>
getReport(const ore::analytics::Analytic::analytic_reports& reports, const std::string& analytic,
          const std::string& report) {
    if (auto r1 = reports.find(analytic); r1 != reports.end()) {
        if (auto r2 = r1->second.find(report); r2 != r1->second.end()) {
            return r2->second;
        }
    }
    return QuantLib::ext::make_shared<ore::data::InMemoryReport>();
}

}
}

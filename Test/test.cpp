/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2016 Quaternion Risk Management Ltd.
  All rights reserved.
*/

#include <iostream>
#include <qlw/wrap.hpp>

using namespace std;
using namespace openxva::portfolio;
using namespace openxva::marketdata;
using namespace openxva::utilities;

#ifdef BOOST_MSVC
#include <qlw/auto_link.hpp>
#include <qle/auto_link.hpp>
#include <ql/auto_link.hpp>
#endif

void npv () {

    string dir = "Input/";

    Log::instance().registerLogger(boost::make_shared<StderrLogger>());
    Log::instance().switchOn();

    Date asof(23, Oct, 2015);
    Settings::instance().evaluationDate() = asof;

    CSVLoader loader(dir + "market.txt", dir + "fixings.txt");

    CurveConfigurations curveConfigs;
    curveConfigs.fromFile(dir + "curveconfig.xml");

    Conventions conventions;
    conventions.fromFile(dir + "conventions.xml");

    TodaysMarketParameters marketParams;
    marketParams.fromFile(dir + "todaysmarket.xml");

    // market data
    cout << "Load Market" << endl;
    boost::shared_ptr<Market> market = boost::make_shared<TodaysMarket>
        (asof, marketParams, loader, curveConfigs, conventions);
    cout << "Got Market" << endl;

    // dump curves to std out
    vector<string> curveNames = { "EUR-EURIBOR-3M", "EUR-EURIBOR-6M", "USD-LIBOR-3M",
                                  "GBP-LIBOR-3M", "GBP-LIBOR-6M" };
    vector<Handle<YieldTermStructure>> curves;
    for (const string& s : curveNames)
        curves.push_back(market->iborIndex(s)->forwardingTermStructure());

    // fx vols
    vector<string> fxVolCurves = { "EURUSD" };
    vector<Handle<BlackVolTermStructure>> fxVols;
    for (const string& s : fxVolCurves) {
        curveNames.push_back(s);
        fxVols.push_back(market->fxVol(s));
    }

    cout << "Time";
    for (const string& s : curveNames)
        cout << "," << s;
    cout << endl;
    for (Time t = 0; t <= 30.05; t += 0.2) {
        cout << t;
        for (auto c : curves)
            cout << "," << c->discount(t, true);

        for (auto c : fxVols)
            cout << "," << c->blackVol(t, Null<Real>());

        cout << endl;
    }

    // swaption vols
    Handle<SwaptionVolatilityStructure> swVol = market->swaptionVol("EUR");
    cout << "Swaption Vol EUR has type = " << swVol->volatilityType()
         << " with shift(1,1) = " << swVol->shift(1.0, 1.0) << std::endl;
    cout << "Swaption Vol EUR has swap index bases "
         << market->shortSwapIndexBase("EUR") << ", "
         << market->swapIndexBase("EUR") << std::endl;
    boost::shared_ptr<SwaptionVolatilityDiscrete> swVolD = boost::dynamic_pointer_cast<SwaptionVolatilityDiscrete>(*swVol);
    QL_REQUIRE(swVolD,"swaption vol is not discrete");
    for (auto option : swVolD->optionTenors()) {
        for (auto swap : swVolD->swapTenors()) {
            cout << swVol->volatility(option, swap, 0.05) << " ";
        }
        cout << "\n";
    }
    cout.flush();

    // default curves
    Handle<DefaultProbabilityTermStructure> df1 = market->defaultCurve("FMS");
    cout << "CDS default curve, 1Y "
         << -std::log(df1->survivalProbability(1.0)) / 1.0 << " 5Y "
         << -std::log(df1->survivalProbability(5.0)) / 5.0 << " RR " << market->recoveryRate("FMS")->value() << std::endl;
    Handle<DefaultProbabilityTermStructure> df2 =
        market->defaultCurve("CUST_M1");
    cout << "Hazard default curve, 1Y "
         << -std::log(df2->survivalProbability(1.0)) / 1.0 << " 5Y "
         << -std::log(df2->survivalProbability(5.0)) / 5.0 << " RR " << market->recoveryRate("CUST_M1")->value() << std::endl;

    //foo <- read.delim(pipe('pbpaste'), sep=',')
    //ggplot(data=foo, aes(x=Time, y=EONIA, colour="EONIA")) + geom_line() + geom_line(aes(x=Time, y=FedFunds, colour="FedFunds")) +ylab("Zero Rate") +  geom_line(aes(x=Time, y=EURIBOR.3M, colour="EURIBOR-3M"))+ geom_line(aes(x=Time, y=EURIBOR.6M, colour="EURIBOR-6M"))+ geom_line(aes(x=Time, y=LIBOR.3M, colour="LIBOR-3M")) + ggtitle("YieldCurves asof 4 Dec 2015") + labs(colour = "Curve")
    //exit(1);


    // engine factory
    cout << "EngineFactory" << endl;
    boost::shared_ptr<EngineFactory> factory = boost::make_shared<EngineFactory>(market);

    // porfolio
    cout << "Load Portfolio" << endl;
    Portfolio portfolio;
    portfolio.load(dir + "instruments.xml");

    portfolio.build(factory);

    cout << "TradeId,Type,Description,Maturity,Notional,NPV,NPV Currency,"
         << "OpenXVA NPV (EUR), QRE NPV (EUR), Summit NPV (EUR),"
         << "OX-QRE bps,OX-Summit bps,QRE-Summit bps" << endl;
    cout << setprecision(2) << showpoint << fixed;
    for (auto trade : portfolio.trades()) {

        Real notional = 0.0;
        Real qreNPV = 0.0;
        Real summitNPV = 0.0;
        string desc = "";
        const map<string,string>& rd = trade->envelope().reportingDimensions();
        auto it = rd.find("Notional");
        if (it != rd.end())
            notional = parseReal(it->second);
        it = rd.find("QRENPV");
        if (it != rd.end())
            qreNPV = parseReal(it->second);
        it = rd.find("SummitNPV");
        if (it != rd.end())
            summitNPV = parseReal(it->second);
        it = rd.find("Description");
        desc = (it == rd.end()) ? "" : it->second;
            
        cout << trade->envelope().id() << ","
             << trade->className() << ","
             << desc << ","
             << io::iso_date(trade->maturity()) << ","
             << notional << ",";
        try {
            Real npv = trade->instrument()->NPV();
            Real eurNPV = npv / market->fxSpot("EUR"+trade->npvCurrency())->value();
            Real oqBPS = 0.0, osBPS = 0.0, qsBPS = 0.0;
            if (notional > 0) {
                oqBPS = 10000 * (eurNPV - qreNPV) / notional;
                osBPS = 10000 * (eurNPV - summitNPV) / notional;
                qsBPS = 10000 * (qreNPV - summitNPV) / notional;
            }

            cout << npv
                 << "," << trade->npvCurrency()
                 << "," << eurNPV << "," << qreNPV << "," << summitNPV
                 << "," << oqBPS << "," << osBPS << "," << qsBPS;
        } catch (...) {
            cout << "#NA,#NA,#NA,#NA,#NA,#NA,#NA,#NA";
        }

        cout << endl;
    }
}

int main () {
    try {
        npv();
    } catch (std::exception& e) {
        cerr << "Exception " << e.what() << endl;
    } catch (...) {
        cerr << "Unhandled Exception" << endl;
    }
    return 0;
}

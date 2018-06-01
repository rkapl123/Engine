/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2016 Quaternion Risk Management Ltd.
  All rights reserved.
*/

#include <iostream>
#include <qlw/wrap.hpp>
#include <qlw/engine/valuationengine.hpp>
#include <ql/time/calendars/all.hpp>
#include <boost/timer.hpp>

using namespace std;
using namespace openxva::model;
using namespace openxva::scenario;
using namespace openxva::portfolio;
using namespace openxva::marketdata;
using namespace openxva::utilities;
using namespace openxva::engine;

#ifdef BOOST_MSVC
#include <qlw/auto_link.hpp>
#include <qle/auto_link.hpp>
#include <ql/auto_link.hpp>
#endif

class PostProcess {
public:
    PostProcess(const boost::shared_ptr<openxva::cube::Cube>& cube,
                Size tradeDim, Size timeDim, Size sampleDim)
        : cube_(cube),
          tradeDimension_(tradeDim),
          timeDimension_(timeDim),
          sampleDimension_(sampleDim) {
    }

    vector<Real> expectedNPV(Size trade);
    vector<Real> expectedExposure(Size trade, bool flip = false);
    vector<Real> quantile(Size trade, Real q);
private:
    Size getx(Size trade, Size time, Size sample);
    Size gety(Size trade, Size time, Size sample);
    Size getz(Size trade, Size time, Size sample);

    boost::shared_ptr<openxva::cube::Cube> cube_;
    Size tradeDimension_;
    Size timeDimension_;
    Size sampleDimension_;   
};

void simulation (bool martingale = false) {

    string dir = "Input/";

    Log::instance().registerLogger(boost::make_shared<FileLogger>("Output/log.txt"));
    Log::instance().switchOn();

    Date asof(23, Oct, 2015);
    Settings::instance().evaluationDate() = asof;

    CSVLoader loader(dir + "market.txt", dir + "fixings.txt");

    CurveConfigurations curveConfigs;
    curveConfigs.fromFile(dir + "curveconfig.xml");

    boost::shared_ptr<Conventions> conventions(new Conventions);
    conventions->fromFile(dir + "conventions.xml");

    cout << "Today's Market Parameters" << endl;
    TodaysMarketParameters marketParameters;
    marketParameters.fromFile(dir + "todaysmarket.xml");
    
    cout << "Today's Market" << endl;
    boost::shared_ptr<Market> market = boost::make_shared<TodaysMarket>
        (asof, marketParameters, loader, curveConfigs, *conventions);

    cout << "Pricing Model Data" << endl;
    boost::shared_ptr<LgmData> lgmData = boost::make_shared<LgmData>();
    lgmData->fromFile(dir + "models.xml", "EUR");

    cout << "Pricing Engine Data" << endl;
    boost::shared_ptr<EngineData> engineData = boost::make_shared<EngineData>();
    engineData->fromFile(dir + "pricingengine.xml");
    
    cout << "Pricing Model" << endl;
    LgmBuilder lgmBuilder(market, lgmData);
    boost::shared_ptr<QuantExt::LGM> lgm = lgmBuilder.model();

    cout << "Engine Factory" << endl;
    boost::shared_ptr<EngineFactory> factory = boost::make_shared<EngineFactory>(market, engineData);

    cout << "Portfolio" << endl;
    boost::shared_ptr<Portfolio> portfolio = boost::make_shared<Portfolio>();
    //portfolio->load(dir + "vanillaswaps.xml");
    portfolio->load(dir + "instruments.xml");
    portfolio->build(factory);

    for (auto trade : portfolio->trades()) {
        cout << trade->envelope().id() << ","
             << trade->className() << ","
             << io::iso_date(trade->maturity()) << ",";
        try {
            Real npv = trade->instrument()->NPV();
            cout << npv << endl;
        } catch (...) {
            cout << "#NA" << endl;
        }
    }
    
    cout << "Simulation Model Data" << endl;
    boost::shared_ptr<CrossAssetModelData> modelData = boost::make_shared<CrossAssetModelData>();
    modelData->fromFile(dir + "simulationmodels.xml");

    cout << "Simulation Model" << endl;
    CrossAssetModelBuilder modelBuilder(market);
    boost::shared_ptr<QuantExt::CrossAssetModel> model = modelBuilder.build(modelData);

    cout << "Simulation Market" << endl;
    boost::shared_ptr<ScenarioSimMarketParameters> simMarketData(new ScenarioSimMarketParameters);
    openxva::utilities::XMLDocument doc(dir + "simulationmarket.xml");
    XMLNode* node = doc.getFirstNode("SimulationMarket");
    simMarketData->fromXML(node);

    cout << "Scenario Generator" << endl;
    ScenarioGeneratorBuilder sb;
    sb.fromFile(dir + "simulationparams.xml");
    boost::shared_ptr<ScenarioGenerator> sg = sb.build(model, simMarketData, asof);
    boost::shared_ptr<openxva::simulation::DateGrid> grid = sb.dateGrid();
        
    cout << "Sim Market" << endl;
    boost::shared_ptr<openxva::simulation::SimMarket> simMarket
        = boost::make_shared<ScenarioSimMarket>(sg, market, simMarketData);

    cout << "Engine Factory linked to Sim Market" << endl;
    boost::shared_ptr<EngineFactory> factorySim = boost::make_shared<EngineFactory>(simMarket, engineData);

    cout << "Portfolio linked to Sim Market" << endl;
    boost::shared_ptr<Portfolio> portfolioSim = boost::make_shared<Portfolio>();
    portfolioSim->load(dir + "vanillaswaps.xml");
    portfolioSim->build(factorySim);

    Size samples = sb.samples();
    ValuationEngine engine(asof, grid, samples, "EUR", simMarket);
    // Size xdim = grid->dates().size();
    // Size ydim = portfolioSim->size();
    // Size zdim = samples;
    Size xdim = portfolioSim->size();
    Size ydim = grid->dates().size();
    Size zdim = samples;
    cout << "Valuation Engine with " << xdim << " trades, " << ydim << " dates, " << zdim << " samples ..." << endl; 
    boost::shared_ptr<openxva::cube::Cube> cube = boost::make_shared<openxva::cube::InMemoryCube<float>>(xdim,ydim,zdim);
    boost::timer timer;
    engine.buildCube(portfolioSim, cube);
    cout << "size/time: " << portfolioSim->size() << " " << timer.elapsed() << endl;
    
    PostProcess post(cube, 1, 0, 2);
    vector<vector<Real>> npv(portfolio->size());
    vector<vector<Real>> exposure1(portfolio->size());
    vector<vector<Real>> exposure2(portfolio->size());
    for (Size i = 0; i < exposure1.size(); ++i) {
        npv[i] = post.expectedNPV(i);
        exposure1[i] = post.expectedExposure(i,false);
        exposure2[i] = post.expectedExposure(i,true);
    }
    
    ofstream file;
    file.open("Output/exposure_1.txt");
    
    file.setf (ios::fixed, ios::floatfield);
    file.setf (ios::showpoint);

    for (Size i = 0; i < exposure1.front().size(); i++) {
        file << setw(2) << i << "  ";
        for(Size j = 0; j < portfolio->size(); ++j) 
            file << setprecision(2) << setw(10) << exposure1[j][i] << " ";
        file << endl;
    }

    file.close();
    
    file.open("Output/exposure_2.txt");
    
    file.setf (ios::fixed, ios::floatfield);
    file.setf (ios::showpoint);

    for (Size i = 0; i < exposure2.front().size(); i++) {
        file << setw(2) << i << "  ";
        for(Size j = 0; j < portfolio->size(); ++j) 
            file << setprecision(2) << setw(10) << exposure2[j][i] << " ";
        file << endl;
    }

    file.close();

    file.open("Output/npv.txt");
    
    file.setf (ios::fixed, ios::floatfield);
    file.setf (ios::showpoint);

    for (Size i = 0; i < npv.front().size(); i++) {
        file << setw(2) << i << "  ";
        for(Size j = 0; j < portfolio->size(); ++j) 
            file << setprecision(2) << setw(10) << npv[j][i] << " ";
        file << endl;
    }

    file.close();

    cout << "done." << endl;
}

int main (int argc, char **argv) {

    bool martingale = false;
    if (argc > 1)
        martingale = true;
    try {
        simulation(martingale);
    } catch (std::exception& e) {
        cerr << "Exception " << e.what() << endl;
    } catch (...) {
        cerr << "Unhandled Exception" << endl;
    }
    return 0;
}


Size PostProcess::getx(Size trade, Size time, Size sample) {
    if (tradeDimension_ == 0)
        return trade;
    else if (timeDimension_ == 0)
        return time;
    else
            return sample;
}

Size PostProcess::gety(Size trade, Size time, Size sample) {
    if (tradeDimension_ == 1)
        return trade;
    else if (timeDimension_ == 1)
        return time;
    else
        return sample;
}

Size PostProcess::getz(Size trade, Size time, Size sample) {
    if (tradeDimension_ == 2)
        return trade;
    else if (timeDimension_ == 2)
        return time;
    else
        return sample;
}

vector<Real> PostProcess::expectedNPV(Size trade) {
    QL_REQUIRE(trade < cube_->dim(tradeDimension_), "trade number out of range");
    vector<Real> result(cube_->dim(timeDimension_), 0.0);
    Size samples = cube_->dim(sampleDimension_);
    Size timeSteps = cube_->dim(timeDimension_);
    
    for (Size time = 0; time < timeSteps; ++time) {
        for (Size sample = 0; sample < samples; ++sample) {
            Size x = getx(trade, time, sample);
            Size y = gety(trade, time, sample);
            Size z = getz(trade, time, sample);
            Real npv = cube_->get(x, y, z);
            result[time] += npv/samples;
        }
    }
    return result;
}

vector<Real> PostProcess::expectedExposure(Size trade, bool flip) {
    QL_REQUIRE(trade < cube_->dim(tradeDimension_), "trade number out of range");
    vector<Real> result(cube_->dim(timeDimension_), 0.0);
    Size samples = cube_->dim(sampleDimension_);
    Size timeSteps = cube_->dim(timeDimension_);
    
    for (Size time = 0; time < timeSteps; ++time) {
        for (Size sample = 0; sample < samples; ++sample) {
            Size x = getx(trade, time, sample);
            Size y = gety(trade, time, sample);
            Size z = getz(trade, time, sample);
            Real npv = cube_->get(x, y, z);
            if (flip) npv = -npv;
            result[time] += std::max(npv, 0.0)/samples;
        }
    }
    return result;
}

//
//  main.cpp
//  qltest_vasicek_callablebonds
//
//  Created by Zesheng Li on 2/28/16.
//  Copyright © 2016 neil. All rights reserved.
//
//  This file is based on the QuantLib example CallableBonds.
//  The original example was written by Allen Kuo.
//  QuantLib is a free-software/open-source library
//  for financial quantitative analysts and developers - http://quantlib.org/

//  The original example sets up a callable fixed rate bond with a Hull White pricing
//  engine and compares to Bloomberg's Hull White price/yield calculations.
//  I re-engineered it here and there, also added Vasicek as an additional
//  PricingEngine to compare with Hull-White.
//  This file is not intended for business use, but for academic purposes only.
//  - Zesheng "Neil" Li
//  <rexeneo@gmail.com>


#include <iostream>
#include <boost/timer.hpp>
#include <ql/quantlib.hpp>
using namespace std;
using namespace QuantLib;

boost::shared_ptr<YieldTermStructure>
flatRate(const Date& today,
         const boost::shared_ptr<Quote>& forward,
         const DayCounter& dc,
         const Compounding& compounding,
         const Frequency& frequency) {
    return boost::shared_ptr<YieldTermStructure>(new FlatForward(today,
                                                                 Handle<Quote>(forward),
                                                                 dc,
                                                                 compounding,
                                                                 frequency)
                                                 );
}

boost::shared_ptr<YieldTermStructure>
flatRate(const Date& today,
         Rate forward,
         const DayCounter& dc,
         const Compounding &compounding,
         const Frequency &frequency) {
    return flatRate(today,
                    boost::shared_ptr<Quote>(new SimpleQuote(forward)),
                    dc,
                    compounding,
                    frequency);
}

int main(int argc, const char * argv[]) {
    try {
        boost::timer timer;
        
        Date today = Date(16,October,2007);
        Settings::instance().evaluationDate() = today;
        
        cout <<  endl;
        cout << "This example prices a callable fixed rate bond using" << endl;
        cout << "Hull White model w/ reversion parameter = 0.03" << endl;
        cout << "Vasicek model w/ long term mean & r0 = 0.055" <<endl;
        cout << "& same reversion parameter" <<endl;
        cout << endl;
        cout << "The bond:" <<endl;
        cout << "BAC4.65 09/15/12  ISIN: US06060WBJ36" << endl;
        cout << "roughly five year tenor, ";
        cout << "quarterly coupon and call dates" << endl;
        cout << "reference date is : " << today << endl << endl;
        
        /* Bloomberg OAS1: "N" model (Hull White)
         varying volatility parameter
         
         The curve entered into Bloomberg OAS1 is a flat curve,
         at constant yield = 5.5%, semiannual compounding.
         Assume here OAS1 curve uses an ACT/ACT day counter,
         as documented in PFC1 as a "default" in the latter case.
         */
        
        // set up a flat curve corresponding to Bloomberg flat curve
        
        Rate bbCurveRate = 0.055;
        DayCounter bbDayCounter = ActualActual(ActualActual::Bond);
        InterestRate bbIR(bbCurveRate,bbDayCounter,Compounded,Semiannual);
        
        Handle<YieldTermStructure> termStructure(flatRate(today,
                                                          bbIR.rate(),
                                                          bbIR.dayCounter(),
                                                          bbIR.compounding(),
                                                          bbIR.frequency()));
        
        
        // set up the call schedule
        
        CallabilitySchedule callSchedule;   // Comment: a vector of Callability*   -neil
        Real callPrice = 100.;
        Size numberOfCallDates = 24;
        Date callDate = Date(15,September,2006);
        
        for (Size i=0; i< numberOfCallDates; i++) {
            Calendar nullCalendar = NullCalendar();
            
            Callability::Price myPrice(callPrice,
                                       Callability::Price::Clean);
            callSchedule.push_back(boost::shared_ptr<Callability>(new Callability(myPrice,
                                                                                  Callability::Call,
                                                                                  callDate )));
    //        cout<<callSchedule[i]->date()<<endl;    // by neil
            callDate = nullCalendar.advance(callDate, 3, Months);
        }
        
        /******** set up the callable bond ********/
        
        Date dated = Date(16,September,2004);
        Date issue = dated;
        Date maturity = Date(15,September,2012);
        Natural settlementDays = 3;  // Bloomberg OAS1 settle is Oct 19, 2007
        Calendar bondCalendar = UnitedStates(UnitedStates::GovernmentBond);
        Real coupon = .0465;
        Frequency frequency = Quarterly;
        Real redemption = 100.0;
        Real faceAmount = 100.0;
        
        /* The 30/360 day counter Bloomberg uses for this bond cannot
         reproduce the US Bond/ISMA (constant) cashflows used in PFC1.
         Therefore use ActAct(Bond)
         */
        DayCounter bondDayCounter = ActualActual(ActualActual::Bond);
        
        // PFC1 shows no indication dates are being adjusted
        // for weekends/holidays for vanilla bonds
        BusinessDayConvention accrualConvention = Unadjusted;
        BusinessDayConvention paymentConvention = Unadjusted;
        
        Schedule sch(dated, maturity, Period(frequency), bondCalendar,
                     accrualConvention, accrualConvention,
                     DateGeneration::Backward, false);  // Comment: Payment schedule -neil
        
        Size maxIterations = 1000;
        Real accuracy = 1e-8;
        Integer gridIntervals = 40; // timeSteps -neil
        Real reversionParameter = .03;

        // output price/yield results for varying volatility parameter
        

    //    Real sigma=QL_EPSILON; // core dumps if sigma=zero on Cygwin
    //    // HullWhite Engine
    //    boost::shared_ptr<ShortRateModel> hw0(new HullWhite(termStructure,reversionParameter,sigma));
    //    cout<<boost::static_pointer_cast<HullWhite>(hw0)->a()<<endl;  // neil testing
    //    cout<<boost::static_pointer_cast<HullWhite>(hw0)->b()<<endl;
    //    cout<<boost::static_pointer_cast<HullWhite>(hw0)->sigma()<<endl;
    //    cout<<boost::static_pointer_cast<HullWhite>(hw0)->lambda()<<endl;
    //    
    //    boost::shared_ptr<PricingEngine> engine0(new TreeCallableFixedRateBondEngine(hw0,gridIntervals));
        // commented out by neil. moved to print loop.
        
        
        /******** bond object ********/
        CallableFixedRateBond callableBond(settlementDays, faceAmount, sch,
                                           vector<Rate>(1, coupon),
                                           bondDayCounter, paymentConvention,
                                           redemption, issue, callSchedule);
    //    callableBond.setPricingEngine(engine0);   // moved to print loop -neil
        
        /******** setting engines and print price ********/
        
        for (auto sigma: vector<Real>({QL_EPSILON, .01, .03, .06, .12})) {
            // core dumps if sigma=zero on Cygwin // Comment: Defines the machine precision for operations over doubles
            
            // HullWhite Engine
            boost::shared_ptr<ShortRateModel> hw(new HullWhite(termStructure,reversionParameter,sigma));
            boost::shared_ptr<PricingEngine> engine_hw(new TreeCallableFixedRateBondEngine(hw,gridIntervals));
            
            // Vasicek Engine   -by neil
            boost::shared_ptr<ShortRateModel> vsc(new Vasicek(bbCurveRate,
                                                              reversionParameter,
                                                              bbCurveRate,
                                                              sigma)
                                                  );
            boost::shared_ptr<PricingEngine> engine_vsc(new TreeCallableFixedRateBondEngine(vsc,
                                                                                            gridIntervals,
                                                                                            termStructure)
                                                        );
            // Vasicek Model doesn't provide termStructure, need to specify it for PricingEngine object. -by neil
            
            
            // print -by neil
            cout << setprecision(2)
            << showpoint
            << fixed
            << "sigma/vol (%) = "
            << 100.*sigma
            << endl;
            
            callableBond.setPricingEngine(engine_vsc);
            cout << "QL Vasicek price/yld (%)   ";
            cout << callableBond.cleanPrice() << " / "
            << 100. * callableBond.yield(bondDayCounter,
                                         Compounded,
                                         frequency,
                                         accuracy,
                                         maxIterations)
            << endl;
            
            callableBond.setPricingEngine(engine_hw);
            cout << "QL HullWhite price/yld (%) ";
            cout << callableBond.cleanPrice() << " / "
            << 100. * callableBond.yield(bondDayCounter,
                                         Compounded,
                                         frequency,
                                         accuracy,
                                         maxIterations)
            << endl;
            
            cout << "Bloomberg price/yld (%)    ";
            if (fabs(sigma - QL_EPSILON) < QL_EPSILON) cout << "96.50 / 5.47";
            else if (fabs(sigma - .01) < QL_EPSILON) cout << "95.68 / 5.66";
            else if (fabs(sigma - .03) < QL_EPSILON) cout << "92.34 / 6.49";
            else if (fabs(sigma - .06) < QL_EPSILON) cout << "87.16 / 7.83";
            else if (fabs(sigma - .12) < QL_EPSILON) cout << "77.31 / 10.65";
            cout << endl
            << endl;
        }
        

        
        double seconds = timer.elapsed();
        Integer hours = int(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = int(seconds/60);
        seconds -= minutes * 60;
        cout << " \nRun completed in ";
        if (hours > 0)
            cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            cout << minutes << " m ";
        cout << fixed << setprecision(1)
        << seconds << " s\n" << endl;
        return 0;
        
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }

}

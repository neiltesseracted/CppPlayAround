//
//  main.cpp
//  qltest
//
//  Created by Zesheng Li on 2/25/16.
//  Copyright © 2016 neil. All rights reserved.
//

#include <cstdlib>
#include <iostream>
#ifndef BOOST_AUTO_TEST_MAIN
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <ql/quantlib.hpp>
#include <boost/format.hpp>
#include <boost/math/distributions.hpp>
#include <boost/function.hpp>

namespace {
    
    using namespace QuantLib;
    
    Real expectedValueCallPayoff(Real spot, Real strike, Rate r, Volatility sigma, Time t, Real x) {
        Real mean = log(spot)+(r - 0.5 * sigma * sigma) * t;
        Real stdDev = sigma * sqrt(t);
        boost::math::lognormal d(mean, stdDev);
        return PlainVanillaPayoff(Option::Type::Call, strike)(x) * boost::math::pdf(d, x);
    }
    
    BOOST_AUTO_TEST_CASE(testPriceCallOption) {
        Real spot = 100.0; //current price of the underlying stock (S)
        Rate r = 0.03; //risk-free rate
        Time t = 0.5;  //half a year
        Volatility vol = 0.20; //estimated volatility of underlying
        Real strike = 110.0; // strike price of the call (K)
        
        //b need not be infinity, but can just be a large number that is improbable
        Real a = strike, b = strike * 10.0;
        
        boost::function< Real(Real) > ptrToExpectedValueCallPayoff =
        boost::bind(&expectedValueCallPayoff, spot, strike, r, vol, t, _1);
        
        Real absAcc = 0.00001;
        Size maxEval = 1000;
        SimpsonIntegral numInt(absAcc, maxEval);
        
        /* numerically integrate the call option payoff function from a to b and
         * calculate the present value using the risk free rate as the discount factor
         */
        Real callOptionValue = numInt(ptrToExpectedValueCallPayoff, a, b) * std::exp(-r * t);
        
        std::cout << "Call option value is: " << callOptionValue << std::endl;
    }
}

#endif
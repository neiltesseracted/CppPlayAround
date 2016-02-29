//
//  main.cpp
//  boostplayaround
//
//  Created by Zesheng Li on 2/24/16.
//  Copyright © 2016 neil. All rights reserved.
//

#include <iostream>
#include <boost/math/special_functions.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>

void ex1(){
    using namespace boost::numeric::ublas;
    matrix<double> m (3, 3);
    for (unsigned i = 0; i < m.size1 (); ++ i)
        for (unsigned j = 0; j < m.size2 (); ++ j)
            m (i, j) = 3 * i + j;
    std::cout << -m << std::endl;
}

void ex2(){
    using namespace boost::numeric::ublas;
    matrix<std::complex<double> > m (3, 3);
    for (unsigned i = 0; i < m.size1 (); ++ i)
        for (unsigned j = 0; j < m.size2 (); ++ j)
            m (i, j) = std::complex<double> (3 * i + j, 3 * i + j);
    
    std::cout << - m << std::endl;
    std::cout << "conj: "<< conj (m) << std::endl;
    std::cout << "real: "<< real (m) << std::endl;
    std::cout << "imag: "<<imag (m) << std::endl;
    std::cout << "trans: "<<trans (m) << std::endl;
    std::cout << "herm: "<<herm (m) << std::endl;
}
using namespace boost::numeric::ublas;
template<class T>
bool InvertMatrix(const matrix<T>& input, matrix<T>& inverse)
{
    typedef permutation_matrix<std::size_t> pmatrix;
    
    // create a working copy of the input
    matrix<T> A(input);
    
    // create a permutation matrix for the LU-factorization
    pmatrix pm(A.size1());
    
    // perform LU-factorization
    int res = lu_factorize(A, pm);
    if (res != 0)
        return false;
    
    // create identity matrix of "inverse"
    inverse.assign(identity_matrix<T> (A.size1()));
    
    // backsubstitute to get the inverse
    lu_substitute(A, pm, inverse);
    
    return true;
}
void ex3(){
//    double initialValues[3][3] = {
//        1, 2, 3,
//        5, 1, 4,
//        6, 7, 1
//    };
    
    matrix<double> A(3, 3), Z(3, 3);
    
    for (unsigned i = 0; i < A.size1 (); ++ i)
        for (unsigned j = 0; j < A.size2 (); ++ j)
            A (i, j) = 3 * i + j;
    InvertMatrix(A, Z);
    
    std::cout << "A=" << A << std::endl << "Z=" << Z << std::endl;
}

void ex4(){
    using namespace boost::numeric::ublas;
    matrix<double> m1 (3, 3), m2 (3, 3);
    for (unsigned i = 0; i < std::min (m1.size1 (), m2.size1 ()); ++ i)
        for (unsigned j = 0; j < std::min (m1.size2 (), m2.size2 ()); ++ j)
            m1 (i, j) = m2 (i, j) = 3 * i + j;
    
    std::cout << "m1="<<m1<<std::endl<<"m2="<<m2<<std::endl<<"prod=" <<prod (m1, m2) << std::endl;
}

void ex5(){
    using namespace boost::numeric::ublas;
    matrix<double> A (5, 5);
    A(0, 0) =  2.; A(0, 1) =  3.;
    A(1, 0) =  3.; A(1, 2) =  4.; A(1, 4) =  6.;
    A(2, 1) = -1.; A(2, 2) = -3.; A(2, 3) =  2.;
    A(3, 2) =  1.;
    A(4, 1) =  4.; A(4, 2) =  2.; A(4, 4) =  1.;
    
    vector<double> y(5);
    y(0) =  8.;
    y(1) = 45.;
    y(2) = -3.;
    y(3) =  3.;
    y(4) = 19.;
//    matrix<double> y(5,1);
//        y(0,0) =  8.;
//        y(1,0) = 45.;
//        y(2,0) = -3.;
//        y(3,0) =  3.;
//        y(4,0) = 19.;
    std::cout<<y<<std::endl;
    permutation_matrix<size_t> pm(A.size1()); std::cout<<pm<<std::endl;
    lu_factorize(A, pm);
    lu_substitute(A, pm, y);
    
    std::cout << pm<<std::endl<<y << std::endl;
}





int main(int argc, const char * argv[]) {
//    ex1();
//    ex2();
//    ex3();
//    ex4();
    ex5();
    
    return 0;
}

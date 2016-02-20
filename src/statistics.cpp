/**
  * Implements statistical methods for line-fitting
  * and outlier detection.
  *
  * Author: phugen
  */

#include "include/statistics.hpp"

#include <random>
#include <chrono>
#include <type_traits>


using namespace std;
using namespace cv;



template<
    typename T, //real type
    typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
> struct S{};

// Returns the median of the input list.
template<typename T> double median (vector<T> elements)
{
    static_assert(is_arithmetic<T>::value, "Non-arithmetic typed passed to median().");

    if(elements.size() == 0)
        return 0.;

    // sort list
    sort(elements.begin(), elements.end());

    // number of elements is odd
    if((elements.size() % 2) == 1)
    {
        return elements.at(elements.size() / 2);
    }

    // number of elements is even
    else
        return 0.5 * (elements.at(((elements.size() + 1) / 2)) + elements.at(((elements.size() + 1) / 2) - 1));
}

// Perform Least Squares for a 2D-line.
parametricLine leastSquaresLine(std::vector<ConnectedComponent> cluster)
{
    // only one data point, fitting impossible
    assert(cluster.size() > 1);

    double reci_term, reci_term_2, m, m_2, m_3, b, b_2, b_3, b_4;
    reci_term = reci_term_2 = m = m_2 = m_3 = b = b_2 = b_3 = b_4 = 0.;

    // calculate reciprocal and matrix terms
    // (using pos in list as x and area of component at pos x as y)

    // reci_term: n * (sum(i=1 to n) [(x_i)^2]) - (sum(i=1 to n) [x_i])^2
    for(int i = 0; i < (int) cluster.size(); i++)
    {
        reci_term += (i * i);
    }

    for(int i = 0; i < (int) cluster.size(); i++)
    {
        reci_term_2 += i;
    }
    reci_term_2 *= reci_term_2;
    reci_term = (reci_term * cluster.size()) - reci_term_2;


    // m^-1: n * (sum(i=1 to n) [x_i * y_i]) - (sum(i=1 to n) [x_i]) * sum(i=1 to n) [y_i])
    for(int i = 0; i < (int) cluster.size(); i++)
    {
        m += (i * cluster.at(i).area);
    }

    for(int i = 0; i < (int) cluster.size(); i++)
    {
        m_2 += i;
    }

    for(int i = 0; i < (int) cluster.size(); i++)
    {
        m_3 += cluster.at(i).area;
    }

    m = (cluster.size() * m) - (m_2 * m_3);


    // b^-1: ((sum(i=1 to n) [(x_i)^2]) * (sum(i=1 to n) [y_i])) - (sum(i=1 to n) [x_i] * sum(i=1) to n)[x_i * y_i])
    for(int i = 0; i < (int) cluster.size(); i++)
    {
        b += (i * i);
    }

    for(int i = 0; i < (int) cluster.size(); i++)
    {
        b_2 += cluster.at(i).area;
    }

    for(int i = 0; i < (int) cluster.size(); i++)
    {
        b_3 += i;
    }

    for(int i = 0; i < (int) cluster.size(); i++)
    {
        b_4 += (i * cluster.at(i).area);
    }

    b = (b * b_2) - (b_3 * b_4);


    // [m, b]^-1 = [m, b] * (1 / reci_term)
    m *= 1. / reci_term;
    b *= 1. / reci_term;

    parametricLine line;
    line.m = m;
    line.b = b;

    return line;
}

// Perform Least Median Squares for a 2D-line, that is, for k = 2.
//
// p = needed probability of 1 non-outlier in m_min
// t = assumed percentage of outliers in entire set
parametricLine leastMedianSquaresLine(std::vector<ConnectedComponent> cluster, float p, float t)
{
    // Setup robust line fitting
    double e_min = INT_MAX; // current error
    int k = 2; // number of parameters needed to define suspected fitting function (here: line)
    int m_min = ceil(log(1 - p) / log(1 - pow((1 - t), k))); // number of random points that are inspected
    parametricLine finalLine; // final fitted line parameters

    // Setup random number generator
    typedef std::chrono::high_resolution_clock clock;
    clock::time_point beginning = clock::now();
    clock::duration d = clock::now() - beginning;

    unsigned seed = d.count();
    std::minstd_rand0 rnd(seed);


    // Perform fitting by Least Median Squares (LMS)
    for(int i = 0; i < m_min; i++)
    {
        vector<ConnectedComponent> subSample;
        parametricLine line;
        vector<double> errors;
        double e_med; // median of squared errors

        // find k random points p = (pos, area)
        for(int i = 0; i < k; i++)
        {
            uniform_int_distribution<> dis(0, distance(cluster.begin(), cluster.end()) - 1);
            subSample.push_back(cluster.at(dis(rnd)));
        }

        // calculate least squares for subsample
        line = leastSquaresLine(subSample);

        // calculate median error for fitted line
        for(int i = 0; i < (int) cluster.size(); i++)
        {
            // squared error: y-difference from prognosis y value
            double cur = abs((cluster.at(i).area) - (line.m * i + line.b));
            cur *= cur;

            errors.push_back(cur);
        }

        e_med = median(errors);

        // accept these parameters only if they are
        // better than the previous parameters
        if(e_med < e_min)
        {
            e_min = e_med;
            finalLine = line;
        }
    }

    return finalLine;
}




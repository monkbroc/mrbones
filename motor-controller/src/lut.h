#pragma once
#include <vector>

template <typename T, typename U>
U interpolate(
    std::vector<T> ranges,
    std::vector<U> values,
    T input
) {
    if (ranges.size() != values.size() || ranges.size() < 1) {
        return 0;
    }
    auto rangeStart = ranges.front();
    auto valueStart = values.front();
    // input out of range -> return first value
    if (input <= rangeStart) {
        return valueStart;
    }
    for (auto it = ++ranges.begin(), it2 = ++values.begin(); it < ranges.end(); ++it, ++it2) {
        auto rangeEnd = *it;
        auto valueEnd = *it2;

        // did we find the right range?
        if (input <= rangeEnd) {
            if (rangeEnd > rangeStart) {
                auto rangeDiff = rangeEnd - rangeStart;
                auto valueDiff = valueEnd - valueStart;
                auto inputOffset = input - rangeStart;

                // interpolate
                return (valueDiff * inputOffset / rangeDiff) + valueStart;
            } else {
                // range is zero -> nothing to interpolate
                return valueStart;
            }
        }
        rangeStart = rangeEnd;
        valueStart = valueEnd;
    }
    // input out of range -> return last value
    return values.back();
}

#ifndef PLATFORM_ID
// tests

#include <iostream>

int failures = 0;

template <typename T>
void expect(T actual, T expected) {
    if (expected != actual) {
        failures++;
        std::cout << "Failed expected=" << expected << " actual=" << actual << std::endl;
    }
}

int main() {
    std::vector<int> range1 = { 0, 10, 20 };
    std::vector<int> values1 = { 100, 200, 400 };

    expect(interpolate(range1, values1, -10), 100);
    expect(interpolate(range1, values1, 0), 100);
    expect(interpolate(range1, values1, 1), 110);
    expect(interpolate(range1, values1, 5), 150);
    expect(interpolate(range1, values1, 10), 200);
    expect(interpolate(range1, values1, 15), 300);
    expect(interpolate(range1, values1, 20), 400);
    expect(interpolate(range1, values1, 25), 400);

    return failures;
}
    

#endif
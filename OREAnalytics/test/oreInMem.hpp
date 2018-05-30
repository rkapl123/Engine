#pragma once

#include <boost/test/unit_test.hpp>

namespace testsuite {

class OreInMemTest {
public:
    //! Test storing and retrieving a few artificial data points
    static void testStartOreInMem();
    static boost::unit_test_framework::test_suite* suite();
};
} // namespace testsuite

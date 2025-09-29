## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.

set(CTEST_PROJECT_NAME "Theater")
set(CTEST_NIGHTLY_START_TIME "00:00:00 EST")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=Theater")
set(CTEST_DROP_SITE_CDASH TRUE)

# Additional CTest configuration
set(CTEST_TEST_TIMEOUT 300)  # 5 minutes timeout for tests
set(CTEST_OUTPUT_ON_FAILURE TRUE)
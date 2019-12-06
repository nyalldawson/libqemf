INCLUDE_DIRECTORIES(
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

INCLUDE_DIRECTORIES(SYSTEM
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${Qt5Test_LIBRARIES}
                    )

FIND_PACKAGE(Qt5Test REQUIRED)

SET(CMAKE_AUTOMOC ON)

MACRO (ADD_TEST_WRAPPER TESTSRC)
  SET (TESTNAME  ${TESTSRC})
  STRING(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  SET (TESTNAME  "${TESTNAME}")
  ADD_EXECUTABLE(${TESTNAME} ${TESTSRC})
  TARGET_LINK_LIBRARIES(${TESTNAME}
    ${Qt5Core_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    libemf)
  ADD_TEST(${TESTNAME} ${CMAKE_BINARY_DIR}/tests/${TESTNAME} -maxwarnings 10000)
ENDMACRO (ADD_TEST_WRAPPER)

#############################################################
# Tests:

SET(TESTS
 testlibqemf.cpp
)

FOREACH(TESTSRC ${TESTS})
    ADD_TEST_WRAPPER(${TESTSRC})
ENDFOREACH(TESTSRC)

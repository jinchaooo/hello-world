test_a_SOURCES = test_a.cc
test_a_LDADD = -lboost_system -ldl -lpthread
test_a_CXXFLAGS = --std=c++11 -Wall -Wextra \
            -Wno-unknown-warning-option -Wno-unused-parameter \
            -Wno-unused-local-typedefs -Wno-null-dereference \
            -DGTEST_USE_OWN_TR1_TUPLE=1 -D__STDC_FORMAT_MACROS
bin_PROGRAMS = test_a


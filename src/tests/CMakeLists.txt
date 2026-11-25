#
# -- GLE regression tests
#

add_test(
    NAME basic_math
    COMMAND $<TARGET_FILE:gle> "${CMAKE_CURRENT_SOURCE_DIR}/basic_math.gle"
)

set_tests_properties(
    basic_math
    PROPERTIES
        TIMEOUT 10
)
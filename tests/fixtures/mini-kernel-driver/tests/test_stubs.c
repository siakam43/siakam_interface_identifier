/* tests/test_stubs.c — test stubs and mock functions */

#include <linux/types.h>

/*
 * [SIAKAM_EXPECT] exclude
 * exclusion_reason=quick_exclusion_test_code
 *
 * Reason: File path contains "test" as a path component (tests/test_stubs.c).
 * This function resides in test infrastructure, not production code.
 */
int mock_test_handler(int cmd, void *data)
{
    if (cmd == 0)
        return 42;
    return -1;
}

/*
 * [SIAKAM_EXPECT] exclude
 * exclusion_reason=quick_exclusion_empty_body
 *
 * Reason: Function body is effectively empty — only returns 0 with no logic.
 * This is a stub/placeholder, not a real interface.
 */
int empty_noop(void)
{
    return 0;
}

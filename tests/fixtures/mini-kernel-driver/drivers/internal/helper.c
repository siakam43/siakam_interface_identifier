/* drivers/internal/helper.c — internal utility functions */

#include <linux/types.h>
#include <linux/string.h>

/*
 * [SIAKAM_EXPECT] confidence=medium
 * interface_type=callback_table_entry
 * registration: unknown — identified by implementation analysis
 * external_module: external configuration source
 * data_flow: external config data → config_parser(const u8 *, size_t, struct config_out *)
 *
 * Reason: No grep registration point found, but function parses structured TLV
 * (Type-Length-Value) data from a const u8 * buffer — an external data pattern.
 * The buffer pointer and size parameters suggest data originates outside this
 * module. Medium confidence because the registration mechanism (possibly macro-based
 * or indirect callback table) is not visible to grep.
 */
int config_parser(const u8 *data, size_t len, struct config_out *out)
{
    size_t pos = 0;
    while (pos + 2 <= len) {
        u8 type  = data[pos++];
        u8 vlen  = data[pos++];
        if (pos + vlen > len)
            return -EINVAL;
        out->entries[type].value = data[pos];
        pos += vlen;
    }
    return 0;
}

/*
 * [SIAKAM_EXPECT] exclude
 * exclusion_reason=no_registration_no_interface_characteristics
 *
 * Reason: Pure internal computation. Adds two integers. No external data source,
 * no registration point, no interface-characteristic signature. Used only by
 * other functions within the same module.
 */
int internal_add(int a, int b)
{
    return a + b;
}

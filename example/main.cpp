#include <cassert>
#include <cstdio>
#include <jacobs_register_helper.h>

DECLARE_REGISTER_32(
  link_capabilites_register,
  max_link_speed, 0, 3,
  max_link_width, 4, 9,
  aspm_support, 10, 11,
  l0s_exit_latency, 12, 14,
  l1_exit_latency, 15, 17,
  clock_power_management, 18, 18,
  surprise_down_error_reporting_capable, 19, 19,
  data_link_layer_link_active_reporting_capable, 20, 20,
  link_bandwidth_notification_capability, 21, 21,
  aspm_optionality_compliance, 22, 22,
  port_number, 24, 31
);

int main (int argc, char *argv[]) {
    // Check setting whole register
    link_capabilites_register link_cap_reg;
    link_cap_reg.set_register_value(0xDEADBEEF);
    assert(link_cap_reg.get_register_value() == 0xDEADBEEF);

    // Check individual get method
    assert(link_cap_reg.get_aspm_support() == 0b11);

    // Check the clear method
    link_cap_reg.clear_register_value();
    assert(link_cap_reg.get_register_value() == 0x0);

    // Check individual set method
    link_cap_reg.set_max_link_speed(0xF);
    assert(link_cap_reg.get_register_value() == 0x0000'000F);

    return 0;
}

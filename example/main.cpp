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

// WIP, IMPLEMENTATION NOT COMPLETE
DECLARE_REGISTER_16_WITH_PERMS(
    link_control_register,
    aspm_control, 0, 1, REGISTER_PERMS::READ_WRITE,
    root_completion_boundary, 3, 3, REGISTER_PERMS::READ,
    link_disable, 4, 4, REGISTER_PERMS::READ_WRITE,
    retrain_link, 5, 5, REGISTER_PERMS::READ_WRITE,
    common_clock_configuration, 6, 6, REGISTER_PERMS::READ_WRITE,
    extended_sync, 7, 7, REGISTER_PERMS::READ_WRITE,
    enable_clock_power_management, 8, 8, REGISTER_PERMS::READ_WRITE,
    hardware_autonomous_width_disable, 9, 9, REGISTER_PERMS::READ_WRITE,
    link_bandwidth_management_interrupt_enable, 10, 10, REGISTER_PERMS::READ_WRITE,
    link_autonomous_bandwidth_interrupt_enable, 11, 11, REGISTER_PERMS::READ_WRITE
)

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

    // Test writing to read only bit
    link_control_register link_ctrl_reg;
    assert(link_ctrl_reg.get_register_value() == 0x0);
    // Asserting that the write returns a failure
    assert(link_ctrl_reg.set_root_completion_boundary(1) == false);
    // Assert that the change has not occurred anyway
    assert(link_ctrl_reg.get_register_value() == 0x0);

    // Test that you can read/write to a read_write bit field
    assert(link_ctrl_reg.set_link_disable(1) == true);
    assert(link_ctrl_reg.get_register_value() == 0b10000);
    assert(link_ctrl_reg.get_link_disable() == 0b1);

    return 0;
}

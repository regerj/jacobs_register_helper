# Documentation
> [!NOTE]
> I will document the 32 bit register macros, but this extends to every size of register. The implementations are near identical.

## Contents
<!--ts-->
  * [Declaring a Register](#declaring-a-register)
  * [Writing the Whole Register](#writing-the-whole-register)
  * [Reading the Whole Register](#reading-the-whole-register)
  * [Clearing the Whole Register](#clearing-the-whole-register)
  * [Reading Fields](#reading-fields)
  * [Writing Fields](#writing-fields)
  * [Permission Protected Registers (WIP)](#permission-protected-registers)
<!--te-->

## Declaring a Register

Let's say we are working with a PCIe driver, and we need to control PCIe registers. For this example we will work with the Link Capabilities Register, the definition of which can be found in the **PCI Express Base r3.0** specification in **Section 7.8.6**. The register definition is as follows:

| Bit Location | Description |
| --- | --- |
| 3:0 | Max Link Speed |
| 9:4 | Max Link Width |
| 11:10 | Active State Power Management Support |
| 14:12 | L0s Exit Latency |
| 17:15 | L1 Exit Latency |
| 18 | Clock Power Management |
| 19 | Surprise Down Error Reporting Capable |
| 20 | Data Link Layer Link Active Reporting Capable |
| 21 | Link Bandwidth Notification Capability |
| 22 | ASPM Optionality Compliance |
| 23 | Reserved |
| 31:24 | Port Number |

Let's say we want to figure out if ASPM is supported, we will need to check bits 11:10.

The encoding for those bits is:

| Encoding | Meaning |
| --- | --- |
| 00b | No ASPM Support |
| 01b | L0s Supported |
| 10b | L1 Supported |
| 11b | L0s and L1 Supported |

The declaration of the register class would look like the below snippet. I recommend placing all of your register declarations into a single `registers.h` file for portability into any and all files that will require working with registers.

```cpp
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
```

The declaration is done by first providing a name for the register class, followed by any (reasonable) number of fields for the register. These fields need to each be comprised of 3 arguments. First, a name for the field. Second, the low bit of the field. Finally, the high bit of the field. This can be repeated for each of the fields in your register.

This is only the declaration of the register object. It defines the class of the name you passed into it. When you need to work with this regsiter, you can instantiate it like so:

```cpp
link_capabilites_register link_cap_reg;
```

## Writing the Whole Register
```cpp
link_capabilites_register link_cap_reg;
link_cap_reg.set_register_value(0xDEAD'BEEF);
```

This will set the underlying 32 bit integer to be `0xDEADBEEF`. This would most often be used to populate the register using some `read_register()` method that might exist in your codebase. See below:

```cpp
uint32_t read_register(uint32_t offset) {
    /* --snip-- */
}

/* --snip-- */

link_capabilites_register link_cap_reg;
link_cap_reg.set_register_value(read_register(0x1234'5678));
```

This will populate the `link_cap_reg` with whatever is at the address `0x1234'5678`.

## Reading the Whole Register
```cpp
link_capabilites_register link_cap_reg;
uint32_t register_value = link_cap_reg.get_register_value();
```

`register_value` now contains the raw register value. This can be useful if you have some write register function like:

```cpp
void write_register(uint32_t offset, uint32_t value) {
    /* --snip-- */
}

/* --snip-- */

link_capabilites_register link_cap_reg;
link_cap_reg.set_register_value(0xFFFF'FFFF);
write_register(0xDEAD'BEEF, link_cap_reg.get_register_value());
```

This code will write the value `0xFFFF'FFFF` to the address `0xDEAD'BEEF`.

## Clearing the Whole Register
```cpp
uint32_t read_register(uint32_t offset) {
    /* --snip-- */
}

/* --snip-- */

link_capabilites_register link_cap_reg;
link_cap_reg.set_register_value(read_register(0x1234'5678));

// Do some work with link_cap_reg

// You can clear the register if needed to reuse the same object
link_cap_reg.clear_register_value();
```

This is a niche use case, but it's available if you want it. It is pretty much the same as just setting the register value to be `0x0`.

## Reading Fields
Reading fields is done by calling the get method for that respective field. This method will be called `get_ + {field name}`, for example: `get_your_field_name()`.
```cpp
uint32_t read_register(uint32_t offset) {
    /* --snip-- */
}

/* --snip-- */

// Assume that *(0x1234'5678) == 0xDEADBEEF
link_capabilites_register link_cap_reg;
link_cap_reg.set_register_value(read_register(0x1234'5678));

assert(link_cap_reg.get_max_link_speed() == 0x0000'000F);
```

## Writing Fields
Writing fields is done in much the same way as reading. It is done by calling the set method for that respective field. The method will be called in a similar naming convention: `set_ + {field name}`, for example `set_your_field_name()`.

```cpp
void write_register(uint32_t offset, uint32_t value) {
    /* --snip-- */
}

/* --snip-- */

link_capabilites_register link_cap_reg;
link_cap_reg.set_max_link_speed(0xF);
link_cap_reg.set_port_number(0xFF);

assert(link_cap_reg.get_register_value() == 0xFF00'000F);

// Possibly write our newly calculated register value
write_register(0x1234'5678, link_cap_reg.get_register_value());
```

This code shows how you can use these register objects to easily configure a register with certain bit field values in a memory safe and self documenting manner, and write it to memory with your usual 32 bit input write API.

## Permission Protected Registers
> [!WARNING]  
> This is still in progress. Weighing the cost benefit of changing the API to account for gets and sets failing based on permissions.

Let's imagine we have a mixed permission MMIO register. Let's call it the Port State register. It's register definition is as follows:

| Bit Location | Description | Permissions |
| --- | --- | --- |
| 0:3 | Link Speed | R/W |
| 4:7 | Link Width | R |
| 8:11 | Key | W |
| 12:15 | Tag | NULL |

The declarations for permission protected registers looks very similar, but a provided enum class member from `REGISTER_PERMS` should be appended to each field in the declaration like so:

```cpp
DECLARE_REGISTER_16_WITH_PERMS(
    port_state_register,
    link_speed, 0, 3, REGISTER_PERMS::READ_WRITE,
    link_width, 4, 7, REGISTER_PERMS::READ,
    key, 8, 11, REGISTER_PERMS::WRITE,
    tag, 12, 15, REGISTER_PERMS::NONE
)
```

I am not sure if there is really a point to allowing for `NONE` permissions as you could instead just omit that field entirely from the register declaration, but it is available if you want it.

With these permissions, the get methods will return failure if called on a field without read permissions and the set methods will return failure if called on a field without write permissions. These values can still be accessed through the register wide methods: `get_register_value()`, `set_register_value()`, `clear_register_value()`. These permissions are only present to help indicate when a read value is valid or when a write will not actually occur when it is done on the actual register.
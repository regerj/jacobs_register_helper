# Jacob's Register Helper

## Why?
As a systems software engineer, I am often working with registers. These registers might have different fields within them, often on a sub-byte width. This makes accessing different bits or lengths of bits somewhat obtuse I think. There are a couple of methods you could use to approach this issue. Let's talk about each potential method, and then we will go over this implementation and its advantages.

### Situation
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

Let's imagine that you already have some method called `read_register(uint32_t offset)` which reads a register in from DRAM or MMIO or something.

Let's also imagine that the link capabilites register is currently `0xdeadbeef`.

The bit representation of `0xdeadbeef` can be seen below. This shows that the bits 11:10 are `3` or `0b11`.
![image](https://github.com/regerj/jacobs_register_helper/assets/71850611/b11440f5-59f6-409e-8c05-6b65fc58620c)

### Method 1: Bit Shifting and Masking
Our first, and most often used method, is with bit shifting and masking to retrieve only a certain chunk of bits from within a register. Our C++ code for such a method might look something like this:

```cpp
// Read the register from memory
uint32_t link_capabilites_reg = read_register(0x0C);
uint32_t aspm_support = 0x0;

aspm_support = (link_capabilites_reg >> 10) | 0b11;
```

This bit shifts the register by 10 bits to the right, so that the tenth bit is now the zero'th bit. Next, it is bit masked with `0b11` to only read the two bits we are interested. This will result in `aspm_support == 0b11`.

This method works well from a functionality standpoint, and perhaps a comment above could indicate where these numbers are coming from, but it is not reproducable without going to the documentation. Checking certain fields of a register may be done repeatedly throughout the codebase, and each time you will have to reference the documentation to retrieve these magic numbers (the 10th bit start and the 2 bit width). This problem could possibly be mitigated by using some constant definitions, such as a `#define` for the field start and another for the field width. This would change our solution to be more like this:

```cpp
// Some constant definitions header
#define ASPM_SUPPORT_START 10
#define ASPM_SUPPORT_WIDTH 0b11

// main.cpp
// Read the register from memory
uint32_t link_capabilites_reg = read_register(0x0C);
uint32_t aspm_support = 0x0;

aspm_support = (link_capabilites_reg >> ASPM_SUPPORT_START) | ASPM_SUPPORT_WIDTH;
```

Adding this to the approach mitigates the need for looking up the magic numbers each time, but it introduces it's own set of problems.
1. Potential name conflicts. For example, some bit fields have extremely common names like `LINK_WIDTH` which would collide. You could mitigate this with more verbose names, but that can't be the best solution...
2. Misuse of these constants for different registers. The constant definition file which contains all of these magic numbers will likely contain the constants for a number of different registers, possibly dozens or hundreds. There is nothing preventing you from unintentionally using a constant meant for, say, the Device Status register on a Link Capabilites register, which could cause disasterous effects.
3. No code insights into the contents of the register. This approach still requires you to reference the documentation every time you go to access a member of a register, in order to know what fields are present, and infer from that the names of the constants you would need to use. This could potentially be mitigated by organizing your constant defines such that they are grouped by the register which owns them, which could be described in a comment above the constants. This would mean you no longer have to reference the documentation, but you would have to find the register in the constant header file. There has got to be a more robust solution...

You could provide MACROs to simplify the access to certain fields, perhaps it might look something like this:
```cpp
// Some constant definitions header
#define ASPM_SUPPORT_START 10
#define ASPM_SUPPORT_WIDTH 0b11

#define GET_ASPM_SUPPORT(link_capabilites_register) (link_capabilites_register >> ASPM_SUPPORT_START) | ASPM_SUPPORT_WIDTH

// main.cpp
// Read the register from memory
uint32_t link_capabilites_reg = read_register(0x0C);
uint32_t aspm_support = 0x0;

aspm_support = GET_ASPM_SUPPORT(link_capabilites_reg);
```

This makes the code look a bit cleaner, but it fails to mitigate the major issues discussed above, it simply moves them or changes in what way they come up.

### Bit Fields
Another possible method that fixes many of these problems is using bitfields within a struct. This allows you to assign names to bit length defined fields. Your registers could then be defined as follows:

```cpp
// Some register.h file

struct link_capabilities_register {
  uint32_t max_link_speed : 4;
  uint32_t max_link_width : 6;
  uint32_t aspm_support : 2;
  uint32_t l0s_exit_latency : 3;
  uint32_t l1_exit_latency : 3;
  uint32_t clock_power_management : 1;
  uint32_t surprise_down_error_reporting_capable : 1;
  uint32_t data_link_layer_link_active_reporting_capable : 1;
  uint32_t link_bandwidth_notification_capability : 1;
  uint32_t aspm_optional_compliance : 1;
  uint32_t reserved : 1;
  uint32_t port_number : 8;
};

// main.cpp
// Read the register from memory
link_capabilites_register link_capabilites_reg = read_register(0x0C);
uint32_t aspm_support = 0x0;

aspm_support = link_capabilites_reg.aspm_support;

```

This, at face value, seems to be a GREAT solution. We now have named bit fields within a struct, which allows us to instantiate an object of a static type `link_capabilites_reg` and we have insight into the fields of this register through LSP autocompletion. This solution comes with a different set of problems though:

1. Reserved bits must be declared in the bitfield, otherwise locations may become messed up. This means that it is possible for someone to write to those reserved bits unintentionally, without violating the access pattern.
2. There is no way to nicely retrieve the actual full register value again, which you would need for writing the register back. The method for doing so would probably look something like `void write_register(uint32_t offset, uint32_t value)`. You would need to turn your object back into a `uint32_t` for the write call. This is not possible with this method without some sketchy and probably very undefined pointer casting.
3. This approach is FULL of implementation defined behaviour. The C++ standard makes few gurantees about the functioning of bitfields within a struct, which leaves it up to the implementation. This creates unportable and fragile code. This method is advised against very strongly.

For the sake of completeness, we could alleviate problem 2 by wrapping this struct in a union. That implementation might look something like this:
```cpp
// Some register.h file
union link_capabilites_register {
  uint32_t raw;
  struct link_capabilities_register {
    uint32_t max_link_speed : 4;
    uint32_t max_link_width : 6;
    uint32_t aspm_support : 2;
    uint32_t l0s_exit_latency : 3;
    uint32_t l1_exit_latency : 3;
    uint32_t clock_power_management : 1;
    uint32_t surprise_down_error_reporting_capable : 1;
    uint32_t data_link_layer_link_active_reporting_capable : 1;
    uint32_t link_bandwidth_notification_capability : 1;
    uint32_t aspm_optional_compliance : 1;
    uint32_t reserved : 1;
    uint32_t port_number : 8;
  } data;
};

// main.cpp
// Read the register from memory
link_capabilites_register link_capabilites_reg = read_register(0x0C);
uint32_t aspm_support = 0x0;

aspm_support = link_capabilites_reg.data.aspm_support;

```

You would then be able to access the raw register value using `link_capabilites_reg.raw`, but again this leaves the much bigger issues remaining, so is still not advisable.

## My Solution
My solution involves some MACRO based metaprogramming. Essentially, it provides a set of MACROs for creating register definitions of varying widths. An implementation of the above problem using my header might look something like this:

```cpp
// Some register definition file (lets call it "pcie_registers.h"
#include "registers.h" // My implementation file (to be renamed)

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

// main.cpp
#include "pcie_registers.h"

int main() {
  link_capabilites_register reg;
  reg.set_regist

er_value(read_register(0x0C));
  uint32_t aspm_support = 0x0;
  aspm_support = reg.get_aspm_support();
}
```

The setup here is that you would have some header file containing repeated calls to my provided MACROs to define the structure of all the registers you might read/write in your code. The arg list looks like `(NAME, FIELD, START, END, FIELD, START, END, ...)`. You provide a name for your register type as the first argument, and you follow it by a series of `(FIELD, START, END)` where `FIELD` represents the name of the field, `START` represents the beginnig bit, and `END` represents the ending bit (inclusive). This allows for readable, easy to create register definitions that clearly state the start and end bits of the field for self documenting code.




dssjkkjkjdskfjlskdsfs


Memory safe, C++ standard compliant, register object creator with named access methods.

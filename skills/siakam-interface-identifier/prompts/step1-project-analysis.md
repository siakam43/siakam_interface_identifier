# Step 1: Project Architecture Analysis

Analyze a C project to produce architecture documentation for parallel subagents classifying candidate functions. Be thorough but concise — subagents must digest this quickly.

**CRITICAL**: All analysis MUST be done by LLM reasoning — reading source files and grep results. Do NOT write or execute any scripts, code, or programs.

## Required Sections

Produce `.siakam_out/SII/arch.md` with these 4 sections in order. Use clear markdown headings.

### 1. Module Composition and Boundaries

For each sub-directory representing a distinct sub-module:
- Path relative to `project_dir`
- Purpose (1-2 sentences from file naming, Makefile/Kbuild, header comments)
- Public headers it exposes (files under `include/` or headers referenced across module boundaries)

Focus on what an external developer needs to know to use this module. Ignore internal details.

### 2. External Framework / Subsystem Dependencies

Which kernel frameworks or external subsystems does the project depend on? For each:
- Framework name (e.g., Linux I2C, SPI, PCI, UEFI Boot Services, HarmonyOS HDF)
- How the project registers — grep for:

```bash
grep -rn "module_init\|module_exit" --include="*.c"
grep -rn "_driver_register\|_driver_unregister" --include="*.c"
grep -rn "class_create\|device_create\|sysfs_create" --include="*.c"
grep -rn "proc_create\|debugfs_create\|register_sysctl" --include="*.c"
grep -rn "register_.*_notifier" --include="*.c"
grep -rn "EFI_SERVICE_BINDING\|EFI_DRIVER_BINDING" --include="*.c" --include="*.h"
grep -rn "HdfDriverEntry\|HDF_DRIVER" --include="*.c" --include="*.h"
```

- What entities this project registers (e.g., `struct platform_driver`, `struct i2c_driver`)

### 3. Typical Interface Registration Patterns

Callback tables and registration structures used. For each pattern:
- The struct type (e.g., `struct file_operations`, `struct i2c_algorithm`)
- Where defined and registered (file path + approximate line)
- Which callback slots are assigned

Search with:

```bash
grep -rn "struct file_operations\|struct device_attribute\|struct attribute_group" --include="*.c"
grep -rn "struct i2c_driver\|struct spi_driver\|struct pci_driver\|struct platform_driver" --include="*.c"
grep -rn "struct usb_driver\|struct usb_class_driver" --include="*.c"
grep -rn "struct proto_ops\|struct netlink_kernel_cfg" --include="*.c"
grep -rn "static.*struct.*_ops\b" --include="*.c"
```

### 4. Module Communication Topology

Which sub-module talks to which external entity, through what mechanism, with what data flow direction.

Format as a bullet list or text diagram:

```
Module: drivers/foo/
  → Linux VFS (userspace) via file_operations (ioctl, read, write) [bidirectional]
  → I2C bus via i2c_driver.probe [inbound — hardware enumeration]

Module: drivers/bar/
  → Platform bus via platform_driver [inbound — device tree match]
  → Userspace via sysfs device_attribute [outbound]
```

## Constraints

1. Target 200-400 lines of markdown. Subagents have limited context.
2. Do NOT classify individual functions as interfaces or non-interfaces. That is Step 3's job. Your job is only to describe the architecture framework they operate in.
3. Use actual grep results — do not guess. Run the grep commands listed above.
4. Write the output to `.siakam_out/SII/arch.md`.

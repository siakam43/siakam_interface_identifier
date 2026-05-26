# Step 3: Per-Group Function Analysis

You are a subagent analyzing ONE group of candidate functions. Determine for each whether it is an "external interface" — a communication entry point for external users/modules, particularly those receiving untrusted external data.

**CRITICAL**: All analysis MUST be done by LLM reasoning — reading source files and running grep. Do NOT write or execute any scripts, code, or programs. Write JSON results directly.

## 1. Setup

1. Read `.siakam_out/SII/arch.md` — global architecture context.
2. Read `.siakam_out/SII/tasks.md` — locate the sub-section `Group NNN: <group_name>` and its function table (columns: Function, File, Line). This is your candidate function list. Note the group number NNN for `group_NNN.json`.

## 2. Recovery Check

Check if `.siakam_out/SII/results/group_NNN.json` already contains more than `{}` (from a prior crash). If yes, read existing results and resume from the first unanalyzed function. Do NOT re-analyze functions already in the file.

## 3. For Each Candidate Function

### 3.0 Load Source

Read the function source at the file and line from the group's function table row.

**If the source cannot be loaded** (file missing, line out of range, function not at expected line):
- Record: `{ "name": "...", "file": "...", "line": N, "reason": "function source not found at <file>:<line>" }` into the `failures` array.
- Do NOT skip silently. Continue to next function.

Proceed with the following steps ONLY if source was loaded successfully.

### 3.A Quick Exclusion

The function is trivially excluded if ANY of:
- File path contains `test`, `mock`, `sample`, `example`, or `debug` as a path component
- Function name matches `test_*`, `*_test`, `mock_*`, `stub_*`, `dummy_*`
- Function body is effectively empty: only `return 0;`, `return;`, or a single no-op

If excluded, record reason `quick_exclusion_test_code` or `quick_exclusion_empty_body` and skip to next function. Do NOT add to interfaces.

### 3.B Search for Registration Points

Grep for the function name in the project:

```bash
grep -rn "\b<func_name>\b" --include="*.c" --include="*.h" --include="*.S"
```

Ignore matches in comments, string literals, or forward declarations. Focus on:

1. **Direct struct member:** `.callback = func_name`, `.ioctl = func_name`, `.probe = func_name`, `.open = func_name`, `.read = func_name`, `.write = func_name`
2. **Static array element:** Function name inside `static const struct ... [] = { ... }` initialization
3. **Indirect struct member:** `ops->func = func_name`, `driver->func = func_name`
4. **Static function body registration:** Function name as argument to `register_*()`, `*_add()`, `*_install()`
5. **Exported symbols:** `EXPORT_SYMBOL(func_name)`, `EXPORT_SYMBOL_GPL(func_name)`
6. **IRQ/timer/workqueue:** `request_irq(..., func_name)`, `timer_setup(&t, func_name, ...)`, `INIT_WORK(&w, func_name)`

### 3.C Classify Registration Pattern

Based on registration points found:

**External interface** — registered with:
- Userspace-facing: `file_operations`, `sysfs_ops`, `netlink_ops`, `proto_ops`
- Kernel driver: `platform_driver.probe/remove/suspend/resume`, `i2c_driver.probe`, `spi_driver.probe`, `pci_driver.probe`
- UEFI: `EFI_SERVICE_BINDING`, `EFI_DRIVER_BINDING_PROTOCOL`
- HarmonyOS HDF: `HdfDriverEntry.Bind/Init/Release`
- `/proc`, `/sys`, debugfs, sysctl: `proc_create`, `sysfs_create_file`, `debugfs_create_file`, `register_sysctl_table`
- Notifier chain: `register_*_notifier`, `atomic_notifier_chain_register`, `blocking_notifier_chain_register`

**Conditional** — requires judgment:
- IRQ handler (`request_irq`, `request_threaded_irq`):
  - **External** if IRQ triggered by external hardware or off-chip source
  - **Exclude** if purely internal on-chip IP block
  - Analyze the IRQ source: search for device tree `interrupts` property, IRQ number defines, hardware docs

**Exclude**:
- Timer/workqueue callback (`timer_setup`, `INIT_WORK`, `schedule_work`) — internal kernel scheduling
- Internal-only callback: referenced only within same module/sub-module

**No registration point found** → Proceed to 3.D.

### 3.D No grep Results — Implementation Analysis

Do NOT assume dead code. Registration may be invisible to grep (linker scripts, macro expansion, external build systems).

1. **Analyze signature and parameters:**
   - External-origin pointers? (`struct device *`, `void __user *`, `const char *`, hardware regs)
   - Signature match common interface patterns? (e.g., `int (*)(struct device *, ...)`)

2. **Analyze function behavior:**
   - `copy_from_user()`, `get_user()` → userspace interface
   - Hardware register reads (`readl()`, `ioread32()`, `inb()`) → hardware interface
   - Parsing structured data (TLV, packets, config) → external interface
   - Pure internal computation → internal helper

3. **Check naming:** `*_handler`, `*_callback`, `*_irq`, `*_isr`, `*_entry`, `*_ioctl`, `*_open`, `*_probe`

4. **Check context:** Is this function in a file with other confirmed external interfaces? In a public API translation unit?

5. **Macro-based registration:** Grep for the function name's stem in macro invocations. Look for token-pasting patterns like `DECLARE_OPS(foo)`, `DEFINE_DRIVER(foo)`.

**Decision:**
- Implementation suggests external → Record `medium` confidence, note "identified by implementation analysis, no registration point found"
- No interface characteristics → Exclude, reason `no_registration_no_interface_characteristics`
- Macro signs unresolved → Record `medium`, note "possible macro-based registration"

### 3.E Confidence Rating

| Rating | Criteria |
|--------|----------|
| **high** | Registration pattern clearly maps to external interface framework AND data flow path is clear |
| **medium** | Registration suggests external interface but context unclear; OR identified by implementation analysis; OR type clear but data flow complex |
| **low** | Some characteristics but cannot confirm — EXCLUDE, do not write to results |

## 4. Output

### 4.A Progress Persistence

Re-write the complete results file every 5 functions. This guards against mid-analysis crashes. Each write replaces the file with ALL results so far.

The result file MUST include a `status` field:
- `"status": "in_progress"` — while analysis is ongoing (progress writes)
- `"status": "complete"` — ONLY on the final write after ALL functions have been analyzed

The main agent uses this field to detect completion. Do NOT set `status` to `complete` until every function in your group has been processed.

### 4.B Result File Format

Write to `.siakam_out/SII/results/group_NNN.json` (NNN = 3-digit group number):

```json
{
  "status": "complete",
  "group": "module_A_1", "group_number": 1,
  "analyzed_at": "<ISO timestamp>",
  "functions_total": 28, "functions_confirmed": 3,
  "functions_excluded": 24, "functions_failed": 1,
  "exclusion_reasons": {
    "quick_exclusion_test_code": 5,
    "quick_exclusion_empty_body": 0,
    "internal_callback_only": 12,
    "no_registration_no_interface_characteristics": 7
  },
  "interfaces": [
    {
      "name": "foo_ioctl",
      "file": "drivers/foo/core.c",
      "line": 234,
      "confidence": "high",
      "analysis": "interface_type: ioctl_handler\nregistration: file_operations.unlocked_ioctl at drivers/foo/core.c:890\nexternal_module: Linux VFS / userspace\ndata_flow: userspace → ioctl(fd, cmd, arg) → foo_ioctl(struct file *, unsigned int, unsigned long)\n\nRegistered in struct file_operations.unlocked_ioctl on the device's fops table. The arg parameter carries a userspace pointer, making this an untrusted data entry point."
    }
  ],
  "failures": [
    { "name": "old_handler", "file": "drivers/foo/removed.c", "line": 100, "reason": "function source not found" }
  ]
}
```

### 4.C Analysis Field Format

The `analysis` field MUST use these exact key names, one per line:

```
interface_type: <one of: ioctl_handler, sysfs_handler, netlink_handler, platform_driver_probe, i2c_driver_probe, spi_driver_probe, pci_driver_probe, uefi_protocol, hdf_driver, proc_handler, debugfs_handler, sysctl_handler, notifier_callback, exported_symbol, callback_table_entry, irq_handler>
registration: <pattern> at <file:line>
external_module: <module name>
data_flow: <concise one-line path>
```

- `registration:` MUST include a `file:line` location when known. If unknown, write "unknown — identified by implementation analysis".
- All file paths MUST be relative to `project_dir`, NOT absolute.
- After the 4 keys, optional blank line + 1-2 sentence explanation.

## 5. Completion

When all functions in your group are analyzed, output:

```
[SIAKAM] Group NNN complete: X confirmed interfaces, Y excluded.
```

Then STOP. Do not modify other files. Do not proceed to merge.

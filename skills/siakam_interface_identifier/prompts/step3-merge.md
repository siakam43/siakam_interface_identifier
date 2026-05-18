# Step 4: Result Merge

Merge all group result files into the final `apis.json`.

**CRITICAL**: You MUST perform the merge using LLM reasoning directly. Do NOT write any scripts, code, or programs. Read the files, apply the rules below with your own reasoning, and write the output directly.

## 1. Load All Results

Read every `.siakam_out/SII/results/group_*.json` file. Sort by group number. Note incomplete files (placeholder `{}`) as failed groups.

## 2. Merge Interfaces

Collect all `interfaces` arrays into one list.

**Deduplicate** by composite key `file:name`. If same `file:name` appears in multiple groups, keep the entry with higher confidence. If equal, keep the later (higher group number).

**Filter**: EXCLUDE all `confidence: "low"` entries. KEEP `high` and `medium`. PLACE `confidence: "error"` entries into the `failures` array.

**Sort** by: top-level directory of `file`, then by file path, then by line number.

## 3. Aggregate Statistics

**Exclusion reasons:** Sum `exclusion_reasons` counts across all group files into `summary.exclusion_reasons`. Sum counts for identical reason keys.

**Interface counts:** Count `high` and `medium` entries for `summary.high_confidence` and `summary.medium_confidence`.

**Total candidates:** Sum all `functions_total` values from all groups.

**Confirmed interfaces:** Count of `interfaces` array after filtering (high + medium only).

## 4. Partition Errors and Warnings

**`errors`** — Runtime issues affecting result completeness. Array of strings:
- `"group_NNN failed after 2 retries: <reason>"`
- `"group_NNN: partial results — only X/Y functions analyzed"`

**`warnings`** — Non-blocking data quality issues. Array of strings:
- `"entry.json contained 0 candidates"`
- `"N functions could not be located — see failures array"`

Use empty arrays `[]` if none.

## 5. Write Final Output

Write `.siakam_out/SII/apis.json`:

```json
{
  "project": "<project_dir basename>",
  "analysis_date": "<YYYY-MM-DD>",
  "total_candidates": <N>,
  "confirmed_interfaces": <X>,
  "summary": {
    "high_confidence": <H>,
    "medium_confidence": <M>,
    "exclusion_reasons": {
      "<reason_key>": <count>,
      "...": 0
    }
  },
  "interfaces": [
    {
      "name": "<function name>",
      "file": "<path relative to project_dir>",
      "line": <line>,
      "confidence": "<high or medium>",
      "analysis": "<multi-line structured analysis>"
    }
  ],
  "failures": [
    {
      "name": "<function name>",
      "file": "<path>",
      "line": <line>,
      "reason": "<failure reason>"
    }
  ],
  "errors": ["<error string>"],
  "warnings": ["<warning string>"]
}
```

## 6. Verify

Before finishing, confirm:
- Every `interfaces` entry has all 4 required fields: `name`, `file`, `line`, `confidence`
- `confirmed_interfaces` = length of `interfaces` array = `high_confidence + medium_confidence`
- All `file` paths are relative to `project_dir` (not absolute)
- `errors` and `warnings` correctly partitioned (runtime vs data-quality)
- No `low` confidence entries in `interfaces`
- No duplicate `file:name` entries

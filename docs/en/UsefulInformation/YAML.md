# YAML

YAML (YAML Ain't Markup Language) is a human-readable data serialization language. In Clover2, it is used to describe ArUco marker maps, camera configs, and parameters.

## Basic Syntax Rules

- **Key-value pairs:** `key: value` (the space after the colon is required)
- **Comments:** `#` — everything up to the end of the line is ignored

  ```yaml
    name: test          # this is a comment
    # and this entire line is a comment

  ```

  Comments can only be placed at the end of a line or on a standalone line. YAML does not support block comments (/* */).

* **Indentation:** Spaces only (2 or 4 spaces); tab characters are strictly prohibited.
* **Strings:** Quotes are optional unless special characters are present. Otherwise, use double `" "` or single `' '` quotes.
* **Numbers:** `42`, `3.14`, `-1.5`, `1e-3`
* **Booleans:** `true` / `false`, `yes` / `no`, `on` / `off`
* **Null values:** `~`, `null`, or an empty field
* **Lists (block style):** Lines starting with `- ` (dash + space):

  ```yaml
  markers:
    - id: 0
    - id: 1

  ```

* **Lists (inline style):** Square brackets separated by commas:

  ```yaml
  markers: [{id: 0, size: 0.3}, {id: 1, size: 0.3}]
  # or
  markers: [0, 1, 2, 3]
  ```


* **List of dictionaries (explicit block style):**

  ```yaml
  markers:
    - {id: 0, size: 0.3, x: 1.0, y: 2.0}
    - {id: 1, size: 0.3, x: 3.0, y: 4.0}

  ```

* **Multiline keyless arrays:**

  ```yaml
  ids:
    - 10
    - 20
    - 30
  ```

* **Dictionaries (nested):** Indentation defines nesting levels:

  ```yaml
  pose:
    x: 1.0
    y: 2.0

  ```

* **Dictionaries (inline style):** Curly braces separated by commas:

  ```yaml
  pose: {x: 1.0, y: 2.0, z: 0.5}

  ```

:::{note}
Indentation must be consistent. If the first level uses 2 spaces, all subsequent levels must use 2 spaces. Indentation errors will prevent programs from parsing the file.
:::

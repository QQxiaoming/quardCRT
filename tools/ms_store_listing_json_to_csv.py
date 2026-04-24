#!/usr/bin/env python3

import argparse
import csv
import json
import io
from pathlib import Path


EXPECTED_FORMAT = "microsoft_store_listing"


def parse_args():
    parser = argparse.ArgumentParser(
        description="Convert a structured Microsoft Store listing JSON file back into CSV."
    )
    parser.add_argument("input_json", help="Path to the Microsoft Store listing JSON file.")
    parser.add_argument(
        "output_csv",
        nargs="?",
        help="Optional output CSV path. Defaults to the JSON path with a .csv suffix.",
    )
    return parser.parse_args()


def load_json(json_path):
    with json_path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def validate_columns(columns):
    if not isinstance(columns, dict):
        raise ValueError("JSON field 'columns' must be an object.")

    base_columns = columns.get("base")
    value_columns = columns.get("values")
    if not isinstance(base_columns, list) or not all(isinstance(item, str) for item in base_columns):
        raise ValueError("JSON field 'columns.base' must be a string array.")
    if not isinstance(value_columns, list) or not all(isinstance(item, str) for item in value_columns):
        raise ValueError("JSON field 'columns.values' must be a string array.")
    return base_columns, value_columns


def normalize_scalar(value):
    if value is None:
        return ""
    return str(value)


def build_rows(data):
    if data.get("format") != EXPECTED_FORMAT:
        raise ValueError(
            f"Unsupported JSON format '{data.get('format')}'. Expected '{EXPECTED_FORMAT}'."
        )

    base_columns, value_columns = validate_columns(data.get("columns"))
    entries = data.get("entries")
    if not isinstance(entries, dict):
        raise ValueError("JSON field 'entries' must be an object.")

    rows = []
    for field_name, entry in entries.items():
        if not isinstance(entry, dict):
            raise ValueError(f"Entry '{field_name}' must be an object.")
        values = entry.get("values")
        if not isinstance(values, dict):
            raise ValueError(f"Entry '{field_name}' is missing a valid 'values' object.")

        row = {
            base_columns[0]: field_name,
            base_columns[1]: normalize_scalar(entry.get("id", "")),
            base_columns[2]: normalize_scalar(entry.get("type", "")),
        }
        for column in value_columns:
            row[column] = normalize_scalar(values.get(column, ""))
        rows.append(row)

    skipped_empty_rows = data.get("skipped_empty_rows", 0)
    if not isinstance(skipped_empty_rows, int) or skipped_empty_rows < 0:
        raise ValueError("JSON field 'skipped_empty_rows' must be a non-negative integer.")

    empty_row_template = {column: "" for column in [*base_columns, *value_columns]}
    for _ in range(skipped_empty_rows):
        rows.append(empty_row_template.copy())

    return [*base_columns, *value_columns], rows


def write_csv(csv_path, header, rows):
    buffer = io.StringIO(newline="")
    writer = csv.DictWriter(buffer, fieldnames=header, lineterminator="\r\n")
    writer.writeheader()
    writer.writerows(rows)

    content = buffer.getvalue().removesuffix("\r\n")
    csv_path.write_text(content, encoding="utf-8-sig", newline="")


def main():
    args = parse_args()
    input_path = Path(args.input_json).resolve()
    output_path = Path(args.output_csv).resolve() if args.output_csv else input_path.with_suffix(".csv")

    data = load_json(input_path)
    header, rows = build_rows(data)
    write_csv(output_path, header, rows)
    print(f"Wrote {output_path}")


if __name__ == "__main__":
    main()
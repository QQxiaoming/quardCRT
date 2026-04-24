#!/usr/bin/env python3

import argparse
import csv
import json
from pathlib import Path


BASE_COLUMNS = ["Field", "ID", "Type (类型)"]


def parse_args():
    parser = argparse.ArgumentParser(
        description="Convert a Microsoft Store listing CSV file into a structured JSON file."
    )
    parser.add_argument("input_csv", help="Path to the Microsoft Store listing CSV file.")
    parser.add_argument(
        "output_json",
        nargs="?",
        help="Optional output JSON path. Defaults to the CSV path with a .json suffix.",
    )
    parser.add_argument(
        "--indent",
        type=int,
        default=2,
        help="JSON indentation width. Default: 2",
    )
    return parser.parse_args()


def load_rows(csv_path):
    with csv_path.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        header = reader.fieldnames or []
        rows = list(reader)
    return header, rows


def split_columns(header):
    if len(header) < len(BASE_COLUMNS):
        raise ValueError("CSV header is missing required base columns.")
    if header[: len(BASE_COLUMNS)] != BASE_COLUMNS:
        raise ValueError(
            "CSV header does not match expected base columns: "
            + ", ".join(BASE_COLUMNS)
        )
    extra_columns = header[len(BASE_COLUMNS) :]
    return BASE_COLUMNS, extra_columns


def is_empty_row(row):
    return not any((value or "").strip() for value in row.values())


def normalize_row(row, value_columns):
    field_name = (row.get("Field") or "").strip()
    if not field_name:
        raise ValueError("Encountered a non-empty row without a Field value.")

    row_id = (row.get("ID") or "").strip()
    item_type = (row.get("Type (类型)") or "").strip()
    values = {column: row.get(column, "") for column in value_columns}

    return {
        "id": int(row_id) if row_id.isdigit() else row_id,
        "type": item_type,
        "values": values,
    }, field_name


def convert_csv_to_json(csv_path):
    header, rows = load_rows(csv_path)
    _, value_columns = split_columns(header)

    entries = {}
    skipped_empty_rows = 0

    for index, row in enumerate(rows, start=2):
        if is_empty_row(row):
            skipped_empty_rows += 1
            continue

        entry, field_name = normalize_row(row, value_columns)
        if field_name in entries:
            raise ValueError(f"Duplicate Field value '{field_name}' found on CSV line {index}.")
        entries[field_name] = entry

    return {
        "format": "microsoft_store_listing",
        "source_csv": csv_path.name,
        "columns": {
            "base": BASE_COLUMNS,
            "values": value_columns,
        },
        "skipped_empty_rows": skipped_empty_rows,
        "entries": entries,
    }


def main():
    args = parse_args()
    input_path = Path(args.input_csv).resolve()
    output_path = Path(args.output_json).resolve() if args.output_json else input_path.with_suffix(".json")

    data = convert_csv_to_json(input_path)

    output_path.write_text(
        json.dumps(data, ensure_ascii=False, indent=args.indent) + "\n",
        encoding="utf-8",
    )
    print(f"Wrote {output_path}")


if __name__ == "__main__":
    main()
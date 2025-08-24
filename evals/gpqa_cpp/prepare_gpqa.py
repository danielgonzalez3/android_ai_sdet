#!/usr/bin/env python3
import argparse
import csv
from datasets import load_dataset


def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("--out", default="evals/gpqa_cpp/gpqa_test.csv")
	parser.add_argument("--dataset", default="casimiir/gpqa")
	args = parser.parse_args()

	ds = load_dataset(args.dataset, split="test")
	# Ensure required columns
	required = {"id", "question", "choices", "answer"}
	missing = required - set(ds.column_names)
	if missing:
		raise RuntimeError(f"Dataset missing columns: {missing}")

	rows = []
	for ex in ds:
		rows.append({
			"id": ex["id"],
			"subdomain": ex.get("subdomain", ""),
			"question": ex["question"],
			"choices": str(list(ex["choices"])),
			"answer": ex["answer"],
		})

	with open(args.out, "w", newline="") as f:
		writer = csv.DictWriter(f, fieldnames=["id", "subdomain", "question", "choices", "answer"])
		writer.writeheader()
		writer.writerows(rows)
	print(f"Wrote {len(rows)} rows to {args.out}")


if __name__ == "__main__":
	main() 
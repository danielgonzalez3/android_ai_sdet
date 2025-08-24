#!/usr/bin/env python3
import argparse
import json
import os
from typing import List, Dict, Any

from instruction_following_eval import get_examples, evaluate_instruction_following
from transformers import pipeline
from tqdm import tqdm


def run_ifeval(model_name: str, max_examples: int, output_path: str, batch_size: int = 1, max_new_tokens: int = 256) -> Dict[str, Any]:
	os.makedirs(os.path.dirname(output_path), exist_ok=True)

	print(f"Loading model pipeline: {model_name}")
	# Use text2text for instruction-tuned models like flan-t5; fallback to text-generation
	try:
		generator = pipeline("text2text-generation", model=model_name, device=-1)
	except Exception:
		generator = pipeline("text-generation", model=model_name, device=-1)

	examples: List[Dict[str, Any]] = get_examples()
	if max_examples and max_examples > 0:
		examples = examples[:max_examples]

	prompts = [ex["prompt"] for ex in examples]
	responses: List[str] = []

	print(f"Generating responses for {len(prompts)} prompts (batch_size={batch_size})...")
	for i in tqdm(range(0, len(prompts), batch_size)):
		batch = prompts[i:i+batch_size]
		outs = generator(batch, max_new_tokens=max_new_tokens, do_sample=False)
		if outs and isinstance(outs[0], list):
			for j in range(len(batch)):
				responses.append(outs[j][0]["generated_text"])  # type: ignore
		else:
			for o in outs:
				responses.append(o.get("generated_text", o.get("summary_text", "")))

	metrics = evaluate_instruction_following(examples, responses)
	with open(output_path, "w") as f:
		json.dump({"model": model_name, "num_examples": len(examples), "metrics": metrics}, f, indent=2)

	print("Metrics:")
	print(json.dumps(metrics, indent=2))
	return metrics


def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("--model", default="google/flan-t5-large")
	parser.add_argument("--max_examples", type=int, default=50)
	parser.add_argument("--output", default="evals/ifeval/results/ifeval_metrics.json")
	parser.add_argument("--batch_size", type=int, default=4)
	parser.add_argument("--max_new_tokens", type=int, default=192)
	args = parser.parse_args()

	run_ifeval(
		model_name=args.model,
		max_examples=args.max_examples,
		output_path=args.output,
		batch_size=args.batch_size,
		max_new_tokens=args.max_new_tokens,
	)


if __name__ == "__main__":
	main() 
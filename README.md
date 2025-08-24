## Evaluations: IFEval and GPQA (C++)

### Quick start

1. Setup

```bash
make setup
```

2. Run IFEval (subset with a small open model)

```bash
make ifeval
```

- Results saved to `evals/ifeval/results/ifeval_metrics.json`.
- Change model and size via `evals/ifeval/run_ifeval.py` flags.

3. GPQA data + C++ evaluator

```bash
make gpqa_data
make gpqa_build
make gpqa_random
```

- Dataset CSV: `evals/gpqa_cpp/gpqa_test.csv` (from `casimiir/gpqa` test split)
- Random baseline predictions: `evals/gpqa_cpp/preds_random.csv`
- Evaluator binary: `evals/gpqa_cpp/build/gpqa_eval`

### One command

```bash
make all
```

### Requirements
- Linux, Python 3.10+, CMake 3.16+, a C++17 compiler
- A CPU-only Python env is bootstrapped automatically in `.venv_eval`

### Custom GPQA predictions
Provide a CSV `id,choice` with choices in `A|B|C|D`, then run:

```bash
./evals/gpqa_cpp/build/gpqa_eval \
  --data ./evals/gpqa_cpp/gpqa_test.csv \
  --pred /absolute/path/to/your_preds.csv
``` 
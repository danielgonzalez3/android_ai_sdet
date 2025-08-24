# Makefile for evaluations

REPO_ROOT := $(CURDIR)
VENV := $(REPO_ROOT)/.venv_eval
PY := $(VENV)/bin/python

.PHONY: help setup ifeval gpqa_data gpqa_build gpqa_random all clean

help:
	@echo "Targets:"
	@echo "  setup       - create venv and install deps"
	@echo "  ifeval      - run IFEval on a small model"
	@echo "  gpqa_data   - download/export GPQA CSV"
	@echo "  gpqa_build  - build C++ GPQA evaluator"
	@echo "  gpqa_random - run GPQA with random baseline"
	@echo "  all         - setup, ifeval, gpqa_data, build, random"

setup:
	bash $(REPO_ROOT)/scripts/bootstrap_eval.sh

ifeval:
	$(PY) $(REPO_ROOT)/evals/ifeval/run_ifeval.py \
		--model microsoft/Orca-2-7b  \
		--max_examples 50 \
		--output $(REPO_ROOT)/evals/ifeval/results/ifeval_metrics.json \
		--batch_size 4 \
		--max_new_tokens 128

gpqa_data:
	$(PY) $(REPO_ROOT)/evals/gpqa_cpp/prepare_gpqa.py \
		--out $(REPO_ROOT)/evals/gpqa_cpp/gpqa_test.csv

gpqa_build:
	cmake -S $(REPO_ROOT)/evals/gpqa_cpp -B $(REPO_ROOT)/evals/gpqa_cpp/build
	cmake --build $(REPO_ROOT)/evals/gpqa_cpp/build -j 4

gpqa_random: gpqa_build gpqa_data
	$(REPO_ROOT)/evals/gpqa_cpp/build/gpqa_eval \
		--data $(REPO_ROOT)/evals/gpqa_cpp/gpqa_test.csv \
		--random --seed 123 \
		--out $(REPO_ROOT)/evals/gpqa_cpp/preds_random.csv

all: setup ifeval gpqa_data gpqa_build gpqa_random

clean:
	rm -rf $(REPO_ROOT)/evals/gpqa_cpp/build
	rm -f $(REPO_ROOT)/evals/gpqa_cpp/gpqa_test.csv $(REPO_ROOT)/evals/gpqa_cpp/preds_random.csv
	rm -f $(REPO_ROOT)/evals/ifeval/results/ifeval_metrics.json 
#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VENV_DIR="$REPO_ROOT/.venv_eval"
REQ_FILE="$REPO_ROOT/evals/requirements.txt"

python3 -m venv "$VENV_DIR"
source "$VENV_DIR/bin/activate"
python -V
pip install --upgrade pip
# CPU-only torch from official index with extra index URL
pip install --extra-index-url https://download.pytorch.org/whl/cpu torch==2.3.1+cpu torchvision==0.18.1+cpu torchaudio==2.3.1+cpu
pip install -r "$REQ_FILE"

echo "Environment ready at $VENV_DIR" 
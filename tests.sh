#!/bin/bash
# -e exit on any command non-zero exit status
# -u unset variables are errors
# -o pipeline failure on any command failure
set -euo pipefail

# If not $1, fallback to bin/rpn
binary_path="${1:-bin/rpn}"

# Exit if file not found or not executable
if [ ! -x "$binary_path" ]; then
  echo "Binary not found or not executable: $binary_path"
  exit 1
fi

# Format: "expr;precision;expected_output;expected_exit_code"
tests=(
  "1+2+3;3;6.000;0"
  "2^3^2;3;512.000;0"
  "(2+3)*4;3;20.000;0"
  "50%;3;0.500;0"
  "200+10%;3;220.000;0"
  "200-10%;3;180.000;0"
  "200+(10%);3;200.100;0"
  "(200+10%);3;220.000;0"
  "50%*200;3;100.000;0"
  "200/10%;3;2000.000;0"
  "5*5%;3;0.250;0"
  "5+5%;3;5.250;0"
  "1+cos(1)%*5+5%;3;1.078;0"
  "sqrt(9);3;3.000;0"
  "sin(0);3;0.000;0"
  "cos(0);3;1.000;0"
  "tan(0);3;0.000;0"
  "exp(0);3;1.000;0"
  "exp(1);3;2.718;0"
  "ln(e);3;1.000;0"
  "min(2,3);3;2.000;0"
  "max(2,3);3;3.000;0"
  "log(2,8);3;3.000;0"
  "sqrt(min(log(8,64),64));3;1.414;0"
  "3!;3;6.000;0"
  "-5+2;3;-3.000;0"
  "(-5)+2;3;-3.000;0"
  "-(2+3);3;-5.000;0"
  "p;3;3.142;0"
  "e;3;2.718;0"
  "p+e;3;5.860;0"
  "p*e;3;8.540;0"
  "p*2;3;6.283;0"
  "p/2;3;1.571;0"
  "e^1;3;2.718;0"
  "sin(p);3;0.000;0"
  "cos(p);3;-1.000;0"
  "sin(p/2);3;1.000;0"
  "1+2+3;1;6.0;0"
  "1+2+3;5;6.00000;0"
  "1+5%+5%+5%+5%*5%+5*5%+5+5%;6;6.731045;0"
)

# Failure flag
fail=0

run_case() {
  local expr="$1"
  local precision="$2"
  local expected="$3"
  local expected_code="$4"

  # temporary disable to not to fail on binary launch failure
  # execute and get output
  set +e
  output="$("$binary_path" -b -p "$precision" -- "$expr")"
  status=$?
  set -e

  if [ "$status" -ne "$expected_code" ]; then
    echo "FAIL: $expr (p=$precision) => exit $status (expected $expected_code)"
    fail=1
    return
  fi

  if [ "$expected_code" -eq 0 ]; then
    ok=$(awk -v a="$output" -v b="$expected" -v p="$precision" '
      BEGIN {
        ra = sprintf("%.*f", p, a);
        rb = sprintf("%.*f", p, b);
        # ra & rb compares to regex ^-0. ; if match - remove minus
        if (ra ~ /^-0\./) ra = substr(ra, 2);
        if (rb ~ /^-0\./) rb = substr(rb, 2);
        if (ra == rb) print "1"; else print "0";
      }'
    )
    if [ "$ok" != "1" ]; then
      echo "FAIL: $expr (p=$precision) => $output (expected $expected, rounded-compare)"
      fail=1
    else
      echo "OK: $expr (p=$precision) => $output (exit $status)"
    fi
  else
    echo "OK: $expr (p=$precision) => $output (exit $status)"
  fi
}

for t in "${tests[@]}"; do
  IFS=";" read -r expr precision expected expected_code <<< "$t"
  run_case "$expr" "$precision" "$expected" "$expected_code"
done

exit $fail

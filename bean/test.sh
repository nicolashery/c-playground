#!/bin/bash

set -e

BEAN="./build/bean/bean"
TEST_DIR="bean/test"
PASSED=0
FAILED=0

run_test() {
    local test_name="$1"
    local command="$2"
    local expected_file="$3"
    local capture_stderr="${4:-false}"

    local actual_file="${TEST_DIR}/actual_output.txt"

    if [ "$capture_stderr" = "true" ]; then
        eval "$command" > "$actual_file" 2>&1 || true
    else
        eval "$command" > "$actual_file" || true
    fi

    if diff -q "$expected_file" "$actual_file" > /dev/null 2>&1; then
        echo "[ok] $test_name"
        PASSED=$((PASSED + 1))
    else
        echo "[FAIL] $test_name"
        FAILED=$((FAILED + 1))
    fi

    rm -f "$actual_file"
}

echo "Running tests..."
echo ""

# Component tests
run_test "bean test ledger" \
    "$BEAN test ledger" \
    "$TEST_DIR/test-ledger.expected.txt"

run_test "bean test scanner bean/test/valid.bean" \
    "$BEAN test scanner $TEST_DIR/valid.bean" \
    "$TEST_DIR/test-scanner-valid.expected.txt"

run_test "bean test parser bean/test/valid.bean" \
    "$BEAN test parser $TEST_DIR/valid.bean" \
    "$TEST_DIR/test-parser-valid.expected.txt"

# End-to-end tests (happy path)
run_test "bean check bean/test/simple.bean" \
    "$BEAN check $TEST_DIR/simple.bean" \
    "$TEST_DIR/check-simple.expected.txt"

run_test "bean balance bean/test/balance.bean" \
    "$BEAN balance $TEST_DIR/balance.bean" \
    "$TEST_DIR/balance-balance.expected.txt"

# End-to-end tests (error cases)
run_test "bean check bean/test/scanner-error.bean" \
    "$BEAN check $TEST_DIR/scanner-error.bean" \
    "$TEST_DIR/check-scanner-error.expected.txt" \
    "true"

run_test "bean check bean/test/parse-error.bean" \
    "$BEAN check $TEST_DIR/parse-error.bean" \
    "$TEST_DIR/check-parse-error.expected.txt" \
    "true"

run_test "bean check bean/test/validation-error.bean" \
    "$BEAN check $TEST_DIR/validation-error.bean" \
    "$TEST_DIR/check-validation-error.expected.txt" \
    "true"

echo ""
echo "=========================================="
echo "Test Results: $PASSED passed, $FAILED failed"
echo "=========================================="

if [ $FAILED -gt 0 ]; then
    exit 1
fi

exit 0

#!/bin/bash
DIR="$1"
TEST_CASE="$2"
REPORT_FILE="$3"
. run-xvfb.sh
cd "${DIR}"
gtester --verbose -k -o "${REPORT_FILE}" "${TEST_CASE}"
RESULT=$?
gtester2xunit "${REPORT_FILE}"
exit $RESULT

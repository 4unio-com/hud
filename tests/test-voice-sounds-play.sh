#!/bin/bash
sleep 1
DIR=$(dirname "$0")
cp "${DIR}/test-voice-sounds/${1}.raw" "${2}"
cp "${DIR}/test-voice-sounds/silence.raw" "${2}"

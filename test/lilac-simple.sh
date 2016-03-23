#!/bin/bash

source test-compilation.sh

cat << EOF | test_compilation "42"
20 + 22
EOF

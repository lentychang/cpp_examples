#!/bin/bash
find . -type f ! -name "*.cpp" ! -name "*.h" ! -name "*.hpp" ! -name "*.sh" ! -name "*.json" -exec rm {} \;
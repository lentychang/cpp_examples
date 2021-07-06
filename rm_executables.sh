#!/bin/bash
find . -type f ! -name "*.cpp" ! -name "*.h" ! -name "*.hpp" ! -name "*.sh" ! -name "*.json" ! -name CMakeLists.txt ! -name .gitignore -exec rm {} \;

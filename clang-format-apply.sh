# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Runs clang-format and apply changes.

alias clang-format=clang-format-9

clang-format --version

find . -name '*.h' -print0 -o -name '*.cc' -print0 -o -path './third_party' \
  -prune -false | xargs -0 -n 1 clang-format --verbose -i

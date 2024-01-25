#!/bin/sh
target_file="$1"
function_name="$2"
result_file="$3"

gcov -b -c -i -f "$target_file" | awk -v fn="$function_name" '$0 ~ "^Function \047"fn"\047", /^Calls executed/' > "$result_file"

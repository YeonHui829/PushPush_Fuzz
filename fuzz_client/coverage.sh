#!/bin/sh
function_name="$1"
result_file="$2"
gcov -b -c -i -f test_client.gcda | awk -v fn="$function_name" '$0 ~ "^Function \047"fn"\047", /^Calls executed/' > "$result_file"

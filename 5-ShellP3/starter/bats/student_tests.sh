#!/usr/bin/env bats

# File: student_tests.sh
# Unit tests for the custom shell (dsh)

@test "Verify 'ls' command executes successfully" {
    run ./dsh <<EOF
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Verify 'pwd' displays the correct working directory" {
    run "./dsh" <<EOF
pwd
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="$(pwd)dsh3>dsh3>cmdloopreturned0"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_output}"

    [ "$clean_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Change directory to /tmp and verify" {
    run "./dsh" <<EOF
cd /tmp
pwd
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmpdsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_output}"

    [ "$clean_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Attempt to change to a non-existent directory" {
    run "./dsh" <<EOF
cd /nonexistentpath
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>error:commandfaileddsh3>cmdloopreturned0"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_output}"

    [ "$clean_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Test the 'exit' command" {
    run "./dsh" <<EOF
exit
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>exiting...cmdloopreturned0"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_output}"

    [ "$clean_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Echo 'Hello, World' with extra spaces" {
    run "./dsh" <<EOF
echo "Hello,     World"
EOF

    clean_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    expected_output="Hello,     Worlddsh3> dsh3> cmd loop returned 0"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_output}"

    [ "$clean_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Echo with mixed quoted and unquoted arguments" {
    run ./dsh <<EOF
echo Test "Test Multi   word unquoted"   unquoted
EOF

    clean_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    expected_output="Test Test Multi   word unquoted unquoteddsh3> dsh3> cmd loop returned 0"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_output}"

    [ "$clean_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Pipeline with an invalid command in the middle" {
    run "./dsh" <<EOF
ls | invalidcmd | wc -l
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_regex="^execvp:Nosuchfileordirectory[0-9]+dsh3>dsh3>cmdloopreturned0$"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_regex}"

    [[ "$clean_output" =~ $expected_regex ]]
    [ "$status" -eq 0 ]
}

@test "Pipeline with multiple spaces between commands" {
    run "./dsh" <<EOF
ls     |    wc -l
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_regex="^[0-9]+dsh3>dsh3>cmdloopreturned0$"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_regex}"

    [[ "$clean_output" =~ $expected_regex ]]
    [ "$status" -eq 0 ]
}

@test "Transform text to lowercase using 'tr'" {
    run "./dsh" <<EOF
echo "ABCdef" | tr 'A-Z' 'a-z'
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="abcdefdsh3>dsh3>cmdloopreturned0"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_output}"

    [ "$clean_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Echo piped to 'cat'" {
    run "./dsh" <<EOF
echo "Hello, Pipeline" | cat
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="Hello,Pipelinedsh3>dsh3>cmdloopreturned0"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_output}"

    [ "$clean_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Pipeline with no spaces between commands" {
    run "./dsh" <<EOF
ls|wc -l
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_regex="^[0-9]+dsh3>dsh3>cmdloopreturned0$"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_regex}"

    [[ "$clean_output" =~ $expected_regex ]]
    [ "$status" -eq 0 ]
}

@test "Pipeline with leading and trailing spaces" {
    run "./dsh" <<EOF
    ls   |   wc -l    
EOF

    clean_output=$(echo "$output" | tr -d '[:space:]')
    expected_regex="^[0-9]+dsh3>dsh3>cmdloopreturned0$"

    echo "Captured output:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${clean_output} -> ${expected_regex}"

    [[ "$clean_output" =~ $expected_regex ]]
    [ "$status" -eq 0 ]
}

@test "Output redirection with '>'" {
    run "./dsh" <<EOF
echo "Hello, World" > output.txt
EOF

    [ "$status" -eq 0 ]
    [ -f "output.txt" ]
    [ "$(cat output.txt)" = "Hello, World" ]
    rm -f output.txt
}

@test "Output redirection with '>>' (append)" {
    echo "Existing content" > output.txt
    run "./dsh" <<EOF
echo "Hello, World" >> output.txt
EOF

    [ "$status" -eq 0 ]
    [ -f "output.txt" ]
    [ "$(cat output.txt)" = "Existing content\nHello, World" ]
    rm -f output.txt
}

@test "Input redirection with '<'" {
    echo "Hello, World" > input.txt
    run "./dsh" <<EOF
cat < input.txt
EOF

    [ "$status" -eq 0 ]
    [ "$output" = "Hello, Worlddsh3> dsh3> cmd loop returned 0" ]
    rm -f input.txt
}

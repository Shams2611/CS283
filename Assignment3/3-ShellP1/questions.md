1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**: The function fgets() can safely read a line of input to a specified maximum length. As a result, buffer overflow gets prevented. Next, it preserves whitespace which is necessary for shell commands. Additionally, fgets() an detect the end-of-file (EOF) condition, allowing the program to handle scripted or redirected input in a headless mode. These factors make fgets() a great choice. 


2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**: malloc() makes the program more flexible and modular. Now, we can dynamically allocate the buffer size specified by our shell constants (e.g., SH_CMD_MAX) rather than hard-coding a fixed array size on the stack. If the maximum command length changes or if we need to adjust memory usage, we can do so easily by modifying the constant instead of altering the stack-allocated array size. Next, we can avoid potential stack limitations on some systems, since large fixed-size arrays can lead to stack overflow issues.



3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**: Trimming whitespace ensures that commands and arguments are parsed correctly. Without removing extra spaces, a command like " ls" might be misinterpreted or result in empty arguments. By trimming, " ls " becomes "ls", preventing errors and ensuring the shell behaves consistently.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  Redirection is when you send a command’s input or output to or from a file (or device). For example, using command > file.txt writes the output to a file, and command < file.txt reads input from a file. In contrast, piping (using |) connects the output of one command directly to the input of another, like in ls | grep main, without using an intermediate file. The key difference is that redirection uses files or devices as endpoints, while piping creates a live, unidirectional data flow between processes.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection changes where a command gets its input or sends its output—using files or devices (e.g., ls > output.txt or cat < input.txt). In contrast, piping uses the | operator to pass the output of one command directly as the input to another without using an intermediate file. Essentially, redirection deals with files and persistent storage, while piping creates a live data stream between commands.



- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**: Keeping STDERR (error output) and STDOUT (regular output) separate makes it easier to distinguish between normal results and error messages. This separation helps users troubleshoot problems since error messages remain visible even if regular output is redirected elsewhere, and it allows for flexible handling

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  Our custom shell should check the exit status of commands and clearly display any error messages from STDERR. It might also offer an option to merge STDOUT and STDERR (like using 2>&1) for debugging, although this requires careful handling of file descriptors.

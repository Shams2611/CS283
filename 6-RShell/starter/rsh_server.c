    #include <sys/socket.h>
    #include <sys/wait.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>

    #include "dshlib.h"
    #include "rshlib.h"

    /*
    * start_server(ifaces, port, is_threaded)
    */
    int start_server(char *ifaces, int port, int is_threaded) {
        (void)is_threaded;
        int svr_socket = boot_server(ifaces, port);
        if (svr_socket < 0) {
            return svr_socket;
        }

        int rc = process_cli_requests(svr_socket);
        stop_server(svr_socket);

        return rc;
    }

    /*
    * stop_server(svr_socket)
    */
    int stop_server(int svr_socket) {
        return close(svr_socket);
    }

    /*
    * boot_server(ifaces, port)
    */
    int boot_server(char *ifaces, int port) {
        int server_sock;
        struct sockaddr_in server_addr;
        int opt = 1;

        if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            return ERR_RDSH_COMMUNICATION;
        }

        if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt");
            close(server_sock);
            return ERR_RDSH_COMMUNICATION;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ifaces);
        server_addr.sin_port = htons(port);

        if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind");
            close(server_sock);
            return ERR_RDSH_COMMUNICATION;
        }

        if (listen(server_sock, 5) < 0) {
            perror("listen");
            close(server_sock);
            return ERR_RDSH_COMMUNICATION;
        }

        printf("Server listening on %s:%d\n", ifaces, port);
        return server_sock;
    }

    /*
    * process_cli_requests(svr_socket)
    */
    int process_cli_requests(int svr_socket) {
        while (1) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_sock = accept(svr_socket, (struct sockaddr *)&client_addr, &client_len);

            if (client_sock < 0) {
                perror("accept");
                return ERR_RDSH_COMMUNICATION;
            }

            printf("Client connected!\n");

            int rc = exec_client_requests(client_sock);
            close(client_sock);

            if (rc == OK_EXIT) {
                printf("Server stopping as requested by client.\n");
                break;
            }
        }
        return OK;
    }

    /*
    * exec_client_requests(cli_socket)
    */
    int exec_client_requests(int cli_socket) {
        char buffer[SH_CMD_MAX];

        while (1) {
            ssize_t bytes_received = recv(cli_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                printf("Client disconnected.\n");
                break;
            }

            buffer[bytes_received] = '\0';
            printf("Executing command: %s\n", buffer);

            if (strcmp(buffer, "stop-server") == 0) {
                close(cli_socket);
                return OK_EXIT;
            } else if (strncmp(buffer, "cd ", 3) == 0) {
                char *dir = buffer + 3;
                if (chdir(dir) == 0) {
                    send_message_string(cli_socket, "Directory changed.");
                } else {
                    send_message_string(cli_socket, "Failed to change directory.");
                }
                continue;
            }

            command_list_t cmd_list;
            int build_result = build_cmd_list(buffer, &cmd_list);
            if (build_result != OK) {
                send_message_string(cli_socket, "Invalid command.");
                continue;
            }

            pid_t pid = fork();
            if (pid == 0) {
                dup2(cli_socket, STDOUT_FILENO);
                dup2(cli_socket, STDERR_FILENO);

                for (int fd = 0; fd < getdtablesize(); fd++) {
                    if (fd != cli_socket) {
                        close(fd);
                    }
                }

                execute_pipeline(&cmd_list);
                fflush(stdout);
                fflush(stderr);
                exit(0);
            } else if (pid > 0) {
                waitpid(pid, NULL, 0);
            } else {
                perror("fork");
                send_message_string(cli_socket, "Failed to execute command.");
            }

            free_cmd_list(&cmd_list);
            send_message_eof(cli_socket);
        }
        return OK;
    }

    /*
    * send_message_eof(cli_socket)
    */
    int send_message_eof(int cli_socket) {
        char eof_char = RDSH_EOF_CHAR;
        if (send(cli_socket, &eof_char, sizeof(eof_char), 0) < 0) {
            perror("send EOF");
            return ERR_RDSH_COMMUNICATION;
        }
        return OK;
    }

    /*
    * send_message_string(cli_socket, char *buff)
    */
    int send_message_string(int cli_socket, char *buff) {
        int len = strlen(buff) + 1;
        if (send(cli_socket, buff, len, 0) < 0) {
            return ERR_RDSH_COMMUNICATION;
        }
        return send_message_eof(cli_socket);
    }

    /*
    * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
    */
    int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
        int num_cmds = clist->num;
        int num_pipes = num_cmds - 1;
        int pipefd[2 * num_pipes];
        pid_t pids[CMD_MAX];

        for (int i = 0; i < num_pipes; i++) {
            int temp_pipe[2];
            if (pipe(temp_pipe) < 0) {
                perror("pipe");
                return ERR_EXEC_CMD;
            }
            pipefd[2 * i] = temp_pipe[0];
            pipefd[2 * i + 1] = temp_pipe[1];
        }

        for (int i = 0; i < num_cmds; i++) {
            pids[i] = fork();
            if (pids[i] < 0) {
                perror("fork");
                return ERR_EXEC_CMD;
            }

            if (pids[i] == 0) {
                if (i == 0) {
                    if (dup2(cli_sock, STDIN_FILENO) < 0) {
                        perror("dup2");
                        exit(ERR_EXEC_CMD);
                    }
                } else {
                    if (dup2(pipefd[2 * (i - 1)], STDIN_FILENO) < 0) {
                        perror("dup2");
                        exit(ERR_EXEC_CMD);
                    }
                }

                if (i == num_cmds - 1) {
                    if (dup2(cli_sock, STDOUT_FILENO) < 0 || dup2(cli_sock, STDERR_FILENO) < 0) {
                        perror("dup2");
                        exit(ERR_EXEC_CMD);
                    }
                } else {
                    if (dup2(pipefd[2 * i + 1], STDOUT_FILENO) < 0) {
                        perror("dup2");
                        exit(ERR_EXEC_CMD);
                    }
                }

                for (int j = 0; j < 2 * num_pipes; j++) {
                    close(pipefd[j]);
                }

                execvp(clist->commands[i].argv[0], clist->commands[i].argv);
                perror("execvp");
                exit(ERR_EXEC_CMD);
            }
        }

        for (int i = 0; i < 2 * num_pipes; i++) {
            close(pipefd[i]);
        }

        int status;
        for (int i = 0; i < num_cmds; i++) {
            waitpid(pids[i], &status, 0);
        }

        return WEXITSTATUS(status);
    }
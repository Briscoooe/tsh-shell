#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <grp.h>
#include <sys/stat.h>

/**
        Function Declarations for built in shell commands:
 */
int tsh_cd(char **args);
int tsh_help(char **args);
int tsh_logout(char **args);
int tsh_pwd(char **args);
int tsh_ifc(char **args);
int tsh_dt(char **args);
int tsh_ud(char **args);
int tsh_ping(char **args);
int ip_validator(const char *s);

/*
        Defining global variables
 */
#define tsh_TOK_BUFSIZE 64
#define tsh_TOK_DELIM " \t\r\n\a"
#define tsh_RL_BUFSIZE 1024

/*
        List of built in commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
        "cd",
        "help",
        "exit",
        "pw",
        "ifc",
        "dt",
        "ud",
        "ping"
};

int (*builtin_func[]) (char **) = {
        &tsh_cd,
        &tsh_help,
        &tsh_logout,
        &tsh_pwd,
        &tsh_ifc,
        &tsh_dt,
        &tsh_ud,
        &tsh_ping
};

int tsh_num_builtins()
{
        return sizeof(builtin_str) / sizeof(char *);
}

/*
        Built in function implementations.
*/

/**
        @brief Bultin command: change directory.
        @param args List of args.  args[0] is "cd".  args[1] is the directory.
        @return Always returns 1, to continue executing.
 */
int tsh_cd(char **args)
{
        // If no argument is passed
        if (args[1] == NULL)
        {
                fprintf(stderr, "tsh: expected argument to \"cd\"\n");
        }
        else
        {
                if (chdir(args[1]) != 0)
                {
                        perror("tsh");
                }
        }
        return 1;
}

/**
        @brief Built in command: print help.
        @param args List of args.  Not examined.
        @return Always returns 1, to continue executing.
 */
int tsh_help(char **args)
{
        int i;
        printf("Brian Briscoes's tsh\n");
        printf("Type program names and arguments, and hit enter.\n");
        printf("The following are built in:\n");

        // Looping through and printing the list of pre-defined commands
        for (i = 0; i < tsh_num_builtins(); i++)
        {
                printf("  %s\n", builtin_str[i]);
        }

        printf("Use the man command for information on other programs.\n");
        return 1;
}

/**
        @brief Built in command: exit.
        @param args List of args.  Not examined.
        @return Always returns 0, to terminate execution.
 */
int tsh_logout(char **args)
{
        return 0;
}

/**
        @brief Built in command: pwd
        @param args List of args. args[0] is "pw", all other args ignored
        @return Always returns 1, to continue execution
 */
int tsh_pwd(char **args)
{
        // If no argument is passed
        if(args[1] == NULL)
        {
                system("pwd");
        }

        else
        {
                system("pwd");
        }
        return 1;
}

/**
        @brief Built in command: ifconfig
        @param args List of args. args[0] is "ifc", args[1], if provided, is a network interface
        @return Always returns 1, to continue execution
 */
int tsh_ifc(char **args)
{
        FILE *fp;
        char ifc_data[1024];
        char command[100];

        // If no argument is passed
        if(args[1] == NULL)
        {
                system("ifconfig eth0");
        }

        // If the user passes in an interface, call the ifconfig command for that interface
        else if(args[2] == NULL)
        {
                // Combine the interface name with the ifconfig command and store the result in the variable
                sprintf(command, "/sbin/ifconfig %s", args[1]);

                // Run the command
                fp = popen(command, "r");

                // Print the results
                while (fgets(ifc_data, sizeof(ifc_data), fp) != NULL)
                {
                        printf("%s", ifc_data);
                }

                pclose(fp);
        }

        // If the user passes an interface AND an IP address, validate the IP address and assign
        // it to the interface
        else if(args[3] == NULL)
        {
                int result;

                // 
                result = ip_validator(args[2]);
                if(result == 1)
                {
                        // Combine the IP address and interface name with the ifconfig command and store the result in the variable
                        sprintf(command, "sudo /sbin/ifconfig %s %s", args[1], args[2]);

                        // Run the command
                        fp = popen(command, "r");

                        // Print the results
                        while (fgets(ifc_data, sizeof(ifc_data), fp) != NULL)
                        {
                                printf("%s", ifc_data);
                        }
                }

                else
                {
                        fprintf(stderr, "tsh: invalid argument to \"ifc\"\n");

                }

                pclose(fp);
        }

        else
        {
                fprintf(stderr, "tsh: invalid argument to \"ifc\"\n");
        }
        return 1;
}

/**
        @brief Internal command: dt - Displays the current date
        @param List of args. Not examined
        @return Always returns 1, to continue execution
 */
int tsh_dt(char **args)
{
        // Storing the current time as a variable
        time_t now = time(0);
        char buffer[100];

        // Formatting the datetime string
        strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", localtime(&now));
        printf("%s\n",buffer);

        return 1;
}

/**
        @brief Internal command: ud - Displays information about the user
        @param List of args. Not examined
        @return Always returns
 */
int tsh_ud(char **args)
{
        // Declaring the structures needed to get the group name inode
        struct group *g;
        struct stat s;

        // Getting the USER global variable
        char *p=getenv("USER");

        uid_t uid;
        gid_t gid;

        char home[100];

        uid = getuid();
        gid = getgid();
        g = getgrgid(gid);

        // Creating the string to be used for the stat function
        sprintf(home, "/home/%s", p);

        // Calling the stat() function on the home network for the user
        stat(home, &s);

        printf("%d, %d, %s, %s, %ld\n", uid, gid, p, g->gr_name,(long) s.st_ino);
}

/**
        @brief External command: ping - pings a specific IP address
        @param List of args, args[1] is the IP address
        @return Always returns 1, to continue execution
 */
int tsh_ping(char **args)
{
        // If no argument is passed
        if(args[1] == NULL)
        {
            fprintf(stderr, "tsh: expected argument to \"ping\"\n");
        }

        else
        {
                FILE *fp;
                char command[100];
                int result;

                // Validate the IP address passed in
                result = ip_validator(args[1]);
                if(result == 1)
                {
                        // Combine the IP address with the ping command and store the result in the variable
                        sprintf(command, "/bin/ping %s", args[1]);

                        // Run the command
                        fp = popen(command, "r");
                        pclose(fp);
                }

                else
                {
                        fprintf(stderr, "tsh: invalid argument to \"ping\"\n");
                }
        }
        return 1;

}

/**
        @brief Function to check if an IP address is in the valid format xxx.xxx.xxx.xxx where xxx is less than 255
        @param s*, the string containing the ip address
        @return returns 0 if the address is invalid, 1 if it is valid
 */
int ip_validator(const char *s)
{
        int len = strlen(s);
        int i = 0;

        if (len < 7 || len > 15)
        {
                return 0;
        }

        char tail[16];
        tail[0] = 0;

        unsigned int d[4];

        // Ensuring that the IP string has 3 numbers in each octet
        int c = sscanf(s, "%3u.%3u.%3u.%3u%s", &d[0], &d[1], &d[2], &d[3], tail);

        // Checking if there are only 4 octets
        if (c != 4 || tail[0])
        {
                return 0;
        }

        // Checking that none of the numbers the octet is less than 255
        for (i = 0; i < 4; i++)
        {
                if (d[i] > 255)
                {
                        return 0;
                }
        }

        return 1;
}
/**
        @brief Launch a program and wait for it to terminate.
        @param args Null terminated list of arguments (including program).
        @return Always returns 1, to continue execution.
 */
int tsh_launch(char **args)
{
        pid_t pid, wpid;
        int status;

        pid = fork();
        // This is the child process
        if (pid == 0)
        {
                if (execvp(args[0], args) == -1)
                {
                        perror("tsh");
                }

                exit(EXIT_FAILURE);
        }

        else if (pid < 0)
        {
                perror("tsh");
        }

        // This is the parent process
        else
        {
                do
                {
                  wpid = waitpid(pid, &status, WUNTRACED);
                }
                while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }

        return 1;
}

/**
        @brief Execute shell built-in or launch program.
        @param args Null terminated list of arguments.
        @return 1 if the shell should continue running, 0 if it should terminate
 */
int tsh_execute(char **args)
{
        int i;

        // If no command has been entered
        if (args[0] == NULL)
        {
                return 1;
        }

        // Looping through the built-in command functions array
        for (i = 0; i < tsh_num_builtins(); i++)
        {
                // If the argument passed in matches one of the built-in commands, run that command
                if (strcmp(args[0], builtin_str[i]) == 0)
                {
                        return (*builtin_func[i])(args);
                }
        }

        return tsh_launch(args);
}

/**
        @brief Read a line of input from stdin.
        @return The line from stdin.
 */
char *tsh_read_line(void)
{
        int bufsize = tsh_RL_BUFSIZE;
        int position = 0;
        char *buffer = malloc(sizeof(char) * bufsize);
        int c;

        if (!buffer)
        {
                fprintf(stderr, "tsh: allocation error\n");
                exit(EXIT_FAILURE);
        }

        while (1)
        {
                c = getchar();

                if (c == EOF || c == '\n')
                {
                        buffer[position] = '\0';
                        return buffer;
                }

                else
                {
                        buffer[position] = c;
                }
                position++;

                if (position >= bufsize)
                {
                        bufsize += tsh_RL_BUFSIZE;
                        buffer = realloc(buffer, bufsize);
                        if (!buffer)
                        {
                                fprintf(stderr, "tsh: allocation error\n");
                                exit(EXIT_FAILURE);
                        }
                }
        }
}

/**
        @brief Split a line into individual elements
        @param line The line
        @return Null-terminated array of tokens
 */
char **tsh_split_line(char *line)
{
        int bufsize = tsh_TOK_BUFSIZE, position = 0;
        char **tokens = malloc(bufsize * sizeof(char*));
        char *token;

        if (!tokens)
        {
                fprintf(stderr, "tsh: allocation error\n");
                exit(EXIT_FAILURE);
        }

        token = strtok(line, tsh_TOK_DELIM);
        while (token != NULL)
        {
                tokens[position] = token;
                position++;

                if (position >= bufsize)
                {
                        bufsize += tsh_TOK_BUFSIZE;
                        tokens = realloc(tokens, bufsize * sizeof(char*));
                        if (!tokens)
                        {
                                fprintf(stderr, "tsh: allocation error\n");
                                exit(EXIT_FAILURE);
                        }
                }

                token = strtok(NULL, tsh_TOK_DELIM);
        }
        tokens[position] = NULL;
        return tokens;
}

/**
        @brief Loop getting input and executing it
 */
void tsh_loop(void)
{
        FILE *fp;
        char *line;
        char **args;
        int status;
        char whoami[100];

        // Running the username command 
        fp = popen("/usr/bin/whoami", "r");
        while(fgets(whoami, sizeof(whoami)-1, fp) != NULL)
        {
                // Removing the trailing new line character
                strtok(whoami, "\n");
        }

        pclose(fp);
        do
        {
                printf("%s@tsh > ", whoami);
                line = tsh_read_line();
                args = tsh_split_line(line);
                status = tsh_execute(args);

                free(line);
                free(args);
        }
        while (status);
}

/**
        @brief Main entry point
        @param argc Argument count
        @param argv Argument vector
        @return status code
 */
int main(int argc, char **argv)
{
        tsh_loop();

        // Perform any shutdown/cleanup.=
        return EXIT_SUCCESS;
}
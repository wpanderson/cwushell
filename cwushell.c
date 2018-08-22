// Author: Weston Anderson
// Class: CS370 Unix
// Assignment: Lab 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <regex.h>
#include <string.h>
#include <unistd.h>

#define MAX 1024

// Takes a linux command and arguments attached to that command and checks if it is a valid linux command.
// This is done by piping the simplified man file output for the command into the a file pointer and checking each
// argument against it. If an argument is invalid the function returns the character of the argument which was invalid,
// otherwise it returns ' ' signifying a valid command.
//
// @param command: Command entered by the user. (ls, ps, df, uname)
// @param arguments: Character string of arguments entered by the user with the command.
//
// Returns: Character value of the argument that is invalid, or ' '
char exists(char *command, char *arguments)
{
    // man file filter: man ls | grep "-" | awk '{print $1}'
    // Better filter: man ls | awk '{print $1}' | grep "-" | grep -v "\--"
    char invalid = ' ';
    bool is_valid = true;
    char *arg = arguments;
    char mancmd[50] = "man /bin/";
    char *grep = " | grep \"-\"";
    char *trim_grep = " | grep -v \"\\--\"";
    char *simplify = " | awk '{print $1}'"; //Grabs only the first column of the man file for searching
    char *manCall = strcat(mancmd, command);
    manCall = strcat(manCall, simplify);
    manCall = strcat(manCall, grep);
    manCall = strcat(manCall, trim_grep);
//    printf("MAN CALL: %s\n", manCall);
    FILE *fp;
    char buffer[MAX];
    if (arg != NULL)
    {
        // Call man file and redirect output to file.
        fp = popen(manCall, "r");
        if (fp == NULL)
        {
            printf("Failed to find man page command not valid\n");
            return ' ';
        }
        else {
            for (int i = 0; i < strlen(arg); i++)
            {
                if (arg[i] != '-' && arg[i] != ' ') //Don't bother searching if this is true
                {
                    bool check = false; //Reset check for each character argument
                    // Search key for man file, checks for "-%c" in man file.
                    char searchKey[3] = {'-', arg[i],'\0'};
//                    printf("Argument to Check: %s\n",searchKey); //DEBUG
//                    rewind(fp);
                    fp = popen(manCall, "r");
                    while (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
                        if (strstr(buffer, searchKey) != NULL) //Check if man page contains argument
                        {
//                            printf("Command is valid %s\n", buffer); //DEBUG
                            check = true;
//                            rewind(fp);
                            break;

                        }
//                        printf("%s\n", buffer); //Instead of printing check each line for existence of command DEBUG
                    }
                    if (check == false)
                    {
                        is_valid = false;
                        invalid = arg[i];
                        break;
                    }

//                    rewind(fp);
                }
            }
        }
        pclose(fp);
    }
    return invalid; //Returns ' ' if there are no errors or the character argument which caused the error
}

// Main function for cwushell, contains main print loop and simple command recognition.
// Utilizes strtok() to tokenize "line" from stdin and determine which command is entered.
// Based on this information tokenizes further for arguments and calls exists() to determine
// if the command is valid or not based on the linux man file.
// Press ctr+d or ctr+c to exit.
int main(int argc, char *argv[])
{
    char *cmd;
    char line[MAX];     //Line data to tokenize
    char newLine[MAX]; //To contain unmodified line data
    char buffer[MAX]; //Buffer for various tokenizing purposes
    printf("cwushell starting... press ctrl-d to quit.\n");
    while (1)
    {
        printf("cwushell> ");
        if (!fgets(line, MAX, stdin)) break;
        // strtok() has an effect on the strings you tokenize so I made a copy for system calls.
        strcpy(newLine, line);
        if ((cmd = strtok(line, " \t\r\n")))
        {
            errno = 0;
            if (strcmp(cmd, "cd") == 0)
            {

                //Execute "cd" command
                char *arg = strtok(0, " \t\r\n");

                if (!arg) fprintf(stderr, "cd missing argument.\n");
                else chdir(arg);

            }
            else if (strcmp(cmd, "ls") == 0)
            {
                char invalid = ' ';
                char *arg = strtok(0, "\n"); //Grab all the arguments passed with command on the line
                if (arg != NULL)
                {
                    invalid = exists(cmd, arg);
                }

                if (invalid != ' ')
                {
                    printf("Command is invalid -%c was not in the man file. \n Try ls --help for options.\n", invalid);
                }
                else
                {
                    system(newLine);
                }
            }
            else if (strcmp(cmd, "mkdir") == 0)
            {
                char *arg = strtok(0, " \t\r\n");

                if (!arg) fprintf(stderr, "mkdir missing argument.\n");
                else
                {
                    int status;
                    status = mkdir(arg, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                }
            }
            else if (strcmp(cmd, "cmp") == 0)
            {
                char *arg = strtok(0, " \t\r\n");
                char *arg2 = strtok(0, " \t\r\n");
                if (arg2 != NULL && arg != NULL) {


                    int pid = fork();
                    if (pid == 0) {
                        execl("/usr/bin/cmp", "cmp", arg, arg2, NULL);
                    } else {
                        wait(NULL);
                    }
                }
                else
                {
                    int pid = fork();
                    if (pid == 0)
                    {
                        if (arg == NULL)
                        {
                            execl("/usr/bin/cmp", "cmp", "", arg2, NULL);
                        }
                        else if (arg2 == NULL)
                        {
                            execl("/usr/bin/cmp", "cmp", arg, "", NULL);
                        }
                        else
                            execl("/usr/bin/cmp", "cmp", arg, arg2, NULL);
                    } else {
                        wait(NULL);
                    }
                }

            }
            else if (strcmp(cmd, "df") == 0)
            {
                char invalid = ' ';
                char *arg = strtok(0, " \t\r\n"); //Grab all the arguments passed with command
                if (arg != NULL)
                {
                    invalid = exists(cmd, arg);
                }

                if (invalid != ' ')
                {
                    printf("Command is invalid -%c was not in the man file. \n Try df --help for options.\n",
                           invalid);
                }
                else
                {
                    system(newLine);
                }
            }
            else if (strcmp(cmd, "uname") == 0)

            {
                char invalid = ' ';
                char *arg = strtok(0, " \t\r\n");
                if (arg != NULL)
                {
                    invalid = exists(cmd, arg);
                }

                if (invalid != ' ')
                {
                    printf("Command is invalid -%c was not in the man file. \nTry uname --help for options.\n", invalid);
                }
                else
                {
                    system(newLine);
                }
            }
            else if (strcmp(cmd, "ps") == 0)
            {
                char invalid = ' ';
                char *arg = strtok(0, " \t\r\n");
                if (arg != NULL)
                {
                    invalid = exists(cmd, arg);
                }

                if (invalid != ' ')
                {
                    printf("Command is invalid -%c was not in the man file. \nTry ps --help for options\n", invalid);
                }
                else
                {
                    system(newLine);
                }
            }
            else if (strcmp(cmd, "exit") == 0)
            {
                return 0;
            }
            else system(newLine); //Execute if command isn't in presets.
        }

    }
    return 0;
}

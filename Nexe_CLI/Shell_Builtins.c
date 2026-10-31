/*
  Function Declarations for builtin shell commands:
 */
int nsh_cd(char **args);
int nsh_help(char **args);
int nsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &nsh_cd,
  &nsh_help,
  &nsh_exit
};

int nsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int nsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "nsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("nsh");
    }
  }
  return 1;
}

int nsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's nsh\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < nsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int nsh_exit(char **args)
{
  return 0;
}


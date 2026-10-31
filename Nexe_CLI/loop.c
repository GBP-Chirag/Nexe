void nsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = nsh_read_line();
    args = nsh_split_line(line);
    status = nsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

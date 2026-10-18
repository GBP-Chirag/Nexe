# Nexe BuildStream Starlark template

`render.star` is the configuration and macro layer. It only assembles Starlark dictionaries and lists, then prints a framed element bundle. It has no host-side effects.

Nexe's trusted renderer consumes that bundle, rejects paths outside `elements/`, and writes static `.bst` YAML:

```text
nexe render-bst buildstream
```

The generated output is `elements/hello.bst`. `project.conf` keeps the required BuildStream project metadata separate from generated elements.

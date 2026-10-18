/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/

package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"

	"github.com/spf13/cobra"
)

var enginePath string

var rootCmd = &cobra.Command{
	Use:           "nexe",
	Short:         "A local, content-addressed build system",
	Long:          "Nexe builds tasks from BUILD.sbl and skips tasks whose command and inputs have not changed. Legacy build.toml files are also supported.",
	SilenceUsage:  true,
	SilenceErrors: true,
}

var buildCmd = &cobra.Command{
	Use:   "build [BUILD.sbl]",
	Short: "Build tasks in dependency order",
	Args:  cobra.MaximumNArgs(1),
	RunE: func(_ *cobra.Command, args []string) error {
		manifest := "BUILD.sbl"
		if len(args) == 1 { manifest = args[0] }
		return runBridge("build", manifest)
	},
}

var initCmd = &cobra.Command{
	Use:   "init [directory]",
	Short: "Create BUILD.sbl, a legacy build.toml, and hello.c",
	Args:  cobra.MaximumNArgs(1),
	RunE: func(_ *cobra.Command, args []string) error {
		directory := "."
		if len(args) == 1 { directory = args[0] }
		return runBridge("init", directory)
	},
}

var hashCmd = &cobra.Command{
	Use:   "hash <file>",
	Short: "Print a BLAKE3 digest for a file",
	Args:  cobra.ExactArgs(1),
	RunE: func(_ *cobra.Command, args []string) error { return runBridge("hash", args[0]) },
}

var renderBuildStreamCmd = &cobra.Command{
	Use:   "render-bst [buildstream-directory]",
	Short: "Render deterministic Starlark templates into BuildStream .bst files",
	Args:  cobra.MaximumNArgs(1),
	RunE: func(_ *cobra.Command, args []string) error {
		directory := "buildstream"
		if len(args) == 1 { directory = args[0] }
		return runBridge("render-bst", directory)
	},
}

func Execute() {
	if err := rootCmd.Execute(); err != nil { fmt.Fprintln(os.Stderr, "nexe:", err); os.Exit(1) }
}

func repositoryRoot() string {
	_, file, _, ok := runtime.Caller(0)
	if !ok { return "." }
	return filepath.Clean(filepath.Join(filepath.Dir(file), ".."))
}

func runBridge(action, target string) error {
	python := os.Getenv("PYTHON")
	if python == "" { python = "python3" }
	bridge := filepath.Join(repositoryRoot(), "_Nexe", "Orchestration", "bridge.py")
	args := []string{bridge, action, target}
	if enginePath != "" { args = append(args, "--engine", enginePath) }
	command := exec.Command(python, args...)
	command.Stdout, command.Stderr, command.Stdin = os.Stdout, os.Stderr, os.Stdin
	return command.Run()
}

func init() {
	rootCmd.PersistentFlags().StringVar(&enginePath, "engine", "", "path to the nexe_engine executable")
	rootCmd.AddCommand(buildCmd, initCmd, hashCmd, renderBuildStreamCmd)
}

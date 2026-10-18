/*  This is the project form creator Chirag Balakrishna.
    This project is licensed under GPLv3
    Thats it ! Feel free to contribute !! Happy Journey 😀
*/

package main

import (
	"fmt"
	"os"
	"os/exec"
)

// ExecuteBinary runs the compiled binary and pipes its output live to the console
func ExecuteBinary(binPath string, args []string) error {
	fmt.Printf("[Nexe] Running target: %s\n", binPath)

	cmd := exec.Command(binPath, args...)
	
	// Connect the binary's input/output directly to the user's terminal
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin

	// Run the command and wait for it to finish
	err := cmd.Run()
	if err != nil {
		return fmt.Errorf("execution failed for %s: %w", binPath, err)
	}

	return nil
}
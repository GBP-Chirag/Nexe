package main

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

const LocThreshold = 50000

// CountProjectLOC walks the source tree and estimates total lines of code
func CountProjectLOC(root string) (int, error) {
	totalLines := 0

	err := filepath.Walk(root, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		// Skip hidden folders, build dirs, and binary artifacts
		if info.IsDir() && (strings.HasPrefix(info.Name(), ".") || info.Name() == "build") {
			return filepath.SkipDir
		}

		// Count lines for source files (.cpp, .c, .go, .py, .sbl)
		if !info.IsDir() && isSourceFile(info.Name()) {
			lines, err := countLinesInFile(path)
			if err == nil {
				totalLines += lines
			}
		}
		return nil
	})

	return totalLines, err
}

func isSourceFile(filename string) bool {
	exts := []string{".cpp", ".c", ".h", ".go", ".py", ".sbl"}
	for _, ext := range exts {
		if strings.HasSuffix(filename, ext) {
			return true
		}
	}
	return false
}

func countLinesInFile(filepath string) (int, error) {
	file, err := os.Open(filepath)
	if err != nil {
		return 0, err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	lines := 0
	for scanner.Scan() {
		lines++
	}
	return lines, scanner.Err()
}

func CheckEnterpriseEscalation(root string) {
	loc, err := CountProjectLOC(root)
	if err != nil {
		fmt.Printf("Error scanning project size: %v\n", err)
		return
	}

	fmt.Printf("[Nexe Analytics] Total Project Size: %d LOC\n", loc)
	if loc > LocThreshold {
		fmt.Println("⚠️  [NOTICE] Codebase exceeds 50K LOC threshold!")
		fmt.Println("   Recommended: Transitioning workspace configuration to BuildStream / REAPI enterprise layer.")
	} else {
		fmt.Println("✨ Codebase fits comfortably within Nexe's Local C/C++ Engine.")
	}
}
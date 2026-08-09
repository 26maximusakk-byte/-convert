// convert.go
package main

import (
	"flag"
	"fmt"
	"image/jpeg"
	"image/png"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"github.com/go-heic/heic"
	_ "golang.org/x/image/webp"
)

func convertHeic(inputPath, outputPath, format string, quality int, resizeW, resizeH int) error {
	// Открываем HEIC
	img, err := heic.DecodeFile(inputPath)
	if err != nil {
		return err
	}
	// Изменение размера (упрощённо, пропускаем для краткости)
	// Для реального ресайза можно использовать imaging
	// Сохранение
	out, err := os.Create(outputPath)
	if err != nil {
		return err
	}
	defer out.Close()
	if format == "jpg" || format == "jpeg" {
		opts := jpeg.Options{Quality: quality}
		return jpeg.Encode(out, img, &opts)
	} else if format == "png" {
		return png.Encode(out, img)
	} else if format == "webp" {
		// использование webp требует отдельной библиотеки
		return fmt.Errorf("WebP поддержка не реализована в этой версии")
	}
	return fmt.Errorf("неизвестный формат")
}

func processFiles(inputs []string, format string, quality int, resizeW, resizeH int,
	outputDir string, overwrite, recursive bool) {
	var files []string
	exts := map[string]bool{".heic": true, ".heif": true}
	for _, item := range inputs {
		info, err := os.Stat(item)
		if err == nil && !info.IsDir() {
			ext := strings.ToLower(filepath.Ext(item))
			if exts[ext] {
				files = append(files, item)
			}
		} else if err == nil && info.IsDir() {
			if recursive {
				filepath.Walk(item, func(path string, info os.FileInfo, err error) error {
					if err == nil && !info.IsDir() {
						ext := strings.ToLower(filepath.Ext(path))
						if exts[ext] {
							files = append(files, path)
						}
					}
					return nil
				})
			} else {
				entries, _ := os.ReadDir(item)
				for _, e := range entries {
					if !e.IsDir() {
						ext := strings.ToLower(filepath.Ext(e.Name()))
						if exts[ext] {
							files = append(files, filepath.Join(item, e.Name()))
						}
					}
				}
			}
		} else if strings.Contains(item, "*") {
			matches, _ := filepath.Glob(item)
			for _, m := range matches {
				ext := strings.ToLower(filepath.Ext(m))
				if exts[ext] {
					files = append(files, m)
				}
			}
		}
	}
	if len(files) == 0 {
		fmt.Println("Не найдено HEIC-файлов.")
		return
	}
	if err := os.MkdirAll(outputDir, 0755); err != nil {
		fmt.Println("Ошибка создания папки:", err)
		return
	}
	total := len(files)
	fmt.Printf("Найдено %d HEIC-файлов.\n", total)
	var wg sync.WaitGroup
	sem := make(chan struct{}, 4)
	for i, f := range files {
		wg.Add(1)
		go func(idx int, inputPath string) {
			defer wg.Done()
			sem <- struct{}{}
			defer func() { <-sem }()
			ext := "." + format
			if format == "jpg" {
				ext = ".jpg"
			}
			outName := strings.TrimSuffix(filepath.Base(inputPath), filepath.Ext(inputPath)) + ext
			outPath := filepath.Join(outputDir, outName)
			if _, err := os.Stat(outPath); err == nil && !overwrite {
				fmt.Printf("[%d/%d] %s уже существует, пропуск.\n", idx+1, total, outPath)
				return
			}
			fmt.Printf("[%d/%d] Конвертация %s -> %s\n", idx+1, total, inputPath, outPath)
			err := convertHeic(inputPath, outPath, format, quality, resizeW, resizeH)
			if err != nil {
				fmt.Printf("  Ошибка при конвертации %s: %v\n", inputPath, err)
			}
		}(i, f)
	}
	wg.Wait()
	fmt.Println("Готово!")
}

func main() {
	format := flag.String("format", "jpg", "Выходной формат (jpg, png, webp)")
	quality := flag.Int("quality", 85, "Качество (1-100)")
	resize := flag.String("resize", "", "Изменение размера (ШxВ)")
	output := flag.String("output", ".", "Папка для сохранения")
	overwrite := flag.Bool("overwrite", false, "Перезаписывать")
	recursive := flag.Bool("recursive", false, "Рекурсивный обход")
	flag.Parse()
	inputs := flag.Args()
	if len(inputs) == 0 {
		fmt.Println("Использование: convert <HEIC-файлы/папки> [--format jpg|png|webp] [--quality N] [--resize ШxВ] [--output DIR] [--overwrite] [--recursive]")
		return
	}
	var resizeW, resizeH int
	if *resize != "" {
		fmt.Sscanf(*resize, "%dx%d", &resizeW, &resizeH)
	}
	processFiles(inputs, *format, *quality, resizeW, resizeH, *output, *overwrite, *recursive)
}

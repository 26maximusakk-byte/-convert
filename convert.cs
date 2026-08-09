// convert.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;
using SixLabors.ImageSharp.Processing;
using SixLabors.ImageSharp.Formats.Jpeg;
using SixLabors.ImageSharp.Formats.Png;
using SixLabors.ImageSharp.Formats.Webp;

class Convert
{
    static void ConvertHeic(string inputPath, string outputPath, string format, int quality, int resizeW, int resizeH)
    {
        using (var image = Image.Load<Rgba32>(inputPath))
        {
            if (resizeW > 0 && resizeH > 0)
                image.Mutate(x => x.Resize(resizeW, resizeH));
            if (format == "jpg" || format == "jpeg")
            {
                var encoder = new JpegEncoder { Quality = quality };
                image.Save(outputPath, encoder);
            }
            else if (format == "png")
            {
                var encoder = new PngEncoder { CompressionLevel = (PngCompressionLevel)(quality / 10) };
                image.Save(outputPath, encoder);
            }
            else if (format == "webp")
            {
                var encoder = new WebpEncoder { Quality = quality };
                image.Save(outputPath, encoder);
            }
            else
                throw new Exception("Неизвестный формат");
        }
    }

    static void ProcessFiles(List<string> inputs, string format, int quality, int resizeW, int resizeH,
                             string outputDir, bool overwrite, bool recursive)
    {
        var files = new List<string>();
        var exts = new HashSet<string> { ".heic", ".heif" };
        foreach (var item in inputs)
        {
            if (File.Exists(item))
            {
                string ext = Path.GetExtension(item).ToLower();
                if (exts.Contains(ext)) files.Add(item);
            }
            else if (Directory.Exists(item))
            {
                var search = recursive ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly;
                files.AddRange(Directory.GetFiles(item, "*.*", search)
                               .Where(f => exts.Contains(Path.GetExtension(f).ToLower())));
            }
            else if (item.Contains("*"))
            {
                // упрощённо
            }
        }
        if (files.Count == 0)
        {
            Console.WriteLine("Не найдено HEIC-файлов.");
            return;
        }
        Directory.CreateDirectory(outputDir);
        int total = files.Count;
        Console.WriteLine($"Найдено {total} HEIC-файлов.");
        for (int i=0; i<total; i++)
        {
            var inputFile = files[i];
            string ext = format == "jpg" ? ".jpg" : $".{format}";
            var outName = Path.GetFileNameWithoutExtension(inputFile) + ext;
            var outPath = Path.Combine(outputDir, outName);
            if (File.Exists(outPath) && !overwrite)
            {
                Console.WriteLine($"[{i+1}/{total}] {outPath} уже существует, пропуск.");
                continue;
            }
            Console.WriteLine($"[{i+1}/{total}] Конвертация {inputFile} -> {outPath}");
            try
            {
                ConvertHeic(inputFile, outPath, format, quality, resizeW, resizeH);
            }
            catch (Exception e)
            {
                Console.WriteLine($"  Ошибка при конвертации {inputFile}: {e.Message}");
            }
        }
        Console.WriteLine("Готово!");
    }

    static void Main(string[] args)
    {
        if (args.Length == 0)
        {
            Console.WriteLine("Использование: dotnet run <HEIC-файлы/папки> [--format jpg|png|webp] [--quality N] [--resize ШxВ] [--output DIR] [--overwrite] [--recursive]");
            return;
        }
        var inputs = new List<string>();
        string format = "jpg";
        int quality = 85;
        int resizeW = 0, resizeH = 0;
        string outputDir = ".";
        bool overwrite = false;
        bool recursive = false;
        for (int i=0; i<args.Length; i++)
        {
            switch (args[i])
            {
                case "--format":
                    if (i+1 < args.Length) format = args[++i];
                    break;
                case "--quality":
                    if (i+1 < args.Length) quality = int.Parse(args[++i]);
                    break;
                case "--resize":
                    if (i+1 < args.Length)
                    {
                        var s = args[++i];
                        var parts = s.Split('x');
                        if (parts.Length == 2)
                        {
                            resizeW = int.Parse(parts[0]);
                            resizeH = int.Parse(parts[1]);
                        }
                    }
                    break;
                case "--output":
                    if (i+1 < args.Length) outputDir = args[++i];
                    break;
                case "--overwrite":
                    overwrite = true;
                    break;
                case "--recursive":
                    recursive = true;
                    break;
                default:
                    inputs.Add(args[i]);
                    break;
            }
        }
        ProcessFiles(inputs, format, quality, resizeW, resizeH, outputDir, overwrite, recursive);
    }
}

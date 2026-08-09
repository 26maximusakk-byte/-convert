// Convert.java
import javax.imageio.*;
import javax.imageio.stream.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class Convert {

    static {
        // Регистрация плагина HEIC (например, TwelveMonkeys)
        // Требуется добавить в classpath библиотеку
    }

    public static void convertHeic(String inputPath, String outputPath, String format, int quality, int resizeW, int resizeH) throws IOException {
        BufferedImage img = ImageIO.read(new File(inputPath));
        if (img == null) {
            throw new IOException("Не удалось прочитать HEIC");
        }
        if (resizeW > 0 && resizeH > 0) {
            Image scaled = img.getScaledInstance(resizeW, resizeH, Image.SCALE_SMOOTH);
            BufferedImage resized = new BufferedImage(resizeW, resizeH, BufferedImage.TYPE_INT_RGB);
            Graphics2D g = resized.createGraphics();
            g.drawImage(scaled, 0, 0, null);
            g.dispose();
            img = resized;
        }
        String formatName = format.equals("jpg") ? "jpeg" : format;
        ImageWriter writer = ImageIO.getImageWritersByFormatName(formatName).next();
        ImageWriteParam param = writer.getDefaultWriteParam();
        if (param.canWriteCompressed()) {
            param.setCompressionMode(ImageWriteParam.MODE_EXPLICIT);
            param.setCompressionQuality(quality / 100.0f);
        }
        try (ImageOutputStream ios = ImageIO.createImageOutputStream(new File(outputPath))) {
            writer.setOutput(ios);
            writer.write(null, new IIOImage(img, null, null), param);
        }
        writer.dispose();
    }

    public static void processFiles(String[] inputs, String format, int quality, int resizeW, int resizeH,
                                    String outputDir, boolean overwrite, boolean recursive) throws IOException {
        java.util.List<String> files = new ArrayList<>();
        Set<String> exts = new HashSet<>(Arrays.asList(".heic", ".heif"));
        for (String item : inputs) {
            Path path = Paths.get(item);
            if (Files.isRegularFile(path)) {
                String ext = getExtension(path.toString()).toLowerCase();
                if (exts.contains(ext)) files.add(path.toString());
            } else if (Files.isDirectory(path)) {
                if (recursive) {
                    Files.walk(path)
                        .filter(p -> Files.isRegularFile(p))
                        .forEach(p -> {
                            String ext = getExtension(p.toString()).toLowerCase();
                            if (exts.contains(ext)) files.add(p.toString());
                        });
                } else {
                    try (DirectoryStream<Path> stream = Files.newDirectoryStream(path)) {
                        for (Path p : stream) {
                            if (Files.isRegularFile(p)) {
                                String ext = getExtension(p.toString()).toLowerCase();
                                if (exts.contains(ext)) files.add(p.toString());
                            }
                        }
                    }
                }
            } else if (item.contains("*")) {
                // упрощённо
            }
        }
        if (files.isEmpty()) {
            System.out.println("Не найдено HEIC-файлов.");
            return;
        }
        Files.createDirectories(Paths.get(outputDir));
        int total = files.size();
        System.out.println("Найдено " + total + " HEIC-файлов.");
        for (int i=0; i<total; i++) {
            String inputFile = files.get(i);
            String ext = (format.equals("jpg") || format.equals("jpeg")) ? ".jpg" : "."+format;
            String outName = getFileNameWithoutExtension(inputFile) + ext;
            String outPath = Paths.get(outputDir, outName).toString();
            if (Files.exists(Paths.get(outPath)) && !overwrite) {
                System.out.printf("[%d/%d] %s уже существует, пропуск.\n", i+1, total, outPath);
                continue;
            }
            System.out.printf("[%d/%d] Конвертация %s -> %s\n", i+1, total, inputFile, outPath);
            try {
                convertHeic(inputFile, outPath, format, quality, resizeW, resizeH);
            } catch (Exception e) {
                System.err.println("  Ошибка при конвертации " + inputFile + ": " + e.getMessage());
            }
        }
        System.out.println("Готово!");
    }

    private static String getExtension(String path) {
        int i = path.lastIndexOf('.');
        return i > 0 ? path.substring(i) : "";
    }

    private static String getFileNameWithoutExtension(String path) {
        String name = Paths.get(path).getFileName().toString();
        int i = name.lastIndexOf('.');
        return i > 0 ? name.substring(0, i) : name;
    }

    public static void main(String[] args) throws IOException {
        if (args.length < 1) {
            System.out.println("Использование: java Convert <HEIC-файлы/папки> [--format jpg|png|webp] [--quality N] [--resize ШxВ] [--output DIR] [--overwrite] [--recursive]");
            return;
        }
        java.util.List<String> inputs = new ArrayList<>();
        String format = "jpg";
        int quality = 85;
        int resizeW = 0, resizeH = 0;
        String outputDir = ".";
        boolean overwrite = false, recursive = false;
        for (int i=0; i<args.length; i++) {
            switch (args[i]) {
                case "--format":
                    if (i+1 < args.length) format = args[++i];
                    break;
                case "--quality":
                    if (i+1 < args.length) quality = Integer.parseInt(args[++i]);
                    break;
                case "--resize":
                    if (i+1 < args.length) {
                        String s = args[++i];
                        String[] parts = s.split("x");
                        if (parts.length == 2) {
                            resizeW = Integer.parseInt(parts[0]);
                            resizeH = Integer.parseInt(parts[1]);
                        }
                    }
                    break;
                case "--output":
                    if (i+1 < args.length) outputDir = args[++i];
                    break;
                case "--overwrite":
                    overwrite = true;
                    break;
                case "--recursive":
                    recursive = true;
                    break;
                default:
                    inputs.add(args[i]);
            }
        }
        processFiles(inputs.toArray(new String[0]), format, quality, resizeW, resizeH,
                     outputDir, overwrite, recursive);
    }
}

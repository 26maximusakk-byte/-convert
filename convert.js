// convert.js
const fs = require('fs');
const path = require('path');
const sharp = require('sharp');
const glob = require('glob');

async function convertHeic(inputPath, outputPath, format, quality, resizeW, resizeH) {
    let pipeline = sharp(inputPath);
    if (resizeW > 0 && resizeH > 0) {
        pipeline = pipeline.resize(resizeW, resizeH);
    }
    if (format === 'jpg' || format === 'jpeg') {
        await pipeline.jpeg({ quality }).toFile(outputPath);
    } else if (format === 'png') {
        await pipeline.png({ quality }).toFile(outputPath);
    } else if (format === 'webp') {
        await pipeline.webp({ quality }).toFile(outputPath);
    } else {
        throw new Error(`Неизвестный формат: ${format}`);
    }
}

function findFiles(inputs, recursive) {
    const exts = ['.heic', '.heif'];
    const files = [];
    for (const item of inputs) {
        if (fs.existsSync(item) && fs.statSync(item).isFile() && exts.includes(path.extname(item).toLowerCase())) {
            files.push(item);
        } else if (fs.existsSync(item) && fs.statSync(item).isDirectory()) {
            const pattern = recursive ? `${item}/**/*.*` : `${item}/*.*`;
            const matches = glob.sync(pattern, { nodir: true });
            for (const m of matches) {
                if (exts.includes(path.extname(m).toLowerCase())) {
                    files.push(m);
                }
            }
        } else if (item.includes('*')) {
            const matches = glob.sync(item);
            for (const m of matches) {
                if (exts.includes(path.extname(m).toLowerCase())) {
                    files.push(m);
                }
            }
        }
    }
    return files;
}

function processFiles(inputs, format, quality, resizeW, resizeH, outputDir, overwrite, recursive) {
    const files = findFiles(inputs, recursive);
    if (files.length === 0) {
        console.log('Не найдено HEIC-файлов.');
        return;
    }
    if (!fs.existsSync(outputDir)) {
        fs.mkdirSync(outputDir, { recursive: true });
    }
    const total = files.length;
    console.log(`Найдено ${total} HEIC-файлов.`);
    (async () => {
        for (let i=0; i<total; i++) {
            const inputFile = files[i];
            const ext = format === 'jpg' ? '.jpg' : `.${format}`;
            const outName = path.basename(inputFile, path.extname(inputFile)) + ext;
            const outPath = path.join(outputDir, outName);
            if (fs.existsSync(outPath) && !overwrite) {
                console.log(`[${i+1}/${total}] ${outPath} уже существует, пропуск.`);
                continue;
            }
            console.log(`[${i+1}/${total}] Конвертация ${inputFile} -> ${outPath}`);
            try {
                await convertHeic(inputFile, outPath, format, quality, resizeW, resizeH);
            } catch (err) {
                console.error(`  Ошибка при конвертации ${inputFile}: ${err.message}`);
            }
        }
        console.log('Готово!');
    })();
}

function main() {
    const args = process.argv.slice(2);
    if (args.length === 0) {
        console.log('Использование: node convert.js <HEIC-файлы/папки> [--format jpg|png|webp] [--quality N] [--resize ШxВ] [--output DIR] [--overwrite] [--recursive]');
        return;
    }
    let format = 'jpg';
    let quality = 85;
    let resizeW = 0, resizeH = 0;
    let outputDir = '.';
    let overwrite = false;
    let recursive = false;
    const inputs = [];
    for (let i=0; i<args.length; i++) {
        switch (args[i]) {
            case '--format':
                if (i+1 < args.length) format = args[++i];
                break;
            case '--quality':
                if (i+1 < args.length) quality = parseInt(args[++i]);
                break;
            case '--resize':
                if (i+1 < args.length) {
                    const s = args[++i];
                    const parts = s.split('x');
                    if (parts.length === 2) {
                        resizeW = parseInt(parts[0]);
                        resizeH = parseInt(parts[1]);
                    }
                }
                break;
            case '--output':
                if (i+1 < args.length) outputDir = args[++i];
                break;
            case '--overwrite':
                overwrite = true;
                break;
            case '--recursive':
                recursive = true;
                break;
            default:
                inputs.push(args[i]);
        }
    }
    processFiles(inputs, format, quality, resizeW, resizeH, outputDir, overwrite, recursive);
}

main();

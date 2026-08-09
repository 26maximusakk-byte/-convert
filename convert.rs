// convert.rs
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::mpsc;
use std::thread;
use image::{GenericImageView, ImageFormat, DynamicImage, imageops::FilterType};
use heic::HeicDecoder;

fn convert_heic(input_path: &Path, output_path: &Path, format: &str, quality: u8, resize: Option<(u32, u32)>) -> Result<(), String> {
    // Декодируем HEIC
    let decoder = HeicDecoder::from_file(input_path).map_err(|e| e.to_string())?;
    let img = decoder.decode().map_err(|e| e.to_string())?;
    let mut img = DynamicImage::ImageRgba8(img);
    if let Some((w, h)) = resize {
        img = img.resize(w, h, FilterType::Lanczos3);
    }
    // Сохранение
    match format {
        "jpg" | "jpeg" => {
            let mut out = fs::File::create(output_path).map_err(|e| e.to_string())?;
            let mut encoder = image::jpeg::JpegEncoder::new_with_quality(&mut out, quality);
            encoder.encode_image(&img).map_err(|e| e.to_string())?;
        }
        "png" => {
            img.save_with_format(output_path, ImageFormat::Png).map_err(|e| e.to_string())?;
        }
        "webp" => {
            let mut out = fs::File::create(output_path).map_err(|e| e.to_string())?;
            let encoder = image::codecs::webp::WebPEncoder::new(&mut out);
            encoder.encode(&img, quality).map_err(|e| e.to_string())?;
        }
        _ => return Err("Неизвестный формат".to_string()),
    }
    Ok(())
}

fn process_files(inputs: Vec<String>, format: String, quality: u8, resize: Option<(u32, u32)>,
                 output_dir: &str, overwrite: bool, recursive: bool) {
    let mut files: Vec<PathBuf> = Vec::new();
    let exts = ["heic", "heif"];
    for item in inputs {
        let path = Path::new(&item);
        if path.is_file() {
            if let Some(ext) = path.extension() {
                let ext_lower = ext.to_str().unwrap_or("").to_lowercase();
                if exts.contains(&ext_lower.as_str()) {
                    files.push(path.to_path_buf());
                }
            }
        } else if path.is_dir() {
            if recursive {
                for entry in walkdir::WalkDir::new(path) {
                    let entry = entry.unwrap();
                    let p = entry.path();
                    if p.is_file() {
                        if let Some(ext) = p.extension() {
                            let ext_lower = ext.to_str().unwrap_or("").to_lowercase();
                            if exts.contains(&ext_lower.as_str()) {
                                files.push(p.to_path_buf());
                            }
                        }
                    }
                }
            } else {
                for entry in fs::read_dir(path).unwrap() {
                    let entry = entry.unwrap();
                    let p = entry.path();
                    if p.is_file() {
                        if let Some(ext) = p.extension() {
                            let ext_lower = ext.to_str().unwrap_or("").to_lowercase();
                            if exts.contains(&ext_lower.as_str()) {
                                files.push(p);
                            }
                        }
                    }
                }
            }
        } else if item.contains('*') {
            for entry in glob::glob(&item).unwrap() {
                if let Ok(p) = entry {
                    if p.is_file() {
                        if let Some(ext) = p.extension() {
                            let ext_lower = ext.to_str().unwrap_or("").to_lowercase();
                            if exts.contains(&ext_lower.as_str()) {
                                files.push(p);
                            }
                        }
                    }
                }
            }
        }
    }
    if files.is_empty() {
        println!("Не найдено HEIC-файлов.");
        return;
    }
    fs::create_dir_all(output_dir).unwrap();
    let total = files.len();
    println!("Найдено {} HEIC-файлов.", total);
    let (tx, rx) = mpsc::channel();
    let out_dir = output_dir.to_string();
    let threads = 4;
    let mut handles = vec![];
    for chunk in files.chunks((total + threads - 1) / threads) {
        let chunk = chunk.to_vec();
        let tx = tx.clone();
        let format = format.clone();
        let quality = quality;
        let resize = resize.clone();
        let out_dir = out_dir.clone();
        handles.push(thread::spawn(move || {
            for (i, input_path) in chunk.iter().enumerate() {
                let ext = if format == "jpg" || format == "jpeg" { ".jpg" } else { format.clone() };
                let out_name = input_path.file_stem().unwrap().to_str().unwrap().to_string() + &ext;
                let out_path = Path::new(&out_dir).join(out_name);
                if out_path.exists() && !overwrite {
                    tx.send((i+1, format!("{} уже существует, пропуск.", out_path.display()))).unwrap();
                    continue;
                }
                tx.send((i+1, format!("Конвертация {} -> {}", input_path.display(), out_path.display()))).unwrap();
                if let Err(e) = convert_heic(input_path, &out_path, &format, quality, resize) {
                    tx.send((i+1, format!("Ошибка: {}", e))).unwrap();
                }
            }
        }));
    }
    drop(tx);
    let mut count = 0;
    for (idx, msg) in rx {
        count += 1;
        println!("[{}/{}] {}", count, total, msg);
    }
    for h in handles {
        h.join().unwrap();
    }
    println!("Готово!");
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        eprintln!("Использование: {} <HEIC-файлы/папки> [--format jpg|png|webp] [--quality N] [--resize ШxВ] [--output DIR] [--overwrite] [--recursive]", args[0]);
        std::process::exit(1);
    }
    let mut format = "jpg".to_string();
    let mut quality = 85;
    let mut resize = None;
    let mut output_dir = ".".to_string();
    let mut overwrite = false;
    let mut recursive = false;
    let mut inputs = Vec::new();
    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "--format" => {
                if i+1 < args.len() {
                    format = args[i+1].clone();
                    i += 2;
                } else { i += 1; }
            }
            "--quality" => {
                if i+1 < args.len() {
                    quality = args[i+1].parse().unwrap_or(85);
                    i += 2;
                } else { i += 1; }
            }
            "--resize" => {
                if i+1 < args.len() {
                    let s = &args[i+1];
                    let parts: Vec<&str> = s.split('x').collect();
                    if parts.len() == 2 {
                        let w = parts[0].parse().unwrap_or(0);
                        let h = parts[1].parse().unwrap_or(0);
                        if w > 0 && h > 0 {
                            resize = Some((w, h));
                        }
                    }
                    i += 2;
                } else { i += 1; }
            }
            "--output" => {
                if i+1 < args.len() {
                    output_dir = args[i+1].clone();
                    i += 2;
                } else { i += 1; }
            }
            "--overwrite" => {
                overwrite = true;
                i += 1;
            }
            "--recursive" => {
                recursive = true;
                i += 1;
            }
            _ => {
                inputs.push(args[i].clone());
                i += 1;
            }
        }
    }
    process_files(inputs, format, quality, resize, &output_dir, overwrite, recursive);
}

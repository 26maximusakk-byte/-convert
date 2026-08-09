# convert.py
import argparse
import os
import sys
from pathlib import Path
from PIL import Image
import pillow_heif

# Регистрация HEIC
pillow_heif.register_heif_opener()

def convert_heic(input_path, output_path, format='jpg', quality=85, resize=None):
    """Конвертирует HEIC в указанный формат."""
    try:
        img = Image.open(input_path)
        # Изменение размера
        if resize:
            width, height = resize
            img.thumbnail((width, height), Image.Resampling.LANCZOS)
        # Сохранение
        if format.lower() == 'jpg' or format.lower() == 'jpeg':
            img.save(output_path, 'JPEG', quality=quality, optimize=True)
        elif format.lower() == 'png':
            img.save(output_path, 'PNG', quality=quality, optimize=True)
        elif format.lower() == 'webp':
            img.save(output_path, 'WEBP', quality=quality, optimize=True)
        else:
            raise ValueError(f"Неподдерживаемый формат: {format}")
        return True
    except Exception as e:
        print(f"Ошибка при обработке {input_path}: {e}", file=sys.stderr)
        return False

def process_files(inputs, format, quality, resize, output_dir, overwrite, recursive):
    files = []
    extensions = ('.heic', '.heif')
    for item in inputs:
        path = Path(item)
        if path.is_file() and path.suffix.lower() in extensions:
            files.append(path)
        elif path.is_dir():
            if recursive:
                for p in path.rglob('*'):
                    if p.is_file() and p.suffix.lower() in extensions:
                        files.append(p)
            else:
                for p in path.glob('*'):
                    if p.is_file() and p.suffix.lower() in extensions:
                        files.append(p)
        elif '*' in item:
            import glob
            for p in glob.glob(item):
                if Path(p).suffix.lower() in extensions:
                    files.append(Path(p))
    if not files:
        print("Не найдено HEIC-файлов.")
        return
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    total = len(files)
    print(f"Найдено {total} HEIC-файлов.")
    for i, input_file in enumerate(files, 1):
        ext = f'.{format.lower()}'
        out_name = input_file.stem + ('.jpg' if format.lower() in ('jpg','jpeg') else ext)
        out_file = output_path / out_name
        if out_file.exists() and not overwrite:
            print(f"[{i}/{total}] {out_file} уже существует, пропуск.")
            continue
        print(f"[{i}/{total}] Конвертация {input_file} -> {out_file}")
        success = convert_heic(input_file, out_file, format, quality, resize)
        if not success:
            print(f"  Ошибка при конвертации {input_file}")
    print("Готово!")

def parse_resize(s):
    parts = s.split('x')
    if len(parts) != 2:
        raise argparse.ArgumentTypeError("Формат: ШИРИНАxВЫСОТА")
    return int(parts[0]), int(parts[1])

def main():
    parser = argparse.ArgumentParser(description='Конвертер HEIC → JPG/PNG/WEBP')
    parser.add_argument('inputs', nargs='+', help='HEIC-файлы, папки или маски')
    parser.add_argument('--format', default='jpg', choices=['jpg','jpeg','png','webp'], help='Выходной формат')
    parser.add_argument('--quality', type=int, default=85, help='Качество (1-100)')
    parser.add_argument('--resize', type=parse_resize, help='Изменение размера (ШxВ)')
    parser.add_argument('--output', '-o', default='.', help='Папка для сохранения')
    parser.add_argument('--overwrite', action='store_true', help='Перезаписывать существующие файлы')
    parser.add_argument('--recursive', action='store_true', help='Рекурсивный обход папок')
    args = parser.parse_args()

    process_files(args.inputs, args.format, args.quality, args.resize,
                  args.output, args.overwrite, args.recursive)

if __name__ == '__main__':
    main()

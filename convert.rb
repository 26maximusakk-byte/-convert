# convert.rb
require 'mini_magick'
require 'find'
require 'optparse'

def convert_heic(input_path, output_path, format, quality, resize_w, resize_h)
  image = MiniMagick::Image.open(input_path)
  if resize_w > 0 && resize_h > 0
    image.resize "#{resize_w}x#{resize_h}"
  end
  image.format format
  image.quality quality if %w[jpg jpeg webp].include?(format)
  image.write output_path
end

def process_files(inputs, format, quality, resize_w, resize_h, output_dir, overwrite, recursive)
  files = []
  exts = ['.heic', '.heif']
  inputs.each do |item|
    if File.file?(item) && exts.include?(File.extname(item).downcase)
      files << item
    elsif File.directory?(item)
      if recursive
        Find.find(item) do |path|
          files << path if File.file?(path) && exts.include?(File.extname(path).downcase)
        end
      else
        Dir.glob(File.join(item, '*')).each do |path|
          files << path if File.file?(path) && exts.include?(File.extname(path).downcase)
        end
      end
    elsif item.include?('*')
      Dir.glob(item).each { |f| files << f if File.file?(f) && exts.include?(File.extname(f).downcase) }
    end
  end
  if files.empty?
    puts "Не найдено HEIC-файлов."
    return
  end
  Dir.mkdir(output_dir) unless Dir.exist?(output_dir)
  total = files.size
  puts "Найдено #{total} HEIC-файлов."
  files.each_with_index do |input_file, idx|
    ext = format == 'jpg' ? '.jpg' : ".#{format}"
    out_name = File.basename(input_file, '.*') + ext
    out_path = File.join(output_dir, out_name)
    if File.exist?(out_path) && !overwrite
      puts "[#{idx+1}/#{total}] #{out_path} уже существует, пропуск."
      next
    end
    puts "[#{idx+1}/#{total}] Конвертация #{input_file} -> #{out_path}"
    begin
      convert_heic(input_file, out_path, format, quality, resize_w, resize_h)
    rescue => e
      puts "  Ошибка при конвертации #{input_file}: #{e.message}"
    end
  end
  puts "Готово!"
end

options = {}
OptionParser.new do |opts|
  opts.banner = "Использование: ruby convert.rb <HEIC-файлы/папки> [опции]"
  opts.on("--format FORMAT", "Выходной формат (jpg, png, webp)") { |v| options[:format] = v }
  opts.on("--quality N", Integer, "Качество (1-100)") { |v| options[:quality] = v }
  opts.on("--resize ШxВ", String, "Изменение размера") { |v| options[:resize] = v }
  opts.on("--output DIR", String, "Папка для сохранения") { |v| options[:output] = v }
  opts.on("--overwrite", "Перезаписывать") { options[:overwrite] = true }
  opts.on("--recursive", "Рекурсивный обход") { options[:recursive] = true }
end.parse!

format = options[:format] || 'jpg'
quality = options[:quality] || 85
resize = options[:resize] ? options[:resize].split('x').map(&:to_i) : [0,0]
output_dir = options[:output] || '.'
overwrite = options[:overwrite] || false
recursive = options[:recursive] || false
inputs = ARGV

if inputs.empty?
  puts "Не указаны файлы или папки."
  exit
end

process_files(inputs, format, quality, resize[0], resize[1], output_dir, overwrite, recursive)

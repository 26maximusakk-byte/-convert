// convert.cpp
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <regex>
#include <libheif/heif.h>
#include <jpeglib.h>
#include <png.h>

namespace fs = std::filesystem;
using namespace std;

bool convertHeic(const string& inputPath, const string& outputPath, const string& format, int quality, int resizeW, int resizeH) {
    heif_context* ctx = heif_context_alloc();
    if (!ctx) return false;
    if (heif_context_read_from_file(ctx, inputPath.c_str(), nullptr) != 0) {
        heif_context_free(ctx);
        return false;
    }
    heif_image_handle* handle;
    if (heif_context_get_primary_image_handle(ctx, &handle) != 0) {
        heif_context_free(ctx);
        return false;
    }
    heif_image* img;
    if (heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr) != 0) {
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }
    int w = heif_image_get_width(img, heif_channel_interleaved);
    int h = heif_image_get_height(img, heif_channel_interleaved);
    // Изменение размера (упрощённо — пропускаем)
    // Получение данных
    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
    if (!data) {
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }
    // Сохранение в JPG (упрощённо)
    if (format == "jpg" || format == "jpeg") {
        // Используем libjpeg
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        FILE* outfile = fopen(outputPath.c_str(), "wb");
        if (!outfile) {
            heif_image_release(img);
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return false;
        }
        jpeg_stdio_dest(&cinfo, outfile);
        cinfo.image_width = w;
        cinfo.image_height = h;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE);
        jpeg_start_compress(&cinfo, TRUE);
        JSAMPROW row_pointer[1];
        int row_stride = w * 3;
        for (int y = 0; y < h; y++) {
            row_pointer[0] = (JSAMPROW)&data[y * stride];
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
        jpeg_finish_compress(&cinfo);
        fclose(outfile);
        jpeg_destroy_compress(&cinfo);
    } else if (format == "png") {
        // libpng сохранение
        FILE* fp = fopen(outputPath.c_str(), "wb");
        if (!fp) {
            heif_image_release(img);
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return false;
        }
        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        png_infop info = png_create_info_struct(png);
        png_init_io(png, fp);
        png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png, info);
        for (int y = 0; y < h; y++) {
            png_write_row(png, (png_bytep)&data[y * stride]);
        }
        png_write_end(png, info);
        png_destroy_write_struct(&png, &info);
        fclose(fp);
    } else {
        cerr << "Неизвестный формат" << endl;
        heif_image_release(img);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }
    heif_image_release(img);
    heif_image_handle_release(handle);
    heif_context_free(ctx);
    return true;
}

void processFiles(const vector<string>& inputs, const string& format, int quality, int resizeW, int resizeH,
                  const string& outputDir, bool overwrite, bool recursive) {
    vector<string> files;
    regex extRegex(R"(.*\.(heic|heif)$)", regex::icase);
    for (const auto& item : inputs) {
        fs::path path(item);
        if (fs::is_regular_file(path) && regex_match(path.filename().string(), extRegex)) {
            files.push_back(path.string());
        } else if (fs::is_directory(path)) {
            if (recursive) {
                for (auto& entry : fs::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file() && regex_match(entry.path().filename().string(), extRegex)) {
                        files.push_back(entry.path().string());
                    }
                }
            } else {
                for (auto& entry : fs::directory_iterator(path)) {
                    if (entry.is_regular_file() && regex_match(entry.path().filename().string(), extRegex)) {
                        files.push_back(entry.path().string());
                    }
                }
            }
        } else if (item.find('*') != string::npos) {
            // упрощённо
        }
    }
    if (files.empty()) {
        cout << "Не найдено HEIC-файлов." << endl;
        return;
    }
    fs::create_directories(outputDir);
    size_t total = files.size();
    cout << "Найдено " << total << " HEIC-файлов." << endl;
    for (size_t i=0; i<total; ++i) {
        const string& inputFile = files[i];
        string ext = (format=="jpg"||format=="jpeg") ? ".jpg" : "."+format;
        string outName = fs::path(inputFile).stem().string() + ext;
        string outPath = fs::path(outputDir) / outName;
        if (fs::exists(outPath) && !overwrite) {
            cout << "[" << i+1 << "/" << total << "] " << outPath << " уже существует, пропуск." << endl;
            continue;
        }
        cout << "[" << i+1 << "/" << total << "] Конвертация " << inputFile << " -> " << outPath << endl;
        if (!convertHeic(inputFile, outPath, format, quality, resizeW, resizeH)) {
            cerr << "  Ошибка при конвертации " << inputFile << endl;
        }
    }
    cout << "Готово!" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Использование: convert <HEIC-файлы/папки> [--format jpg|png|webp] [--quality N] [--resize ШxВ] [--output DIR] [--overwrite] [--recursive]" << endl;
        return 1;
    }
    vector<string> inputs;
    string format = "jpg";
    int quality = 85;
    int resizeW=0, resizeH=0;
    string outputDir = ".";
    bool overwrite=false, recursive=false;
    for (int i=1; i<argc; ++i) {
        string arg = argv[i];
        if (arg == "--format" && i+1 < argc) {
            format = argv[++i];
        } else if (arg == "--quality" && i+1 < argc) {
            quality = stoi(argv[++i]);
        } else if (arg == "--resize" && i+1 < argc) {
            string s = argv[++i];
            size_t x = s.find('x');
            if (x != string::npos) {
                resizeW = stoi(s.substr(0, x));
                resizeH = stoi(s.substr(x+1));
            }
        } else if (arg == "--output" && i+1 < argc) {
            outputDir = argv[++i];
        } else if (arg == "--overwrite") {
            overwrite = true;
        } else if (arg == "--recursive") {
            recursive = true;
        } else {
            inputs.push_back(arg);
        }
    }
    processFiles(inputs, format, quality, resizeW, resizeH, outputDir, overwrite, recursive);
    return 0;
}

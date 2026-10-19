#include <iostream>
#include <asio.hpp>
#include <filesystem>
#include <array>
#include "FileManager.h"

using namespace std::filesystem;
using namespace std;

FileManagerBK fm;
std::vector<std::string> ends_check = { "*.jpg", "*.png" };
bool stop = false;
int max_w = 40;
double aspect_ratio = 2.5;

#include <fstream>
#include <vector>
#include <cstdint>
#include <string>

// Возвращает {width, height} или {0,0} при ошибке
std::pair<int,int> get_image_size(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {0,0};

    unsigned char sig[8];
    f.read(reinterpret_cast<char*>(sig), 8);

    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (sig[0]==0x89 && sig[1]=='P' && sig[2]=='N' && sig[3]=='G') {
        f.seekg(16, std::ios::beg);  // IHDR: width(4) height(4), big-endian
        unsigned char buf[8];
        f.read(reinterpret_cast<char*>(buf), 8);
        uint32_t w = (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
        uint32_t h = (buf[4]<<24)|(buf[5]<<16)|(buf[6]<<8)|buf[7];
        return { (int)w, (int)h };
    }

    // JPEG: FF D8 ... идём по маркерам
    if (sig[0]==0xFF && sig[1]==0xD8) {
        f.seekg(2, std::ios::beg);
        while (true) {
            unsigned char b;
            f.read(reinterpret_cast<char*>(&b), 1);
            if (!f) return {0,0};
            if (b != 0xFF) continue;  // ищем маркер

            unsigned char marker;
            do {
                f.read(reinterpret_cast<char*>(&marker), 1);
                if (!f) return {0,0};
            } while (marker == 0xFF);  // пропускаем fill-байты

            // SOF0..SOF15, кроме DHT(C4), JPG(C8), DAC(CC)
            if (marker >= 0xC0 && marker <= 0xCF &&
                marker != 0xC4 && marker != 0xC8 && marker != 0xCC) {
                unsigned char len[2];
                f.read(reinterpret_cast<char*>(len), 2);
                unsigned char data[5];
                f.read(reinterpret_cast<char*>(data), 5);
                // data: precision(1), height(2), width(2)
                int h = (data[1]<<8)|data[2];
                int w = (data[3]<<8)|data[4];
                return { w, h };
            } else {
                // пропускаем сегмент: длина(2) + данные
                unsigned char len[2];
                f.read(reinterpret_cast<char*>(len), 2);
                int seg_len = (len[0]<<8)|len[1];
                f.seekg(seg_len - 2, std::ios::cur);
            }
        }
    }

    return {0,0};  // не PNG и не JPEG
}

template<typename T>
bool can_convert(const std::string& str) {
    std::istringstream iss(str);
    T value;
    return (iss >> value) && (iss.eof());
}

bool is_int(const std::string& str) {
    return can_convert<int>(str);
}

bool is_float(const std::string& str) {
    return can_convert<float>(str);
}

bool is_double(const std::string& str) {
    return can_convert<double>(str);
}

bool is_long(const std::string& str) {
    return can_convert<long>(str);
}

void asking();

int main(){

    setlocale(LC_ALL, "Ru");
    asking();

    return 0;
}

void asking() {
    asio::io_context ioc;

    asio::ip::tcp::socket sock(ioc);
    sock.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), 8080));

    while (!stop) {
        cout << "Конвертировать [1]\nИзменить настройки [2]\nЗакрыть [3]\nВыбор команды: ";
        string com;
        getline(cin, com);
        if (com == "1") {
            cout << "Введите путь к файлу: ";
            string path_;
            getline(cin, path_);
            string repath_ = "";
            for (int i = 0; i < path_.length(); ++i) {
                if (path_[i] != '\"') repath_ += path_[i];
            }
            path_ = repath_;

            if (exists(path_) && fm.checker(path_, ends_check)) {
                //Запрос
                // 1. Размеры оригинала
                auto [wm, hm] = get_image_size(path_);
                if (wm == 0 || hm == 0) {
                    cout << "Не удалось определить размеры картинки.\n\n";
                    continue;
                }

                // 2. Размеры арта — та же логика, что в resize(), но локально
                int w, h;
                if (wm <= max_w) {
                    w = wm;
                    h = hm;
                } else {
                    double scale = (double)max_w / wm;
                    h = static_cast<int>(hm * scale);
                    if (h < 1) h = 1;
                    w = max_w;
                }
                // применяем соотношение сторон (вытянутость)
                h = static_cast<int>(h * aspect_ratio);
                if (h < 1) h = 1;

                // 3. Читаем картинку в бинарном режиме
                std::ifstream file(path_, std::ios::binary | std::ios::ate);
                if (!file) {
                    cout << "Не удалось открыть файл.\n\n";
                    continue;
                }
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<char> img(size);
                file.read(img.data(), size);

                // 4. Текстовая шапка: wm hm w h + \n
                std::string meta = std::to_string(wm) + " " +
                                std::to_string(hm) + " " +
                                std::to_string(w)  + " " +
                                std::to_string(h)  + "\n";

                // 5. Заголовки
                size_t body_size = meta.size() + img.size();
                std::string headers;
                headers += "POST /api/pngtoascii HTTP/1.1\r\n";
                headers += "Host: 127.0.0.1:8080\r\n";
                headers += "Content-Type: application/octet-stream\r\n";
                headers += "Content-Length: " + std::to_string(body_size) + "\r\n";
                headers += "Connection: close\r\n\r\n";

                // 6. Отправка: заголовки + meta + байты картинки
                std::array<asio::const_buffer, 3> bufs = {
                    asio::buffer(headers),
                    asio::buffer(meta),
                    asio::buffer(img)
                };
                asio::write(sock, bufs);

                // 7. Чтение ответа до EOF (Connection: close)
                asio::error_code ec;
                std::string response;
                char buf[4096];
                while (true) {
                    size_t n = sock.read_some(asio::buffer(buf), ec);
                    if (ec == asio::error::eof || ec == asio::error::connection_reset) break;
                    if (ec) { cout << "Ошибка чтения: " << ec.message() << "\n"; break; }
                    response.append(buf, n);
                }

                // 8. Тело ответа — после \r\n\r\n
                auto pos = response.find("\r\n\r\n");
                if (pos != std::string::npos) {
                    cout << "\n=== ASCII-арт ===\n" << response.substr(pos + 4) << "\n================\n\n";
                } else {
                    cout << "Некорректный ответ от сервера.\n\n";
                }
                //converted(path_);
            }
            else {
                cout << "Такого пути не существует или разрешение файла не соответствует доступным (jpg, png).\n\n";
            }
        }
        else if (com == "2"){
            cout << "Задать максимальную ширину Арта (Сейчас - " << max_w << " ) [1]\nЗадать соотношение сторон (Сейчас - " << aspect_ratio << ") [2]\nОтмена [любой символ]\nВвод: ";
            string choice;
            getline(cin, choice);
            if (choice == "1") {
                cout << "Введите максимальную ширину Арта (целое число, рекомендуемо от 40 до 400): ";
                getline(cin, choice);
                if (is_int(choice)) {
                    if (stoi(choice) < 1) max_w = 1;
                    else if (stoi(choice) > 750) max_w = 750;
                    else max_w = stoi(choice);
                    cout << "Настройки сохранены. \n\n";
                }
                else {
                    cout << "Введено недопустимое значение!\n\n";
                }
            }
            else if (choice == "2") {
                cout << "Введите соотношение сторон - вытянутость арта (десятичное число, рекомендуемо 2.5): ";
                getline(cin, choice);
                if (is_double(choice)) {
                    if (stod(choice) < 0.5) aspect_ratio = 0.5;
                    else if (stod(choice) > 5) aspect_ratio = 5;
                    else aspect_ratio = stod(choice);
                    cout << "Настройки сохранены. \n\n";
                }
                else {
                    cout << "Введено недопустимое значение!\n\n";
                }
            }
            else {
                cout << "Отмена.\n\n";
            }
        }
        else if (com == "3") {
            stop = true;
            cout << "Работа завершена.\n";
        }
        else {
            cout << "Такой команды нет!\n\n";
        }
    }
}

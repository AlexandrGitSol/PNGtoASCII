#include "ConvertorASCII.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

int ConvertorASCII::gcd(int a, int b) {
    // Обрабатываем отрицательные числа, используя абсолютное значение
    a = std::abs(a);
    b = std::abs(b);

    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

pair<int, int> ConvertorASCII::resize(int width, int height, int max_w) {
    if (width == 0 || height == 0) {
        return { 0, 0 };
    }

    if (width <= max_w) {
        return { width, height };
    }

    double scale_factor = (double)max_w / width;
    int new_height = static_cast<int>(height * scale_factor);

    if (new_height < 1) {
        new_height = 1;
    }

    return { max_w, new_height };
}

unsigned char ConvertorASCII::ptouc(unsigned char r, unsigned char g, unsigned char b) {
    // Коэффициенты восприятия яркости
    const double red_coef = 0.2126;
    const double green_coef = 0.7152;
    const double blue_coef = 0.0722;

    double luminance = red_coef * r + green_coef * g + blue_coef * b;

    const char ascii_chars[] = " .'`^\",:;<|\\/trW%@$";
    const int num_chars = sizeof(ascii_chars) - 1;

    int index = static_cast<int>((luminance / 255.0) * (num_chars - 1));
    index = std::max(0, std::min(index, num_chars - 1));

    return ascii_chars[index];
}

string ConvertorASCII::PNGtoASCII(unsigned char* data, int original_width, int original_height, int width_art, int height_art) {
    std::string result;
    result.reserve(width_art * height_art + height_art); // Резервируем память для эффективности

    // Вычисляем размеры блоков для усреднения
    double block_width = static_cast<double>(original_width) / width_art;
    double block_height = static_cast<double>(original_height) / height_art;

    for (int art_y = 0; art_y < height_art; art_y++) {
        for (int art_x = 0; art_x < width_art; art_x++) {
            // Определяем границы текущего блока в исходном изображении
            int start_x = static_cast<int>(art_x * block_width);
            int start_y = static_cast<int>(art_y * block_height);
            int end_x = static_cast<int>((art_x + 1) * block_width);
            int end_y = static_cast<int>((art_y + 1) * block_height);

            // Гарантируем, что блоки не выходят за границы
            end_x = std::min(end_x, original_width);
            end_y = std::min(end_y, original_height);

            // Переменные для накопления сумм цветов
            unsigned long long sum_r = 0, sum_g = 0, sum_b = 0;
            int pixel_count = 0;

            // Проходим по всем пикселям в текущем блоке
            for (int y = start_y; y < end_y; y++) {
                for (int x = start_x; x < end_x; x++) {
                    // Вычисляем индекс пикселя в массиве data
                    int pixel_index = (y * original_width + x) * 3;

                    // Суммируем компоненты цвета
                    sum_r += data[pixel_index];
                    sum_g += data[pixel_index + 1];
                    sum_b += data[pixel_index + 2];
                    pixel_count++;
                }
            }

            // Вычисляем средние значения цветов
            if (pixel_count > 0) {
                unsigned char avg_r = static_cast<unsigned char>(sum_r / pixel_count);
                unsigned char avg_g = static_cast<unsigned char>(sum_g / pixel_count);
                unsigned char avg_b = static_cast<unsigned char>(sum_b / pixel_count);

                // Преобразуем средний цвет в символ ASCII
                result += ptouc(avg_r, avg_g, avg_b);
            }
            else {
                // Если блок пустой, используем пробел
                result += ' ';
            }
        }

        // Добавляем перевод строки после каждой строки арта
        result += '\n';
    }

    return result;
}

string ConvertorASCII::remake_console(std::string pixels, double aspect_ratio) {
    if (pixels.empty()) return "";

    std::vector<std::string> lines;
    std::stringstream ss(pixels);
    std::string line;

    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    if (lines.empty()) return "";

    int original_width = lines[0].length();
    int new_width = static_cast<int>(std::round(original_width * aspect_ratio));

    std::string result;

    for (const auto& original_line : lines) {
        std::string stretched_line;
        stretched_line.reserve(new_width);

        // Используем линейную интерполяцию для более плавного растяжения
        for (int i = 0; i < new_width; i++) {
            double pos = static_cast<double>(i) / aspect_ratio;
            int original_index = static_cast<int>(pos);

            if (original_index < original_line.length()) {
                stretched_line += original_line[original_index];
            }
            else {
                stretched_line += ' '; // Заполнитель для выравнивания
            }
        }

        result += stretched_line + "\n";
    }

    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    return result;
}

string ConvertorASCII::converted(string path_) {
    const char* filename = path_.c_str();
    int w, h, c;
    unsigned char* data = stbi_load(filename, &w, &h, &c, 0);

    if (!data) {
        cout << "error to load";
        return "error";
    }

    int wm = w, hm = h;

    w = resize(wm, hm, stgs.max_w).first;
    h = resize(wm, hm, stgs.max_w).second;

    cout << remake_console(PNGtoASCII(data, wm, hm, w, h), stgs.aspect_ratio) << endl;
    return remake_console(PNGtoASCII(data, wm, hm, w, h), stgs.aspect_ratio);
}

void ConvertorASCII::asking() {
    while (!stgs.stop) {
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

            if (exists(path_) && fm.checker(path_, ends)) {
                converted(path_);
            }
            else {
                cout << "Такого пути не существует или разрешение файла не соответствует доступным (jpg, png).\n\n";
            }
        }
        else if (com == "2"){
            cout << "Задать максимальную ширину Арта (Сейчас - " << stgs.max_w << " ) [1]\nЗадать соотношение сторон (Сейчас - " << stgs.aspect_ratio <<") [2]\nОтмена [любой символ]\nВвод: ";
            string choice;
            getline(cin, choice);
            if (choice == "1") {
                cout << "Введите максимальную ширину Арта (целое число, рекомендуемо от 40 до 400): ";
                getline(cin, choice);
                if (is_int(choice)) {
                    if (stoi(choice) < 1) stgs.max_w = 1;
                    else if (stoi(choice) > 750) stgs.max_w = 750;
                    else stgs.max_w = stoi(choice);
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
                    if (stod(choice) < 0.5) stgs.aspect_ratio = 0.5;
                    else if (stod(choice) > 5) stgs.aspect_ratio = 5;
                    else stgs.aspect_ratio = stod(choice);
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
            stgs.stop = true;
            cout << "Работа завершена.\n";
        }
        else {
            cout << "Такой команды нет!\n\n";
        }
    }
}
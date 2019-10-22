#include <util/FileUtil.h>
#include <dirent.h>
#include <cstring>
#include <fstream>

void kd::dc::FileUtil::getFileNames(const string path, vector<string> &filenames, const string suffix) {
    DIR *pDir;
    struct dirent *ptr;
    if (!(pDir = opendir(path.c_str())))
        return;
    while ((ptr = readdir(pDir)) != 0) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            std::string file = path + "/" + ptr->d_name;
            DIR *dir = opendir(file.c_str());
            if (dir) {
                getFileNames(file, filenames, suffix);
                closedir(dir);
            } else {
                if (suffix == file.substr(file.size() - suffix.size())) {
                    filenames.push_back(file);
                }
            }
        }
    }
    closedir(pDir);
}

bool kd::dc::FileUtil::LoadFile(string file_path, string &file_content) {
    file_content = "";

    fstream fp(file_path);
    string str;
    while (getline(fp, str))   //按行读取,遇到换行符结束
    {
        file_content += str;
    }
    fp.close();

    if (file_content.length() == 0) {
        return false;
    }
    return true;
}

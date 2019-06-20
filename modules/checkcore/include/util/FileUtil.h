//
// Created by zhangxingang on 19-6-13.
//

#ifndef AUTOHDMAP_DATACHECK_FILEUTIL_H
#define AUTOHDMAP_DATACHECK_FILEUTIL_H

#include <string>
#include <vector>

using namespace std;

namespace kd {
    namespace dc {
        class FileUtil {
        public:
            /**
             * 统计目录中的所有扩展名为suffix的文件
             * @param path
             * @param filenames
             * @param suffix
             */
            static void getFileNames(const string path, vector <string> &filenames, const string suffix = "");

            static bool LoadFile(string file_path, string &file_content);
        };
    }
}


#endif //AUTOHDMAP_DATACHECK_FILEUTIL_H

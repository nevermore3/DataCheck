
#ifndef AUTOHDMAP_DATACHECK_STRINGUTIL_H
#define AUTOHDMAP_DATACHECK_STRINGUTIL_H

#include <set>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>

class StringUtil {
public:
    static void Replace(std::string &str, const std::string &src, const std::string &dst);

    // Add tokenized string by delimiter to vector.
    static bool Token(const char *string, const char *delimiter, std::vector<std::string> &vec);

    // Add tokenized string by delimiter to set.
    static bool Token(const char *string, const char *delimiter, std::set<std::string> &set);

    static bool Token(const std::string &strTarget, const char *delimiter, const int &limitedNum,
                      std::vector<std::string> &vecToken);

    static bool Token_IgnoreQuotes(const char *string, const char *delimiter, std::vector<std::string> &vec);

    // Convert string to uppercase.
    static std::string Upper(const std::string &str);

    // Convert string to lowercase.
    static std::string Lower(const std::string &str);

    // Trim specified character from the string.
    static std::string Trim(const std::string &str, const int &trimChr);

    // Trim specified character array from the string.
    static std::string Trim(const std::string &str, const std::vector<int> &vecTrimChr);

    // Trim left and right space char
    static std::string &TrimFRSpace(std::string &str);

    //Convert string to bool
    static bool str2Bool(const std::string &strTarget);

    //
    static unsigned long atoul(const std::string str);

    static long atol(const std::string str, bool isHex = false);

    static double atod(const std::string str);

    static std::string itoa(const int &inum);

    static std::string ltoa(const long &ln);

    static std::string dtoa(const double &dou);

    static std::string ultoa(const unsigned long &uln);

    static std::string l64toa(const int64_t &uln);

    static bool IsAlphaOrDigit(const char &ch);

    static bool Utf8Str2Vec(const std::string &strUtf8, std::vector<std::string> &vec);

    // Add a space before the uppercase. (First character not eligible.)
    static std::string InsertBlankBeforeCapital(const std::string &str, const int AllowanceNum = 1);

    static std::wstring convMultiByte2WideStr(std::string str);
};

#endif //AUTOHDMAP_DATACHECK_STRINGUTIL_H

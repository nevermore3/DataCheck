//
// Created by zhangxingang on 19-1-15.
//

#include "util/StringUtil.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

void StringUtil::Replace(std::string &str, const std::string &src, const std::string &dst) {
    std::string::size_type pos = 0;
    std::string::size_type srclen = src.size();
    std::string::size_type dstlen = dst.size();

    if (0 == srclen) {
        return;
    }
    while ((pos = str.find(src, pos)) != std::string::npos) {
        str.replace(pos, srclen, dst);
        pos += dstlen;
    }
}

/**
 * Token
 * Add tokenized string by delimiter to vector.
 *
 */
bool StringUtil::Token(const char *string, const char *delimiter, std::vector<std::string> &vec) {
    std::string str;
    size_t head = 0;
    size_t tail = 0;
    size_t size = 0;
    char *buff = NULL;

    // param check.
    if (string == NULL || delimiter == NULL) {
        return false;
    }

    // param check.
    if (string[0] == '\0' || delimiter[0] == '\0') {
        return false;
    }

    // Initialization.
    vec.clear();
    str = string;
    head = 0;
    size = strlen(delimiter);

    buff = new char[strlen(string) + 1];

    while (tail != str.npos) {
        std::string tmp;

        // find delimiter from str.
        tail = str.find(delimiter, head);
        if (tail == str.npos) {
            // case( delimiter no exist ) : add string( from head to NULL terminate) to tmp
            tmp = &str.c_str()[head];
        } else {
            // case( delimiter exist ) : add string( from head to delimiter ) to tmp
            memcpy(buff, &str.c_str()[head], tail - head);
            buff[tail - head] = '\0';
            tmp = buff;
        }
        // add token to vector.
        vec.push_back(tmp);
        head = tail + size;
    }

    if (nullptr != buff) {
        delete[] buff;
    }

    return true;
}

bool StringUtil::Token_IgnoreQuotes(const char *string, const char *delimiter, std::vector<std::string> &vec) {
    std::string str;
    size_t head = 0;
    size_t tail = 0;
    size_t size = 0;
    char *buff = NULL;

    // param check.
    if (string == NULL || delimiter == NULL) {
        return false;
    }

    // param check.
    if (string[0] == '\0' || delimiter[0] == '\0') {
        return false;
    }

    // Initialization.
    str = string;
    head = 0;
    size = strlen(delimiter);

    buff = new char[strlen(string) + 1];

    while (tail != str.npos) {
        std::string tmp;

        // find delimiter from str.
        tail = str.find(delimiter, head);
        if (tail == str.npos) {
            // case( delimiter no exist ) : add string( from head to NULL terminate) to tmp
            tmp = &str.c_str()[head];
        } else {
            // case( delimiter exist ) : add string( from head to delimiter ) to tmp
            //if the last char is not 0x22 that is Quotes("),token.
            if (str.at(tail - 1) != char(0x22)) {
                memcpy(buff, &str.c_str()[head], tail - head);
                buff[tail - head] = '\0';
                tmp = buff;
            } else {
                //if the last char is 0x22 that is Quotes("),continue.
                head = tail + size;
                continue;
            }
        }
        // add token to vector.
        vec.push_back(tmp);
        head = tail + size;
    }

    delete buff;

    return true;
}

bool StringUtil::Token(const std::string &strTarget, const char *delimiter, const int &limitedNum,
                       std::vector<std::string> &vecToken) {
    //	std::string	str;
    size_t head = 0;
    size_t tail = 0;
    size_t sizeDlm = 0;
    size_t limit = 0;
    size_t cntlimit = 0;
    char *buff = NULL;

    // param check.
    if (strTarget.empty() == true || delimiter == NULL || delimiter[0] == '\0') {
        return false;
    }

    // Initialization.
    vecToken.clear();
    head = 0;
    sizeDlm = strlen(delimiter);

    if (limitedNum == 0) {
        // No limit if limitedNum is 0
        limit = strTarget.size();
    } else {
        // Limit number is parameter if limitedNum is not 0.
        limit = limitedNum;
    }

    buff = new char[strTarget.size() + 1];

    while (tail != strTarget.npos) {
        std::string tmp;

        // check limit.
        if (cntlimit >= limit) {
            break;
        }

        // find delimiter from str.
        tail = strTarget.find(delimiter, head);
        if (tail == strTarget.npos) {
            // case( delimiter no exist ) : add string( from head to NULL terminate) to tmp
            tmp = &strTarget.c_str()[head];
        } else {
            // case( delimiter exist ) : add string( from head to delimiter ) to tmp
            memcpy(buff, &strTarget.c_str()[head], tail - head);
            buff[tail - head] = '\0';
            tmp = buff;
        }
        // add token to vector.
        vecToken.push_back(tmp);
        head = tail + sizeDlm;
        cntlimit++;
    }
    delete buff;
    return true;
}

/**
 * Token
 * Add tokenized string by delimiter to set.
 *
 */
bool StringUtil::Token(const char *string, const char *delimiter, std::set<std::string> &set) {
    std::vector<std::string> vec;

    if (Token(string, delimiter, vec) == false) {
        return false;
    }

    std::vector<std::string>::iterator it = vec.begin();
    std::vector<std::string>::iterator end = vec.end();

    while (it != end) {
        set.insert(*it);
        it++;
    }

    return true;
}


/**
 * Upper
 * Convert string to uppercase.
 *
 */
std::string StringUtil::Upper(const std::string &str) {
    std::string strUpper = str;
#ifdef _WIN32
    std::transform( strUpper.begin(), strUpper.end(), strUpper.begin(),  std::toupper );
#else
    std::transform(strUpper.begin(), strUpper.end(), strUpper.begin(),
                   [](unsigned char c) -> unsigned char { return std::toupper(c); });

#endif
    return strUpper;
}

/**
 * Lower
 * Convert string to lowercase.
 *
 */
std::string StringUtil::Lower(const std::string &str) {
    std::string strLower = str;
#ifdef _WIN32
    std::transform( strLower.begin(), strLower.end(), strLower.begin(), std::tolower );
#else
    std::transform(strLower.begin(), strLower.end(), strLower.begin(),
                   [](unsigned char c) -> unsigned char { return std::tolower(c); });
#endif
    return strLower;
}


/**
 * Trim
 * Trim specified character from the string.
 *
 */
std::string StringUtil::Trim(const std::string &str, const int &trimChr) {
    std::string strTrim = str;
    strTrim.erase(std::remove(strTrim.begin(), strTrim.end(), trimChr), strTrim.end());
    return strTrim;
}

/**
 * Trim
 * Trim specified character array from the string.
 *
 */
std::string StringUtil::Trim(const std::string &str, const std::vector<int> &vecTrimChr) {
    std::string strTrim = str;
    std::string::iterator it_strTrim = strTrim.end();
    {
        std::vector<int>::const_iterator it = vecTrimChr.begin();
        std::vector<int>::const_iterator end = vecTrimChr.end();
        while (it != end) {
            it_strTrim = std::remove(strTrim.begin(), it_strTrim, (*it));
            it++;
        }
    }
    strTrim.erase(it_strTrim, strTrim.end());
    return strTrim;
}

std::string &StringUtil::TrimFRSpace(std::string &str) {
    int halfSpace = 0x0020; // half space
    int fullSpace = 0xa1; // full space 0xa1a1

    while ((!str.empty()) &&
           (str.find_first_not_of(halfSpace) != 0
            || (str.find_first_not_of(fullSpace) != 0 && str.find_first_not_of(fullSpace) != 1)
            || str.find_last_not_of(halfSpace) != str.length() - 1
            ||
            (str.find_last_not_of(fullSpace) != str.length() - 1 && str.find_last_not_of(fullSpace) != str.length() - 2)
           )) {
        // erase half space
        str.erase(str.find_last_not_of(halfSpace) + 1);
        str.erase(0, str.find_first_not_of(halfSpace));

        // erase full space
        int pos = 0;
        int len = str.length();
        int offset = 0;
        pos = str.find_last_not_of(fullSpace);
        offset = ((len - pos) % 2 ? 0 : 1);
        str.erase(str.find_last_not_of(fullSpace) + offset + 1);

        pos = str.find_first_not_of(fullSpace);
        offset = (pos % 2) ? 1 : 0;
        str.erase(0, str.find_first_not_of(fullSpace) - offset);
    }
    return str;
}

bool StringUtil::str2Bool(const std::string &strTarget) {
    bool rv = false;
    if (strTarget.empty() == true) {
        rv = false;
    } else {
        std::stringstream sstream;
        sstream << strTarget;
        sstream >> rv;
        if (sstream.fail() == true) {
            rv = false;
        }
    }
    return rv;
}

unsigned long StringUtil::atoul(const std::string str) {
    std::stringstream ss;
    unsigned long ul;
    ss << str;
    ss >> ul;
    return ul;
}

long StringUtil::atol(const std::string str, bool isHex /*= false*/) {
    std::stringstream ss;
    long l;
    if (isHex) {
        ss << std::hex << str;
    } else {
        ss << str;
    }
    ss >> l;
    return l;
}


std::string StringUtil::ultoa(const unsigned long &uln) {
    std::stringstream ss;
    std::string str;
    ss << uln;
    ss >> str;
    return str;
}

std::string StringUtil::itoa(const int &inum) {
    std::stringstream ss;
    std::string str;
    ss << inum;
    ss >> str;
    return str;
}

std::string StringUtil::ltoa(const long &ln) {
    std::stringstream ss;
    std::string str;
    ss << ln;
    ss >> str;
    return str;
}

std::string StringUtil::l64toa(const int64_t &l64) {
    std::stringstream ss;
    std::string str;
    ss << l64;
    ss >> str;
    return str;
}


bool StringUtil::IsAlphaOrDigit(const char &ch) {
    if (ch >= 0 && ch <= 255)
        return true;
    return false;
}

bool StringUtil::Utf8Str2Vec(
        const std::string &strUtf8,
        std::vector<std::string> &vec) {
    unsigned int numBytes = strUtf8.size();

    for (unsigned int cntByte = 0; cntByte < numBytes; cntByte++) {
        char chr = strUtf8.at(cntByte);
        unsigned char posZero = 0;
        unsigned char numBytesOfChr = 0;

        // Explore the 0 from the upper bits of a byte.
        for (int cntBit = 7; cntBit >= 0; cntBit--) {
            // Record the bit position if bit = 0. and break.
            if (((chr >> cntBit) & 0x01) == 0) {
                posZero = cntBit;
                break;
            }
        }
        // Byte number is 1 if bit7 = 0
        if (posZero == 7) {
            numBytesOfChr = 1;
        } else if (posZero == 6)    // This byte is 0 if bit6 = 0
        {
            numBytesOfChr = 0;
        } else if (posZero <= 0) {
            numBytesOfChr = 0xFF;
        } else    // 1,2,3,4,5
        {
            numBytesOfChr = 7 - posZero;
        }

        unsigned int headCharacter = cntByte;
        unsigned int tailCharacter = cntByte + numBytesOfChr;
        std::string strCharacter;

        while (headCharacter != tailCharacter) {
            strCharacter += strUtf8.at(headCharacter);
            headCharacter++;
        }
        vec.push_back(strCharacter);
        cntByte += numBytesOfChr - 1;
    }

    return true;
}


/**
 * InsertBlankBeforeCapital
 * Add a space before the uppercase. (First character not eligible.)
 *
 * @param[in]	str
 * @param[in]	AllowanceNum	Set to the number of characters that should not be separated by a space .
 *
 * @return	std::string
 * @retval
 * @note
 */
std::string StringUtil::InsertBlankBeforeCapital(const std::string &str, const int AllowanceNum) {
    std::string convStr;
    int strSize = str.size();
    int iUppercaseNum = (AllowanceNum <= 1 ? 1 : AllowanceNum);
    unsigned int basePos = 0;
    unsigned int nextPos = 0;
    bool isFirstUpperStr = false;

    while (basePos < strSize) {
        basePos = nextPos;
        std::string setStr;
        setStr.clear();
        setStr += str[nextPos];

        // Split a string uppercase. //
        if (basePos == 0) {
            if (('A' <= str[basePos]) && (str[basePos] <= 'Z')) {
                isFirstUpperStr = true;
            }
        } else {
            if (('A' <= str[basePos]) && (str[basePos] <= 'Z')) {
                int CntUpper = 1;        // Continuous uppercase number
                int changePos = 0;        // Change position

                // if the first character is uppercase , adds to the number of consecutive.
                if ((basePos == 1) && (isFirstUpperStr == true)) {
                    CntUpper++;
                }

                // Consider the continuous allowable number of uppercase. //
                for (int checkUpperPos = 1; checkUpperPos <= strSize; checkUpperPos++) {
                    if ((basePos + checkUpperPos) < strSize) {
                        if (('A' <= str[basePos + checkUpperPos]) && (str[basePos + checkUpperPos] <= 'Z')) {
                            CntUpper++;
                            changePos = basePos + checkUpperPos;
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }

                // Insert blank //
                if (str[basePos - 1] != ' ') {
                    // Continuous number of characters is the first case.
                    // Case of less than the number of characters allowed.
                    if ((iUppercaseNum == 1) || (CntUpper < iUppercaseNum)) {
                        convStr += ' ';
                    } else {
                        if (('A' > str[basePos - 1]) || (str[basePos - 1] > 'Z')) {
                            convStr += ' ';
                        }
                        // Concatenation of string that is not in capital letters. //
                        for (int i = basePos + 1; i <= changePos; i++) {
                            setStr += str[i];
                            nextPos++;
                        }
                    }
                }
            }
        }

        convStr += setStr;
        nextPos++;
    }
    return convStr;
}

double StringUtil::atod(const std::string str) {
    std::stringstream ss;
    double doubleVal = 0.0;
    ss << str;
    ss >> doubleVal;
    return doubleVal;
}

std::string StringUtil::dtoa(const double &dou) {
    std::stringstream ss;
    std::string str;
    ss << dou;
    ss >> str;
    return str;
}

std::wstring StringUtil::convMultiByte2WideStr(std::string str) {
#ifdef _WIN32
    WCHAR wszName[MAX_PATH] = {0};
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wszName, MAX_PATH);
    return (wszName);
#else
    return std::wstring();
#endif
}

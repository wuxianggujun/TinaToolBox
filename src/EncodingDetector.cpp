#include "EncodingDetector.hpp"
#include <spdlog/spdlog.h>
#include <QTextCodec>

namespace TinaToolBox {
    QString EncodingDetector::detect(const QByteArray &data) {
        // 1. 首先检查 BOM
        if (data.size() >= 3) {
            if (data.startsWith("\xEF\xBB\xBF")) {
                spdlog::debug("Detected UTF-8 with BOM");
                return "UTF-8";
            }
            if (data.startsWith("\xFE\xFF")) {
                spdlog::debug("Detected UTF-16BE");
                return "UTF-16BE";
            }
            if (data.startsWith("\xFF\xFE")) {
                spdlog::debug("Detected UTF-16LE");
                return "UTF-16LE";
            }
        }

        // 2. 尝试各种编码并评分
        struct EncodingScore {
            QString encoding;
            int score;
        };

        QList<EncodingScore> scores;

        // 测试常见编码
        QStringList encodings = {"UTF-8", "GBK", "GB18030", "GB2312", "Big5"};

        for (const QString &encoding: encodings) {
            QTextCodec *codec = QTextCodec::codecForName(encoding.toLatin1());
            if (!codec) continue;

            QTextCodec::ConverterState state;
            QString text = codec->toUnicode(data.constData(), data.size(), &state);

            int score = 0;

            // 检查转换是否有效
            if (state.invalidChars == 0) {
                score += 60; // 基础分：可以完整转换

                // 检查中文字符的比例
                int chineseCount = 0;
                for (const QChar &ch: text) {
                    if (ch.unicode() >= 0x4E00 && ch.unicode() <= 0x9FFF) {
                        chineseCount++;
                    }
                }

                // 计算中文字符比例得分
                double chineseRatio = static_cast<double>(chineseCount) / text.length();
                score += static_cast<int>(chineseRatio * 40); // 最高40分

                // 检查是否有乱码特征（如大量特殊字符）
                int specialCharCount = 0;
                for (const QChar &ch: text) {
                    if (ch.unicode() < 0x20 || (ch.unicode() > 0x7E && ch.unicode() < 0x4E00)) {
                        specialCharCount++;
                    }
                }

                // 特殊字符比例越高，扣分越多
                double specialRatio = static_cast<double>(specialCharCount) / text.length();
                score -= static_cast<int>(specialRatio * 50);
            }

            scores.append({encoding, score});
            spdlog::debug("Encoding {} score: {}", encoding.toStdString(), score);
        }

        // 3. 选择得分最高的编码
        QString bestEncoding = "UTF-8"; // 默认UTF-8
        int bestScore = -1;

        for (const auto &score: scores) {
            if (score.score > bestScore) {
                bestScore = score.score;
                bestEncoding = score.encoding;
            }
        }

        // 如果最高分太低，可能是纯ASCII文本
        if (bestScore < 30) {
            bool isAscii = true;
            for (char c: data) {
                if (static_cast<unsigned char>(c) > 0x7F) {
                    isAscii = false;
                    break;
                }
            }
            if (isAscii) {
                spdlog::debug("Detected ASCII text");
                return "ASCII";
            }
        }

        spdlog::debug("Best encoding detected: {} with score {}",
                      bestEncoding.toStdString(), bestScore);
        return bestEncoding;
    }

    bool EncodingDetector::isValidUtf8(const QByteArray &data) {
        int i = 0;
        while (i < data.size()) {
            if ((data[i] & 0x80) == 0) {
                // ASCII
                i += 1;
            } else if ((data[i] & 0xE0) == 0xC0) {
                // 2字节序列
                if (i + 1 >= data.size()) return false;
                if ((data[i + 1] & 0xC0) != 0x80) return false;
                i += 2;
            } else if ((data[i] & 0xF0) == 0xE0) {
                // 3字节序列
                if (i + 2 >= data.size()) return false;
                if ((data[i + 1] & 0xC0) != 0x80 ||
                    (data[i + 2] & 0xC0) != 0x80)
                    return false;
                i += 3;
            } else if ((data[i] & 0xF8) == 0xF0) {
                // 4字节序列
                if (i + 3 >= data.size()) return false;
                if ((data[i + 1] & 0xC0) != 0x80 ||
                    (data[i + 2] & 0xC0) != 0x80 ||
                    (data[i + 3] & 0xC0) != 0x80)
                    return false;
                i += 4;
            } else {
                return false;
            }
        }
        return true;
    }

    bool EncodingDetector::hasChineseChars(const QByteArray &data) {
        for (int i = 0; i < data.size() - 1; i++) {
            unsigned char c = static_cast<unsigned char>(data[i]);
            if (c > 0x7F) return true;
        }
        return false;
    }
}

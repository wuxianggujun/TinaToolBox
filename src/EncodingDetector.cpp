// EncodingDetector.cpp
#include "EncodingDetector.hpp"
#include <spdlog/spdlog.h>
#include <QTextCodec>
#include <utf8.h>

namespace TinaToolBox {
    QString EncodingDetector::detect(const QByteArray &data) {
        // 1. 检查 BOM
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

        // 2. 检查是否是有效的 UTF-8
        if (isLikelyUtf8(data)) {
           spdlog::debug("Detected Likely UTF-8 without BOM");
           return "UTF-8";
        }

        // 3. 尝试其他编码并评分
        struct EncodingScore {
            QString encoding;
            int score;
        };

        QList<EncodingScore> scores;
        QStringList encodings = {"GBK", "GB18030", "GB2312", "Big5"}; // 添加其他编码

         for (const QString &encoding: encodings) {
           QTextCodec *codec = QTextCodec::codecForName(encoding.toLatin1());
           if (!codec) continue;

           QTextCodec::ConverterState state;
           codec->toUnicode(data.constData(), data.size(), &state);

           int score = 0;
           if (state.invalidChars == 0) {
             score = 60;  // 可以转换，基础分
             scores.append({encoding, score});
             spdlog::debug("Encoding {} score: {}", encoding.toStdString(), score);
           }
         }

        // 4. 选择得分最高的编码
        QString bestEncoding = "UTF-8"; // 默认UTF-8
        int bestScore = -1;

        for (const auto &score: scores) {
           if (score.score > bestScore) {
               bestScore = score.score;
               bestEncoding = score.encoding;
             }
        }

        // 5. 如果最高分太低，可能是纯ASCII文本
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

   bool EncodingDetector::isLikelyUtf8(const QByteArray& data) {
     try {
       return utf8::is_valid(data.begin(), data.end());
     }
      catch (const utf8::exception& e) {
         return false;
     }
   }
}
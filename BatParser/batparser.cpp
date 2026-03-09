#include "batparser.h"

#include <QFile>
#include <QDebug>
#include <QTextStream>

QStringList BatParser::parse(QString path) {
  QStringList result;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return result;
  }

  QTextStream in(&file);

  while (!in.atEnd()) {
    auto s = in.readLine();
    QStringList line = s.split(" ", Qt::SkipEmptyParts);

    bool readNext = false;

    if (line.size() > 0 && (line[0].compare("start") == 0 || readNext)) {
      readNext = false;
      QString temp;

      for (const auto &phrase : line) {
        if (phrase.size() == 0 || phrase.at(0) == '/') {
          continue;
        }

        if (temp.count("\"") % 2 == 1) {
          temp += " " + phrase;
        } else {
          temp = phrase;
        }

        if (temp.count("\"") % 2 == 0) {
          temp.remove('\"');

          if (temp.size() >= 4 && temp.last(4) == ".exe") {
            int lastSymbol =
                std::max(temp.lastIndexOf("/"), temp.lastIndexOf("%"));

            QString fileName = temp.right(temp.size() - lastSymbol - 1);

            result.append(fileName);
          }

          if (temp == "^") {
            readNext = true;
          }
        }
      }
    }
  }

  return result;
}

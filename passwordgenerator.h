#ifndef __PASSWORDGENERATOR_H_
#define __PASSWORDGENERATOR_H_

#include <QObject>
#include <QByteArray>

class PasswordGenerator : public QObject
{
  Q_OBJECT
public:
  explicit PasswordGenerator(QObject *parent = 0);
  bool generate(const QByteArray &domain, const QByteArray &salt, const QByteArray &masterPwd, const QString &availableChars, const int passwordLength, int iterations, bool &doQuit, qreal &elapsed, QString &key, QByteArray &hexKey);

signals:

public slots:

};

#endif // __PASSWORDGENERATOR_H_

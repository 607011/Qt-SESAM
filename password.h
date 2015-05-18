#ifndef __PASSWORD_H_
#define __PASSWORD_H_

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QScopedPointer>

class PasswordPrivate;

class Password : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString key READ key)
  Q_PROPERTY(QString hexKey READ hexKey)
  Q_PROPERTY(QRegExp validator READ validator WRITE setValidator)

public:
  explicit Password(QObject *parent = 0);
  ~Password();

  static const QString LowerChars;
  static const QString UpperChars;
  static const QString UpperCharsNoAmbiguous;
  static const QString Digits;
  static const QString ExtraChars;

  bool generate(const QByteArray &domain,
                const QByteArray &salt,
                const QByteArray &masterPwd,
                const QString &availableChars,
                const int passwordLength,
                const int iterations,
                bool &doQuit,
                qreal *elapsed);
  bool setValidator(const QRegExp &);
  const QRegExp &validator(void) const;
  bool setValidCharacters(const QStringList &canContain, const QStringList &mustContain);
  bool isValid(void) const;
  const QString &key(void) const;
  const QString &hexKey(void) const;

  QString errorString(void) const;

private:
  QScopedPointer<PasswordPrivate> d_ptr;
  Q_DECLARE_PRIVATE(Password)
  Q_DISABLE_COPY(Password)
};

#endif // __PASSWORD_H_

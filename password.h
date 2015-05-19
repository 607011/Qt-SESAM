#ifndef __PASSWORD_H_
#define __PASSWORD_H_

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QScopedPointer>

class PasswordPrivate;


struct PasswordParam {
  PasswordParam(
      const QByteArray &domain,
      const QByteArray &salt,
      const QByteArray &masterPwd,
      const QString &availableChars,
      const int passwordLength,
      const int iterations)
    : domain(domain)
    , salt(salt)
    , masterPwd(masterPwd)
    , availableChars(availableChars)
    , passwordLength(passwordLength)
    , iterations(iterations)
  { /* ... */  }
  PasswordParam(const PasswordParam &o)
    : domain(o.domain)
    , salt(o.salt)
    , masterPwd(o.masterPwd)
    , availableChars(o.availableChars)
    , passwordLength(o.passwordLength)
    , iterations(o.iterations)
  { /* ... */  }
  const QByteArray domain;
  const QByteArray salt;
  const QByteArray masterPwd;
  const QString availableChars;
  const int passwordLength;
  const int iterations;
};


class Password : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString key READ key)
  Q_PROPERTY(QString hexKey READ hexKey)
  Q_PROPERTY(QString elapsedSeconds READ elapsedSeconds)
  Q_PROPERTY(QRegExp validator READ validator WRITE setValidator)

public:
  explicit Password(QObject *parent = 0);
  ~Password();

  static const QString LowerChars;
  static const QString UpperChars;
  static const QString UpperCharsNoAmbiguous;
  static const QString Digits;
  static const QString ExtraChars;

  void abortGeneration(void);
  bool generate(const PasswordParam &p);
  void generateAsync(const PasswordParam &p);
  bool setValidator(const QRegExp &);
  const QRegExp &validator(void) const;
  bool setValidCharacters(const QStringList &canContain, const QStringList &mustContain);
  bool isValid(void) const;
  const QString &key(void) const;
  const QString &hexKey(void) const;
  qreal elapsedSeconds(void) const;
  bool isRunning(void) const;
  void waitForFinished(void);

  QString errorString(void) const;

signals:
  void generated(void);
  void generationAborted(void);

private:
  QScopedPointer<PasswordPrivate> d_ptr;
  Q_DECLARE_PRIVATE(Password)
  Q_DISABLE_COPY(Password)
};

#endif // __PASSWORD_H_

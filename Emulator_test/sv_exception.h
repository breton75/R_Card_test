#ifndef SV_EXCEPTION_H
#define SV_EXCEPTION_H

#include <QString>
#include <QException>

class SvException: public QException
{
public:
    void raise(QString error) { err = error; throw *this; }
    SvException *clone() const { return new SvException(*this); }

    QString err;

};



#endif // SV_EXCEPTION_H

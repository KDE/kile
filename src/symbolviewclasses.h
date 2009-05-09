#ifndef SYMBOLVIEWCLASSES_H
#define SYMBOLVIEWCLASSES_H

#include <QtCore/QObject>
#include <QtCore/QString>

struct Preamble{
   QString className;
   QString classArguments;
   QString additional;
};

struct Package{
   QString name;
   QString arguments;
};

struct Command{
   QString latexCommand;
   QString unicodeCommand;
   QString ImageCommand;
   QString comment;
   bool mathMode;
   QList<Package> packages;
   QList<Package> unicodePackages;
   int referenceCount;
   QString path;
};

struct Version{
   QString major;
   QString minor;
};

#endif //SYMBOLVIEWCLASSES_H

#ifndef CDBVERSIONCONVERTER_H
#define CDBVERSIONCONVERTER_H

#include <QString>

extern const char* FILE_FORMAT_PREFIX;
const int FILE_FORMAT_PREFIX_SIZE = 5;

int getDBVersion(const QString& FileName);
bool convertToVersion3(const QString& SrcFileName,const QString& DstFileName, bool makeBackup = true);
bool convertToVersion4(const QString& SrcFileName,const QString& DstFileName, bool makeBackup = true);


#endif // CDBVERSIONCONVERTER_H

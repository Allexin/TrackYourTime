#ifndef CDBVERSIONCONVERTER_H
#define CDBVERSIONCONVERTER_H

#include <QString>

extern const char* FILE_FORMAT_PREFIX;
const int FILE_FORMAT_PREFIX_SIZE = 5;

int getDBVersion(const QString& FileName);
bool convertToVersion2(const QString& SrcFileName,const QString& DstFileName, bool makeBackup = true);


#endif // CDBVERSIONCONVERTER_H

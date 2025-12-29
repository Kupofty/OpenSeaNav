#include "utils.h"


/////////////////////////
/// General Functions ///
/////////////////////////
QString getTimeStamp()
{
    return "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz") + "] ";
}


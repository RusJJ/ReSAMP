#ifndef __OBJECT_H
#define __OBJECT_H

#include <samp.h>

class CObject;

class CRemoteObject
{
public:
    
    inline CObject* GetEntity() { return m_pEntity; }
    
    int m_nID;
    int m_nGtaID;
    CObject* m_pEntity;
};

#endif // __OBJECT_H

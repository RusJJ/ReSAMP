#include "ui.h"

class SpawnScreen : public SimpleUI
{
public:
    virtual void Draw();
};

extern SpawnScreen* spawnui;

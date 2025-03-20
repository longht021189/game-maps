#ifndef GAME_MAPS_CORE_APP_H
#define GAME_MAPS_CORE_APP_H

#include "stdafx.h"

typedef struct {
  const char* resources_path;
} AppConfig;

CORE_EXTERN_C_BEGIN

CORE_PUBLIC void core_app_config(AppConfig config);

CORE_EXTERN_C_END

#endif // GAME_MAPS_CORE_APP_H
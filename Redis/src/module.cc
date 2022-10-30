#include "util/redismodule.h"
#include "util/version.h"

#include "Soft/rm_Soft.h"
#include "Hard/rm_Hard.h"

#include <assert.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        if (RedisModule_Init(ctx, "CocoSketch", SKETCHES_MODULE_VERSION, REDISMODULE_APIVER_1) !=
            REDISMODULE_OK) {
            return REDISMODULE_ERR;
        }
        
        SoftModule_onLoad(ctx, argv, argc);
        HardModule_onLoad(ctx, argv, argc);

        // TestModule_onLoad(ctx, argv, argc);

        return REDISMODULE_OK;
    }

#ifdef __cplusplus
}
#endif
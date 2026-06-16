#include "module.h"

using namespace sw::redis;

Redis* g_redis = NULL;
Redis* g_subscriber_redis = NULL;

ConnectionOptions g_connection_options;
sw::redis::Subscriber *sub = nullptr;

const char* convertToCString(const OptionalString& optStr) {
    if (optStr) {
        return optStr->c_str();
    } else {
        return nullptr;
    }
}

// native redis_connect(const hostip[], const port, const username[] = "", const password[] = "");
cell redis_connect(AMX *amx, cell *params)
{
	int len = 0;
	g_connection_options.host = MF_GetAmxString(amx, params[1], 0, &len);
	g_connection_options.port = params[2];

	std::string username = MF_GetAmxString(amx, params[3], 1, &len);

	if (len > 0) 
	{
		g_connection_options.user = username;
		g_connection_options.password = MF_GetAmxString(amx, params[4], 2, &len);
	}

	stop_subscribe();

	if (g_redis)
	{
		delete g_redis;
		g_redis = nullptr;
	}

	try 
    {
        g_redis = new Redis(g_connection_options);

		if (HasRedisOnMessage)
		{
			redis_register_subscriber_forward(HasRedisOnMessage);
			redis_start_subscribe(HasRedisOnMessage);
		}

    } catch (const Error &e) {
		if (g_redis)
		{
			delete g_redis;
			g_redis = nullptr;
		}
		MF_LogError(amx, AMX_ERR_NATIVE, "Redis Connecting Error.");
        return -1;
    }
    return 0;
}

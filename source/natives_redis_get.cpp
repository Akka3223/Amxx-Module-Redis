#include "module.h"
#include <string>

using namespace sw::redis;

// native redis_get_string(const key[], value[], maxlength);
cell redis_get_string(AMX *amx, cell *params)
{
	int len = 0;
	std::string key = MF_GetAmxString(amx, params[1], 0, &len);

	if (g_redis != NULL)
	{
		try
		{
			OptionalString value = g_redis->get(key);
			const char *result = convertToCString(value);
			MF_SetAmxString(amx, params[2], result, params[3]);
		}
		catch (const Error&)
		{
			return -1;
		}
	}
	else
		return -1;

	return 0;
}

// native redis_get_integer(const key[]);
cell redis_get_integer(AMX *amx, cell *params)
{
	int len = 0;
	std::string key = MF_GetAmxString(amx, params[1], 0, &len);

	if (g_redis != NULL)
	{
		try
		{
			OptionalString value = g_redis->get(key);
			if (!value)
				return 0;

			try
			{
				return static_cast<cell>(std::stoi(value.value()));
			}
			catch (...)
			{
				return 0;
			}
		}
		catch (const Error&)
		{
			return 0;
		}
	}

	return 0;
}

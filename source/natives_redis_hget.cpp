#include "module.h"
#include <string>

using namespace sw::redis;

// native redis_hget_string(const key[], const field[], value[], maxlength);
cell redis_hget_string(AMX *amx, cell *params)
{
	int len = 0;
	std::string key   = MF_GetAmxString(amx, params[1], 0, &len);
	std::string field = MF_GetAmxString(amx, params[2], 1, &len);

	if (g_redis != NULL)
	{
		try
		{
			OptionalString value = g_redis->hget(key, field);
			const char *result = convertToCString(value);
			MF_SetAmxString(amx, params[3], result, params[4]);
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

// native redis_hget_integer(const key[], const field[]);
cell redis_hget_integer(AMX *amx, cell *params)
{
	int len = 0;
	std::string key = MF_GetAmxString(amx, params[1], 0, &len);
	std::string field = MF_GetAmxString(amx, params[2], 1, &len);

	if (g_redis != NULL)
	{
		try
		{
			OptionalString value = g_redis->hget(key, field);
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

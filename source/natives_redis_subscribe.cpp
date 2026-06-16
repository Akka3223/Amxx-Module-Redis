#include "module.h"

using namespace sw::redis;

std::vector<std::string> channels;
std::thread *th_subscriber = NULL;
std::atomic<bool> isSubscriberRunning{false};
ConnectionOptions g_subscriber_options;

cell redis_register_subscriber_forward(bool hasOnMessage)
{
	if (hasOnMessage)
	{
		if (sub)
		{
			delete sub;
			sub = nullptr;
		}
		if (g_subscriber_redis)
		{
			delete g_subscriber_redis;
			g_subscriber_redis = nullptr;
		}

		channels.clear();
		g_subscriber_options = g_connection_options;
		g_subscriber_options.socket_timeout = std::chrono::milliseconds(300);

		try
		{
			g_subscriber_redis = new Redis(g_subscriber_options);
			sub = new Subscriber(g_subscriber_redis->subscriber());
		}
		catch (const Error&)
		{
			delete g_subscriber_redis;
			g_subscriber_redis = nullptr;
			return -1;
		}

		sub->on_message([](std::string channel, std::string msg) {
			MF_ExecuteForward(ForwardRedisOnMessage,
				channel.c_str(), msg.c_str());
		});
	}
	return 0;
}

// native redis_subscribe(const channel[]);
cell redis_register_subscriber(AMX *amx, cell *params)
{
	if (!HasRedisOnMessage)
		return -1;

	if (isSubscriberRunning)
	{
		MF_LogError(amx, AMX_ERR_NATIVE,
			"Cannot register channels while subscriber is running");
		return -1;
	}

	if (sub == nullptr)
	{
		MF_LogError(amx, AMX_ERR_NATIVE,
			"Subscriber not initialized; call redis_connect first");
		return -1;
	}

	int len = 0;
	std::string channel = MF_GetAmxString(amx, params[1], 0, &len);

	if (g_redis != NULL)
	{
		channels.push_back(channel);
	}
	else
		return -1;

	return 0;
}

void consumeThread()
{
	while (isSubscriberRunning)
	{
		try
		{
			sub->consume();
		}
		catch (const TimeoutError& e)
		{
			continue;
		}
		catch (const Error& err)
		{
			isSubscriberRunning = false;
			return;
		}
	}
}

// native redis_start_subscribe();
cell redis_start_subscribe(bool hasOnMessage)
{
	if (!hasOnMessage)
	{
		return -1;
	}

	if (isSubscriberRunning || th_subscriber != nullptr)
	{
		return -1;
	}

	if (sub == nullptr)
	{
		return -1;
	}

	if (channels.empty())
	{
		return -1;
	}

	for (auto& ch : channels)
	{
		sub->subscribe(ch);
	}

	th_subscriber = new std::thread(consumeThread);
	isSubscriberRunning = true;

	return 0;
}

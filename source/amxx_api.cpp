#include "module.h"
int ForwardRedisOnMessage = -1;
int HasRedisOnMessage = -1;

void OnAmxxAttach()
{
    MF_AddNatives(g_natives);
}

void OnPluginsLoaded()
{
	isSubscriberRunning = false;
	ForwardRedisOnMessage = MF_RegisterForward("Redis_Subscriber_OnMessage",
		ET_STOP, FP_STRING, FP_STRING, FP_DONE);
	HasRedisOnMessage = UTIL_CheckForPublic("Redis_Subscriber_OnMessage");

	if (g_redis)
	{
		redis_register_subscriber_forward(HasRedisOnMessage);
		redis_start_subscribe(HasRedisOnMessage);
	}
}

void stop_subscribe()
{
	isSubscriberRunning = false;

	if (th_subscriber)
	{
		if (th_subscriber->joinable())
		{
			th_subscriber->join();
		}
		delete th_subscriber;
		th_subscriber = nullptr;
	}

	if (sub)
	{
		try
		{
			sub->unsubscribe();
		}
		catch (const Error&)
		{
		}

		delete sub;
		sub = nullptr;
	}

	if (g_subscriber_redis)
	{
		delete g_subscriber_redis;
		g_subscriber_redis = nullptr;
	}
}

void OnPluginsUnloaded()
{
	stop_subscribe();
	channels.clear();

	if (g_redis)
	{
		try
		{
			g_redis->bgsave();
		}
		catch (const Error&)
		{
		}

		delete g_redis;
		g_redis = nullptr;
	}
}

#ifndef API_HANDLER_BASE_H
#define API_HANDLER_BASE_H

#include "API.h"

namespace csconnector
{
	struct APIHandlerBase
	{
		enum class APIRequestStatusType : uint8_t
		{
			SUCCESS,
			FAILURE,
			NOT_IMPLEMENTED,
			MAX
		};

		static void SetResponseStatus(api::APIResponse& response, APIRequestStatusType status, const std::string& details = "");
		static void SetResponseStatus(api::APIResponse& response, bool commandWasHandled);
	};
}

#endif // API_HANDLER_BASE_H
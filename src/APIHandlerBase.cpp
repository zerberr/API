#include "APIHandlerBase.h"

using namespace api;

namespace csconnector
{
	namespace detail
	{
		struct APIRequestStatus
		{
			uint8_t code;
			std::string message;
		};

		APIRequestStatus statuses[static_cast<size_t>(APIHandlerBase::APIRequestStatusType::MAX)] =
		{
			{ 0, "Success" },
			{ 1, "Failure" },
			{ 2, "Not Implemented" },
		};
	}

	void APIHandlerBase::SetResponseStatus(APIResponse& response, APIRequestStatusType status, const std::string& details /*= ""*/)
	{
		using namespace detail;

		response.code = statuses[(uint8_t)status].code;
		response.message = statuses[(uint8_t)status].message + details;
	}

	void APIHandlerBase::SetResponseStatus(APIResponse& response, bool commandWasHandled)
	{
		SetResponseStatus(response, (commandWasHandled ? APIRequestStatusType::SUCCESS : APIRequestStatusType::NOT_IMPLEMENTED));
	}
}

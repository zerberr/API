#include "APIHandler.h"
#include "DebugLog.h"
#include "csconnector/csconnector.h"
#include "CallStats.h"
#include "Validation.h"

using namespace api;

VALIDATE_BEGIN(Transaction)
	VALIDATE(source).Length<Less>(101);
	VALIDATE(target).Length<Less>(101);
	VALIDATE(currency).Length<Less>(101);
VALIDATE_END()

namespace csconnector
{
	APIHandler::APIHandler()
	{
		// TODO: Use gtest
#ifdef TEST_VALIDATION
		extern void validationTest();
		validationTest();
#endif

//#define API_TEST
#ifdef API_TEST
		{
			PoolListGetResult plrg;
			PoolListGet(plrg, 0, 2);

			PoolGetResult pgr;
			PoolGet(pgr, plrg.pools[0].hash);

			TransactionGetResult tgr;
			TransactionGet(tgr, pgr.pool.hash + std::to_string(0));

			TransactionsGetResult tgr1;
			TransactionsGet(tgr1, "1_GJnW172n4CdN0", 0, 2);

			BalanceGetResult bgr;
			BalanceGet(bgr, "1_GJnW172n4CdN0", "cs");

			StatsGetResult sgr;
			StatsGet(sgr);
		}
#endif // API_TEST
	}

	APIHandler::~APIHandler()
	{
	}

	void APIHandler::BalanceGet(BalanceGetResult& _return, const Address& address, const Currency& currency) 
	{
		Log("BalanceGet");

		bool handled = fire<Commands::BalanceGet>(_return, address, currency);

		SetResponseStatus(_return.status, handled);
	}

	void APIHandler::TransactionGet(TransactionGetResult& _return, const TransactionId& transactionId)
	{
		Log("TransactionGet");

		bool handled = fire<Commands::TransactionGet>(_return, transactionId);

		SetResponseStatus(_return.status, handled);
	}

	void APIHandler::TransactionsGet(TransactionsGetResult& _return, const Address& address, const int64_t offset, const int64_t limit) 
	{
		Log("TransactionsGet");

		bool handled = fire<Commands::TransactionsGet>(_return, address, offset, limit);

		SetResponseStatus(_return.status, handled);
	}

	namespace detail
	{
		template <class T, class D = validation::Dummy>
		validation::ValidationResult Validate(const T& arg)
		{
			using namespace validation;

			ValidationResult result = ValidationTraits<T, D>::validate(arg);
			return result;
		}

		template <class T, class D = validation::Dummy>
		std::string GetErrorMessage(const T& arg, const validation::ValidationResult& validationResult)
		{
			using namespace validation;

			std::string field;
			std::string error;
			std::tie(field, error) = ValidationTraits<T, D>::formatErrorMessage(validationResult);
			
			return ": '" + field + "' - " + error;
		}
	}

	void APIHandler::TransactionFlow(TransactionFlowResult& _return, const Transaction& transaction)
	{
		//Log("TransactionFlow");

		{
			auto result = detail::Validate(transaction);
			if ( result != validation::NoError )
			{
				std::string errorMessage = detail::GetErrorMessage(transaction, result);
				SetResponseStatus(_return.status, APIRequestStatusType::FAILURE, errorMessage);

				return;
			}
		}

		bool handled = fire<Commands::TransactionFlow>(_return, transaction);

		SetResponseStatus(_return.status, handled);

		call_stats::count(Commands::TransactionFlow);
	}

	void APIHandler::PoolListGet(PoolListGetResult& _return, const int64_t offset, const int64_t limit) 
	{
		Log("PoolListGet");

		bool handled = fire<Commands::PoolListGet>(_return, offset, limit);

		SetResponseStatus(_return.status, handled);
	}

	void APIHandler::PoolGet(PoolGetResult& _return, const PoolHash& hash) 
	{
		Log("PoolGet");

		bool handled = fire<Commands::PoolGet>(_return, hash);

		SetResponseStatus(_return.status, handled);
	}

	void APIHandler::StatsGet(api::StatsGetResult& _return)
	{
		Log("StatsGet");
		
		bool handled = fire<Commands::StatsGet>(_return);

        SetResponseStatus(_return.status, handled);
    }

    void APIHandler::NodesCountGet(api::NodesCountGetResult& _return)
    {
        Log("NodesCountGet");

        bool isHandled = fire<Commands::NodesCountGet>(_return);

        SetResponseStatus(_return.status, isHandled);
    }
}

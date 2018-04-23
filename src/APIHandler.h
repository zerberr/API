#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <mutex>
#include "APIHandlerInterface.h"

namespace csconnector
{
	class APIHandler : public csconnector::APIHandlerInterface
	{
	public:

		APIHandler();
		~APIHandler();

		virtual void BalanceGet(api::BalanceGetResult& _return, const api::Address& address, const api::Currency& currency) override;

		virtual void TransactionGet(api::TransactionGetResult& _return, const api::TransactionId& transactionId) override;
		virtual void TransactionsGet(api::TransactionsGetResult& _return, const api::Address& address, const int64_t offset, const int64_t limit) override;
		virtual void TransactionFlow(api::TransactionFlowResult& _return, const api::Transaction& transaction) override;
	
		// Get list of pools from last one (head pool) to the first one.
		virtual void PoolListGet(api::PoolListGetResult& _return, const int64_t offset, const int64_t limit) override;
	
		// Get pool info by pool hash. Starts looking from last one (head pool).
		virtual void PoolGet(api::PoolGetResult& _return, const api::PoolHash& hash) override;

		virtual void StatsGet(api::StatsGetResult& _return) override;

        virtual void NodesCountGet(api::NodesCountGetResult& _return) override;

	private:

		bool GetTransaction(const api::TransactionId& transactionId, api::Transaction& transaction);
	};
}


#endif // APIHANDLER_H

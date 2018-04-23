#include "DBHandlers.h"
#include <csdb/csdb.h>
#include "csconnector/csconnector.h"
#include <algorithm>
#include <cstring>

using namespace api;

namespace db_handlers
{
	void init()
	{
		using namespace csconnector;

		registerHandler<Commands::BalanceGet>(BalanceGet);
		registerHandler<Commands::TransactionGet>(TransactionGet);
		registerHandler<Commands::TransactionsGet>(TransactionsGet);
		registerHandler<Commands::PoolGet>(PoolGet);
		registerHandler<Commands::PoolListGet>(PoolListGet);
	}

	void deinit()
	{
		using namespace csconnector;

		unregisterHandler<Commands::BalanceGet>();
		unregisterHandler<Commands::TransactionGet>();
		unregisterHandler<Commands::TransactionsGet>();
		unregisterHandler<Commands::PoolGet>();
		unregisterHandler<Commands::PoolListGet>();
	}

	void BalanceGet(BalanceGetResult& _return, const Address& address, const Currency& currency)
	{
		csdb::Balance balance = {};

		memcpy(balance.A_source, address.c_str(), std::min((size_t)MAX_STR, address.length()));
		memcpy(balance.Currency, currency.c_str(), std::min((size_t)MAX_STR, currency.length()));

		csdb::GetBalance(&balance);

		_return.amount.integral = (int32_t)balance.amount;
		_return.amount.fraction = (int64_t)balance.amount1;
	}

	// NOTE: uuid is struct on Windows, but array on Unix
	// TODO: We've for some reason refused from boost::uuid, 
	// so another cross-platform library is needed

#ifdef _MSC_VER

	void string_to_uuid(const std::string& uuid_str, uuid_t& uuid)
	{
		char* uuid_str_ptr = const_cast<char*>(&uuid_str[0]);

		::UuidFromStringA((RPC_CSTR)(uuid_str_ptr), &uuid);
	}

#else

	void string_to_uuid(const std::string& uuid_str, uuid_t uuid)
	{
		char* uuid_str_ptr = const_cast<char*>(&uuid_str[0]);

		// FIXME: Probably, this line won't compile
		uuid_parse(uuid_str_ptr, uuid);
	}

#endif

	static Transaction ConvertTransaction(const csdb::Tran& tran)
	{
		Transaction transaction;

		transaction.hash = std::to_string(tran.Hash);
		transaction.innerId = csdb::uuid_to_string(tran.InnerID);

		transaction.source = tran.A_source;
		transaction.target = tran.A_target;

		transaction.currency = tran.Currency;
		transaction.amount.integral = tran.Amount;
		transaction.amount.fraction = tran.Amount1;

		return transaction;
	}

	static csdb::Tran ConvertTransaction(const Transaction& transaction)
	{
		csdb::Tran tran = {};

		auto copyStr = [](const std::string& src, char* dst)
		{
			memcpy(dst, src.c_str(), std::min((size_t)MAX_STR, src.length()));
		};

		if (!transaction.hash.empty())
		{
			size_t numCharsProcessed = 0;
			tran.Hash = std::stoull(transaction.hash, &numCharsProcessed, 10);
		}

		string_to_uuid(transaction.innerId, tran.InnerID);

		copyStr(transaction.source, tran.A_source);
		copyStr(transaction.target, tran.A_target);

		copyStr(transaction.currency, tran.Currency);

		tran.Amount = transaction.amount.integral;
		tran.Amount1 = transaction.amount.fraction;

		return tran;
	}

	bool GetTransaction(const TransactionId& transactionId, Transaction& transaction)
	{
		csdb::Tran t = {};

		bool found = csdb::GetTransactionInfo(t, transactionId);
		if (!found)
			return false;

		transaction = ConvertTransaction(t);

		return true;
	}

	void TransactionGet(TransactionGetResult& _return, const TransactionId& transactionId)
	{
		_return.found = GetTransaction(transactionId, _return.transaction);
	}

	/// Thats actually a hack
	// This is because transaction hash should be 'POOL_HASH.TRANSACTION_NUMBER', but it comes from db as integer value.
	// WORKAROUND: Substitute transaction hash with proper one
	void SubstituteTransactionHash(api::Transaction& transaction, const std::string& newHash)
	{
		transaction.hash = newHash;
	}

	std::string FormatTransactionHash(const std::string& poolHash, size_t transacionNumber)
	{
		return poolHash + "." + std::to_string(transacionNumber);
	}

	void TransactionsGet(TransactionsGetResult& _return, const Address& address, const int64_t offset, const int64_t limit)
	{
		typedef std::string TransactionId;
		typedef std::vector<TransactionId> TransactionIds;

		TransactionIds transactionIds;

		_return.result = csdb::GetTransactions(transactionIds, address.c_str(), (size_t)limit, (size_t)offset);

		for (auto& id : transactionIds)
		{
			Transaction transaction;
			if (GetTransaction(id, transaction))
			{
				SubstituteTransactionHash(transaction, id);
				_return.transactions.push_back(transaction);
			}
		}
	}

	using PoolNumber = uint64_t;

	void PoolListGet(PoolListGetResult& _return, const int64_t offset, const int64_t limit)
	{
		size_t o = (size_t)offset;

		std::string h = csdb::GetHeadHash();
		while (!h.empty())
		{
			if (static_cast<int64_t>(_return.pools.size()) >= limit)
				break;

			typedef std::vector<csdb::Tran> Transactions;
			Transactions transactions;

			api::Pool pool;
			pool.hash = h;

			time_t pool_time = 0;
			PoolNumber poolNumber = 0;
			
			csdb::GetPool(&h, &pool.prevHash, &transactions, &pool_time, &poolNumber);
			
			pool.time = (Time)pool_time;
			pool.poolNumber = (api::PoolNumber)poolNumber;

			pool.transactionsCount = static_cast<int32_t>(transactions.size());

			h = pool.prevHash;

			if (o > 0)
			{
				o--;
				continue;
			}

			_return.pools.push_back(pool);
		}
	}

	void PoolGet(PoolGetResult& _return, const PoolHash& hash)
	{
		std::string h = csdb::GetHeadHash();
		while (!h.empty())
		{
			if (hash == h)
			{
				typedef std::vector<csdb::Tran> Transactions;
				Transactions transactions;

				api::Pool& pool = _return.pool;
				pool.hash = hash;

				time_t pool_time = 0;
				PoolNumber poolNumber = 0;

				csdb::GetPool(&pool.hash, &pool.prevHash, &transactions, &pool_time, &poolNumber);

				pool.time = (Time)pool_time;
				pool.poolNumber = (api::PoolNumber)poolNumber;

				pool.transactionsCount = static_cast<int32_t>(transactions.size());
				_return.pool = pool;

				for (size_t i = 0; i < transactions.size(); ++i)
				{
					auto& t = transactions[i];

					Transaction transaction = ConvertTransaction(t);
					std::string newTransactionHash = FormatTransactionHash(hash, i);
					SubstituteTransactionHash(transaction, newTransactionHash);

					_return.transactions.push_back(transaction);
				}

				break;
			}
			else
			{
				std::string ph;

				csdb::GetPool(&h, &ph, nullptr, nullptr, nullptr);

				h = ph;
			}
		}
	}
}

//
// Created by 范軒瑋 on 2020/5/13.
//

#ifndef MVCC_DATABASE_H
#define MVCC_DATABASE_H

#include "fwd.h"
#include "log.h"
#include <shared_mutex>
#include <utility>

namespace mvcc {

class LogList {
public:
    LogList() : curItem(0) {}

    int GetNewestLog(TxnId id, TxnLog &txnLog, const TxnStamp &readStamp) const;
    int AddData(TxnLog txnLog, int &index);
    int AddData(const TxnLog &txnLog);

private:
    int curItem;
    const static int maxItem = 128;
    TxnLog logs[maxItem];
};

class Database {
public:
    Database();
    virtual int Read(TxnId id, const KeyType &key, TxnLog &res, const TxnStamp &readStamp) const = 0;
    virtual int Insert(TxnId id, const KeyType &key, const ValueType &val, TxnStamp stamp) = 0;
    virtual int Commit(TxnId id, std::map<KeyType, TxnLog> &logs, TxnStamp &commitStamp, int threadIdx) = 0;
    virtual int Insert(TxnId id, const KeyType &key, const ValueType &val) = 0;
    std::shared_ptr<std::mutex> RequestDbLock();
    void BeginTxn(TxnId id, bool includeSet);

    virtual inline void Commit() const {
        std::lock_guard<std::mutex> lockGuard(commitLock);
    }

    virtual bool Executed(int threadIdx, TxnId id) {
        return false;
    }

    virtual int Update(TxnId, const KeyType &key, ValueType val, const TxnStamp &stamp) = 0;
protected:
    mutable std::mutex commitLock;
    std::shared_ptr<std::mutex> mLock;
};

class MemoryDB : public Database {
public:
    MemoryDB();
    int Read(TxnId id, const KeyType &key, TxnLog &res, const TxnStamp &readStamp) const override;
    int Insert(TxnId id, const KeyType &key, const ValueType &val) override ;
    int Insert(TxnId id, const KeyType &key, const ValueType &val, TxnStamp stamp) override;
    int Update(TxnId, const KeyType &key, ValueType val, const TxnStamp &stamp) override;
    int Commit(TxnId id, std::map<KeyType, TxnLog> &logs, TxnStamp &commitStamp, int threadIdx) override;
    friend std::ostream &operator<<(std::ostream &output, const MemoryDB &momoryDB);

    int RecordNum() const {
        return mStorage.size();
    }

protected:
    std::map<KeyType, LogList> mStorage; // Save the key-value data in a STL map.
};

class PersistDB : public MemoryDB {
public:
    explicit PersistDB(std::string fileName, std::string logName, int threadNum);
    int LoadSnapshot();
    int SaveSnapshot();
    int Commit(TxnId id, std::map<KeyType, TxnLog> &logs, TxnStamp &commitStamp, int threadIdx) override;
    int ResetLog() {
        logManager->ResetLogFile();
    }
    int Redo();
    int CheckSave();

    bool Executed(int threadIdx, TxnId id) override {
        DCHECK_LT(threadIdx, logManager->threadNum);
        return id <= executedId[threadIdx];
    }

protected:
    std::string fileName;
    std::unique_ptr<LogManager> logManager;
    std::unique_ptr<int[]> executedId;
    int threadNum;
    int commitCount;
};


} // namespace mvcc

#endif //MVCC_DATABASE_H

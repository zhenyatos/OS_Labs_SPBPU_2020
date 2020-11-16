#include "conn.h"
#include "sys_exception.h"
#include <mqueue.h>
#include <new>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include <syslog.h>

const mode_t permissions = 0666;
const char* prefix = "/mq_";

namespace {
    class ConnectionImpl : public Connection {
    public:
        ConnectionImpl(size_t id, bool create);
        int read() const override;
        void write(int msg) const override;

        ~ConnectionImpl() override;

    private:
        mqd_t _descr;
        std::string _id;
        bool _creator;
    };

    ConnectionImpl::ConnectionImpl(size_t id, bool create) {
        _id = prefix + std::to_string(id);
        _creator = create;

        if (create) {
            mq_attr attr = {0, 1, sizeof(int), 0 };
            _descr = mq_open(_id.c_str(), O_CREAT | O_RDWR, permissions, &attr);
            if (_descr == (mqd_t)-1)
                throw SysException("Failed to create message queue", errno);
        }
        else {
            _descr = mq_open(_id.c_str(), O_RDWR);
            if (_descr == (mqd_t)-1)
                throw SysException("Failed to open message queue", errno);
        }
    }

    int ConnectionImpl::read() const {
        int msg;
        if (mq_receive(_descr, (char*)&msg, sizeof(int), 0) == -1)
            throw SysException("Failed to read message from MQ", errno);
        return msg;
    }

    void ConnectionImpl::write(int msg) const {
        if (mq_send(_descr, (char*)&msg, sizeof(int), 0) == -1)
            throw SysException("Failed to write to the MQ", errno);
    }

    ConnectionImpl::~ConnectionImpl() {
        if (mq_close(_descr) == -1)
            syslog(LOG_ERR, "An unhandled exception occured while closing MQ descriptor %d", _descr);
    }
}

Connection* Connection::create(size_t id, bool create) {
    return new (std::nothrow) ConnectionImpl(id, create);
}

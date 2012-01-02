#include <QDebug>
#include <QtCore>

#include <ortp/ortp.h>

#define RTP_PORT 10010
#define RTP_BUFFER_LENGTH 1024

class RTP : public QObject {
    Q_OBJECT

    public:
        RTP();
        virtual ~RTP();
        int init(const char* host,int port);
        void run();
        void send(unsigned char* buffer,int length);
        void shutdown();
        void dump_buffer(unsigned char* buffer,int length);
        RtpSession* rtpSession;
    signals:
        void rtp_remote_set(int set);
        void rtp_set_session(RtpSession* session);

    public slots:
        void setRemote(char* host,int port);
        void shutdown();
//    public slots:
//        void setRemote(char* host,int port);

    private:
        int initialized;
        int remote_set;
        int jittcomp;
        bool_t adapt;
        uint32_t send_ts;
        int cont;

 };


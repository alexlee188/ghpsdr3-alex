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
        int init(char* host,int port);
        void send(unsigned char *buffer, int length);
        void dump_buffer(unsigned char* buffer,int length);
        RtpSession* rtpSession;
    signals:
        void rtp_remote_set(int set);
        void rtp_set_session(RtpSession* session);

    public slots:
        void setRemote(char* host,int port);
        void shutdown();
    private:
        int initialized;
        int remote_set;
        int jittcomp;
        bool_t adapt;
        uint32_t send_ts;
        int cont;

 };


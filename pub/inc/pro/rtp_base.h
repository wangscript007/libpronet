/*
 * Copyright (C) 2018 Eric Tung <libpronet@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is part of LibProNet (http://www.libpro.org)
 */

/*         ______________________________________________________
 *        |                                                      |
 *        |                        ProRtp                        |
 *        |______________________________________________________|
 *        |          |                              |            |
 *        |          |             ProNet           |            |
 *        |          |______________________________|            |
 *        |                    |                                 |
 *        |                    |             ProUtil             |
 *        |      MbedTLS       |_________________________________|
 *        |                              |                       |
 *        |                              |       ProShared       |
 *        |______________________________|_______________________|
 *                     Fig.1 module hierarchy diagram
 */

/*       __________________                     _________________
 *      |    ServiceHub    |<----------------->|   RtpService    |
 *      |   ____________   |    ServicePipe    |(Acceptor Shadow)| Audio-Process
 *      |  |            |  |                   |_________________|
 *      |  |  Acceptor  |  |        ...         _________________
 *      |  |____________|  |                   |   RtpService    |
 *      |                  |    ServicePipe    |(Acceptor Shadow)| Video-Process
 *      |__________________|<----------------->|_________________|
 *           Hub-Process
 *                Fig.2 structure diagram of RtpService
 */

/*
 * 1) client ----->                connect()                -----> server
 * 2) client <-----                 accept()                <----- server
 * 3) client <-----                  nonce                  <----- server
 * 4) client ----->  serviceId + serviceOpt + (r) + (r+1)   -----> server
 * 5) client::[password hash]
 * 6) client ----->          rtp(RTP_SESSION_INFO)          -----> server
 * 7)                                             [password hash]::server
 * 8) client <-----          rtp(RTP_SESSION_ACK)           <----- server
 *                 Fig.3 TCP_EX handshake protocol flow chart
 */

/*
 * 1) client ----->                connect()                -----> server
 * 2) client <-----                 accept()                <----- server
 * 3) client <-----                  nonce                  <----- server
 * 4) client ----->  serviceId + serviceOpt + (r) + (r+1)   -----> server
 * 5) client <<====              ssl handshake              ====>> server
 * 6) client::[password hash]
 * 7) client ----->          rtp(RTP_SESSION_INFO)          -----> server
 * 8)                                             [password hash]::server
 * 9) client <-----          rtp(RTP_SESSION_ACK)           <----- server
 *                 Fig.4 SSL_EX handshake protocol flow chart
 */

/*
 * RFC-1889/1890, RFC-3550/3551, RFC-4571
 *
 * PSP-v1.0 (PRO Session Protocol version 1.0)
 */

#if !defined(____RTP_BASE_H____)
#define ____RTP_BASE_H____

#include "pro_net.h"

#if defined(__cplusplus)
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////
////

#if defined(PRO_RTP_LIB)
#define PRO_RTP_API
#elif defined(PRO_RTP_EXPORTS)
#if defined(_MSC_VER)
#define PRO_RTP_API /* .def */
#else
#define PRO_RTP_API PRO_EXPORT
#endif
#else
#define PRO_RTP_API PRO_IMPORT
#endif

class IRtpBucket;          /* rtp流控桶 */
class IRtpService;         /* rtp服务 */
class IRtpSessionObserver; /* rtp会话回调目标 */

/*
 * [[[[ rtp媒体类型. 0无效, 1~127保留, 128~255自定义
 */
#if !defined(____RTP_MM_TYPE____)
#define ____RTP_MM_TYPE____
typedef unsigned char RTP_MM_TYPE;

static const RTP_MM_TYPE RTP_MMT_MSG       = 11; /* 消息[11 ~ 20] */
static const RTP_MM_TYPE RTP_MMT_MSGII     = 12;
static const RTP_MM_TYPE RTP_MMT_MSGIII    = 13;
static const RTP_MM_TYPE RTP_MMT_MSG_MIN   = 11;
static const RTP_MM_TYPE RTP_MMT_MSG_MAX   = 20;
static const RTP_MM_TYPE RTP_MMT_AUDIO     = 21; /* 音频[21 ~ 30] */
static const RTP_MM_TYPE RTP_MMT_AUDIO_MIN = 21;
static const RTP_MM_TYPE RTP_MMT_AUDIO_MAX = 30;
static const RTP_MM_TYPE RTP_MMT_VIDEO     = 31; /* 视频[31 ~ 40] */
static const RTP_MM_TYPE RTP_MMT_VIDEOII   = 32;
static const RTP_MM_TYPE RTP_MMT_VIDEO_MIN = 31;
static const RTP_MM_TYPE RTP_MMT_VIDEO_MAX = 40;
static const RTP_MM_TYPE RTP_MMT_CTRL      = 41; /* 控制[41 ~ 50] */
static const RTP_MM_TYPE RTP_MMT_CTRL_MIN  = 41;
static const RTP_MM_TYPE RTP_MMT_CTRL_MAX  = 50;
#endif /* ____RTP_MM_TYPE____ */
/*
 * ]]]]
 */

/*
 * [[[[ rtp扩展打包模式
 */
#if !defined(____RTP_EXT_PACK_MODE____)
#define ____RTP_EXT_PACK_MODE____
typedef unsigned char RTP_EXT_PACK_MODE;

static const RTP_EXT_PACK_MODE RTP_EPM_DEFAULT = 0; /* ext8 + rfc12 + payload */
static const RTP_EXT_PACK_MODE RTP_EPM_TCP2    = 2; /* len2 + payload */
static const RTP_EXT_PACK_MODE RTP_EPM_TCP4    = 4; /* len4 + payload */
#endif /* ____RTP_EXT_PACK_MODE____ */
/*
 * ]]]]
 */

/*
 * [[[[ rtp会话类型
 */
typedef unsigned char RTP_SESSION_TYPE;

static const RTP_SESSION_TYPE RTP_ST_UDPCLIENT    =  1; /* udp-标准rtp协议客户端 */
static const RTP_SESSION_TYPE RTP_ST_UDPSERVER    =  2; /* udp-标准rtp协议服务端 */
static const RTP_SESSION_TYPE RTP_ST_TCPCLIENT    =  3; /* tcp-标准rtp协议客户端 */
static const RTP_SESSION_TYPE RTP_ST_TCPSERVER    =  4; /* tcp-标准rtp协议服务端 */
static const RTP_SESSION_TYPE RTP_ST_UDPCLIENT_EX =  5; /* udp-扩展协议客户端 */
static const RTP_SESSION_TYPE RTP_ST_UDPSERVER_EX =  6; /* udp-扩展协议服务端 */
static const RTP_SESSION_TYPE RTP_ST_TCPCLIENT_EX =  7; /* tcp-扩展协议客户端 */
static const RTP_SESSION_TYPE RTP_ST_TCPSERVER_EX =  8; /* tcp-扩展协议服务端 */
static const RTP_SESSION_TYPE RTP_ST_SSLCLIENT_EX =  9; /* ssl-扩展协议客户端 */
static const RTP_SESSION_TYPE RTP_ST_SSLSERVER_EX = 10; /* ssl-扩展协议服务端 */
static const RTP_SESSION_TYPE RTP_ST_MCAST        = 11; /* mcast-标准rtp协议端 */
static const RTP_SESSION_TYPE RTP_ST_MCAST_EX     = 12; /* mcast-扩展协议端 */
/*
 * ]]]]
 */

/*
 * rtp会话信息
 */
struct RTP_SESSION_INFO
{
    PRO_UINT16        localVersion;     /* 本地版本号===[      无需设置      ], for tcp_ex, ssl_ex */
    PRO_UINT16        remoteVersion;    /* 远端版本号===[c无需设置, s必须设置>, for tcp_ex, ssl_ex */
    RTP_SESSION_TYPE  sessionType;      /* 会话类型=====[      无需设置      ] */
    RTP_MM_TYPE       mmType;           /* 媒体类型=====<      必须设置      > */
    RTP_EXT_PACK_MODE packMode;         /* 打包模式=====<      必须设置      >, for tcp_ex, ssl_ex */
    char              reserved1;
    char              passwordHash[32]; /* 口令hash值===[c无需设置, s必须设置>, for tcp_ex, ssl_ex */
    char              reserved2[40];

    PRO_UINT32        someId;           /* 某种id. 比如房间id, 目标节点id等,由上层定义 */
    PRO_UINT32        mmId;             /* 节点id */
    PRO_UINT32        inSrcMmId;        /* 输入媒体流的源节点id. 可以为0 */
    PRO_UINT32        outSrcMmId;       /* 输出媒体流的源节点id. 可以为0 */

    char              userData[64];     /* 用户自定义数据 */
};

/*
 * rtp会话初始化参数
 *
 * observer  : 回调目标
 * reactor   : 反应器
 * localIp   : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * localPort : 要绑定的本地端口号. 如果为0, 系统将随机分配一个
 * bucket    : 流控桶. 如果为NULL, 系统将自动分配一个
 */
struct RTP_INIT_UDPCLIENT
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         localIp[64];      /* = "" */
    unsigned short               localPort;        /* = 0 */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer  : 回调目标
 * reactor   : 反应器
 * localIp   : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * localPort : 要绑定的本地端口号. 如果为0, 系统将随机分配一个
 * bucket    : 流控桶. 如果为NULL, 系统将自动分配一个
 */
struct RTP_INIT_UDPSERVER
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         localIp[64];      /* = "" */
    unsigned short               localPort;        /* = 0 */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer         : 回调目标
 * reactor          : 反应器
 * remoteIp         : 远端的ip地址或域名
 * remotePort       : 远端的端口号
 * localIp          : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * timeoutInSeconds : 握手超时. 默认20秒
 * bucket           : 流控桶. 如果为NULL, 系统将自动分配一个
 */
struct RTP_INIT_TCPCLIENT
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         remoteIp[64];
    unsigned short               remotePort;
    char                         localIp[64];      /* = "" */
    unsigned long                timeoutInSeconds; /* = 0 */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer         : 回调目标
 * reactor          : 反应器
 * localIp          : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * localPort        : 要绑定的本地端口号. 如果为0, 系统将随机分配一个
 * timeoutInSeconds : 握手超时. 默认20秒
 * bucket           : 流控桶. 如果为NULL, 系统将自动分配一个
 */
struct RTP_INIT_TCPSERVER
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         localIp[64];      /* = "" */
    unsigned short               localPort;        /* = 0 */
    unsigned long                timeoutInSeconds; /* = 0 */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer         : 回调目标
 * reactor          : 反应器
 * remoteIp         : 远端的ip地址或域名
 * remotePort       : 远端的端口号
 * localIp          : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * timeoutInSeconds : 握手超时. 默认20秒
 * bucket           : 流控桶. 如果为NULL, 系统将自动分配一个
 */
struct RTP_INIT_UDPCLIENT_EX
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         remoteIp[64];
    unsigned short               remotePort;
    char                         localIp[64];      /* = "" */
    unsigned long                timeoutInSeconds; /* = 0 */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer         : 回调目标
 * reactor          : 反应器
 * localIp          : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * localPort        : 要绑定的本地端口号. 如果为0, 系统将随机分配一个
 * timeoutInSeconds : 握手超时. 默认20秒
 * bucket           : 流控桶. 如果为NULL, 系统将自动分配一个
 */
struct RTP_INIT_UDPSERVER_EX
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         localIp[64];      /* = "" */
    unsigned short               localPort;        /* = 0 */
    unsigned long                timeoutInSeconds; /* = 0 */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer         : 回调目标
 * reactor          : 反应器
 * remoteIp         : 远端的ip地址或域名
 * remotePort       : 远端的端口号
 * password         : 会话口令
 * localIp          : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * timeoutInSeconds : 握手超时. 默认20秒
 * bucket           : 流控桶. 如果为NULL, 系统将自动分配一个
 */
struct RTP_INIT_TCPCLIENT_EX
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         remoteIp[64];
    unsigned short               remotePort;
    char                         password[64];     /* = "" */
    char                         localIp[64];      /* = "" */
    unsigned long                timeoutInSeconds; /* = 0 */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer   : 回调目标
 * reactor    : 反应器
 * sockId     : 套接字id. 来源于IRtpServiceObserver::OnAcceptSession(...)
 * unixSocket : 是否unix套接字
 * bucket     : 流控桶. 如果为NULL, 系统将自动分配一个
 */
struct RTP_INIT_TCPSERVER_EX
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    PRO_INT64                    sockId;
    bool                         unixSocket;
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer         : 回调目标
 * reactor          : 反应器
 * sslConfig        : ssl配置
 * sslSni           : ssl服务名. 如果有效,则参与认证服务端证书
 * remoteIp         : 远端的ip地址或域名
 * remotePort       : 远端的端口号
 * password         : 会话口令
 * localIp          : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * timeoutInSeconds : 握手超时. 默认20秒
 * bucket           : 流控桶. 如果为NULL, 系统将自动分配一个
 *
 * 说明: sslConfig指定的对象必须在会话的生命周期内一直有效
 */
struct RTP_INIT_SSLCLIENT_EX
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    const PRO_SSL_CLIENT_CONFIG* sslConfig;
    char                         sslSni[64];       /* = "" */
    char                         remoteIp[64];
    unsigned short               remotePort;
    char                         password[64];     /* = "" */
    char                         localIp[64];      /* = "" */
    unsigned long                timeoutInSeconds; /* = 0 */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer   : 回调目标
 * reactor    : 反应器
 * sslCtx     : ssl上下文
 * sockId     : 套接字id. 来源于IRtpServiceObserver::OnAcceptSession(...)
 * unixSocket : 是否unix套接字
 * bucket     : 流控桶. 如果为NULL, 系统将自动分配一个
 *
 * 说明: 如果创建成功,会话将成为(sslCtx, sockId)的属主;否则,调用者应该
 *       释放(sslCtx, sockId)对应的资源
 */
struct RTP_INIT_SSLSERVER_EX
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    PRO_SSL_CTX*                 sslCtx;
    PRO_INT64                    sockId;
    bool                         unixSocket;
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer  : 回调目标
 * reactor   : 反应器
 * mcastIp   : 要绑定的多播地址
 * mcastPort : 要绑定的多播端口号. 如果为0, 系统将随机分配一个
 * localIp   : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * bucket    : 流控桶. 如果为NULL, 系统将自动分配一个
 *
 * 说明: 合法的多播地址为[224.0.0.0 ~ 239.255.255.255],
 *       推荐的多播地址为[224.0.1.0 ~ 238.255.255.255],
 *       RFC-1112(IGMPv1), RFC-2236(IGMPv2), RFC-3376(IGMPv3)
 */
struct RTP_INIT_MCAST
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         mcastIp[64];
    unsigned short               mcastPort;        /* = 0 */
    char                         localIp[64];      /* = "" */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数
 *
 * observer  : 回调目标
 * reactor   : 反应器
 * mcastIp   : 要绑定的多播地址
 * mcastPort : 要绑定的多播端口号. 如果为0, 系统将随机分配一个
 * localIp   : 要绑定的本地ip地址. 如果为"", 系统将使用0.0.0.0
 * bucket    : 流控桶. 如果为NULL, 系统将自动分配一个
 *
 * 说明: 合法的多播地址为[224.0.0.0 ~ 239.255.255.255],
 *       推荐的多播地址为[224.0.1.0 ~ 238.255.255.255],
 *       RFC-1112(IGMPv1), RFC-2236(IGMPv2), RFC-3376(IGMPv3)
 */
struct RTP_INIT_MCAST_EX
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
    char                         mcastIp[64];
    unsigned short               mcastPort;        /* = 0 */
    char                         localIp[64];      /* = "" */
    IRtpBucket*                  bucket;           /* = NULL */
};

/*
 * rtp会话初始化参数公共部分
 */
struct RTP_INIT_COMMON
{
    IRtpSessionObserver*         observer;
    IProReactor*                 reactor;
};

/*
 * rtp会话初始化参数集合
 */
struct RTP_INIT_ARGS
{
    union
    {
        RTP_INIT_UDPCLIENT       udpclient;
        RTP_INIT_UDPSERVER       udpserver;
        RTP_INIT_TCPCLIENT       tcpclient;
        RTP_INIT_TCPSERVER       tcpserver;
        RTP_INIT_UDPCLIENT_EX    udpclientEx;
        RTP_INIT_UDPSERVER_EX    udpserverEx;
        RTP_INIT_TCPCLIENT_EX    tcpclientEx;
        RTP_INIT_TCPSERVER_EX    tcpserverEx;
        RTP_INIT_SSLCLIENT_EX    sslclientEx;
        RTP_INIT_SSLSERVER_EX    sslserverEx;
        RTP_INIT_MCAST           mcast;
        RTP_INIT_MCAST_EX        mcastEx;
        RTP_INIT_COMMON          comm;
    };
};

/////////////////////////////////////////////////////////////////////////////
////

/*
 * rtp包
 */
#if !defined(____IRtpPacket____)
#define ____IRtpPacket____
class IRtpPacket
{
public:

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;

    virtual void PRO_CALLTYPE SetMarker(bool m) = 0;

    virtual bool PRO_CALLTYPE GetMarker() const = 0;

    virtual void PRO_CALLTYPE SetPayloadType(char pt) = 0;

    virtual char PRO_CALLTYPE GetPayloadType() const = 0;

    virtual void PRO_CALLTYPE SetSequence(PRO_UINT16 seq) = 0;

    virtual PRO_UINT16 PRO_CALLTYPE GetSequence() const = 0;

    virtual void PRO_CALLTYPE SetTimeStamp(PRO_UINT32 ts) = 0;

    virtual PRO_UINT32 PRO_CALLTYPE GetTimeStamp() const = 0;

    virtual void PRO_CALLTYPE SetSsrc(PRO_UINT32 ssrc) = 0;

    virtual PRO_UINT32 PRO_CALLTYPE GetSsrc() const = 0;

    virtual void PRO_CALLTYPE SetMmId(PRO_UINT32 mmId) = 0;

    virtual PRO_UINT32 PRO_CALLTYPE GetMmId() const = 0;

    virtual void PRO_CALLTYPE SetMmType(RTP_MM_TYPE mmType) = 0;

    virtual RTP_MM_TYPE PRO_CALLTYPE GetMmType() const = 0;

    virtual void PRO_CALLTYPE SetKeyFrame(bool keyFrame) = 0;

    virtual bool PRO_CALLTYPE GetKeyFrame() const = 0;

    virtual void PRO_CALLTYPE SetFirstPacketOfFrame(bool firstPacket) = 0;

    virtual bool PRO_CALLTYPE GetFirstPacketOfFrame() const = 0;

    virtual const void* PRO_CALLTYPE GetPayloadBuffer() const = 0;

    virtual void* PRO_CALLTYPE GetPayloadBuffer() = 0;

    virtual unsigned long PRO_CALLTYPE GetPayloadSize() const = 0;

    virtual PRO_UINT16 PRO_CALLTYPE GetPayloadSize16() const = 0;

    virtual RTP_EXT_PACK_MODE PRO_CALLTYPE GetPackMode() const = 0;

    virtual void PRO_CALLTYPE SetTick(PRO_INT64 tick) = 0;

    virtual PRO_INT64 PRO_CALLTYPE GetTick() const = 0;
};
#endif /* ____IRtpPacket____ */

/*
 * rtp流控桶
 */
class IRtpBucket
{
public:

    virtual void PRO_CALLTYPE Destroy() = 0;

    virtual unsigned long PRO_CALLTYPE GetTotalBytes() const = 0;

    virtual IRtpPacket* PRO_CALLTYPE GetFront() = 0;

    virtual bool PRO_CALLTYPE PushBackAddRef(IRtpPacket* packet) = 0;

    virtual void PRO_CALLTYPE PopFrontRelease(IRtpPacket* packet) = 0;

    virtual void PRO_CALLTYPE Reset() = 0;

    virtual void PRO_CALLTYPE SetRedline(
        unsigned long redlineBytes,  /* = 0 */
        unsigned long redlineFrames  /* = 0 */
        ) = 0;

    virtual void PRO_CALLTYPE GetRedline(
        unsigned long* redlineBytes, /* = NULL */
        unsigned long* redlineFrames /* = NULL */
        ) const = 0;

    virtual void PRO_CALLTYPE GetFlowctrlInfo(
        float*         inFrameRate,  /* = NULL */
        float*         inBitRate,    /* = NULL */
        float*         outFrameRate, /* = NULL */
        float*         outBitRate,   /* = NULL */
        unsigned long* cachedBytes,  /* = NULL */
        unsigned long* cachedFrames  /* = NULL */
        ) const = 0;

    virtual void PRO_CALLTYPE ResetFlowctrlInfo() = 0;
};

/*
 * rtp重排序器
 */
class IRtpReorder
{
public:

    virtual void PRO_CALLTYPE SetMaxPacketCount(
        unsigned char maxPacketCount              /* = 5 */
        ) = 0;

    virtual void PRO_CALLTYPE SetMaxWaitingDuration(
        unsigned char maxWaitingDurationInSeconds /* = 1 */
        ) = 0;

    virtual void PRO_CALLTYPE SetMaxBrokenDuration(
        unsigned char maxBrokenDurationInSeconds  /* = 10 */
        ) = 0;

    virtual void PRO_CALLTYPE PushBack(IRtpPacket* packet) = 0;

    virtual IRtpPacket* PRO_CALLTYPE PopFront() = 0;

    virtual void PRO_CALLTYPE Reset() = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

/*
 * rtp服务回调目标
 *
 * 使用者需要实现该接口
 */
class IRtpServiceObserver
{
public:

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;

    /*
     * 有tcp会话进入时,该函数将被回调
     *
     * 上层应该调用CheckRtpServiceData(...)进行校验,之后,根据remoteInfo,
     * 把sockId包装成RTP_ST_TCPSERVER_EX类型的IRtpSession对象,
     * 或释放sockId对应的资源
     */
    virtual void PRO_CALLTYPE OnAcceptSession(
        IRtpService*            service,
        PRO_INT64               sockId,     /* 套接字id */
        bool                    unixSocket, /* 是否unix套接字 */
        const char*             remoteIp,   /* 远端的ip地址. != NULL */
        unsigned short          remotePort, /* 远端的端口号. > 0 */
        const RTP_SESSION_INFO* remoteInfo, /* 远端的会话信息. != NULL */
        PRO_UINT64              nonce       /* 会话随机数 */
        ) = 0;

    /*
     * 有ssl会话进入时,该函数将被回调
     *
     * 上层应该调用CheckRtpServiceData(...)进行校验,之后,根据remoteInfo,
     * 把(sslCtx, sockId)包装成RTP_ST_SSLSERVER_EX类型的IRtpSession对象,
     * 或释放(sslCtx, sockId)对应的资源
     */
    virtual void PRO_CALLTYPE OnAcceptSession(
        IRtpService*            service,
        PRO_SSL_CTX*            sslCtx,     /* ssl上下文 */
        PRO_INT64               sockId,     /* 套接字id */
        bool                    unixSocket, /* 是否unix套接字 */
        const char*             remoteIp,   /* 远端的ip地址. != NULL */
        unsigned short          remotePort, /* 远端的端口号. > 0 */
        const RTP_SESSION_INFO* remoteInfo, /* 远端的会话信息. != NULL */
        PRO_UINT64              nonce       /* 会话随机数 */
        ) = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

/*
 * rtp会话
 */
class IRtpSession
{
public:

    /*
     * 获取会话信息
     */
    virtual void PRO_CALLTYPE GetInfo(RTP_SESSION_INFO* info) const = 0;

    /*
     * 获取会话的加密套件
     *
     * 仅用于RTP_ST_SSLCLIENT_EX, RTP_ST_SSLSERVER_EX类型的会话
     */
    virtual PRO_SSL_SUITE_ID PRO_CALLTYPE GetSslSuite(char suiteName[64]) const = 0;

    /*
     * 获取会话的套接字id
     *
     * 如非必需,最好不要直接操作底层的套接字
     */
    virtual PRO_INT64 PRO_CALLTYPE GetSockId() const = 0;

    /*
     * 获取会话的本地ip地址
     */
    virtual const char* PRO_CALLTYPE GetLocalIp(char localIp[64]) const = 0;

    /*
     * 获取会话的本地端口号
     */
    virtual unsigned short PRO_CALLTYPE GetLocalPort() const = 0;

    /*
     * 获取会话的远端ip地址
     */
    virtual const char* PRO_CALLTYPE GetRemoteIp(char remoteIp[64]) const = 0;

    /*
     * 获取会话的远端端口号
     */
    virtual unsigned short PRO_CALLTYPE GetRemotePort() const = 0;

    /*
     * 设置会话的远端ip地址和端口号
     *
     * 仅用于RTP_ST_UDPCLIENT, RTP_ST_UDPSERVER类型的会话
     */
    virtual void PRO_CALLTYPE SetRemoteIpAndPort(
        const char*    remoteIp,  /* = NULL */
        unsigned short remotePort /* = 0 */
        ) = 0;

    /*
     * 检查会话是否已连接
     *
     * 仅用于tcp协议家族的会话
     */
    virtual bool PRO_CALLTYPE IsTcpConnected() const = 0;

    /*
     * 检查会话是否就绪(握手完成)
     */
    virtual bool PRO_CALLTYPE IsReady() const = 0;

    /*
     * 直接发送rtp包
     *
     * 如果返回false, 表示发送池已满,上层应该缓冲数据以待
     * OnSendSession(...)回调拉取
     */
    virtual bool PRO_CALLTYPE SendPacket(IRtpPacket* packet) = 0;

    /*
     * 通过定时器平滑发送rtp包(for CRtpSessionWrapper only)
     *
     * sendDurationMs为平滑周期. 默认1毫秒
     */
    virtual bool PRO_CALLTYPE SendPacketByTimer(
        IRtpPacket*   packet,
        unsigned long sendDurationMs = 0
        ) = 0;

    /*
     * 获取会话的发送时间信息
     *
     * 用于粗略地判断tcp链路的发送延迟,判断tcp链路是否老化
     */
    virtual void PRO_CALLTYPE GetSendOnSendTick(
        PRO_INT64* sendTick,  /* = NULL */
        PRO_INT64* onSendTick /* = NULL */
        ) const = 0;

    /*
     * 请求会话回调一个OnSend事件
     */
    virtual void PRO_CALLTYPE RequestOnSend() = 0;

    /*
     * 挂起会话的接收能力
     */
    virtual void PRO_CALLTYPE SuspendRecv() = 0;

    /*
     * 恢复会话的接收能力
     */
    virtual void PRO_CALLTYPE ResumeRecv() = 0;

    /*
     * 为会话添加额外的多播接收地址
     *
     * 仅用于RTP_ST_MCAST, RTP_ST_MCAST_EX类型的会话
     */
    virtual bool PRO_CALLTYPE AddMcastReceiver(const char* mcastIp) = 0;

    /*
     * 为会话删除额外的多播接收地址
     *
     * 仅用于RTP_ST_MCAST, RTP_ST_MCAST_EX类型的会话
     */
    virtual void PRO_CALLTYPE RemoveMcastReceiver(const char* mcastIp) = 0;

    /*
     * rtp链路使能(for CRtpSessionWrapper only)------------------------------
     */

    virtual void PRO_CALLTYPE EnableInput(bool enable) = 0;

    virtual void PRO_CALLTYPE EnableOutput(bool enable) = 0;

    /*
     * rtp流量控制(for CRtpSessionWrapper only)------------------------------
     */

    virtual void PRO_CALLTYPE SetOutputRedline(
        unsigned long redlineBytes,  /* = 0 */
        unsigned long redlineFrames  /* = 0 */
        ) = 0;

    virtual void PRO_CALLTYPE GetOutputRedline(
        unsigned long* redlineBytes, /* = NULL */
        unsigned long* redlineFrames /* = NULL */
        ) const = 0;

    virtual void PRO_CALLTYPE GetFlowctrlInfo(
        float*         inFrameRate,  /* = NULL */
        float*         inBitRate,    /* = NULL */
        float*         outFrameRate, /* = NULL */
        float*         outBitRate,   /* = NULL */
        unsigned long* cachedBytes,  /* = NULL */
        unsigned long* cachedFrames  /* = NULL */
        ) const = 0;

    virtual void PRO_CALLTYPE ResetFlowctrlInfo() = 0;

    /*
     * rtp数据统计(for CRtpSessionWrapper only)------------------------------
     */

    virtual void PRO_CALLTYPE GetInputStat(
        float* frameRate, /* = NULL */
        float* bitRate,   /* = NULL */
        float* lossRate,  /* = NULL */
        float* lossCount  /* = NULL */
        ) const = 0;

    virtual void PRO_CALLTYPE GetOutputStat(
        float* frameRate, /* = NULL */
        float* bitRate,   /* = NULL */
        float* lossRate,  /* = NULL */
        float* lossCount  /* = NULL */
        ) const = 0;

    virtual void PRO_CALLTYPE ResetInputStat() = 0;

    virtual void PRO_CALLTYPE ResetOutputStat() = 0;

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;
};

/*
 * rtp会话回调目标
 *
 * 使用者需要实现该接口
 */
class IRtpSessionObserver
{
public:

    virtual unsigned long PRO_CALLTYPE AddRef() = 0;

    virtual unsigned long PRO_CALLTYPE Release() = 0;

    /*
     * 握手完成时,该函数将被回调
     */
    virtual void PRO_CALLTYPE OnOkSession(IRtpSession* session) = 0;

    /*
     * rtp包到来时,该函数将被回调
     */
    virtual void PRO_CALLTYPE OnRecvSession(
        IRtpSession* session,
        IRtpPacket*  packet
        ) = 0;

    /*
     * 发送能力空闲时,该函数将被回调
     */
    virtual void PRO_CALLTYPE OnSendSession(
        IRtpSession* session,
        bool         packetErased /* 是否有rtp包被流控队列擦除 */
        ) = 0;

    /*
     * 网络错误或超时时,该函数将被回调
     */
    virtual void PRO_CALLTYPE OnCloseSession(
        IRtpSession* session,
        long         errorCode,   /* 系统错误码 */
        long         sslCode,     /* ssl错误码. 参见"mbedtls/error.h, ssl.h, x509.h, ..." */
        bool         tcpConnected /* tcp连接是否已经建立 */
        ) = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

/*
 * 功能: 初始化rtp库
 *
 * 参数: 无
 *
 * 返回值: 无
 *
 * 说明: 无
 */
PRO_RTP_API
void
PRO_CALLTYPE
ProRtpInit();

/*
 * 功能: 获取该库的版本号
 *
 * 参数:
 * major : 主版本号
 * minor : 次版本号
 * patch : 补丁号
 *
 * 返回值: 无
 *
 * 说明: 该函数格式恒定
 */
PRO_RTP_API
void
PRO_CALLTYPE
ProRtpVersion(unsigned char* major,  /* = NULL */
              unsigned char* minor,  /* = NULL */
              unsigned char* patch); /* = NULL */

/*
 * 功能: 创建一个rtp包
 *
 * 参数:
 * payloadBuffer : 媒体数据指针
 * payloadSize   : 媒体数据长度
 * packMode      : 打包模式
 *
 * 返回值: rtp包对象或NULL
 *
 * 说明: 调用者应该继续初始化rtp包的头部字段
 *
 *       如果packMode为RTP_EPM_DEFAULT或RTP_EPM_TCP2, 那么payloadSize最多
 *       (1024 * 63)字节
 */
PRO_RTP_API
IRtpPacket*
PRO_CALLTYPE
CreateRtpPacket(const void*       payloadBuffer,
                unsigned long     payloadSize,
                RTP_EXT_PACK_MODE packMode = RTP_EPM_DEFAULT);

/*
 * 功能: 创建一个rtp包
 *
 * 参数:
 * payloadSize : 媒体数据长度
 * packMode    : 打包模式
 *
 * 返回值: rtp包对象或NULL
 *
 * 说明: 该版本主要用于减少内存拷贝次数.
 *       例如,视频编码器可以通过IRtpPacket::GetPayloadBuffer(...)得到媒体
 *       数据指针,然后直接进行媒体数据的初始化等操作
 *
 *       如果packMode为RTP_EPM_DEFAULT或RTP_EPM_TCP2, 那么payloadSize最多
 *       (1024 * 63)字节
 */
PRO_RTP_API
IRtpPacket*
PRO_CALLTYPE
CreateRtpPacketSpace(unsigned long     payloadSize,
                     RTP_EXT_PACK_MODE packMode = RTP_EPM_DEFAULT);

/*
 * 功能: 克隆一个rtp包
 *
 * 参数:
 * packet : 原始的rtp包对象
 *
 * 返回值: 克隆的rtp包对象或NULL
 *
 * 说明: 无
 */
PRO_RTP_API
IRtpPacket*
PRO_CALLTYPE
CloneRtpPacket(const IRtpPacket* packet);

/*
 * 功能: 解析一段标准的rtp流
 *
 * 参数:
 * streamBuffer : 流指针
 * streamSize   : 流长度
 *
 * 返回值: rtp包对象或NULL
 *
 * 说明: 解析过程会忽略IRtpPacket不支持的字段
 */
PRO_RTP_API
IRtpPacket*
PRO_CALLTYPE
ParseRtpStreamToPacket(const void* streamBuffer,
                       PRO_UINT16  streamSize);

/*
 * 功能: 从一个rtp包中查找一段标准的rtp流
 *
 * 参数:
 * packet     : 要找的rtp包对象
 * streamSize : 找到的流长度
 *
 * 返回值: 找到的流指针或NULL
 *
 * 说明: 返回时, (*streamSize)包含了流长度
 */
PRO_RTP_API
const void*
PRO_CALLTYPE
FindRtpStreamFromPacket(const IRtpPacket* packet,
                        PRO_UINT16*       streamSize);

/*
 * 功能: 设置rtp端口号的分配范围
 *
 * 参数:
 * minUdpPort : 最小udp端口号
 * maxUdpPort : 最大udp端口号
 * minTcpPort : 最小tcp端口号
 * maxTcpPort : 最大tcp端口号
 *
 * 返回值: 无
 *
 * 说明: 默认的端口号分配范围为[3000 ~ 5999]
 */
PRO_RTP_API
void
PRO_CALLTYPE
SetRtpPortRange(unsigned short minUdpPort,
                unsigned short maxUdpPort,
                unsigned short minTcpPort,
                unsigned short maxTcpPort);

/*
 * 功能: 获取rtp端口号的分配范围
 *
 * 参数:
 * minUdpPort : 返回的最小udp端口号
 * maxUdpPort : 返回的最大udp端口号
 * minTcpPort : 返回的最小tcp端口号
 * maxTcpPort : 返回的最大tcp端口号
 *
 * 返回值: 无
 *
 * 说明: 默认的端口号分配范围为[3000 ~ 5999]
 */
PRO_RTP_API
void
PRO_CALLTYPE
GetRtpPortRange(unsigned short* minUdpPort,  /* = NULL */
                unsigned short* maxUdpPort,  /* = NULL */
                unsigned short* minTcpPort,  /* = NULL */
                unsigned short* maxTcpPort); /* = NULL */

/*
 * 功能: 自动分配一个udp端口号
 *
 * 参数: 无
 *
 * 返回值: udp端口号. [偶数]
 *
 * 说明: 返回的端口号不一定空闲,应该多次分配尝试
 */
PRO_RTP_API
unsigned short
PRO_CALLTYPE
AllocRtpUdpPort();

/*
 * 功能: 自动分配一个tcp端口号
 *
 * 参数: 无
 *
 * 返回值: tcp端口号. [偶数]
 *
 * 说明: 返回的端口号不一定空闲,应该多次分配尝试
 */
PRO_RTP_API
unsigned short
PRO_CALLTYPE
AllocRtpTcpPort();

/*
 * 功能: 设置会话的保活超时
 *
 * 参数:
 * keepaliveInSeconds : 保活超时. 默认60秒
 *
 * 返回值: 无
 *
 * 说明: 配合reactor的心跳定时器使用. 保活超时应该大于心跳周期的2倍
 */
PRO_RTP_API
void
PRO_CALLTYPE
SetRtpKeepaliveTimeout(unsigned long keepaliveInSeconds); /* = 60 */

/*
 * 功能: 获取会话的保活超时
 *
 * 参数: 无
 *
 * 返回值: 保活超时. 默认60秒
 *
 * 说明: 配合reactor的心跳定时器使用. 保活超时应该大于心跳周期的2倍
 */
PRO_RTP_API
unsigned long
PRO_CALLTYPE
GetRtpKeepaliveTimeout();

/*
 * 功能: 设置会话的流控时间窗
 *
 * 参数:
 * flowctrlInSeconds : 流控时间窗. 默认1秒
 *
 * 返回值: 无
 *
 * 说明: 无
 */
PRO_RTP_API
void
PRO_CALLTYPE
SetRtpFlowctrlTimeSpan(unsigned long flowctrlInSeconds); /* = 1 */

/*
 * 功能: 获取会话的流控时间窗
 *
 * 参数: 无
 *
 * 返回值: 流控时间窗. 默认1秒
 *
 * 说明: 无
 */
PRO_RTP_API
unsigned long
PRO_CALLTYPE
GetRtpFlowctrlTimeSpan();

/*
 * 功能: 设置会话的统计时间窗
 *
 * 参数:
 * statInSeconds : 统计时间窗. 默认5秒
 *
 * 返回值: 无
 *
 * 说明: 无
 */
PRO_RTP_API
void
PRO_CALLTYPE
SetRtpStatTimeSpan(unsigned long statInSeconds); /* = 5 */

/*
 * 功能: 获取会话的统计时间窗
 *
 * 参数: 无
 *
 * 返回值: 统计时间窗. 默认5秒
 *
 * 说明: 无
 */
PRO_RTP_API
unsigned long
PRO_CALLTYPE
GetRtpStatTimeSpan();

/*
 * 功能: 设置底层udp套接字的系统参数
 *
 * 参数:
 * mmType          : 媒体类型
 * sockBufSizeRecv : 底层套接字的系统接收缓冲区字节数. 默认(1024 * 56)
 * sockBufSizeSend : 底层套接字的系统发送缓冲区字节数. 默认(1024 * 56)
 * recvPoolSize    : 底层接收池的字节数. 默认(1024 * 65)
 *
 * 返回值: 无
 *
 * 说明: 某项的值为0时,表示不改变该项的设置
 */
PRO_RTP_API
void
PRO_CALLTYPE
SetRtpUdpSocketParams(RTP_MM_TYPE   mmType,
                      unsigned long sockBufSizeRecv, /* = 0 */
                      unsigned long sockBufSizeSend, /* = 0 */
                      unsigned long recvPoolSize);   /* = 0 */

/*
 * 功能: 获取底层udp套接字的系统参数
 *
 * 参数:
 * mmType          : 媒体类型
 * sockBufSizeRecv : 返回的底层套接字的系统接收缓冲区字节数. 默认(1024 * 56)
 * sockBufSizeSend : 返回的底层套接字的系统发送缓冲区字节数. 默认(1024 * 56)
 * recvPoolSize    : 返回的底层接收池的字节数. 默认(1024 * 65)
 *
 * 返回值: 无
 *
 * 说明: 无
 */
PRO_RTP_API
void
PRO_CALLTYPE
GetRtpUdpSocketParams(RTP_MM_TYPE    mmType,
                      unsigned long* sockBufSizeRecv, /* = NULL */
                      unsigned long* sockBufSizeSend, /* = NULL */
                      unsigned long* recvPoolSize);   /* = NULL */

/*
 * 功能: 设置底层tcp套接字的系统参数
 *
 * 参数:
 * mmType          : 媒体类型
 * sockBufSizeRecv : 底层套接字的系统接收缓冲区字节数. 默认(1024 * 56)
 * sockBufSizeSend : 底层套接字的系统发送缓冲区字节数. 默认(1024 * 8)
 * recvPoolSize    : 底层接收池的字节数. 默认(1024 * 65)
 *
 * 返回值: 无
 *
 * 说明: 某项的值为0时,表示不改变该项的设置
 */
PRO_RTP_API
void
PRO_CALLTYPE
SetRtpTcpSocketParams(RTP_MM_TYPE   mmType,
                      unsigned long sockBufSizeRecv, /* = 0 */
                      unsigned long sockBufSizeSend, /* = 0 */
                      unsigned long recvPoolSize);   /* = 0 */

/*
 * 功能: 获取底层tcp套接字的系统参数
 *
 * 参数:
 * mmType          : 媒体类型
 * sockBufSizeRecv : 返回的底层套接字的系统接收缓冲区字节数. 默认(1024 * 56)
 * sockBufSizeSend : 返回的底层套接字的系统发送缓冲区字节数. 默认(1024 * 8)
 * recvPoolSize    : 返回的底层接收池的字节数. 默认(1024 * 65)
 *
 * 返回值: 无
 *
 * 说明: 无
 */
PRO_RTP_API
void
PRO_CALLTYPE
GetRtpTcpSocketParams(RTP_MM_TYPE    mmType,
                      unsigned long* sockBufSizeRecv, /* = NULL */
                      unsigned long* sockBufSizeSend, /* = NULL */
                      unsigned long* recvPoolSize);   /* = NULL */

/*
 * 功能: 创建一个rtp服务
 *
 * 参数:
 * sslConfig        : ssl配置. 可以为NULL
 * observer         : 回调目标
 * reactor          : 反应器
 * mmType           : 服务的媒体类型
 * serviceHubPort   : 服务hub的端口号
 * timeoutInSeconds : 握手超时. 默认10秒
 *
 * 返回值: rtp服务对象或NULL
 *
 * 说明: 如果sslConfig为NULL, 表示该服务不支持ssl
 */
PRO_RTP_API
IRtpService*
PRO_CALLTYPE
CreateRtpService(const PRO_SSL_SERVER_CONFIG* sslConfig, /* = NULL */
                 IRtpServiceObserver*         observer,
                 IProReactor*                 reactor,
                 RTP_MM_TYPE                  mmType,
                 unsigned short               serviceHubPort,
                 unsigned long                timeoutInSeconds = 0);

/*
 * 功能: 删除一个rtp服务
 *
 * 参数:
 * service : rtp服务对象
 *
 * 返回值: 无
 *
 * 说明: 无
 */
PRO_RTP_API
void
PRO_CALLTYPE
DeleteRtpService(IRtpService* service);

/*
 * 功能: 校验rtp服务收到的口令hash值
 *
 * 参数:
 * serviceNonce       : 服务端的会话随机数
 * servicePassword    : 服务端留存的会话口令
 * clientPasswordHash : 客户端发来的口令hash值
 *
 * 返回值: true成功, false失败
 *
 * 说明: 服务端使用该函数校验客户端的访问合法性
 */
PRO_RTP_API
bool
PRO_CALLTYPE
CheckRtpServiceData(PRO_UINT64  serviceNonce,
                    const char* servicePassword,
                    const char  clientPasswordHash[32]);

/*
 * 功能: 创建一个会话包装
 *
 * 参数:
 * sessionType : 会话类型
 * initArgs    : 会话初始化参数
 * localInfo   : 会话信息
 *
 * 返回值: 会话包装对象或NULL
 *
 * 说明: 会话包装对象包含流控策略
 */
PRO_RTP_API
IRtpSession*
PRO_CALLTYPE
CreateRtpSessionWrapper(RTP_SESSION_TYPE        sessionType,
                        const RTP_INIT_ARGS*    initArgs,
                        const RTP_SESSION_INFO* localInfo);

/*
 * 功能: 删除一个会话包装
 *
 * 参数:
 * sessionWrapper : 会话包装对象
 *
 * 返回值: 无
 *
 * 说明: 无
 */
PRO_RTP_API
void
PRO_CALLTYPE
DeleteRtpSessionWrapper(IRtpSession* sessionWrapper);

/*
 * 功能: 创建一个基础类型的流控桶
 *
 * 参数: 无
 *
 * 返回值: 流控桶对象或NULL
 *
 * 说明: 流控桶的线程安全性由调用者负责
 */
PRO_RTP_API
IRtpBucket*
PRO_CALLTYPE
CreateRtpBaseBucket();

/*
 * 功能: 创建一个音频类型的流控桶
 *
 * 参数: 无
 *
 * 返回值: 流控桶对象或NULL
 *
 * 说明: 流控桶的线程安全性由调用者负责
 */
PRO_RTP_API
IRtpBucket*
PRO_CALLTYPE
CreateRtpAudioBucket();

/*
 * 功能: 创建一个视频类型的流控桶
 *
 * 参数: 无
 *
 * 返回值: 流控桶对象或NULL
 *
 * 说明: 流控桶的线程安全性由调用者负责
 */
PRO_RTP_API
IRtpBucket*
PRO_CALLTYPE
CreateRtpVideoBucket();

/*
 * 功能: 创建一个重排序器
 *
 * 参数: 无
 *
 * 返回值: 重排序器对象或NULL
 *
 * 说明: 重排序器的线程安全性由调用者负责
 */
PRO_RTP_API
IRtpReorder*
PRO_CALLTYPE
CreateRtpReorder();

/*
 * 功能: 删除一个重排序器
 *
 * 参数:
 * reorder : 重排序器对象
 *
 * 返回值: 无
 *
 * 说明: 无
 */
PRO_RTP_API
void
PRO_CALLTYPE
DeleteRtpReorder(IRtpReorder* reorder);

/////////////////////////////////////////////////////////////////////////////
////

#if defined(__cplusplus)
}
#endif

#endif /* ____RTP_BASE_H____ */

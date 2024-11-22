/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_mqtt_port_public.h
 * Abstract   : 给fm_mqtt_port.h提供支持
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-08-08
 * Modifies   : [2024-08-08]新增[Author:LTH](无)
************************************************************************************************/
#ifndef	FM_MQTT_PORT_PUBLIC_H
#define FM_MQTT_PORT_PUBLIC_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Determines the maximum number of MQTT PUBLISH messages, pending
 * acknowledgment at a time, that are supported for incoming and outgoing
 * direction of messages, separately.
 *
 * QoS 1 and 2 MQTT PUBLISHes require acknowledgment from the server before
 * they can be completed. While they are awaiting the acknowledgment, the
 * client must maintain information about their state. The value of this
 * macro sets the limit on how many simultaneous PUBLISH states an MQTT
 * context maintains, separately, for both incoming and outgoing direction of
 * PUBLISHes.
 *
 * @note The MQTT context maintains separate state records for outgoing
 * and incoming PUBLISHes, and thus, 2 * MQTT_STATE_ARRAY_MAX_COUNT amount
 * of memory is statically allocated for the state records.
 *
 * <b>Possible values:</b> Any positive 32 bit integer. <br>
 * <b>Default value:</b> `10`
 */
#ifndef MQTT_STATE_ARRAY_MAX_COUNT
    /* Default value for the maximum acknowledgment pending PUBLISH messages. */
    #define MQTT_STATE_ARRAY_MAX_COUNT    ( 10U )
#endif

/**
 * @addtogroup mqtt_constants
 * @{
 */
#ifndef MQTT_PACKET_TYPE_PUBLISH
    #define MQTT_PACKET_TYPE_PUBLISH        ( ( uint8_t ) 0x30U )  /**< @brief PUBLISH (bidirectional). */
#endif





/**
 * @ingroup mqtt_enum_types
 * @brief Return codes from MQTT functions.
 */
typedef enum MQTTStatus
{
    MQTTSuccess = 0,     /**< Function completed successfully. */
    MQTTBadParameter,    /**< At least one parameter was invalid. */
    MQTTNoMemory,        /**< A provided fixed_buffer_buf was too small. */
    MQTTSendFailed,      /**< The transport send function failed. */
    MQTTRecvFailed,      /**< The transport receive function failed. */
    MQTTBadResponse,     /**< An invalid packet was received from the server. */
    MQTTServerRefused,   /**< The server refused a CONNECT or SUBSCRIBE. */
    MQTTNoDataAvailable, /**< No data available from the transport interface. */
    MQTTIllegalState,    /**< An illegal state in the state record. */
    MQTTStateCollision,  /**< A collision with an existing state record entry. */
    MQTTKeepAliveTimeout /**< Timeout while waiting for PINGRESP. */
} MQTTStatus_t;





/**
 * @ingroup mqtt_enum_types
 * @brief MQTT Quality of Service values.
 */
typedef enum MQTTQoS
{
    MQTTQoS0 = 0, /**< Delivery at most once. */
    MQTTQoS1 = 1, /**< Delivery at least once. */
    MQTTQoS2 = 2  /**< Delivery exactly once. */
} MQTTQoS_t;

/**
 * @ingroup mqtt_struct_types
 * @brief MQTT PUBLISH packet parameters.
 */
typedef struct MQTTPublishInfo
{
    /**
     * @brief Quality of Service for message.
     */
    MQTTQoS_t qos;

    /**
     * @brief Whether this is a retained message.
     */
    bool retain;

    /**
     * @brief Whether this is a duplicate publish message.
     */
    bool dup;

    /**
     * @brief Topic name on which the message is published.
     */
    const char * pTopicName;

    /**
     * @brief Length of topic name.
     */
    uint16_t topicNameLength;

    /**
     * @brief Message payload.
     */
    const void * pPayload;

    /**
     * @brief Message payload length.
     */
    size_t payloadLength;
} MQTTPublishInfo_t;

/**
 * @ingroup mqtt_struct_types
 * @brief Struct to hold deserialized packet information for an #MQTTEventCallback_t
 * callback.
 */
typedef struct MQTTDeserializedInfo
{
    uint16_t packetIdentifier;          /**< @brief Packet ID of deserialized packet. */
    MQTTPublishInfo_t * pPublishInfo;   /**< @brief Pointer to deserialized publish info. */
    MQTTStatus_t deserializationResult; /**< @brief Return code of deserialization. */
} MQTTDeserializedInfo_t;

/**
 * @ingroup mqtt_struct_types
 * @brief MQTT incoming packet parameters.
 */
typedef struct MQTTPacketInfo
{
    /**
     * @brief Type of incoming MQTT packet.
     */
    uint8_t type;

    /**
     * @brief Remaining serialized data in the MQTT packet.
     */
    uint8_t * pRemainingData;

    /**
     * @brief Length of remaining serialized data.
     */
    size_t remainingLength;
} MQTTPacketInfo_t;

/* Structures defined in this file. */
struct MQTTContext;

/**
 * @ingroup mqtt_callback_types
 * @brief Application callback for receiving incoming publishes and incoming
 * acks.
 *
 * @note This callback will be called only if packets are deserialized with a
 * result of #MQTTSuccess or #MQTTServerRefused. The latter can be obtained
 * when deserializing a SUBACK, indicating a broker's rejection of a subscribe.
 *
 * @param[in] pContext Initialized MQTT context.
 * @param[in] pPacketInfo Information on the type of incoming MQTT packet.
 * @param[in] pDeserializedInfo Deserialized information from incoming packet.
 */
typedef void (* MQTTEventCallback_t )( struct MQTTContext * pContext,
                                       struct MQTTPacketInfo * pPacketInfo,
                                       struct MQTTDeserializedInfo * pDeserializedInfo );



/**
 * @ingroup mqtt_callback_types
 * @brief Application provided function to query the current time in
 * milliseconds.
 *
 * @return The current time in milliseconds.
 */
typedef uint32_t (* MQTTGetCurrentTimeFunc_t )( void );



/**
 * @ingroup mqtt_enum_types
 * @brief Values indicating if an MQTT connection exists.
 */
typedef enum MQTTConnectionStatus
{
    MQTTNotConnected, /**< @brief MQTT Connection is inactive. */
    MQTTConnected     /**< @brief MQTT Connection is active. */
} MQTTConnectionStatus_t;



/**
 * @ingroup mqtt_struct_types
 * @brief Buffer passed to MQTT library.
 *
 * These buffers are not copied and must remain in scope for the duration of the
 * MQTT operation.
 */
typedef struct MQTTFixedBuffer
{
    uint8_t * pBuffer; /**< @brief Pointer to fixed_buffer_buf. */
    size_t size;       /**< @brief Size of fixed_buffer_buf. */
} MQTTFixedBuffer_t;



/**
 * @transportstruct
 * @typedef NetworkContext_t
 * @brief The NetworkContext is an incomplete type. An implementation of this
 * interface must define struct NetworkContext for the system requirements.
 * This context is passed into the network interface functions.
 */
/* @[define_networkcontext] */
struct NetworkContext {
    int socket_fd;
    char *server_addr;
    char *server_port;
};
typedef struct NetworkContext NetworkContext_t;
/* @[define_networkcontext] */

/**
 * @transportcallback
 * @brief Transport interface for receiving data on the network.
 *
 * @note It is RECOMMENDED that the transport receive implementation
 * does NOT block when requested to read a single byte. A single byte
 * read request can be made by the caller to check whether there is a
 * new frame available on the network for reading.
 * However, the receive implementation MAY block for a timeout period when
 * it is requested to read more than 1 byte. This is because once the caller
 * is aware that a new frame is available to read on the network, then
 * the likelihood of reading more than one byte over the network becomes high.
 *
 * @param[in] pNetworkContext Implementation-defined network context.
 * @param[in] pBuffer Buffer to receive the data into.
 * @param[in] bytesToRecv Number of bytes requested from the network.
 *
 * @return The number of bytes received or a negative value to indicate
 * error.
 *
 * @note If no data is available on the network to read and no error
 * has occurred, zero MUST be the return value. A zero return value
 * SHOULD represent that the read operation can be retried by calling
 * the API function. Zero MUST NOT be returned if a network disconnection
 * has occurred.
 */
/* @[define_transportrecv] */
typedef int32_t ( * TransportRecv_t )( NetworkContext_t * pNetworkContext,
                                       void * pBuffer,
                                       size_t bytesToRecv );
/* @[define_transportrecv] */

/**
 * @transportcallback
 * @brief Transport interface for sending data over the network.
 *
 * @param[in] pNetworkContext Implementation-defined network context.
 * @param[in] pBuffer Buffer containing the bytes to send over the network stack.
 * @param[in] bytesToSend Number of bytes to send over the network.
 *
 * @return The number of bytes sent or a negative value to indicate error.
 *
 * @note If no data is transmitted over the network due to a full TX fixed_buffer_buf and
 * no network error has occurred, this MUST return zero as the return value.
 * A zero return value SHOULD represent that the send operation can be retried
 * by calling the API function. Zero MUST NOT be returned if a network disconnection
 * has occurred.
 */
/* @[define_transportsend] */
typedef int32_t ( * TransportSend_t )( NetworkContext_t * pNetworkContext,
                                       const void * pBuffer,
                                       size_t bytesToSend );

typedef int32_t ( * TransportOpen_t )( NetworkContext_t * pNetworkContext );

typedef int32_t ( * TransportClose_t )( NetworkContext_t * pNetworkContext );
/* @[define_transportsend] */

/**
 * @transportstruct
 * @brief The transport layer interface.
 */
/* @[define_transportinterface] */
typedef struct TransportInterface
{
    TransportOpen_t open;
    TransportClose_t close;
    TransportRecv_t recv;               /**< Transport receive interface. */
    TransportSend_t send;               /**< Transport send interface. */
    NetworkContext_t * pNetworkContext; /**< Implementation-defined network context. */
} TransportInterface_t;
/* @[define_transportinterface] */



/**
 * @ingroup mqtt_enum_types
 * @brief The state of QoS 1 or QoS 2 MQTT publishes, used in the state engine.
 */
typedef enum MQTTPublishState
{
    MQTTStateNull = 0,  /**< @brief An empty state with no corresponding PUBLISH. */
    MQTTPublishSend,    /**< @brief The library will send an outgoing PUBLISH packet. */
    MQTTPubAckSend,     /**< @brief The library will send a PUBACK for a received PUBLISH. */
    MQTTPubRecSend,     /**< @brief The library will send a PUBREC for a received PUBLISH. */
    MQTTPubRelSend,     /**< @brief The library will send a PUBREL for a received PUBREC. */
    MQTTPubCompSend,    /**< @brief The library will send a PUBCOMP for a received PUBREL. */
    MQTTPubAckPending,  /**< @brief The library is awaiting a PUBACK for an outgoing PUBLISH. */
    MQTTPubRecPending,  /**< @brief The library is awaiting a PUBREC for an outgoing PUBLISH. */
    MQTTPubRelPending,  /**< @brief The library is awaiting a PUBREL for an incoming PUBLISH. */
    MQTTPubCompPending, /**< @brief The library is awaiting a PUBCOMP for an outgoing PUBLISH. */
    MQTTPublishDone     /**< @brief The PUBLISH has been completed. */
} MQTTPublishState_t;

/**
 * @ingroup mqtt_struct_types
 * @brief An element of the state engine records for QoS 1 or Qos 2 publishes.
 */
typedef struct MQTTPubAckInfo
{
    uint16_t packetId;               /**< @brief The packet ID of the original PUBLISH. */
    MQTTQoS_t qos;                   /**< @brief The QoS of the original PUBLISH. */
    MQTTPublishState_t publishState; /**< @brief The current state of the publish process. */
} MQTTPubAckInfo_t;

/**
 * @ingroup mqtt_struct_types
 * @brief A struct representing an MQTT connection.
 */
typedef struct MQTTContext
{
    /**
     * @brief State engine records for outgoing publishes.
     */
    MQTTPubAckInfo_t outgoingPublishRecords[ MQTT_STATE_ARRAY_MAX_COUNT ];

    /**
     * @brief State engine records for incoming publishes.
     */
    MQTTPubAckInfo_t incomingPublishRecords[ MQTT_STATE_ARRAY_MAX_COUNT ];

    /**
     * @brief The transport interface used by the MQTT connection.
     */
    TransportInterface_t transportInterface;

    /**
     * @brief The fixed_buffer_buf used in sending and receiving packets from the network.
     */
    MQTTFixedBuffer_t networkBuffer;

    /**
     * @brief The next available ID for outgoing MQTT packets.
     */
    uint16_t nextPacketId;

    /**
     * @brief Whether the context currently has a connection to the broker.
     */
    MQTTConnectionStatus_t connectStatus;

    /**
     * @brief Function used to get millisecond timestamps.
     */
    MQTTGetCurrentTimeFunc_t getTime;

    /**
     * @brief Callback function used to give deserialized MQTT packets to the application.
     */
    MQTTEventCallback_t appCallback;

    /**
     * @brief Timestamp of the last packet sent by the library.
     */
    uint32_t lastPacketTime;

    /**
     * @brief Whether the library sent a packet during a call of #MQTT_ProcessLoop or
     * #MQTT_ReceiveLoop.
     */
    bool controlPacketSent;

    /* Keep alive members. */
    uint16_t keepAliveIntervalSec; /**< @brief Keep Alive interval. */
    uint32_t pingReqSendTimeMs;    /**< @brief Timestamp of the last sent PINGREQ. */
    bool waitingForPingResp;       /**< @brief If the library is currently awaiting a PINGRESP. */
} MQTTContext_t;





/**
 * @ingroup mqtt_struct_types
 * @brief MQTT CONNECT packet parameters.
 */
typedef struct MQTTConnectInfo
{
    /**
     * @brief Whether to establish a new, clean session or resume a previous session.
     */
    bool cleanSession;

    /**
     * @brief MQTT keep alive period.
     */
    uint16_t keepAliveSeconds;

    /**
     * @brief MQTT client identifier. Must be unique per client.
     */
    const char * pClientIdentifier;

    /**
     * @brief Length of the client identifier.
     */
    uint16_t clientIdentifierLength;

    /**
     * @brief MQTT user name. Set to NULL if not used.
     */
    const char * pUserName;

    /**
     * @brief Length of MQTT user name. Set to 0 if not used.
     */
    uint16_t userNameLength;

    /**
     * @brief MQTT password. Set to NULL if not used.
     */
    const char * pPassword;

    /**
     * @brief Length of MQTT password. Set to 0 if not used.
     */
    uint16_t passwordLength;
} MQTTConnectInfo_t;





/**
 * @ingroup mqtt_struct_types
 * @brief MQTT SUBSCRIBE packet parameters.
 */
typedef struct MQTTSubscribeInfo
{
    /**
     * @brief Quality of Service for subscription.
     */
    MQTTQoS_t qos;

    /**
     * @brief Topic filter to subscribe to.
     */
    const char * pTopicFilter;

    /**
     * @brief Length of subscription topic filter.
     */
    uint16_t topicFilterLength;
} MQTTSubscribeInfo_t;



#endif




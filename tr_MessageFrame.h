#include "tr_J2735.h"
#include "header.h"
#include "utcTime.h"
#include "const.h"
#include <iostream>

using namespace std;

#pragma pack(1)
struct St_Position{
    int32_t lat;
    int32_t lon;
    int32_t elev;
};
#pragma pack()

#pragma pack(1)
struct St_Payload_ACK{
    unsigned char Code;
    uint16_t ServiceFlag;
};
#pragma pack()

#pragma pack(1)
struct St_Payload_SERVER_TX_J2735_DATA{
    St_UTCTime UTCTime;
    St_Position Position;
    unsigned char Tx_Result;
    uint PSID;
    unsigned char Channel;
    uint8_t timeslot;
    unsigned char Tx_Power;
    unsigned char Tx_Rate;
    unsigned char priority;
    unsigned char dot2;
    uint16_t MessageID;
    uint16_t EncodedMessageLength;
    uint8_t* EncodedMessage;
};
#pragma pack()

#pragma pack(1)
struct St_Payload_SIMULATOR_TX_J2735_DATA_WITHOUT_MSG{
    St_UTCTime UTCTime;
    uint16_t MessageID;
    uint16_t EncodedMessageLength;
};
#pragma pack()

#pragma pack(1)
struct St_Payload_SIMULATOR_TX_J2735_DATA{
    St_UTCTime UTCTime;
    uint16_t MessageID;
    uint16_t EncodedMessageLength;
    uint8_t* EncodedMessage;
};
#pragma pack()

#pragma pack(1)
struct St_Payload_JSON{
    St_UTCTime UTCTime;
    uint16_t MessageID;
    uint16_t JsonMessageLength;
    uint8_t* JsonMessage;
};
#pragma pack()

#pragma pack(1)
struct St_Payload_SIGN{
    uint8_t FullDeviceID[6];
    uint16_t KeyCodeLength;
    uint8_t KeyCode[4];
};
#pragma pack()

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//              MessageFrame
////////////////////////////////////////////////////
////////////////////////////////////////////////////

#pragma pack(1)
struct St_SERVER_TX_J2735_DATA{
    St_Header header;
    St_Payload_SERVER_TX_J2735_DATA payload;
};
#pragma pack()

#pragma pack(1)
struct St_SIMULATOR_TX_J2735_DATA{
    St_Header header;
    St_Payload_SIMULATOR_TX_J2735_DATA payload;
};
#pragma pack()

#pragma pack(1)
struct St_SERVER_OR_CLIENT_JSON_DATA{
    St_Header header;
    St_Payload_JSON payload;
};
#pragma pack()

#pragma pack(1)
struct St_SERVER_OR_CLIENT_SIGN{
    St_Header header;
    St_Payload_SIGN payload;
};
#pragma pack()

#pragma pack(1)
struct St_SERVER_OR_SIMULATOR_SIGN_ACK{
    St_Header header;
    St_Payload_ACK payload;
};
#pragma pack()


class messageProcessor{
public:
    static size_t ConvertHexStrToArray(const char* hex_str, size_t array_size, uint8_t* array){
        size_t i;
        char t[3];

        for(i = 0; i<array_size; i++){
            memcpy(t, (hex_str + i*2), 2);
            t[2] = '\0';
            *(array + i) = (uint8_t)strtoul(t, NULL, 16);
        }

        return array_size;

    }
    static void TEST_JSON_C_ITERATOR(json_object* obj){

        struct json_object_iterator it;
        struct json_object_iterator itEnd;
        it = json_object_iter_begin(obj);
        itEnd = json_object_iter_end(obj);
        while (!json_object_iter_equal(&it, &itEnd)) {
            printf("%s\n", json_object_iter_peek_name(&it));
            json_object_iter_next(&it);
        }
    }

    static St_SERVER_OR_SIMULATOR_SIGN_ACK set_ACK(uint8_t code, uint16_t serviceFlag){
        St_SERVER_OR_SIMULATOR_SIGN_ACK ack;
        ack.payload.Code = code;
        ack.payload.ServiceFlag = serviceFlag;
        uint8_t devID[3];
        devID[0] = 0x01;
        devID[1] = 0x02;
        devID[2] = 0x03;
        ack.header = messageProcessor::set_header(MESSAGE_TYPE_SERVER_OR_SIMULATOR_SIGN_ACK, 0, MESSAGEFRAME_PAYLOAD_SIZE_ACK, DEVICE_TYPE_SERVER, devID);

        return ack;
    }

    static void print_ACK(St_Payload_ACK payload_ack){
        printf("[ACK]\n");
        printf("Code: %d\n", (int)payload_ack.Code);
        printf("ServiceFlag: %d\n", (int)payload_ack.ServiceFlag);
    }

    static St_Payload_ACK parse_Payload_ACK(uint8_t* payload_buf){
        St_Payload_ACK payload;
        payload.Code = payload_buf[PAYLOAD_IDX_ACK_CODE];
        payload.ServiceFlag = (uint16_t)(payload_buf[PAYLOAD_IDX_ACK_SERVICEFLAG] + (payload_buf[PAYLOAD_IDX_ACK_SERVICEFLAG+1]<<8));
        return payload;
    }

    static void print_SIMULATOR_TX_payload(St_Payload_SIMULATOR_TX_J2735_DATA payload){
        print_UTCTime(payload.UTCTime);
        printf("MessageID: %d\n", payload.MessageID);
        printf("EncodedMessageLength: %d\n", payload.EncodedMessageLength);
        printf("EncodedMessage: ");
        for(int i = 0; i< payload.EncodedMessageLength; i++){
            printf("%02x", payload.EncodedMessage[i]);
        }
        printf("\n");
    }

    static St_Payload_SIMULATOR_TX_J2735_DATA parse_SIMULATOR_TX_payload(uint8_t* data){

        St_Payload_SIMULATOR_TX_J2735_DATA payload;

        uint8_t UTCTime_buf[PAYLOAD_SIZE_UTCTIME];
        memcpy(UTCTime_buf, &(data[0]), PAYLOAD_SIZE_UTCTIME);
        payload.UTCTime = parse_UTCTime(UTCTime_buf);

        payload.MessageID = ushort(data[PAYLOAD_IDX_MESSAGEID] + (data[PAYLOAD_IDX_MESSAGEID+1]<<8));
        payload.EncodedMessageLength = ushort(data[PAYLOAD_IDX_ENCODEDMESSAGELENGTH] + (data[PAYLOAD_IDX_ENCODEDMESSAGELENGTH+1]<<8));  

        payload.EncodedMessage = (uint8_t*)malloc(sizeof(uint8_t)*payload.EncodedMessageLength);
        memcpy(payload.EncodedMessage, &data[PAYLOAD_IDX_ENCODEDMESSAGE], payload.EncodedMessageLength);

        return payload;
    }

    static St_SIMULATOR_TX_J2735_DATA set_SIMULATOR_TX_DATA(ushort ID){

        St_SIMULATOR_TX_J2735_DATA sim_tx;

        sim_tx.payload.UTCTime = set_UTCTime_default();
        sim_tx.payload.MessageID = ID;
        sim_tx.payload.EncodedMessage = (uint8_t*)malloc(sizeof(uint8_t)*4);
        sim_tx.payload.EncodedMessage[0] = 0x11;
        sim_tx.payload.EncodedMessage[1] = 0x22;
        sim_tx.payload.EncodedMessage[2] = 0x33;
        sim_tx.payload.EncodedMessage[3] = 0x44;
        sim_tx.payload.EncodedMessageLength = 4;

        int payload_len = PAYLOAD_SIZE_SIMULATOR_TX_WITHOUT_MSG + sim_tx.payload.EncodedMessageLength;

        uint8_t devID[3];
        devID[0] = 0x01;
        devID[1] = 0x02;
        devID[2] = 0x03;

        sim_tx.header = set_header(MESSAGE_TYPE_SIMULATOR_TX_J2735_DATA, 0, payload_len, DEVICE_TYPE_SIMULATOR, devID);

        return sim_tx;
    }

    static uint8_t* serialize_SERVER_TX_DATA(St_SERVER_TX_J2735_DATA data){
        int msg_size = MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG + data.payload.EncodedMessageLength;
        uint8_t* msg = (uint8_t*)malloc(sizeof(uint8_t)*(msg_size));
        //printf("%d\n", MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG + data.payload.EncodedMessageLength);
        //printf("%d\n", data.payload.EncodedMessageLength);
        memcpy(msg, &data, MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG);

        memcpy(&msg[MESSAGEFRAME_PAYLOAD_IDX_SERVER_TX_J2735_ENCODED_MESSAGE], data.payload.EncodedMessage, data.payload.EncodedMessageLength);

        return msg;
    }

    static void print_SERVER_TX_payload(St_Payload_SERVER_TX_J2735_DATA payload){

        print_UTCTime(payload.UTCTime);
        printf("lat: %d\n", payload.Position.lat);
        printf("lon: %d\n", payload.Position.lon);
        printf("elev: %d\n", payload.Position.elev);
        printf("Tx_Result: %d\n", payload.Tx_Result);
        printf("PSID: %u\n", payload.PSID);
        printf("Channel: %d\n", payload.Channel);
        printf("timeslot: %d\n", payload.timeslot);
        printf("Tx_Power: %d\n", payload.Tx_Power);
        printf("Tx_Rate: %d\n", payload.Tx_Rate);
        printf("priority: %d\n", payload.priority);
        printf("dot2: %d\n", payload.dot2);
        printf("MessageID: %d\n", payload.MessageID);
        printf("EncodedMessageLength: %d\n", payload.EncodedMessageLength);
    }

    static St_Payload_SERVER_TX_J2735_DATA parse_SERVER_TX_payload(uint8_t* data){

        St_Payload_SERVER_TX_J2735_DATA payload;

        uint8_t UTCTime_buf[PAYLOAD_SIZE_UTCTIME];
        memcpy(UTCTime_buf, &(data[0]), PAYLOAD_SIZE_UTCTIME);
        payload.UTCTime = parse_UTCTime(UTCTime_buf);

        memcpy(&payload.Position, &(data[PAYLOAD_IDX_LAT]), 12);

        payload.Tx_Result = (uint8_t)data[PAYLOAD_IDX_SERVER_TX_J2735_TX_RESULT];
        memcpy(&payload.PSID , &data[PAYLOAD_IDX_SERVER_TX_J2735_PSID], sizeof(int));
        payload.Channel = (uint8_t)data[PAYLOAD_IDX_SERVER_TX_J2735_CHANNEL];
        payload.timeslot = (uint8_t)data[PAYLOAD_IDX_SERVER_TX_J2735_TIMESLOT];
        payload.Tx_Power = (uint8_t)data[PAYLOAD_IDX_SERVER_TX_J2735_TX_POWER];
        payload.Tx_Rate = (uint8_t)data[PAYLOAD_IDX_SERVER_TX_J2735_TX_RATE];
        payload.priority = (uint8_t)data[PAYLOAD_IDX_SERVER_TX_J2735_PRIORITY];
        payload.dot2 = (uint8_t)data[PAYLOAD_IDX_SERVER_TX_J2735_DOT2];

        payload.MessageID = ushort(data[PAYLOAD_IDX_SERVER_TX_J2735_MESSAGE_ID] + (data[PAYLOAD_IDX_SERVER_TX_J2735_MESSAGE_ID+1]<<8));
        payload.EncodedMessageLength = ushort(data[PAYLOAD_IDX_SERVER_TX_J2735_ENCODED_MESSAGE_LENGTH] + (data[PAYLOAD_IDX_SERVER_TX_J2735_ENCODED_MESSAGE_LENGTH+1]<<8));  

        payload.EncodedMessage = (uint8_t*)malloc(sizeof(uint8_t)*payload.EncodedMessageLength);
        memcpy(payload.EncodedMessage, &data[PAYLOAD_IDX_SERVER_TX_J2735_ENCODED_MESSAGE], payload.EncodedMessageLength);

        return payload;
        
    }

    static St_SERVER_TX_J2735_DATA set_SERVER_TX_BSM(){
        St_SERVER_TX_J2735_DATA server_tx;
        server_tx.payload.UTCTime = set_UTCTime_default();

        server_tx.payload.EncodedMessage = (uint8_t*)malloc(sizeof(uint8_t)*1);
        server_tx.payload.EncodedMessage[0] = (uint8_t)0xFF;

        //server_tx.payload.EncodedMessage = (uint8_t*)malloc(sizeof(uint8_t)*1);
        //server_tx.payload.EncodedMessage[0] = (uint8_t)0xFF;

        server_tx.payload.Position.lat = 0xf0f0f0f0;
        server_tx.payload.Position.lon = 0xe0e0e0e0;
        server_tx.payload.Position.elev = 0xd0d0d0d0;
        server_tx.payload.Tx_Result = 1;
        server_tx.payload.PSID = 65535;
        server_tx.payload.Channel = 1;
        server_tx.payload.timeslot = 2;
        server_tx.payload.Tx_Power = 0;
        server_tx.payload.Tx_Rate = 0;
        server_tx.payload.priority = 1;
        server_tx.payload.dot2 = 0;
        server_tx.payload.MessageID = MESSAGE_ID_VALUE_BSM;
        server_tx.payload.EncodedMessageLength = sizeof(server_tx.payload.EncodedMessage)/sizeof(uint8_t*);

        uint8_t devID[3];
        devID[0] = 0x01;
        devID[1] = 0x02;
        devID[2] = 0x03;

        int payload_len = PAYLOAD_SIZE_SERVER_TX_WITHOUT_MSG + server_tx.payload.EncodedMessageLength;

        server_tx.header = set_header(MESSAGE_TYPE_SERVER_TX_J2735_DATA, 0, payload_len, DEVICE_TYPE_SERVER, devID);

        return server_tx;
    }

    static St_SERVER_OR_CLIENT_SIGN set_SIGN(){
        St_SERVER_OR_CLIENT_SIGN sign;
        sign.payload.FullDeviceID[0] = 0x64;
        sign.payload.FullDeviceID[1] = 0x77;
        sign.payload.FullDeviceID[2] = 0x74;
        sign.payload.FullDeviceID[3] = 0x65;
        sign.payload.FullDeviceID[4] = 0x73;
        sign.payload.FullDeviceID[5] = 0x74;
        sign.payload.KeyCodeLength = 4;
        sign.payload.KeyCode[0] = 0x63;
        sign.payload.KeyCode[1] = 0x65;
        sign.payload.KeyCode[2] = 0x73;
        sign.payload.KeyCode[3] = 0x74;
        uint8_t devID[3];
        devID[0] = 0x01;
        devID[1] = 0x02;
        devID[2] = 0x03;
        sign.header = set_header(MESSAGE_TYPE_SERVER_OR_SIMULATOR_SIGN, 0, MESSAGEFRAME_PAYLOAD_SIZE_SIGN, DEVICE_TYPE_SERVER, devID);

        return sign;
    }

    static St_Payload_SIGN parse_SIGN(uint8_t* payload_buf){
        St_Payload_SIGN payload_SIGN;

        memcpy(payload_SIGN.FullDeviceID, &payload_buf[PAYLOAD_IDX_FULL_DEVICE_ID], PAYLOAD_SIZE_FULL_DEVICE_ID);

        payload_SIGN.KeyCodeLength = ushort(payload_buf[PAYLOAD_IDX_KEY_CODE_LENGTH] + (payload_buf[PAYLOAD_IDX_KEY_CODE_LENGTH+1]<<8));

        memcpy(payload_SIGN.KeyCode, &payload_buf[PAYLOAD_IDX_KEY_CODE], PAYLOAD_SIZE_KEY_CODE);

        return payload_SIGN;
    }

    static void print_SIGN(St_Payload_SIGN payload_sign){
        printf("[SIGN]\n");
        printf("Full Device ID : 0x%02X %02X %02X %02X %02X %02X\n", payload_sign.FullDeviceID[0], payload_sign.FullDeviceID[1], payload_sign.FullDeviceID[2], payload_sign.FullDeviceID[3], payload_sign.FullDeviceID[4], payload_sign.FullDeviceID[5]);
        printf("Key Code Length: %d\n", payload_sign.KeyCodeLength);
        char tmp[4];
        tmp[0] = (char(payload_sign.KeyCode[0]));
        tmp[1] = (char(payload_sign.KeyCode[1]));
        tmp[2] = (char(payload_sign.KeyCode[2]));
        tmp[3] = (char(payload_sign.KeyCode[3]));

        printf("Key Code: %c%c%c%c\n", tmp[0], tmp[1], tmp[2], tmp[3]);

        uint8_t serial_sign[MESSAGEFRAME_PAYLOAD_SIZE_SIGN];

        memcpy(serial_sign, &payload_sign, MESSAGEFRAME_PAYLOAD_SIZE_SIGN);

        printf("--");
        for (int i = 0; i<MESSAGEFRAME_PAYLOAD_SIZE_SIGN; i++){
            printf("%02X", serial_sign[i]);
        }
        printf("--");
        printf("\n");
    }

    static uint8_t* serialize_JSON_DATA(St_SERVER_OR_CLIENT_JSON_DATA json_data){
        //printf("start serialize\n");

        int messageFrameSize = HEADER_SIZE + PAYLOAD_SIZE_UTCTIME + PAYLOAD_SIZE_MESSAGE_ID_AND_JSON_MESSAGE_LENGTH + json_data.payload.JsonMessageLength;
        //printf("%d\n", messageFrameSize);
        uint8_t* msg = (uint8_t*)malloc(sizeof(uint8_t)*messageFrameSize);

        //Header
        uint8_t* ptr = (uint8_t*)&json_data.header.MessageType;
        for(int i = 0; i<HEADER_SIZE_MESSAGETYPE; i++){
            msg[HEADER_IDX_MESSAGETYPE+i] = *(ptr + i);
            //printf("%02x\n", *(ptr + i));
        }

        msg[HEADER_IDX_SEQUENCE] = *(uint8_t*)&json_data.header.Sequence;

        ptr = (uint8_t*)&json_data.header.PayloadLength;
        for(int i = 0; i<HEADER_SIZE_PAYLOADLENGTH; i++){
            msg[HEADER_IDX_PAYLOADLENGTH+i] = *(ptr + i);
            //printf("%02x\n", *(ptr + i));
        }

        msg[HEADER_IDX_DEVICETYPE] = *(uint8_t*)&json_data.header.DeviceType;

        ptr = (uint8_t*)&json_data.header.DeviceID;
        for(int i = 0; i<HEADER_SIZE_DEVICEID; i++){
            msg[HEADER_IDX_DEVICEID+i] = *(ptr + i);
            //printf("%02x\n", *(ptr + i));
        }

        //UTCTime
        ptr = (uint8_t*)&json_data.payload.UTCTime.year;
        for(int i = 0; i<UTCTIME_SIZE_YEAR; i++){
            msg[MESSAGEFRAME_UTCTIME_IDX_YEAR+i] = *(ptr + i);
            //printf("%02x\n", *(ptr + i));
        }

        msg[MESSAGEFRAME_UTCTIME_IDX_MONTH] = *(uint8_t*)&json_data.payload.UTCTime.month;
        msg[MESSAGEFRAME_UTCTIME_IDX_DAY] =  *(uint8_t*)&json_data.payload.UTCTime.day;
        msg[MESSAGEFRAME_UTCTIME_IDX_HOUR] =  *(uint8_t*)&json_data.payload.UTCTime.hour;
        msg[MESSAGEFRAME_UTCTIME_IDX_MIN] =  *(uint8_t*)&json_data.payload.UTCTime.min;

        ptr = (uint8_t*)&json_data.payload.UTCTime.msec;
        for(int i = 0; i<UTCTIME_SIZE_MSEC; i++){
            msg[MESSAGEFRAME_UTCTIME_IDX_MSEC+i] = *(ptr + i);
            //printf("%02x\n", *(ptr + i));
        }

        ptr = (uint8_t*)&json_data.payload.MessageID;
        for(int i = 0; i<PAYLOAD_SIZE_MESSAGE_ID; i++){
            msg[MESSAGEFRAME_PAYLOAD_IDX_MESSAGEID+i] = *(ptr + i);
            //printf("%02x\n", *(ptr + i));
        }

        ptr = (uint8_t*)&json_data.payload.JsonMessageLength;
        for(int i = 0; i<PAYLOAD_SIZE_JSONMESSAGELENGTH; i++){
            msg[MESSAGEFRAME_PAYLOAD_IDX_JSONMESSAGELENGTH+i] = *(ptr + i);
            //printf("%02x\n", *(ptr + i));
        }

        ptr = (uint8_t*)&json_data.payload.JsonMessage;
        for(int i = 0; i<json_data.payload.JsonMessageLength; i++){
            msg[MESSAGEFRAME_PAYLOAD_IDX_JSONMESSAGE+i] = *(ptr + i);
            //printf("%02x\n", *(ptr + i));
        }

        memcpy(&msg[MESSAGEFRAME_PAYLOAD_IDX_JSONMESSAGE], json_data.payload.JsonMessage, json_data.payload.JsonMessageLength);

        //for(int i=0; i<messageFrameSize; i++){
        //    printf("%02x", *(msg+i));
        //}
        //printf("[END OF MSG]\n");

        //const char* json_char = (const char*)&msg[PAYLOAD_IDX_JSONMESSAGE];
        //char* json_char = (char*)malloc(json_data.payload.JsonMessageLength);
        //memcpy(json_char, &msg[MESSAGEFRAME_PAYLOAD_IDX_JSONMESSAGE], json_data.payload.JsonMessageLength);
        
        //json_object *root = json_tokener_parse(json_char);

        //printf("The json string:\n\n%s\n\n", json_object_to_json_string(root));
        //printf("The json object to string:\n\n%s\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
        //cout << (json_char) << endl;
        //cout << malloc_usable_size(json_char) << endl;

        return msg;
    }

    static St_SERVER_OR_CLIENT_JSON_DATA set_JSON_DATA_SPAT(){
        St_SERVER_OR_CLIENT_JSON_DATA json_data;
        DATA_SPAT spat = set_Json_SPAT();
        const char* json_str_SPAT = get_str_JSON_SPAT(spat);
        St_Payload_JSON payload;
        json_data.payload.UTCTime = set_UTCTime_default();
        json_data.payload.MessageID = MESSAGE_ID_VALUE_SPAT;
        json_data.payload.JsonMessageLength = strlen((char*)json_str_SPAT);
        json_data.payload.JsonMessage = (uint8_t*)json_str_SPAT;

        uint8_t devID[3];
        devID[0] = 0x01;
        devID[1] = 0x02;
        devID[2] = 0x03;

        json_data.header = set_header(MESSAGE_TYPE_SERVER_OR_CLIENT_JSON_DATA, 0, PAYLOAD_SIZE_UTCTIME + PAYLOAD_SIZE_MESSAGE_ID_AND_JSON_MESSAGE_LENGTH + json_data.payload.JsonMessageLength, DEVICE_TYPE_SERVER, devID);

        //printf("[MAP header]\n");
        //print_header(json_data.header);
        //printf("[MAP payload]\n");
        //print_json_payload(json_data.payload);

        return json_data;
    }


    static St_SERVER_OR_CLIENT_JSON_DATA set_JSON_DATA_MAP(){
        St_SERVER_OR_CLIENT_JSON_DATA json_data;
        DATA_MAP map = set_Json_MAP();
        const char* json_str_MAP = get_str_JSON_MAP(map);
        St_Payload_JSON payload;
        json_data.payload.UTCTime = set_UTCTime_default();
        json_data.payload.MessageID = MESSAGE_ID_VALUE_MAP;
        //json_data.payload.JsonMessageLength = strlen(json_str_MAP);
        json_data.payload.JsonMessageLength = strlen((char*)json_str_MAP);
        json_data.payload.JsonMessage = (uint8_t*)json_str_MAP;

        uint8_t devID[3];
        devID[0] = 0x01;
        devID[1] = 0x02;
        devID[2] = 0x03;

        json_data.header = set_header(MESSAGE_TYPE_SERVER_OR_CLIENT_JSON_DATA, 0, PAYLOAD_SIZE_UTCTIME + PAYLOAD_SIZE_MESSAGE_ID_AND_JSON_MESSAGE_LENGTH + json_data.payload.JsonMessageLength, DEVICE_TYPE_SERVER, devID);

        //printf("[MAP header]\n");
        //print_header(json_data.header);
        //printf("[MAP payload]\n");
        //print_json_payload(json_data.payload);

        return json_data;
    }

    static St_Header set_header(uint16_t msgType, uint8_t seq, uint16_t payloadLen, uint8_t devType, uint8_t devID[3]){
        St_Header header;

        header.MessageType = msgType;
        header.Sequence = seq;
        header.PayloadLength = payloadLen;
        header.DeviceType = devType;
        memcpy(header.DeviceID, devID, HEADER_SIZE_DEVICEID);

        return header;
    }

    static St_Header parse_header(uint8_t header_buf[9]){
        St_Header header;
        header.MessageType = uint16_t(header_buf[HEADER_IDX_MESSAGETYPE] + (header_buf[HEADER_IDX_MESSAGETYPE+1]<<8));
        header.Sequence = header_buf[HEADER_IDX_SEQUENCE];
        header.PayloadLength = uint16_t(header_buf[HEADER_IDX_PAYLOADLENGTH] + (header_buf[HEADER_IDX_PAYLOADLENGTH+1]<<8));
        header.DeviceType = header_buf[HEADER_IDX_DEVICETYPE];
        memset(header.DeviceID, 0x00, HEADER_SIZE_DEVICEID);
        header.DeviceID[0] = header_buf[HEADER_IDX_DEVICEID];
        header.DeviceID[1] = header_buf[HEADER_IDX_DEVICEID+1];
        header.DeviceID[2] = header_buf[HEADER_IDX_DEVICEID+2];

        return header;
    }

    static void print_header(St_Header header){
        printf("[Header]\n");
        printf("header size: %ld\n", sizeof(header));
        printf("MessageType: 0x%04X\n", header.MessageType);
        printf("Sequence: %u\n", header.Sequence);
        printf("PayloadLength: %u\n", header.PayloadLength);
        printf("DeviceType: 0x%02X\n", header.DeviceType);
        printf("DeviceID: 0x%02X %02X %02X\n", header.DeviceID[0], header.DeviceID[1], header.DeviceID[2]);
        printf("\n");
    }

    static St_UTCTime parse_UTCTime(uint8_t* UTCTime_buf){
        St_UTCTime UTCTime;
        UTCTime.year = ushort(UTCTime_buf[UTCTIME_IDX_YEAR] + (UTCTime_buf[UTCTIME_IDX_YEAR+1]<<8));
        UTCTime.month = UTCTime_buf[UTCTIME_IDX_MONTH];
        UTCTime.day = UTCTime_buf[UTCTIME_IDX_DAY];
        UTCTime.hour = UTCTime_buf[UTCTIME_IDX_HOUR];
        UTCTime.min = UTCTime_buf[UTCTIME_IDX_MIN];
        UTCTime.msec = ushort(UTCTime_buf[UTCTIME_IDX_MSEC] + (UTCTime_buf[UTCTIME_IDX_MSEC+1]<<8));

        return UTCTime;
    }

    static St_UTCTime set_UTCTime_default(){
        St_UTCTime UTCTime;
        UTCTime.year = 2023;
        UTCTime.month = 1;
        UTCTime.day = 1;
        UTCTime.hour = 1;
        UTCTime.min = 1;
        UTCTime.msec = 1;

        return UTCTime;
    }

    static DATA_MAP set_Json_MAP(){
        DATA_MAP map;
        map.timeStamp = 12;
        map.msgIssueRevision = 12;
        map.layerType = LayerType::none;
        map.layerID = 0;

        map.intersections = (IntersectionGeometry*)malloc(sizeof(IntersectionGeometry)*1);
        memcpy(map.intersections[0].name, "Intersection_01", strlen("Intersection_01"));
        map.intersections[0].id.region = 0;
        map.intersections[0].id.id = 1;
        map.intersections[0].revision = 0;
        //{Lat=35.8922573646559, Lng=128.61466884613}
        map.intersections[0].refPoint.lat = 358922573;
        map.intersections[0].refPoint.Long = 1286146688;
        map.intersections[0].refPoint.elevation = 0;
        map.intersections[0].laneWidth = 5;
        map.intersections[0].speedLimits = (RegulatorySpeedLimit*)malloc(sizeof(RegulatorySpeedLimit)*1);
        map.intersections[0].speedLimits[0].type = SpeedLimitType::vehicleMinSpeed;
        map.intersections[0].speedLimits[0].speed = 0;
        map.intersections[0].laneSet = (GenericLane*)malloc(sizeof(GenericLane)*1);
        map.intersections[0].laneSet[0].laneID = 0;
        memcpy(map.intersections[0].laneSet[0].name, "Lane_01", strlen("Lane_01"));
        map.intersections[0].laneSet[0].ingressApproach = 0;
        map.intersections[0].laneSet[0].egressApproach = 0;
        map.intersections[0].laneSet[0].laneAttributes.directionalUse = 0b10;
        map.intersections[0].laneSet[0].laneAttributes.sharedWith = 0b1000000000;
        //map.intersections[0].laneSet[0].laneAttributes.laneType.vehicle = 0b10000000;

        map.intersections[0].laneSet[0].laneAttributes.laneType.type = LaneTypeAttributes_Type::vehicle;
        map.intersections[0].laneSet[0].laneAttributes.laneType.val.vehicle = 128;

        map.intersections[0].laneSet[0].maneuvers = 0b10000000;

        map.intersections[0].laneSet[0].nodeList.type = NodeListXY_Type::nodes;
        map.intersections[0].laneSet[0].nodeList.val.nodes = (NodeXY*)malloc(sizeof(NodeXY)*1);
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.type = NodeOffsetPointXY_Type::node_XY1;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.val.node_XY1.x = 100;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.val.node_XY1.y = 101;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.localNode = (NodeAttributeXY*)malloc(sizeof(NodeAttributeXY)*1);
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.localNode[0] = NodeAttributeXY::stopLine;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.disabled = (SegmentAttributeXY*)malloc(sizeof(SegmentAttributeXY)*1);
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.disabled[0] = SegmentAttributeXY::whiteLine;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.enabled = (SegmentAttributeXY*)malloc(sizeof(SegmentAttributeXY)*1);
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.enabled[0] = SegmentAttributeXY::whiteLine;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.data = (LaneDataAttribute*)malloc(sizeof(LaneDataAttribute)*1);
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.data[0].type = LaneDataAttribute_Type::pathEndPointAngle;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.data[0].val.pathEndPointAngle = 0;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.dWidth = 10;
        map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.dElevation = 0;

        map.intersections[0].laneSet[0].connectsTo = (Connection*)malloc(sizeof(Connection)*1);
        map.intersections[0].laneSet[0].connectsTo[0].connectingLane.lane = 0;
        map.intersections[0].laneSet[0].connectsTo[0].connectingLane.maneuvers = 0b100000000000;
        map.intersections[0].laneSet[0].connectsTo[0].remoteIntersection.region = 0;
        map.intersections[0].laneSet[0].connectsTo[0].remoteIntersection.id = 0;
        map.intersections[0].laneSet[0].connectsTo[0].signalGroup = 0;
        map.intersections[0].laneSet[0].connectsTo[0].userClass = 0;
        map.intersections[0].laneSet[0].connectsTo[0].connectionID = 0;
        map.intersections[0].laneSet[0].overlays = (LaneID*)malloc(sizeof(LaneID)*3);
        map.intersections[0].laneSet[0].overlays[0] = 0;

        map.intersections[0].preemptPriorityData = (SignalControlZone*)malloc(sizeof(SignalControlZone)*1);
        map.intersections[0].roadAuthorityID.type = RoadAuthorityID_Type::fullRdAuthID;
        map.intersections[0].roadAuthorityID.val.fullRdAuthID.length = 1;
        map.intersections[0].roadAuthorityID.val.fullRdAuthID.value = (unsigned char*)malloc(1);
        memcpy(map.intersections[0].roadAuthorityID.val.fullRdAuthID.value , "a", 1);

        map.roadSegments = (RoadSegment*)malloc(sizeof(RoadSegment)*1);
        memcpy(map.roadSegments[0].name, "roadSegments name", strlen("roadSegments name"));
        map.roadSegments[0].id.region = 0;
        map.roadSegments[0].id.id = 0;
        map.roadSegments[0].revision = 0;
        map.roadSegments[0].refPoint.lat = 358922573;
        map.roadSegments[0].refPoint.Long = 1286146688;
        map.roadSegments[0].refPoint.elevation = 0;
        map.roadSegments[0].laneWidth = 5;

        map.roadSegments[0].speedLimits = (RegulatorySpeedLimit*)malloc(sizeof(RegulatorySpeedLimit)*1);
        map.roadSegments[0].speedLimits[0].type = SpeedLimitType::vehicleMinSpeed;
        map.roadSegments[0].speedLimits[0].speed = 0;

        map.roadSegments[0].roadLaneSet = 0;

        memcpy(map.dataParameters.processMethod, "processMethod", sizeof("processMethod"));
        memcpy(map.dataParameters.processAgency, "processAgency", sizeof("processAgency"));
        memcpy(map.dataParameters.lastCheckedDate, "lastCheckedDate", sizeof("lastCheckedDate"));
        memcpy(map.dataParameters.geoidUsed, "geoidUsed", sizeof("geoidUsed"));

        map.restrictionList = (RestrictionClassAssignment*)malloc(sizeof(RestrictionClassAssignment)*1);
        map.restrictionList[0].id = 0;
        map.restrictionList[0].users = (RestrictionUserType*)malloc(sizeof(RestrictionUserType)*1);
        map.restrictionList[0].users[0].basicType = RestrictionAppliesTo::none;

        return map;
    }

    static DATA_SPAT set_Json_SPAT(){
        DATA_SPAT spat;
        spat.timeStamp = 0;
        strcpy(spat.name, "SPAT");
        spat.intersections = (IntersectionState*)malloc(sizeof(IntersectionState)*1);
        strcpy(spat.intersections[0].name, "intersection1");
        spat.intersections[0].id.region = 0;
        spat.intersections[0].id.id = 0;
        spat.intersections[0].revision = 0;
        spat.intersections[0].status = 32768;
        spat.intersections[0].moy = 0;
        spat.intersections[0].timeStamp = 0;
        spat.intersections[0].enabledLanes = (LaneID*)malloc(sizeof(LaneID)*3);
        spat.intersections[0].enabledLanes[0] = 0;
        spat.intersections[0].enabledLanes[1] = 0;
        spat.intersections[0].enabledLanes[2] = 0;
        spat.intersections[0].states = (MovementState*)malloc(sizeof(MovementState)*1);
        strcpy(spat.intersections[0].states[0].movementName, "movement 1");
        spat.intersections[0].states[0].signalGroup = 0;
        spat.intersections[0].states[0].state_time_speed = (MovementEvent*)malloc(sizeof(MovementEvent)*1);
        spat.intersections[0].states[0].state_time_speed[0].eventState = MovementPhaseState::unavailable;
        spat.intersections[0].states[0].state_time_speed[0].timing.startTime = 0;
        spat.intersections[0].states[0].state_time_speed[0].timing.minEndTime = 0;
        spat.intersections[0].states[0].state_time_speed[0].timing.maxEndTime = 0;
        spat.intersections[0].states[0].state_time_speed[0].timing.likelyTime = 0;
        spat.intersections[0].states[0].state_time_speed[0].timing.confidence = 0;
        spat.intersections[0].states[0].state_time_speed[0].timing.nextTime = 0;
        spat.intersections[0].states[0].state_time_speed[0].speeds = (AdvisorySpeed*)malloc(sizeof(AdvisorySpeed)*1);
        spat.intersections[0].states[0].state_time_speed[0].speeds[0].type = AdvisorySpeedType::none;
        spat.intersections[0].states[0].state_time_speed[0].speeds[0].speed = 0;
        spat.intersections[0].states[0].state_time_speed[0].speeds[0].confidence = SpeedConfidence::unavailable;
        spat.intersections[0].states[0].state_time_speed[0].speeds[0].distance = 0;
        spat.intersections[0].states[0].state_time_speed[0].speeds[0].Class = 0;        
        spat.intersections[0].states[0].maneuverAssistList = (ConnectionManeuverAssist*)malloc(sizeof(ConnectionManeuverAssist)*1);
        spat.intersections[0].states[0].maneuverAssistList[0].connectionID = 0;
        spat.intersections[0].states[0].maneuverAssistList[0].queueLength = 0;
        spat.intersections[0].states[0].maneuverAssistList[0].availableStorageLength = 0;
        spat.intersections[0].states[0].maneuverAssistList[0].waitOnStop = true;
        spat.intersections[0].states[0].maneuverAssistList[0].pedBicycleDetect = true;
        spat.intersections[0].maneuverAssistList = (ConnectionManeuverAssist*)malloc(sizeof(ConnectionManeuverAssist)*1);
        spat.intersections[0].maneuverAssistList[0].connectionID = 0;
        spat.intersections[0].maneuverAssistList[0].queueLength = 0;
        spat.intersections[0].maneuverAssistList[0].availableStorageLength = 0;
        spat.intersections[0].maneuverAssistList[0].waitOnStop = true;
        spat.intersections[0].maneuverAssistList[0].pedBicycleDetect = true;
        spat.intersections[0].roadAuthorityID.type = RoadAuthorityID_Type::fullRdAuthID;
        spat.intersections[0].roadAuthorityID.val.fullRdAuthID.length = strlen("FullRoadAuthorityID");
        spat.intersections[0].roadAuthorityID.val.fullRdAuthID.value = (unsigned char*)malloc(sizeof(unsigned char)*spat.intersections[0].roadAuthorityID.val.fullRdAuthID.length);
        memcpy(spat.intersections[0].roadAuthorityID.val.fullRdAuthID.value, "FullRoadAuthorityID", strlen("FullRoadAuthorityID"));

        return spat;
    }

    static void print_Json_SPAT(DATA_SPAT spat){
        printf("spat.timeStamp: %d\n", spat.timeStamp);
        printf("spat.name: %s\n", spat.name);
        printf("spat.intersections[0].name: %s\n", spat.intersections[0].name);
        printf("spat.intersections[0].id.region: %d\n", spat.intersections[0].id.region);
        printf("spat.intersections[0].id.id: %d\n", spat.intersections[0].id.id);
        printf("spat.intersections[0].revision: %d\n", spat.intersections[0].revision);
        printf("spat.intersections[0].status: %d\n", spat.intersections[0].status);
        printf("spat.intersections[0].moy: %d\n", spat.intersections[0].moy);
        printf("spat.intersections[0].timeStamp: %d\n", spat.intersections[0].timeStamp);
        printf("spat.intersections[0].enabledLanes[0]: %d\n", spat.intersections[0].enabledLanes[0]);
        printf("spat.intersections[0].enabledLanes[1]: %d\n", spat.intersections[0].enabledLanes[1]);
        printf("spat.intersections[0].enabledLanes[2]: %d\n", spat.intersections[0].enabledLanes[2]);
        printf("spat.intersections[0].states[0].movementName: %s\n", spat.intersections[0].states[0].movementName);
        printf("spat.intersections[0].states[0].signalGroup: %d\n", spat.intersections[0].states[0].signalGroup);
        printf("spat.intersections[0].states[0].state_time_speed[0].eventState: %d\n", (int)spat.intersections[0].states[0].state_time_speed[0].eventState);
        printf("spat.intersections[0].states[0].state_time_speed[0].timing.startTime: %d\n", spat.intersections[0].states[0].state_time_speed[0].timing.startTime);
        printf("spat.intersections[0].states[0].state_time_speed[0].timing.minEndTime: %d\n", spat.intersections[0].states[0].state_time_speed[0].timing.minEndTime);
        printf("spat.intersections[0].states[0].state_time_speed[0].timing.maxEndTime: %d\n", spat.intersections[0].states[0].state_time_speed[0].timing.maxEndTime);
        printf("spat.intersections[0].states[0].state_time_speed[0].timing.likelyTime: %d\n", spat.intersections[0].states[0].state_time_speed[0].timing.likelyTime);
        printf("spat.intersections[0].states[0].state_time_speed[0].timing.confidence: %d\n", spat.intersections[0].states[0].state_time_speed[0].timing.confidence);
        printf("spat.intersections[0].states[0].state_time_speed[0].timing.nextTime: %d\n", spat.intersections[0].states[0].state_time_speed[0].timing.nextTime);
        printf("spat.intersections[0].states[0].state_time_speed[0].speeds[0].type: %d\n", (int)spat.intersections[0].states[0].state_time_speed[0].speeds[0].type);
        printf("spat.intersections[0].states[0].state_time_speed[0].speeds[0].speed: %d\n", spat.intersections[0].states[0].state_time_speed[0].speeds[0].speed);
        printf("spat.intersections[0].states[0].state_time_speed[0].speeds[0].confidence: %d\n", (int)spat.intersections[0].states[0].state_time_speed[0].speeds[0].confidence);
        printf("spat.intersections[0].states[0].state_time_speed[0].speeds[0].distance: %d\n", spat.intersections[0].states[0].state_time_speed[0].speeds[0].distance);
        printf("spat.intersections[0].states[0].state_time_speed[0].speeds[0].Class: %d\n", spat.intersections[0].states[0].state_time_speed[0].speeds[0].Class);
        printf("spat.intersections[0].states[0].maneuverAssistList[0].connectionID: %d\n", spat.intersections[0].states[0].maneuverAssistList[0].connectionID);
        printf("spat.intersections[0].states[0].maneuverAssistList[0].queueLength: %d\n", spat.intersections[0].states[0].maneuverAssistList[0].queueLength);
        printf("spat.intersections[0].states[0].maneuverAssistList[0].availableStorageLength: %d\n", spat.intersections[0].states[0].maneuverAssistList[0].availableStorageLength);
        printf("spat.intersections[0].states[0].maneuverAssistList[0].waitOnStop: %d\n", spat.intersections[0].states[0].maneuverAssistList[0].waitOnStop);
        printf("spat.intersections[0].states[0].maneuverAssistList[0].pedBicycleDetect: %d\n", spat.intersections[0].states[0].maneuverAssistList[0].pedBicycleDetect);
        printf("spat.intersections[0].maneuverAssistList[0].connectionID: %d\n", spat.intersections[0].maneuverAssistList[0].connectionID);
        printf("spat.intersections[0].maneuverAssistList[0].queueLength: %d\n", spat.intersections[0].maneuverAssistList[0].queueLength);
        printf("spat.intersections[0].maneuverAssistList[0].availableStorageLength: %d\n", spat.intersections[0].maneuverAssistList[0].availableStorageLength);
        printf("spat.intersections[0].maneuverAssistList[0].waitOnStop: %d\n", spat.intersections[0].maneuverAssistList[0].waitOnStop);
        printf("spat.intersections[0].maneuverAssistList[0].pedBicycleDetect: %d\n", spat.intersections[0].maneuverAssistList[0].pedBicycleDetect);
        printf("spat.intersections[0].roadAuthorityID.type: %d\n", (int)spat.intersections[0].roadAuthorityID.type);
        printf("spat.intersections[0].roadAuthorityID.val.fullRdAuthID.length: %d\n", spat.intersections[0].roadAuthorityID.val.fullRdAuthID.length);
        printf("spat.intersections[0].roadAuthorityID.val.fullRdAuthID.value: %s\n", spat.intersections[0].roadAuthorityID.val.fullRdAuthID.value);

    }

    static const char* get_str_JSON_SPAT(DATA_SPAT spat){
        json_object* root = json_object_new_object();
        json_object_object_add(root, "timeStamp", json_object_new_int((int)spat.timeStamp));
        json_object_object_add(root, "name", json_object_new_string((const char*)spat.name));

        json_object* intersections = json_object_new_array();
        json_object_object_add(root, "intersections", intersections);

        json_object* intersections_element = json_object_new_object();

        json_object_object_add(intersections_element, "name", json_object_new_string((const char*)spat.intersections[0].name));

        json_object* intersections_id = json_object_new_object();
        json_object_object_add(intersections_id, "region", json_object_new_int((int)spat.intersections[0].id.region));
        json_object_object_add(intersections_id, "id", json_object_new_int((int)spat.intersections[0].id.id));

        json_object_object_add(intersections_element, "id", intersections_id);

        json_object_object_add(intersections_element, "revision", json_object_new_int((int)spat.intersections[0].revision));
        json_object_object_add(intersections_element, "status", json_object_new_int((int)spat.intersections[0].status));
        json_object_object_add(intersections_element, "moy", json_object_new_int((int)spat.intersections[0].moy));
        json_object_object_add(intersections_element, "timeStamp", json_object_new_int((int)spat.intersections[0].timeStamp));

        json_object* intersections_enabledLanes = json_object_new_array();

        int siz = malloc_usable_size(spat.intersections[0].enabledLanes)/sizeof(LaneID*);
        for(int i = 0; i<siz; i++){
            json_object_array_add(intersections_enabledLanes, json_object_new_int((int)spat.intersections[0].enabledLanes[i]));
        }

        json_object_object_add(intersections_element, "enabledLanes", intersections_enabledLanes);

        json_object* intersections_states = json_object_new_array();
        json_object* states_element = json_object_new_object();
        json_object_object_add(states_element, "movementName", json_object_new_string((const char*)spat.intersections[0].states[0].movementName));
        json_object_object_add(states_element, "signalGroup", json_object_new_int((int)spat.intersections[0].states[0].signalGroup));

        json_object* states_state_time_speed = json_object_new_array();
        json_object* state_time_speed_element = json_object_new_object();
        json_object_object_add(state_time_speed_element, "eventState", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].eventState));
        
        json_object* state_time_speed_timing = json_object_new_object();
        json_object_object_add(state_time_speed_timing, "startTime", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].timing.startTime));
        json_object_object_add(state_time_speed_timing, "minEndTime", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].timing.minEndTime));
        json_object_object_add(state_time_speed_timing, "maxEndTime", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].timing.maxEndTime));
        json_object_object_add(state_time_speed_timing, "likelyTime", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].timing.likelyTime));
        json_object_object_add(state_time_speed_timing, "confidence", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].timing.confidence));
        json_object_object_add(state_time_speed_timing, "nextTime", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].timing.nextTime));

        json_object_object_add(state_time_speed_element, "timing", state_time_speed_timing);

        json_object* state_time_speed_speed = json_object_new_array();
        json_object* speed_element = json_object_new_object();
        json_object_object_add(speed_element, "type", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].speeds[0].type));
        json_object_object_add(speed_element, "speed", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].speeds[0].speed));
        json_object_object_add(speed_element, "confidence", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].speeds[0].confidence));
        json_object_object_add(speed_element, "distance", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].speeds[0].distance));
        json_object_object_add(speed_element, "Class", json_object_new_int((int)spat.intersections[0].states[0].state_time_speed[0].speeds[0].Class));

        json_object_array_add(state_time_speed_speed, speed_element);

        json_object_object_add(state_time_speed_element, "speed", state_time_speed_speed);

        json_object_array_add(states_state_time_speed, state_time_speed_element);

        json_object_object_add(states_element, "state_time_speed", states_state_time_speed);

        json_object* states_maneuverAssistList = json_object_new_array();
        json_object* states_maneuverAssistList_element = json_object_new_object();

        json_object_object_add(states_maneuverAssistList_element, "connectionID", json_object_new_int((int)spat.intersections[0].states[0].maneuverAssistList[0].connectionID));
        json_object_object_add(states_maneuverAssistList_element, "queueLength", json_object_new_int((int)spat.intersections[0].states[0].maneuverAssistList[0].queueLength));
        json_object_object_add(states_maneuverAssistList_element, "availableStorageLength", json_object_new_int((int)spat.intersections[0].states[0].maneuverAssistList[0].availableStorageLength));
        //cout << spat.intersections[0].states[0].maneuverAssistList[0].waitOnStop << endl;
        json_object_object_add(states_maneuverAssistList_element, "waitOnStop", json_object_new_boolean(spat.intersections[0].states[0].maneuverAssistList[0].waitOnStop));
        json_object_object_add(states_maneuverAssistList_element, "pedBicycleDetect", json_object_new_boolean(spat.intersections[0].states[0].maneuverAssistList[0].pedBicycleDetect));

        json_object_array_add(states_maneuverAssistList, states_maneuverAssistList_element);
        json_object_object_add(states_element, "maneuverAssistList", states_maneuverAssistList);

        json_object_array_add(intersections_states, states_element);
        json_object_object_add(intersections_element, "states", intersections_states);

        json_object* intersections_maneuverAssistList = json_object_new_array();
        json_object* intersections_maneuverAssistList_element = json_object_new_object();

        json_object_object_add(intersections_maneuverAssistList_element, "connectionID", json_object_new_int((int)spat.intersections[0].states[0].maneuverAssistList[0].connectionID));
        json_object_object_add(intersections_maneuverAssistList_element, "queueLength", json_object_new_int((int)spat.intersections[0].states[0].maneuverAssistList[0].queueLength));
        json_object_object_add(intersections_maneuverAssistList_element, "availableStorageLength", json_object_new_int((int)spat.intersections[0].states[0].maneuverAssistList[0].availableStorageLength));
        //cout << spat.intersections[0].states[0].maneuverAssistList[0].waitOnStop << endl;
        json_object_object_add(intersections_maneuverAssistList_element, "waitOnStop", json_object_new_boolean(spat.intersections[0].states[0].maneuverAssistList[0].waitOnStop));
        json_object_object_add(intersections_maneuverAssistList_element, "pedBicycleDetect", json_object_new_boolean(spat.intersections[0].states[0].maneuverAssistList[0].pedBicycleDetect));

        json_object_array_add(intersections_maneuverAssistList, intersections_maneuverAssistList_element);
        json_object_object_add(intersections_element, "maneuverAssistList", intersections_maneuverAssistList);

        json_object* intersections_roadAuthorityID = json_object_new_object();
        json_object_object_add(intersections_roadAuthorityID, "type", json_object_new_int((int)spat.intersections[0].roadAuthorityID.type));
        json_object* roadAuthorityID_val = json_object_new_object();
        json_object_object_add(roadAuthorityID_val, "length", json_object_new_int(spat.intersections[0].roadAuthorityID.val.fullRdAuthID.length));
        json_object_object_add(roadAuthorityID_val, "value", json_object_new_string((const char*)spat.intersections[0].roadAuthorityID.val.fullRdAuthID.value));
        json_object_object_add(intersections_roadAuthorityID, "val", roadAuthorityID_val);

        json_object_object_add(intersections_element, "roadAuthorityID", intersections_roadAuthorityID);

        json_object_array_add(intersections, intersections_element);

        //printf("\n\n%s\n\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));

        const char* json_str = json_object_get_string(root);
        //printf("%s\n", json_str);

        return json_str;
    }

    static const char* get_str_JSON_MAP(DATA_MAP map){
        json_object* root = json_object_new_object();
        json_object_object_add(root, "timeStamp", json_object_new_int((int)map.timeStamp));
        json_object_object_add(root, "msgIssueRevision", json_object_new_int((int)map.msgIssueRevision));
        json_object_object_add(root, "layerType", json_object_new_int((int)map.layerType));
        json_object_object_add(root, "layerID", json_object_new_int((int)map.layerID));

        json_object* intersections = json_object_new_array();
        json_object_object_add(root, "intersections", intersections);

        json_object* intersections_element = json_object_new_object();

        json_object* intersections_id = json_object_new_object();
        json_object_object_add(intersections_id, "region", json_object_new_int((int)map.intersections[0].id.region));
        json_object_object_add(intersections_id, "id", json_object_new_int((int)map.intersections[0].id.id));

        json_object_object_add(intersections_element, "id", intersections_id);

        json_object_object_add(intersections_element, "revision", json_object_new_int((int)map.intersections[0].revision));

        json_object* intersections_refPoint = json_object_new_object();
        json_object_object_add(intersections_refPoint, "lat", json_object_new_int((int)map.intersections[0].refPoint.lat));
        json_object_object_add(intersections_refPoint, "Long", json_object_new_int((int)map.intersections[0].refPoint.Long));
        json_object_object_add(intersections_refPoint, "elevation", json_object_new_int((int)map.intersections[0].refPoint.elevation));
        json_object_object_add(intersections_element, "refPoint", intersections_refPoint);

        json_object_object_add(intersections_element, "laneWidth", json_object_new_int((int)map.intersections[0].laneWidth));

        json_object* intersections_speedLimits = json_object_new_array();
        json_object* speedLimits_element = json_object_new_object();
        json_object_object_add(speedLimits_element, "type", json_object_new_int((int)map.intersections[0].speedLimits[0].type));
        json_object_object_add(speedLimits_element, "speed", json_object_new_int((int)map.intersections[0].speedLimits[0].speed));
        json_object_array_add(intersections_speedLimits, speedLimits_element);
        json_object_object_add(intersections_element, "speedLimits", intersections_speedLimits);

        json_object* intersections_laneSet = json_object_new_array();
        json_object* laneSet_element = json_object_new_object();
        json_object_object_add(laneSet_element, "laneID", json_object_new_int((int)map.intersections[0].laneSet[0].laneID));
        json_object_object_add(laneSet_element, "ingressApproach", json_object_new_int((int)map.intersections[0].laneSet[0].laneID));
        json_object_object_add(laneSet_element, "egressApproach", json_object_new_int((int)map.intersections[0].laneSet[0].laneID));

        json_object* laneSet_laneAttributes = json_object_new_object();
        json_object_object_add(laneSet_laneAttributes, "directionalUse", json_object_new_int((int)map.intersections[0].laneSet[0].laneAttributes.directionalUse));
        json_object_object_add(laneSet_laneAttributes, "sharedWith", json_object_new_int((int)map.intersections[0].laneSet[0].laneAttributes.sharedWith));

        json_object* laneAttributes_laneType = json_object_new_object();
        json_object_object_add(laneAttributes_laneType, "type", json_object_new_int((int)map.intersections[0].laneSet[0].laneAttributes.laneType.type));
        json_object_object_add(laneAttributes_laneType, "val", json_object_new_int((int)map.intersections[0].laneSet[0].laneAttributes.laneType.val.vehicle));

        json_object_object_add(laneSet_laneAttributes, "laneAttributes", laneAttributes_laneType);

        json_object_object_add(laneSet_laneAttributes, "maneuvers", json_object_new_int((int)map.intersections[0].laneSet[0].maneuvers));

        json_object* laneAttributes_nodeList = json_object_new_object();
        json_object_object_add(laneAttributes_nodeList, "type", json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.type));

        json_object* nodeList_val = json_object_new_array();

        json_object* nodeList_val_element = json_object_new_object();
        json_object* nodeList_val_element_delta = json_object_new_object();
        json_object_object_add(nodeList_val_element_delta, "type", json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.type));

        json_object* nodeList_val_element_delta_val = json_object_new_object();
        json_object_object_add(nodeList_val_element_delta_val, "x", json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.val.node_XY1.x));
        json_object_object_add(nodeList_val_element_delta_val, "y", json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.val.node_XY1.y));
        json_object_object_add(nodeList_val_element_delta, "val", nodeList_val_element_delta_val);

        json_object_object_add(nodeList_val_element, "delta", nodeList_val_element_delta);
        json_object_array_add(nodeList_val, nodeList_val_element);
        json_object_object_add(laneAttributes_nodeList, "val", nodeList_val);

        json_object* nodeList_val_element_attributes = json_object_new_object();

        json_object* nodeList_val_element_attributes_localNode = json_object_new_array();
        json_object_array_add(nodeList_val_element_attributes_localNode, json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.localNode[0]));
        json_object_object_add(nodeList_val_element_attributes, "localNode", nodeList_val_element_attributes_localNode);

        json_object* nodeList_val_element_attributes_disabled = json_object_new_array();
        json_object_array_add(nodeList_val_element_attributes_disabled, json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.localNode[0]));
        json_object_object_add(nodeList_val_element_attributes, "disabled", nodeList_val_element_attributes_disabled);

        json_object* nodeList_val_element_attributes_enabled = json_object_new_array();
        json_object_array_add(nodeList_val_element_attributes_enabled, json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.localNode[0]));
        json_object_object_add(nodeList_val_element_attributes, "enabled", nodeList_val_element_attributes_enabled);
 
        json_object* nodeList_val_element_attributes_data = json_object_new_array();
        json_object_array_add(nodeList_val_element_attributes_data, json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.localNode[0]));
        json_object_object_add(nodeList_val_element_attributes, "data", nodeList_val_element_attributes_data);

        json_object_object_add(nodeList_val_element_attributes, "dWidth", json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.dWidth));
        json_object_object_add(nodeList_val_element_attributes, "dElevation", json_object_new_int((int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.dElevation));

        json_object_object_add(nodeList_val_element, "attributes", nodeList_val_element_attributes);

        json_object_object_add(laneSet_laneAttributes, "nodeList", laneAttributes_nodeList);


        json_object_object_add(laneSet_element, "laneAttributes", laneSet_laneAttributes);


        json_object* laneSet_connectsTo = json_object_new_array();
        json_object* laneSet_connectsTo_element = json_object_new_object();

        json_object* laneSet_connectsTo_element_connectingLane = json_object_new_object();
        json_object_object_add(laneSet_connectsTo_element_connectingLane, "lane", json_object_new_int((int)map.intersections[0].laneSet[0].connectsTo[0].connectingLane.lane));
        json_object_object_add(laneSet_connectsTo_element_connectingLane, "maneuvers", json_object_new_int((int)map.intersections[0].laneSet[0].connectsTo[0].connectingLane.maneuvers));
        json_object_object_add(laneSet_connectsTo_element, "connectingLane", laneSet_connectsTo_element_connectingLane);

        json_object* laneSet_connectsTo_element_remoteIntersection = json_object_new_object();
        json_object_object_add(laneSet_connectsTo_element_remoteIntersection, "region", json_object_new_int((int)map.intersections[0].laneSet[0].connectsTo[0].remoteIntersection.region));
        json_object_object_add(laneSet_connectsTo_element_remoteIntersection, "id", json_object_new_int((int)map.intersections[0].laneSet[0].connectsTo[0].remoteIntersection.id));
        json_object_object_add(laneSet_connectsTo_element, "remoteIntersection", laneSet_connectsTo_element_remoteIntersection);

        json_object_object_add(laneSet_connectsTo_element, "signalGroup", json_object_new_int((int)map.intersections[0].laneSet[0].connectsTo[0].signalGroup));
        json_object_object_add(laneSet_connectsTo_element, "userClass", json_object_new_int((int)map.intersections[0].laneSet[0].connectsTo[0].userClass));
        json_object_object_add(laneSet_connectsTo_element, "connectionID", json_object_new_int((int)map.intersections[0].laneSet[0].connectsTo[0].connectionID));

        json_object_array_add(laneSet_connectsTo, laneSet_connectsTo_element);

        json_object_object_add(laneSet_element, "connectsTo", laneSet_connectsTo);

        json_object* laneSet_overlays = json_object_new_array();
        int siz = malloc_usable_size(map.intersections[0].laneSet[0].overlays)/sizeof(LaneID*);
        for(int i = 0; i<siz; i++){
            json_object_array_add(laneSet_overlays, json_object_new_int((int)map.intersections[0].laneSet[0].overlays[i]));

        }
        json_object_object_add(laneSet_element, "overlays", laneSet_overlays);

        json_object_array_add(intersections_laneSet, laneSet_element);
        json_object_object_add(intersections_element, "laneSet", intersections_laneSet);


        json_object* intersections_preemptPriorityData = json_object_new_array();
        //json_object_array_add(intersections_preemptPriorityData, json_object_new_object());
        json_object_array_add(intersections_preemptPriorityData, NULL);
        json_object_object_add(intersections_element, "preemptPriorityData", intersections_preemptPriorityData);

        json_object* intersections_roadAuthorityID = json_object_new_object();
        json_object_object_add(intersections_roadAuthorityID, "type", json_object_new_int((int)map.intersections[0].roadAuthorityID.type));
        json_object* roadAuthorityID_val = json_object_new_object();
        json_object_object_add(roadAuthorityID_val, "length", json_object_new_int((int)map.intersections[0].roadAuthorityID.val.fullRdAuthID.length));
        json_object_object_add(roadAuthorityID_val, "value", json_object_new_string((const char*)map.intersections[0].roadAuthorityID.val.fullRdAuthID.value));
        json_object_object_add(intersections_roadAuthorityID, "val", roadAuthorityID_val);
        json_object_object_add(intersections_element, "roadAuthorityID", intersections_roadAuthorityID);

        json_object_array_add(intersections, intersections_element);

        json_object* roadSegments = json_object_new_array();
        json_object* roadSegments_element = json_object_new_object();
        json_object_object_add(roadSegments_element, "name", json_object_new_string((const char*)map.roadSegments[0].name));
        json_object* roadSegments_id = json_object_new_object();
        json_object_object_add(roadSegments_id, "region", json_object_new_int((int)map.roadSegments[0].id.region));
        json_object_object_add(roadSegments_id, "id", json_object_new_int((int)map.roadSegments[0].id.id));
        json_object_object_add(roadSegments_element, "id", roadSegments_id);
        json_object_object_add(roadSegments_element, "revision", json_object_new_int((int)map.roadSegments[0].revision));

        json_object* roadSegments_refPoint = json_object_new_object();
        json_object_object_add(roadSegments_refPoint, "lat", json_object_new_int((int)map.roadSegments[0].refPoint.lat));
        json_object_object_add(roadSegments_refPoint, "Long", json_object_new_int((int)map.roadSegments[0].refPoint.Long));
        json_object_object_add(roadSegments_refPoint, "elevation", json_object_new_int((int)map.roadSegments[0].refPoint.elevation));
        json_object_object_add(roadSegments_element, "refPoint", roadSegments_refPoint);

        json_object_object_add(roadSegments_element, "laneWidth", json_object_new_int((int)map.roadSegments[0].laneWidth));

        json_object* roadSegments_speedLimits = json_object_new_array();
        json_object* roadSegments_speedLimits_element = json_object_new_object();
        json_object_object_add(roadSegments_speedLimits_element, "type", json_object_new_int((int)map.roadSegments[0].speedLimits[0].type));
        json_object_object_add(roadSegments_speedLimits_element, "speed", json_object_new_int((int)map.roadSegments[0].speedLimits[0].speed));
        json_object_array_add(roadSegments_speedLimits, roadSegments_speedLimits_element);
        json_object_object_add(roadSegments_element, "speedLimits", roadSegments_speedLimits);
        json_object_object_add(roadSegments_element, "roadLaneSet", json_object_new_int((int)map.roadSegments[0].roadLaneSet));

        json_object_array_add(roadSegments, roadSegments_element);
        json_object_object_add(root, "roadSegments", roadSegments);

        json_object* dataParameters = json_object_new_object();
        json_object_object_add(dataParameters, "processMethod", json_object_new_string((const char*)map.dataParameters.processMethod));
        json_object_object_add(dataParameters, "processAgency", json_object_new_string((const char*)map.dataParameters.processAgency));
        json_object_object_add(dataParameters, "lastCheckedDate", json_object_new_string((const char*)map.dataParameters.lastCheckedDate));
        json_object_object_add(dataParameters, "geoidUsed", json_object_new_string((const char*)map.dataParameters.geoidUsed));
        json_object_object_add(root, "dataParameters", dataParameters);

        json_object* restrictionList = json_object_new_array();
        json_object* restrictionList_element = json_object_new_object();
        json_object_object_add(restrictionList_element, "id", json_object_new_int((int)map.restrictionList[0].id));

        json_object* restrictionList_users = json_object_new_array();
        json_object* restrictionList_users_element = json_object_new_object();
        json_object_object_add(restrictionList_users_element, "basicType", json_object_new_int((int)map.restrictionList[0].users[0].basicType));
        json_object_array_add(restrictionList_users, restrictionList_users_element);
        json_object_object_add(restrictionList_element, "users", restrictionList_users);

        json_object_array_add(restrictionList, restrictionList_element);
        json_object_object_add(root, "restrictionList", restrictionList);


        //cout << endl;
        //printf("\n\n%s\n\n", json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));
        const char* json_str = json_object_get_string(root);
        //printf("%s\n", json_str);

        return json_str;
    }

    static void print_Json_MAP(DATA_MAP map){
        printf("map.timeStamp : %d\n", (int)map.timeStamp);
        printf("map.msgIssueRevision : %d\n", (int)map.msgIssueRevision);
        printf("map.layerType : %d\n", (int)map.layerType);
        printf("map.layerID : %d\n", (int)map.layerID);
        printf("map.intersections[0].id.region : %d\n", (int)map.intersections[0].id.region);
        printf("map.intersections[0].id.id : %d\n", (int)map.intersections[0].id.id);
        printf("map.intersections[0].revision : %d\n", (int)map.intersections[0].revision);
        printf("map.intersections[0].refPoint.lat : %d\n", (int)map.intersections[0].refPoint.lat);
        printf("map.intersections[0].refPoint.Long : %d\n", (int)map.intersections[0].refPoint.Long);
        printf("map.intersections[0].refPoint.elevation : %d\n", (int)map.intersections[0].refPoint.elevation);
        printf("map.intersections[0].laneWidth : %d\n", (int)map.intersections[0].laneWidth);
        printf("map.intersections[0].speedLimits[0].type : %d\n", (int)map.intersections[0].speedLimits[0].type);
        printf("map.intersections[0].speedLimits[0].speed : %d\n", (int)map.intersections[0].speedLimits[0].speed);
        printf("map.intersections[0].laneSet[0].laneID : %d\n", (int)map.intersections[0].laneSet[0].laneID);
        printf("map.intersections[0].laneSet[0].ingressApproach : %d\n", (int)map.intersections[0].laneSet[0].ingressApproach);
        printf("map.intersections[0].laneSet[0].egressApproach : %d\n", (int)map.intersections[0].laneSet[0].egressApproach );
        cout << "map.intersections[0].laneSet[0].laneAttributes.directionalUse : 0b" << bitset<2>(map.intersections[0].laneSet[0].laneAttributes.directionalUse ) << endl;
        cout << "map.intersections[0].laneSet[0].laneAttributes.sharedWith : 0b" << bitset<10>(map.intersections[0].laneSet[0].laneAttributes.sharedWith) << endl;
        cout << "map.intersections[0].laneSet[0].laneAttributes.laneType.value.vehicle : 0b" << bitset<16>(map.intersections[0].laneSet[0].laneAttributes.laneType.val.vehicle ) << endl;
        cout << "map.intersections[0].laneSet[0].maneuvers : 0b" << bitset<12>(map.intersections[0].laneSet[0].maneuvers) << endl;
        //printf("map.intersections[0].laneSet[0].laneAttributes.directionalUse : %d\n", (map.intersections[0].laneSet[0].laneAttributes.directionalUse ));
        //printf("map.intersections[0].laneSet[0].laneAttributes.sharedWith : %d\n", (map.intersections[0].laneSet[0].laneAttributes.sharedWith));
        //printf("map.intersections[0].laneSet[0].laneAttributes.laneType.vehicle : %d\n", (map.intersections[0].laneSet[0].laneAttributes.laneType.vehicle ));
        //printf("map.intersections[0].laneSet[0].maneuvers : %d\n", (map.intersections[0].laneSet[0].maneuvers));

        printf("map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.val.node_XY1.x : %d\n", (int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.val.node_XY1.x);
        printf("map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.val.node_XY1.y : %d\n", (int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].delta.val.node_XY1.y);
        printf("map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.localNode[0] : %d\n", (int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.localNode[0]);
        printf("map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.disabled[0] : %d\n", (int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.disabled[0]);
        printf("map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.enabled[0] : %d\n", (int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.enabled[0]);
        printf("map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.data[0].pathEndPointAngle : %d\n", (int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.data[0].val.pathEndPointAngle);
        printf("map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.dWidth : %d\n", (int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.dWidth);
        printf("map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.dElevation : %d\n", (int)map.intersections[0].laneSet[0].nodeList.val.nodes[0].attributes.dElevation);
        printf("map.intersections[0].laneSet[0].connectsTo[0].connectingLane.lane : %d\n", (int)map.intersections[0].laneSet[0].connectsTo[0].connectingLane.lane );;
        cout << "map.intersections[0].laneSet[0].connectsTo[0].connectingLane.maneuvers : 0b" << (map.intersections[0].laneSet[0].connectsTo[0].connectingLane.maneuvers) << endl;
        printf("map.intersections[0].laneSet[0].connectsTo[0].remoteIntersection.region : %d\n", (int)map.intersections[0].laneSet[0].connectsTo[0].remoteIntersection.region);
        printf("map.intersections[0].laneSet[0].connectsTo[0].remoteIntersection.id : %d\n", (int)map.intersections[0].laneSet[0].connectsTo[0].remoteIntersection.id);
        printf("map.intersections[0].laneSet[0].connectsTo[0].signalGroup : %d\n", (int)map.intersections[0].laneSet[0].connectsTo[0].signalGroup );
        printf("map.intersections[0].laneSet[0].connectsTo[0].userClass : %d\n", (int)map.intersections[0].laneSet[0].connectsTo[0].userClass );
        printf("map.intersections[0].laneSet[0].connectsTo[0].connectionID : %d\n", (int)map.intersections[0].laneSet[0].connectsTo[0].connectionID );
        printf("map.intersections[0].laneSet[0].overlays[0] : %d\n", (int)map.intersections[0].laneSet[0].overlays[0] );
        printf("map.intersections[0].roadAuthorityID.val.fullRdAuthID.length : %d\n", (int)map.intersections[0].roadAuthorityID.val.fullRdAuthID.length);
        printf("map.intersections[0].roadAuthorityID.val.fullRdAuthID.value : %s\n", map.intersections[0].roadAuthorityID.val.fullRdAuthID.value);
        printf("map.roadSegments[0].name : %s\n", map.roadSegments[0].name);
        printf("map.roadSegments[0].id.region : %d\n", (int)map.roadSegments[0].id.region);
        printf("map.roadSegments[0].id.id : %d\n", (int)map.roadSegments[0].id.id);
        printf("map.roadSegments[0].revision : %d\n", (int)map.roadSegments[0].revision );
        printf("map.roadSegments[0].refPoint.lat : %d\n", (int)map.roadSegments[0].refPoint.lat);
        printf("map.roadSegments[0].refPoint.Long : %d\n", (int)map.roadSegments[0].refPoint.Long);
        printf("map.roadSegments[0].refPoint.elevation : %d\n", (int)map.roadSegments[0].refPoint.elevation);
        printf("map.roadSegments[0].laneWidth : %d\n", (int)map.roadSegments[0].laneWidth);
        printf("map.roadSegments[0].speedLimits[0].type : %d\n", (int)map.roadSegments[0].speedLimits[0].type);
        printf("map.roadSegments[0].speedLimits[0].speed : %d\n", (int)map.roadSegments[0].speedLimits[0].speed);
        printf("map.roadSegments[0].roadLaneSet : %d\n", (int)map.roadSegments[0].roadLaneSet );
        printf("map.dataParameters.processMethod : %s\n", map.dataParameters.processMethod);
        printf("map.dataParameters.processAgency : %s\n", map.dataParameters.processAgency);
        printf("map.dataParameters.lastCheckedDate : %s\n", map.dataParameters.lastCheckedDate);
        printf("map.dataParameters.geoidUsed : %s\n", map.dataParameters.geoidUsed);
        printf("map.restrictionList[0].id : %d\n", (int)map.restrictionList[0].id );
        printf("map.restrictionList[0].users[0].basicType : %d\n", (int)map.restrictionList[0].users[0].basicType);
    }

    static St_Payload_JSON parse_json_payload(uint8_t* payload){
        St_Payload_JSON payload_json;

        uint8_t UTCTime_buf[PAYLOAD_SIZE_UTCTIME];
        memcpy(UTCTime_buf, &(payload[0]), PAYLOAD_SIZE_UTCTIME);
        payload_json.UTCTime = parse_UTCTime(UTCTime_buf);

        payload_json.MessageID = ushort(payload[PAYLOAD_IDX_MESSAGEID] + (payload[PAYLOAD_IDX_MESSAGEID+1]<<8));
        payload_json.JsonMessageLength = ushort(payload[PAYLOAD_IDX_JSONMESSAGELENGTH] + (payload[PAYLOAD_IDX_JSONMESSAGELENGTH+1]<<8));  

        payload_json.JsonMessage = (uint8_t*)malloc(sizeof(uint8_t)*payload_json.JsonMessageLength);
        memcpy(payload_json.JsonMessage, &payload[PAYLOAD_IDX_JSONMESSAGE], payload_json.JsonMessageLength);

        return payload_json;
    }

    static void print_UTCTime(St_UTCTime UTCTime){
        printf("year: %d, month: %d, day: %d\n", UTCTime.year, UTCTime.month, UTCTime.day);
        printf("hour: %d, min: %d, msec: %05d\n", UTCTime.hour, UTCTime.min, UTCTime.msec);
    }

    static void print_json_payload(St_Payload_JSON payload_json){
        printf("[Json Payload]\n");
        print_UTCTime(payload_json.UTCTime);
        printf("Message ID: %d\n", (int)payload_json.MessageID);
        printf("JsonMessageLength: %d\n", payload_json.JsonMessageLength);
        printf("JsonMessage: %s\n", (char*)payload_json.JsonMessage);
        printf("\n");
    }

    static void print_VehicleClassification(VehicleClassification obj){
        printf("keyType: %d\n", (int)obj.keyType);
        printf("role: %d\n", (int)obj.role);
        printf("iso3883: %d\n", (int)obj.iso3883);
        printf("hpmsType: %d\n", (int)obj.hpmsType);
        printf("vehicleType: %d\n", (int)obj.vehicleType);
        printf("responseEquip: %d\n", (int)obj.responseEquip);
        printf("responderType: %d\n", (int)obj.responderType);
        printf("fuelType: %d\n", (int)obj.fuelType);
    }

    static void print_FullPositionVector(FullPositionVector fullPositionVector){
        printf("utcTime.year: %d\n", fullPositionVector.utcTime.year);
        printf("utcTime.month: %d\n", fullPositionVector.utcTime.month);
        printf("utcTime.day: %d\n", fullPositionVector.utcTime.day);
        printf("utcTime.hour: %d\n", fullPositionVector.utcTime.hour);
        printf("utcTime.minute: %d\n", fullPositionVector.utcTime.minute);
        printf("utcTime.second: %d\n", fullPositionVector.utcTime.second);
        printf("utcTime.offset: %d\n", fullPositionVector.utcTime.offset);
        printf("Long: %d\n", fullPositionVector.Long);
        printf("lat: %d\n", fullPositionVector.lat);
        printf("elev: %d\n", fullPositionVector.elev);
        printf("heading: %d\n", fullPositionVector.heading);

        printf("speed.transmission: %d\n", (int)fullPositionVector.speed.transmission);
        printf("speed.speed: %d\n", fullPositionVector.speed.speed);

        printf("posAccuracy.semiMajor: %d\n", fullPositionVector.posAccuracy.semiMajor);
        printf("posAccuracy.semiMinor: %d\n", fullPositionVector.posAccuracy.semiMinor);
        printf("posAccuracy.orientation: %d\n", fullPositionVector.posAccuracy.orientation);

        printf("timeConfidence: %d\n", (int)fullPositionVector.timeConfidence);


        printf("posConfidence.pos: %d\n", (int)fullPositionVector.posConfidence.pos);
        printf("posConfidence.elevation: %d\n", (int)fullPositionVector.posConfidence.elevation);

        printf("speedConfidence.heading: %d\n", (int)fullPositionVector.speedConfidence.heading);
        printf("speedConfidence.speed: %d\n", (int)fullPositionVector.speedConfidence.speed);
        printf("speedConfidence.throttle: %d\n", (int)fullPositionVector.speedConfidence.throttle);
    }

    static void print_Snapshots(SnapshotList snapshots){
        for(int i = 0; i<snapshots.size; i++){
            print_FullPositionVector(snapshots.val[i].thePosition);
        }
    }

    static void print_Json_PVD(DATA_PVD pvd){

        printf("timeStamp: %d\n", pvd.timeStamp);
        printf("segNum: %d\n", pvd.segNum);
        printf("probeID.name: %s\n", pvd.probeID.name);

        if (pvd.probeID.vin != NULL){
            printf("probeID.name: %s\n", pvd.probeID.name);
        }

        printf("probeID.ownerCode: %s\n", pvd.probeID.ownerCode);

        printf("probeID.id.type: %d\n", (int)pvd.probeID.id.type);
        if(pvd.probeID.id.type == VehicleID_type::entityID){
            printf("probeID.id.val.entityID: 0x%02x", (uint8_t)pvd.probeID.id.val.entityID[0]);
            printf("%02x", (uint8_t)pvd.probeID.id.val.entityID[1]);
            printf("%02x", (uint8_t)pvd.probeID.id.val.entityID[2]);
            printf("%02x\n", (uint8_t)pvd.probeID.id.val.entityID[3]);
        }
        else if(pvd.probeID.id.type == VehicleID_type::stationID){
            printf("probeID.id.val.stationID: %d\n", pvd.probeID.id.val.stationID);
        }

        printf("probeID.vehicleType: %d\n", (int)pvd.probeID.vehicleType);

        if(pvd.probeID.vehicleClass.type != (vehicleClass_type)-1){
            printf("probeID.vehicleClass.type: %d\n", (int)pvd.probeID.vehicleClass.type);
            if(pvd.probeID.vehicleClass.type == vehicleClass_type::vGroup){
                printf("probeID.vehicleClass.val.vGroup: %d\n", (int)pvd.probeID.vehicleClass.val.vGroup);
            }
            else if(pvd.probeID.vehicleClass.type == vehicleClass_type::rGroup){
                printf("probeID.vehicleClass.val.rGroup: %d\n", (int)pvd.probeID.vehicleClass.val.rGroup);
            }
            else if(pvd.probeID.vehicleClass.type == vehicleClass_type::rEquip){
                printf("probeID.vehicleClass.val.rEquip: %d\n", (int)pvd.probeID.vehicleClass.val.rEquip);
            }
        }

        printf("startVector: \n");
        print_FullPositionVector(pvd.startVector);

        printf("vehicleType: \n");
        print_VehicleClassification(pvd.vehicleType);
        
        printf("snapshots: \n");
        print_Snapshots(pvd.snapshots);
        //pvd.timeStamp;
        //pvd.segNum;
        //pvd.probeID;
        //pvd.startVector;
        //pvd.vehicleType;
        //pvd.snapshots;
    }

    static DATA_PVD parse_json_pvd(uint8_t* JsonMessage){
        DATA_PVD Json_pvd;           

        json_object* obj = json_tokener_parse((const char*)JsonMessage);
        json_object* null = json_object_new_null();

        json_object* timeStamp = json_object_object_get(obj, "timeStamp");
        Json_pvd.timeStamp = (uint32_t)json_object_get_int(timeStamp);

        json_object* segNum = json_object_object_get(obj, "segNum");
        Json_pvd.segNum = (uint16_t)json_object_get_int(segNum);

        json_object* probeID = json_object_object_get(obj, "probeID");

        json_object* probeID_name = json_object_object_get(probeID, "name");
        strcpy(Json_pvd.probeID.name, json_object_get_string(probeID_name));

        json_object* probeID_vin = json_object_object_get(probeID, "vin");

        if (json_object_equal(probeID_vin, null) != 1){
            strcpy(Json_pvd.probeID.vin, json_object_get_string(probeID_vin));
        }

        json_object* probeID_ownerCode = json_object_object_get(probeID, "ownerCode");
        if (json_object_equal(probeID_ownerCode, null) != 1){
            strcpy(Json_pvd.probeID.ownerCode, json_object_get_string(probeID_ownerCode));
        }

        json_object* probeID_id = json_object_object_get(probeID, "id");

        if (json_object_equal(probeID_id, null) != 1){

            json_object* id_type = json_object_object_get(probeID_id, "type");
            Json_pvd.probeID.id.type = (VehicleID_type)json_object_get_int(id_type);

            json_object* id_val = json_object_object_get(probeID_id, "val");
            json_object* id_val_entityID = json_object_object_get(id_val, "entityID");
            json_object* id_val_idx_0 = json_object_array_get_idx(id_val_entityID, 0);
            json_object* id_val_idx_1 = json_object_array_get_idx(id_val_entityID, 1);
            json_object* id_val_idx_2 = json_object_array_get_idx(id_val_entityID, 2);
            json_object* id_val_idx_3 = json_object_array_get_idx(id_val_entityID, 3);

            memset(Json_pvd.probeID.id.val.entityID, 0x00, sizeof(Json_pvd.probeID.id.val.entityID));
            Json_pvd.probeID.id.val.entityID[0] = (unsigned char)json_object_get_int(id_val_idx_0);
            Json_pvd.probeID.id.val.entityID[1] = (unsigned char)json_object_get_int(id_val_idx_1);
            Json_pvd.probeID.id.val.entityID[2] = (unsigned char)json_object_get_int(id_val_idx_2);
            Json_pvd.probeID.id.val.entityID[3] = (unsigned char)json_object_get_int(id_val_idx_3);
        }

        json_object* probeID_vehicleType = json_object_object_get(probeID, "vehicleType");
        if (json_object_equal(probeID_vehicleType, null) != 1){
            Json_pvd.probeID.vehicleType = (VehicleType)json_object_get_int(probeID_vehicleType);
        }

        json_object* probeID_vehicleClass = json_object_object_get(probeID, "vehicleClass");
        if (json_object_equal(probeID_vehicleClass, null) != 1){
            json_object* JO_vehicleClass_type = json_object_object_get(probeID_vehicleClass, "type");
            Json_pvd.probeID.vehicleClass.type = (vehicleClass_type)json_object_get_int(JO_vehicleClass_type);

            json_object* JO_vehicleClass_val = json_object_object_get(probeID_vehicleClass, "val");
            if (Json_pvd.probeID.vehicleClass.type == (vehicleClass_type::vGroup)){
                json_object* vehicleClass_val_vGroup = json_object_object_get(JO_vehicleClass_val, "vGroup");
                Json_pvd.probeID.vehicleClass.val.vGroup = (VehicleGroupAffected)json_object_get_int(vehicleClass_val_vGroup);
            }
            else if (Json_pvd.probeID.vehicleClass.type == (vehicleClass_type::rGroup)){
                json_object* vehicleClass_val_rGroup = json_object_object_get(JO_vehicleClass_val, "rGroup");
                Json_pvd.probeID.vehicleClass.val.rGroup = (ResponderGroupAffected)json_object_get_int(vehicleClass_val_rGroup);
            }
            else if (Json_pvd.probeID.vehicleClass.type == (vehicleClass_type::rEquip)){
                json_object* vehicleClass_val_rEquip = json_object_object_get(JO_vehicleClass_val, "rEquip");
                Json_pvd.probeID.vehicleClass.val.rEquip = (IncidentResponseEquipment)json_object_get_int(vehicleClass_val_rEquip);
            }
        }
        else{
            Json_pvd.probeID.vehicleClass.type = (vehicleClass_type)-1;
        }

        json_object* JO_startVector = json_object_object_get(obj, "startVector");
        if (json_object_equal(JO_startVector, null) != 1){

            Json_pvd.startVector = parse_Json_FullPositionVector(JO_startVector);
        }

        json_object* JO_vehicleType = json_object_object_get(obj, "vehicleType");
        if (json_object_equal(JO_vehicleType, null) != 1){
            Json_pvd.vehicleType.keyType = (BasicVehicleClass)json_object_get_int(json_object_object_get(JO_vehicleType, "keyType"));
            Json_pvd.vehicleType.role = (BasicVehicleRole)json_object_get_int(json_object_object_get(JO_vehicleType, "role"));
            Json_pvd.vehicleType.iso3883 = (Iso3833VehicleType)json_object_get_int(json_object_object_get(JO_vehicleType, "iso3883"));
            Json_pvd.vehicleType.hpmsType = (VehicleType)json_object_get_int(json_object_object_get(JO_vehicleType, "hpmsType"));
            Json_pvd.vehicleType.vehicleType = (VehicleGroupAffected)json_object_get_int(json_object_object_get(JO_vehicleType, "vehicleType"));
            Json_pvd.vehicleType.responseEquip = (IncidentResponseEquipment)json_object_get_int(json_object_object_get(JO_vehicleType, "responseEquip"));
            Json_pvd.vehicleType.responderType = (ResponderGroupAffected)json_object_get_int(json_object_object_get(JO_vehicleType, "responderType"));
            Json_pvd.vehicleType.fuelType = (FuelType)json_object_get_int(json_object_object_get(JO_vehicleType, "fuelType"));
        }

        json_object* JO_snapshots = json_object_object_get(obj, "snapshots");
        if (json_object_equal(JO_snapshots, null) != 1){
            Json_pvd.snapshots.size = json_object_array_length(JO_snapshots);
            Json_pvd.snapshots.val = (Snapshot*)malloc(sizeof(Snapshot)*Json_pvd.snapshots.size);
            for (int i = 0; i < Json_pvd.snapshots.size; i++){

                json_object* JO_snapshot = json_object_array_get_idx(JO_snapshots, i);
                if (json_object_equal(JO_snapshot, null) != 1){

                    json_object* JO_snapshot_thePosition = json_object_object_get(JO_snapshot, "thePosition");
                    if (json_object_equal(JO_snapshot_thePosition, null) != 1){
                        Json_pvd.snapshots.val[i].thePosition = parse_Json_FullPositionVector(JO_snapshot_thePosition);

                    }

                    json_object* JO_snapshot_safetyExt = json_object_object_get(JO_snapshot, "safetyExt");
                    if (json_object_equal(JO_snapshot_safetyExt, null) != 1){

                        json_object* JO_safetyExt_events = json_object_object_get(JO_snapshot_safetyExt, "events");
                        if (json_object_equal(JO_safetyExt_events, null) != 1){
                            Json_pvd.snapshots.val[i].safetyExt.events = (VehicleEventFlags)json_object_get_int(JO_safetyExt_events);
                        }

                        json_object* JO_safetyExt_pathHistory = json_object_object_get(JO_snapshot_safetyExt, "pathHistory");
                        if (json_object_equal(JO_safetyExt_pathHistory, null) != 1){
                            json_object* JO_pathHistory_initialPosition = json_object_object_get(JO_safetyExt_pathHistory, "initialPosition");
                            if (json_object_equal(JO_pathHistory_initialPosition, null) != 1){
                                Json_pvd.snapshots.val[i].safetyExt.pathHistory.initialPosition = parse_Json_FullPositionVector(JO_pathHistory_initialPosition);
                            }

                            json_object* JO_pathHistory_currGNSSstatus = json_object_object_get(JO_safetyExt_pathHistory, "currGNSSstatus");
                            if (json_object_equal(JO_pathHistory_currGNSSstatus, null) != 1){
                                Json_pvd.snapshots.val[i].safetyExt.pathHistory.currGNSSstatus = (GNSSstatus)json_object_get_int(JO_pathHistory_currGNSSstatus);
                            }

                            json_object* JO_pathHistory_crumbData = json_object_object_get(JO_safetyExt_pathHistory, "crumbData");
                            if (json_object_equal(JO_pathHistory_crumbData, null) != 1){

                                long crumbData_len = json_object_array_length(JO_pathHistory_crumbData);
                                Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData = (PathHistoryPoint*)malloc(sizeof(PathHistoryPoint)*crumbData_len);
                                for(int j = 0; j < crumbData_len; j++){

                                    json_object* JO_crumbData = json_object_array_get_idx(JO_pathHistory_crumbData, j);
                                    if (json_object_equal(JO_crumbData, null) != 1){

                                        json_object* JO_crumbData_latOffset = json_object_object_get(JO_crumbData, "latOffset");
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].latOffset = (OffsetLL_B18)json_object_get_int(JO_crumbData_latOffset);

                                        json_object* JO_crumbData_lonOffset = json_object_object_get(JO_crumbData, "lonOffset");
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].lonOffset = (OffsetLL_B18)json_object_get_int(JO_crumbData_lonOffset);

                                        json_object* JO_crumbData_elevationOffset = json_object_object_get(JO_crumbData, "elevationOffset");
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].elevationOffset = (VertOffset_B12)json_object_get_int(JO_crumbData_elevationOffset);

                                        json_object* JO_crumbData_timeOffset = json_object_object_get(JO_crumbData, "timeOffset");
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].timeOffset = (TimeOffset)json_object_get_int(JO_crumbData_timeOffset);

                                        json_object* JO_crumbData_speed = json_object_object_get(JO_crumbData, "speed");
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].speed = (Speed)json_object_get_int(JO_crumbData_speed);

                                        json_object* JO_crumbData_posAccuracy = json_object_object_get(JO_crumbData, "posAccuracy");
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].posAccuracy.semiMajor = (uint8_t)json_object_get_int(json_object_object_get(JO_crumbData_posAccuracy, "semiMajor"));
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].posAccuracy.semiMinor = (uint8_t)json_object_get_int(json_object_object_get(JO_crumbData_posAccuracy, "semiMinor"));
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].posAccuracy.orientation = (uint16_t)json_object_get_int(json_object_object_get(JO_crumbData_posAccuracy, "orientation"));

                                        json_object* JO_crumbData_heading = json_object_object_get(JO_crumbData, "heading");
                                        Json_pvd.snapshots.val[i].safetyExt.pathHistory.crumbData[j].heading = (CoarseHeading)json_object_get_int(JO_crumbData_heading);
                                    }
                                }
                            }

                        }

                        json_object* JO_safetyExt_pathPrediction = json_object_object_get(JO_snapshot_safetyExt, "pathPrediction");
                        if (json_object_equal(JO_safetyExt_pathPrediction, null) != 1){
                            Json_pvd.snapshots.val[i].safetyExt.pathPrediction.radiusOfCurve = (RadiusOfCurvature)json_object_get_int(json_object_object_get(JO_safetyExt_pathPrediction, "radiusOfCurve"));
                            Json_pvd.snapshots.val[i].safetyExt.pathPrediction.confidence = (Confidence)json_object_get_int(json_object_object_get(JO_safetyExt_pathPrediction, "confidence"));

                        }

                        json_object* JO_safetyExt_lights = json_object_object_get(JO_snapshot_safetyExt, "lights");
                        if (json_object_equal(JO_safetyExt_lights, null) != 1){
                            Json_pvd.snapshots.val[i].safetyExt.lights = (ExteriorLights)json_object_get_int(JO_safetyExt_lights);

                        }

                        json_object* JO_safetyExt_timestamp = json_object_object_get(JO_snapshot_safetyExt, "timestamp");
                        if (json_object_equal(JO_safetyExt_timestamp, null) != 1){
                            Json_pvd.snapshots.val[i].safetyExt.timestamp = parse_Json_DDateTime(JO_safetyExt_timestamp);

                        }

                        json_object* JO_safetyExt_height = json_object_object_get(JO_snapshot_safetyExt, "height");
                        if (json_object_equal(JO_safetyExt_height, null) != 1){
                            Json_pvd.snapshots.val[i].safetyExt.height = (VehicleHeight)json_object_get_int(JO_safetyExt_height);

                        }
                    }

                    json_object* JO_snapshot_dataSet = json_object_object_get(JO_snapshot, "dataSet");
                    if (json_object_equal(JO_snapshot_dataSet, null) != 1){

                        printf("[not implement]: VehicleStatus\n");

                        if(0){
                            json_object* dataSet_lights = json_object_object_get(JO_snapshot_dataSet, "lights");
                            if (json_object_equal(dataSet_lights, null) != 1){
                                Json_pvd.snapshots.val[i].dataSet.lights = (ExteriorLights)json_object_get_int(dataSet_lights);
                            }
                            json_object* dataSet_lightBar = json_object_object_get(JO_snapshot_dataSet, "lightBar");
                            if (json_object_equal(dataSet_lightBar, null) != 1){
                                Json_pvd.snapshots.val[i].dataSet.lightBar = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_wipers = json_object_object_get(JO_snapshot_dataSet, "wipers");
                            if (json_object_equal(dataSet_wipers, null) != 1){
                                printf("not implement\n");
                            }
                            json_object* dataSet_brakeStatus = json_object_object_get(JO_snapshot_dataSet, "brakeStatus");
                            if (json_object_equal(dataSet_brakeStatus, null) != 1){

                            }
                            json_object* dataSet_brakePressure = json_object_object_get(JO_snapshot_dataSet, "brakePressure");
                            if (json_object_equal(dataSet_brakePressure, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.brakePressure = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_roadFriction = json_object_object_get(JO_snapshot_dataSet, "roadFriction");
                            if (json_object_equal(dataSet_roadFriction, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.roadFriction = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_sunData = json_object_object_get(JO_snapshot_dataSet, "sunData");
                            if (json_object_equal(dataSet_sunData, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.sunData = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_rainData = json_object_object_get(JO_snapshot_dataSet, "rainData");
                            if (json_object_equal(dataSet_rainData, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.rainData = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_airTemp = json_object_object_get(JO_snapshot_dataSet, "airTemp");
                            if (json_object_equal(dataSet_airTemp, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.airTemp = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_airPres = json_object_object_get(JO_snapshot_dataSet, "airPres");
                            if (json_object_equal(dataSet_airPres, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.airPres = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_steering = json_object_object_get(JO_snapshot_dataSet, "steering");
                            if (json_object_equal(dataSet_steering, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.steering = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_accelSets = json_object_object_get(JO_snapshot_dataSet, "accelSets");
                            if (json_object_equal(dataSet_accelSets, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.accelSets = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_Object = json_object_object_get(JO_snapshot_dataSet, "Object");
                            if (json_object_equal(dataSet_Object, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.Object = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_fullPos = json_object_object_get(JO_snapshot_dataSet, "fullPos");
                            if (json_object_equal(dataSet_fullPos, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.fullPos = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_throttlePos = json_object_object_get(JO_snapshot_dataSet, "throttlePos");
                            if (json_object_equal(dataSet_throttlePos, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.throttlePos = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_speedHeadC = json_object_object_get(JO_snapshot_dataSet, "speedHeadC");
                            if (json_object_equal(dataSet_speedHeadC, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.speedHeadC = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_speedC = json_object_object_get(JO_snapshot_dataSet, "speedC");
                            if (json_object_equal(dataSet_speedC, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.speedC = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_vehicleData = json_object_object_get(JO_snapshot_dataSet, "vehicleData");
                            if (json_object_equal(dataSet_vehicleData, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.vehicleData = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_vehicleIdent = json_object_object_get(JO_snapshot_dataSet, "vehicleIdent");
                            if (json_object_equal(dataSet_vehicleIdent, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.vehicleIdent = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_j1939data = json_object_object_get(JO_snapshot_dataSet, "j1939data");
                            if (json_object_equal(dataSet_j1939data, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.j1939data = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_weatherReport = json_object_object_get(JO_snapshot_dataSet, "weatherReport");
                            if (json_object_equal(dataSet_weatherReport, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.weatherReport = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                            json_object* dataSet_gnssStatus = json_object_object_get(JO_snapshot_dataSet, "gnssStatus");
                            if (json_object_equal(dataSet_gnssStatus, null) != 1){
                                //Json_pvd.snapshots.val[i].dataSet.gnssStatus = (LightbarInUse)json_object_get_int(dataSet_lightBar);
                            }
                        }
                    }
                }
            }
        }

        return Json_pvd;
    }

    static DDateTime parse_Json_DDateTime(json_object* JO_DDateTime){
        DDateTime dDateTime;
        json_object* null = json_object_new_null();

        if (json_object_equal(JO_DDateTime, null) != 1){
            dDateTime.year = (uint8_t)json_object_get_int(json_object_object_get(JO_DDateTime, "year"));
            dDateTime.month = (uint8_t)json_object_get_int(json_object_object_get(JO_DDateTime, "month"));
            dDateTime.day = (uint8_t)json_object_get_int(json_object_object_get(JO_DDateTime, "day"));
            dDateTime.hour = (uint8_t)json_object_get_int(json_object_object_get(JO_DDateTime, "hour"));
            dDateTime.minute = (uint8_t)json_object_get_int(json_object_object_get(JO_DDateTime, "minute"));
            dDateTime.second = (uint8_t)json_object_get_int(json_object_object_get(JO_DDateTime, "second"));
            dDateTime.offset = (uint16_t)json_object_get_int(json_object_object_get(JO_DDateTime, "offset"));
        }

        return dDateTime;
    }

    static FullPositionVector parse_Json_FullPositionVector(json_object* JO_fullPositionVector){

        json_object* null = json_object_new_null();
        FullPositionVector fullPositionVector;

        if (json_object_equal(JO_fullPositionVector, null) != 1){
            json_object* JO_fullPositionVector_utcTime = json_object_object_get(JO_fullPositionVector, "utcTime");
            if (json_object_equal(JO_fullPositionVector_utcTime, null) != 1){
                fullPositionVector.utcTime.year = (uint8_t)json_object_get_int(json_object_object_get(JO_fullPositionVector_utcTime, "year"));
                fullPositionVector.utcTime.month = (uint8_t)json_object_get_int(json_object_object_get(JO_fullPositionVector_utcTime, "month"));
                fullPositionVector.utcTime.day = (uint8_t)json_object_get_int(json_object_object_get(JO_fullPositionVector_utcTime, "day"));
                fullPositionVector.utcTime.hour = (uint8_t)json_object_get_int(json_object_object_get(JO_fullPositionVector_utcTime, "hour"));
                fullPositionVector.utcTime.minute = (uint8_t)json_object_get_int(json_object_object_get(JO_fullPositionVector_utcTime, "minute"));
                fullPositionVector.utcTime.second = (uint8_t)json_object_get_int(json_object_object_get(JO_fullPositionVector_utcTime, "second"));
                fullPositionVector.utcTime.offset = (uint16_t)json_object_get_int(json_object_object_get(JO_fullPositionVector_utcTime, "offset"));
            }

            json_object* JO_fullPositionVector_Long = json_object_object_get(JO_fullPositionVector, "Long");
            if (json_object_equal(JO_fullPositionVector_Long, null) != 1){
                fullPositionVector.Long = (int32_t)json_object_get_int(JO_fullPositionVector_Long);
            }

            json_object* JO_fullPositionVector_lat = json_object_object_get(JO_fullPositionVector, "lat");
            if (json_object_equal(JO_fullPositionVector_lat, null) != 1){
                fullPositionVector.lat = (int32_t)json_object_get_int(JO_fullPositionVector_lat);
            }

            json_object* JO_fullPositionVector_elev = json_object_object_get(JO_fullPositionVector, "elev");
            if (json_object_equal(JO_fullPositionVector_elev, null) != 1){
                fullPositionVector.elev = (int32_t)json_object_get_int(JO_fullPositionVector_elev);
            }

            json_object* JO_fullPositionVector_heading = json_object_object_get(JO_fullPositionVector, "heading");
            if (json_object_equal(JO_fullPositionVector_heading, null) != 1){
                fullPositionVector.heading = (uint16_t)json_object_get_int(JO_fullPositionVector_heading);
            }

            json_object* JO_fullPositionVector_speed = json_object_object_get(JO_fullPositionVector, "speed");
            if (json_object_equal(JO_fullPositionVector_speed, null) != 1){
                fullPositionVector.speed.transmission = (TransmissionState)json_object_get_int(json_object_object_get(JO_fullPositionVector_speed, "transmission"));
                fullPositionVector.speed.speed = (uint16_t)json_object_get_int(json_object_object_get(JO_fullPositionVector_speed, "speed"));
            }

            json_object* JO_fullPositionVector_posAccuracy = json_object_object_get(JO_fullPositionVector, "posAccuracy");
            if (json_object_equal(JO_fullPositionVector_posAccuracy, null) != 1){
                fullPositionVector.posAccuracy.semiMajor = (SemiMajorAxisAccuracy)json_object_get_int(json_object_object_get(JO_fullPositionVector_posAccuracy, "semiMajor"));
                fullPositionVector.posAccuracy.semiMinor = (SemiMinorAxisAccuracy)json_object_get_int(json_object_object_get(JO_fullPositionVector_posAccuracy, "semiMinor"));
                fullPositionVector.posAccuracy.orientation = (SemiMajorAxisOrientation)json_object_get_int(json_object_object_get(JO_fullPositionVector_posAccuracy, "orientation"));
            }
            else{
                fullPositionVector.posAccuracy.semiMajor = 255;
                fullPositionVector.posAccuracy.semiMinor = 255;
                fullPositionVector.posAccuracy.orientation = 65535;
            }

            json_object* JO_fullPositionVector_timeConfidence = json_object_object_get(JO_fullPositionVector, "timeConfidence");
            if (json_object_equal(JO_fullPositionVector_timeConfidence, null) != 1){
                fullPositionVector.timeConfidence = (TimeConfidence)json_object_get_int(JO_fullPositionVector_timeConfidence);
            }
            else{
                fullPositionVector.timeConfidence = (TimeConfidence)0;
            }

            json_object* JO_fullPositionVector_posConfidence = json_object_object_get(JO_fullPositionVector, "posConfidence");
            if (json_object_equal(JO_fullPositionVector_posConfidence, null) != 1){
                fullPositionVector.posConfidence.pos = (PositionConfidence)json_object_get_int(json_object_object_get(JO_fullPositionVector_posConfidence, "pos"));
                fullPositionVector.posConfidence.elevation = (ElevationConfidence)json_object_get_int(json_object_object_get(JO_fullPositionVector_posConfidence, "elevation"));
            }
            else{
                fullPositionVector.posConfidence.pos = (PositionConfidence)0;
                fullPositionVector.posConfidence.elevation = (ElevationConfidence)0;
            }

            json_object* JO_fullPositionVector_speedConfidence = json_object_object_get(JO_fullPositionVector, "speedConfidence");
            if (json_object_equal(JO_fullPositionVector_speedConfidence, null) != 1){
                fullPositionVector.speedConfidence.heading = (HeadingConfidence)json_object_get_int(json_object_object_get(JO_fullPositionVector_speedConfidence, "heading"));
                fullPositionVector.speedConfidence.speed = (SpeedConfidence)json_object_get_int(json_object_object_get(JO_fullPositionVector_speedConfidence, "speed"));
                fullPositionVector.speedConfidence.throttle = (ThrottleConfidence)json_object_get_int(json_object_object_get(JO_fullPositionVector_speedConfidence, "throttle"));
            }
            else{
                fullPositionVector.speedConfidence.heading = (HeadingConfidence)0;
                fullPositionVector.speedConfidence.speed = (SpeedConfidence)0;
                fullPositionVector.speedConfidence.throttle = (ThrottleConfidence)0;
            }
        }

        return fullPositionVector;
    }

    static DATA_BSM parse_json_bsm(uint8_t* JsonMessage){
        DATA_BSM Json_bsm;           

        json_object* obj = json_tokener_parse((const char*)JsonMessage);     

        json_object* coreData = json_object_object_get(obj, "coreData");
        json_object* msgCnt = json_object_object_get(coreData, "msgCnt");
        json_object* id = json_object_object_get(coreData, "id");
        json_object* secMark = json_object_object_get(coreData, "secMark");
        json_object* lat = json_object_object_get(coreData, "lat");
        json_object* Long = json_object_object_get(coreData, "Long");
        json_object* elev = json_object_object_get(coreData, "elev");
        json_object* accuracy = json_object_object_get(coreData, "accuracy");
        json_object* transmission = json_object_object_get(coreData, "transmission");
        json_object* speed = json_object_object_get(coreData, "speed");
        json_object* heading = json_object_object_get(coreData, "heading");
        json_object* angle = json_object_object_get(coreData, "angle");
        json_object* accelSet = json_object_object_get(coreData, "accelSet");
        json_object* brakes = json_object_object_get(coreData, "brakes");
        json_object* size = json_object_object_get(coreData, "size");
        Json_bsm.coreData.msgCnt = (uint8_t)json_object_get_int(msgCnt);
        json_object* id_idx_0 = json_object_array_get_idx(id, 0);
        json_object* id_idx_1 = json_object_array_get_idx(id, 1);
        json_object* id_idx_2 = json_object_array_get_idx(id, 2);
        json_object* id_idx_3 = json_object_array_get_idx(id, 3);
        //Json_bsm.coreData.id = (char*)malloc(sizeof(char)*4);
        memset(Json_bsm.coreData.id, 0x00, sizeof(Json_bsm.coreData.id));
        Json_bsm.coreData.id[0] = (unsigned char)json_object_get_int(id_idx_0);
        Json_bsm.coreData.id[1] = (unsigned char)json_object_get_int(id_idx_1);
        Json_bsm.coreData.id[2] = (unsigned char)json_object_get_int(id_idx_2);
        Json_bsm.coreData.id[3] = (unsigned char)json_object_get_int(id_idx_3);
        Json_bsm.coreData.secMark = (uint16_t)json_object_get_int(secMark);
        Json_bsm.coreData.lat = (int)json_object_get_int(lat);
        Json_bsm.coreData.Long = (int)json_object_get_int(Long);
        Json_bsm.coreData.elev = (int16_t)json_object_get_int(elev);
        json_object* semiMajor = json_object_object_get(accuracy, "semiMajor");
        json_object* semiMinor = json_object_object_get(accuracy, "semiMinor");
        json_object* orientation = json_object_object_get(accuracy, "orientation");
        Json_bsm.coreData.accuracy.semiMajor = (uint8_t)json_object_get_int(semiMajor);
        Json_bsm.coreData.accuracy.semiMinor = (uint8_t)json_object_get_int(semiMinor);
        Json_bsm.coreData.accuracy.orientation = (uint16_t)json_object_get_int(orientation);
        Json_bsm.coreData.transmission = (TransmissionState)json_object_get_int(transmission);
        Json_bsm.coreData.speed = (uint16_t)json_object_get_int(speed);
        Json_bsm.coreData.heading = (uint16_t)json_object_get_int(heading);
        Json_bsm.coreData.angle = (int8_t)json_object_get_int(angle);
        json_object* accelSet_Long = json_object_object_get(accelSet, "Long");
        json_object* accelSet_lat = json_object_object_get(accelSet, "lat");
        json_object* accelSet_vert = json_object_object_get(accelSet, "vert");
        json_object* accelSet_yaw = json_object_object_get(accelSet, "yaw");
        Json_bsm.coreData.accelSet.Long = (int16_t)json_object_get_int(accelSet_Long);
        Json_bsm.coreData.accelSet.lat = (int16_t)json_object_get_int(accelSet_lat);
        Json_bsm.coreData.accelSet.vert = (int8_t)json_object_get_int(accelSet_vert);
        Json_bsm.coreData.accelSet.yaw = (int16_t)json_object_get_int(accelSet_yaw);                
        json_object* brakes_wheelBrakes = json_object_object_get(brakes, "wheelBrakes");
        json_object* brakes_traction = json_object_object_get(brakes, "traction");
        json_object* brakes_abs = json_object_object_get(brakes, "abs");
        json_object* brakes_scs = json_object_object_get(brakes, "scs");
        json_object* brakes_brakeBoost = json_object_object_get(brakes, "brakeBoost");
        json_object* brakes_auxBrakes = json_object_object_get(brakes, "auxBrakes");
        Json_bsm.coreData.brakes.wheelBrakes = (uint8_t)json_object_get_int(brakes_wheelBrakes);
        Json_bsm.coreData.brakes.traction = (TractionControlStatus)json_object_get_int(brakes_traction);
        Json_bsm.coreData.brakes.abs = (AntiLockBrakeStatus)json_object_get_int(brakes_abs);
        Json_bsm.coreData.brakes.scs = (StabilityControlStatus)json_object_get_int(brakes_scs);
        Json_bsm.coreData.brakes.brakeBoost = (BrakeBoostApplied)json_object_get_int(brakes_brakeBoost);
        Json_bsm.coreData.brakes.auxBrakes = (AuxiliaryBrakeStatus)json_object_get_int(brakes_auxBrakes);                
        json_object* size_width = json_object_object_get(size, "width");
        json_object* size_length = json_object_object_get(size, "length");
        Json_bsm.coreData.size.width = (uint8_t)json_object_get_int(size_width);
        Json_bsm.coreData.size.length = (uint8_t)json_object_get_int(size_length);
        json_object* partII = json_object_object_get(obj, "partII");
        json_object* regional = json_object_object_get(obj, "regional");

        return Json_bsm;
    }

    static void print_json_bsm(DATA_BSM Json_bsm){
        printf("[Json BSM]\n");
        printf("msgCnt: %d\n", Json_bsm.coreData.msgCnt);

        printf("id(hex): 0x%02X%02X%02X%02X\n", Json_bsm.coreData.id[0], Json_bsm.coreData.id[1], Json_bsm.coreData.id[2], Json_bsm.coreData.id[3]);

        printf("secMark: %d\n", Json_bsm.coreData.secMark);
        printf("lat: %d\n", Json_bsm.coreData.lat);
        printf("Long: %d\n", Json_bsm.coreData.Long);
        printf("elev: %d\n", Json_bsm.coreData.elev);

        printf("semiMajor: %d\n",Json_bsm.coreData.accuracy.semiMajor);
        printf("semiMinor: %d\n",Json_bsm.coreData.accuracy.semiMinor);
        printf("orientation: %d\n",Json_bsm.coreData.accuracy.orientation);

        printf("transmission: %d\n",static_cast<int>(Json_bsm.coreData.transmission));
        printf("speed: %d\n",Json_bsm.coreData.speed);
        printf("heading: %d\n",Json_bsm.coreData.heading);
        printf("angle: %d\n",Json_bsm.coreData.angle);
        printf("accelSet.Long: %d\n",Json_bsm.coreData.accelSet.Long);
        printf("accelSet.lat: %d\n",Json_bsm.coreData.accelSet.lat);
        printf("accelSet.vert: %d\n",Json_bsm.coreData.accelSet.vert);
        printf("accelSet.yaw: %d\n",Json_bsm.coreData.accelSet.yaw);
        printf("brakes.wheelBrakes: %d\n", Json_bsm.coreData.brakes.wheelBrakes);
        printf("brakes.traction: %d\n", static_cast<int>(Json_bsm.coreData.brakes.traction));
        printf("brakes.abs: %d\n", static_cast<int>(Json_bsm.coreData.brakes.abs));
        printf("brakes.scs: %d\n", static_cast<int>(Json_bsm.coreData.brakes.scs));
        printf("brakes.brakeBoost: %d\n", static_cast<int>(Json_bsm.coreData.brakes.brakeBoost));
        printf("brakes.auxBrakes: %d\n", static_cast<int>(Json_bsm.coreData.brakes.auxBrakes));
        printf("size.width: %d\n",Json_bsm.coreData.size.width);
        printf("size.length: %d\n",Json_bsm.coreData.size.length);
        printf("\n");

    }
};

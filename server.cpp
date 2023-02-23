#include <iostream>
#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <wait.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>
#include <errno.h>
#include <unordered_map>
#include <stdint.h>
#include <malloc.h>

#include <bitset>
#include <sstream>
#include <iomanip>

#include <mutex>

#include <json-c/json.h>
//#include <json-c/arraylist.h>

#include "tr_MessageFrame.h"


#include <typeinfo>

using namespace std;
char tmp[669]= "001480FA5B1C007F54AFEEE5B1E617DB921E8B092DC8227FFFF00067F1FDFA1FA1007FFF80005D0F6202031882C0FCFFC0434FB7FFFC0FA6CC0A10FBDFFFC0F813C10E6FBFFFFC0F585C183CFBFFFFC0F32641F36FBDFFFC0F0C342590FB1FFFC0EE6DC2C0AFABFFFC0EBA8C33DCFA7FFFC0E93FC3AE4FA3FFFC0E670C432AFA1FFFC0E3F4C4A42FA3FFFC0E17845150FA1FFFC0DF18C5816F9FFFFC0DC8FC5F90F9FFFFC0DA3FC6684FA1FFFC0D7E646D7EF9FFFFC0D5A34744AFA3FFFC0D328C7BBCFA1FFFC0D0C648288F9FFFFC0CE4E489CAF9FFFFC0CBFDC9084FA9FFFC0C97AC9588FA3FFFC0C72049C12F9FFFFC00104200000000810800200C000003014082805773B9B9C6A9F3298083F616926CF279828FEDC49D81849DB36ACC3A1031CDBDCB33C0800A48031D07BF3AD6F14C452052048CBA8A95559B8ECB8F30DD1318DD385CB6F0264EA06879B2";;
uint8_t encodedMessage_sample[334];

json_object* JO_MAP;
json_object* JO_SPAT;

std::string getUtcTimeStr(){
    time_t now = time(0);
    tm* now_tm = gmtime(&now);
    char utc_time[64];
    strftime(utc_time, sizeof(utc_time), "%Y%m%d_%X", now_tm);

    return utc_time;
}

vector<pair<int, uint8_t*>> sim_tx_buf;
mutex m_sim_tx_buf;
vector<pair<int, uint8_t*>> ser_tx_buf;
mutex m_ser_tx_buf;

class clientData{
public:
    int clnt_cnt;
    char ID[16];
    int seq = 0;
    int sock;
    char recv_buffer[1024];
    int conn_state = 0;
    int sign = 0;
    int ack = 0;
    vector<string> recv_log;
    vector<string> send_log;
    vector<uint8_t*> send_buf;
    mutex m_send_buf;
    uint8_t pre_recv_seq;
    uint8_t pre_send_seq;
};

class clientManager{
public:
    int server_sock;
    int total_clnt_cnt = 0;
    int clnt_acpt_idx = 0;
    vector<thread> workers;
    //unordered_map<string, clientData> clientMap;
    unordered_map<string, clientData*> clientMap;
    unordered_map<string, vector<string>> msgLog;
    vector<string> access_log;
    vector<string> whole_log;
    DATA_MAP map;
    DATA_SPAT spat;
    DATA_BSM bsm;
    vector<string> clientList;

    void run(){
        thread run(&clientManager::task_accept, this);
        //run.join();
        //printf("accept task done\n");
        run.detach();
        thread filler(&clientManager::sendbuf_fill, this);
        filler.detach();
    }

    void logger(vector<string> log){

    }

    void task_accept(){
        struct sockaddr_in client_addr;
        //memset(&client_addr, 0x0, sizeof(client_addr));
        unsigned int  addr_len;

        int i = 0;
        while(1){
            int accept_sock;
            printf("waiting for client..\n\n");
            accept_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
            printf("client access\n");
            clientData CD;
            CD.sock = accept_sock;
            CD.clnt_cnt = clnt_acpt_idx;
            clnt_acpt_idx++;
            total_clnt_cnt++;

            CD.conn_state = 1;
            //printf("[accept] send_buf size: %ld\n", CD.send_buf.size());
            thread worker(&clientManager::work, this, &CD);
            worker.detach();

            i++;
        }
    }

    void remove_client(string ID){
        close(clientMap[ID]->sock);
        clientMap.erase(ID);
        cout << "client " << ID << " removed" << endl;
    }

    void work(clientData* CD){
        //CD->conn_state = 1;
        int num = CD->clnt_cnt;
        thread recv_thread(&clientManager::task_recv, this, CD);
        thread send_thread(&clientManager::task_send, this, CD);

        pthread_t H_recv = recv_thread.native_handle();
        pthread_t H_send = send_thread.native_handle();

        //recv_thread.detach();

        //task_send(CD);
        //pthread_cancel(H_recv);

        recv_thread.join();
        cout << "worker kill recv thread" << endl;

        send_thread.join();
        cout << "send_thread end" << endl;

        //pthread_cancel(H_recv);

        string leave;
        leave = getUtcTimeStr() + " leave: " + (string)CD->ID;
        cout << leave << endl;
        access_log.push_back(leave);

        whole_log.push_back(leave);

        close(CD->sock);
        printf("%s\n", (char*)CD->ID);
        clientMap.erase(CD->ID);
    }

    void sendbuf_fill(){
        while(1){
            if (sim_tx_buf.size() > 0){
                vector<pair<int, uint8_t*>>::iterator iter = sim_tx_buf.begin();

                St_Header header = messageProcessor::parse_header(sim_tx_buf.at(0).second);

                uint8_t* payload_buf = (uint8_t*)malloc(header.PayloadLength);
                memcpy(payload_buf, sim_tx_buf.at(0).second + HEADER_SIZE, header.PayloadLength);

                St_Payload_SIMULATOR_TX_J2735_DATA payload = messageProcessor::parse_SIMULATOR_TX_payload(payload_buf);

                free(payload_buf);
                payload_buf = nullptr;

                uint8_t* buf;

                if (header.MessageType == MESSAGE_TYPE_SIMULATOR_TX_J2735_DATA){
                    St_Header json_header;
                    St_Payload_JSON json_payload;

                    if (payload.MessageID == MESSAGE_ID_VALUE_MAP){
                        json_payload.UTCTime = payload.UTCTime;
                        json_payload.MessageID = payload.MessageID;
                        const char* str_MAP = json_object_to_json_string(JO_MAP);
                        json_payload.JsonMessage = (uint8_t*)malloc(strlen(str_MAP));
                        memcpy(json_payload.JsonMessage, str_MAP, strlen(str_MAP));
                        json_payload.JsonMessageLength = strlen(str_MAP);
                    }
                    else if (payload.MessageID == MESSAGE_ID_VALUE_SPAT){
                        json_payload.UTCTime = payload.UTCTime;
                        json_payload.MessageID = payload.MessageID;
                        const char* str_SPAT = json_object_to_json_string(JO_SPAT);
                        json_payload.JsonMessage = (uint8_t*)malloc(strlen(str_SPAT));
                        memcpy(json_payload.JsonMessage, str_SPAT, strlen(str_SPAT));
                        json_payload.JsonMessageLength = strlen(str_SPAT);
                    }

                    json_header = messageProcessor::set_header(
                                                            MESSAGE_TYPE_SERVER_OR_CLIENT_JSON_DATA,
                                                            header.Sequence,
                                                            PAYLOAD_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG + json_payload.JsonMessageLength,
                                                            DEVICE_TYPE_SERVER,
                                                            DEVID
                                                            );

                    //for(int i = 0; i<334; i++){
                        //printf("%02x", encodedMessage_sample[i]);
                    //}

                    buf = (uint8_t*)malloc(MESSAGEFRAME_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG + json_payload.JsonMessageLength);

                    memcpy(buf+HEADER_SIZE+PAYLOAD_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG, json_payload.JsonMessage, json_payload.JsonMessageLength);

                    //memcpy(buf, sim_tx_buf.at(0), MESSAGEFRAME_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG + json_payload.JsonMessageLength);
                    //for(int i = 0; i< MESSAGEFRAME_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG + json_payload.JsonMessageLength; i++){
                        //printf("%02x", buf[i]);
                    //}
                    //printf("\n");

                    for (auto elem = clientMap.begin(); elem!=clientMap.end(); elem++){
                        json_header.Sequence = ++(elem->second->pre_send_seq);

                        memcpy(buf, &json_header, HEADER_SIZE);
                        memcpy(buf+HEADER_SIZE, &json_payload, PAYLOAD_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG);

                        elem->second->m_send_buf.lock();
                        elem->second->send_buf.emplace_back(buf);
                        elem->second->m_send_buf.unlock();
                    }
                    free(json_payload.JsonMessage);
                    json_payload.JsonMessage = nullptr;
                }

                //uint8_t* buf = (uint8_t*)malloc(MESSAGEFRAME_SIZE_SIMULATOR_TX_WITHOUT_MSG + payload.EncodedMessageLength);

                free((uint8_t*)sim_tx_buf.at(0).second);
                sim_tx_buf.at(0).second = nullptr;
                //if(sim_tx_buf.at(0).second == nullptr){
                    //printf("sim_tx_buf free complete\n");
                //}

                //printf("[after free] sim_tx_buf: %0x\n", sim_tx_buf.at(0).second);

                sim_tx_buf.erase(sim_tx_buf.begin());
            }
        }
    }

    void task_send(clientData* CD){
        struct timespec begin;
        struct timespec end;
        char msg[64];
        clock_gettime(CLOCK_MONOTONIC, &begin);
        clock_gettime(CLOCK_MONOTONIC, &end);
        while(1){
            if(CD->ack == -1){
                break;
            }
            if(CD->sign == -1){
                break;
            }
            else if (CD->sign){
                if(CD->ack){
                    if(CD->send_buf.size() > 0){
                        try{
                            int ret;
                            memset(msg, 0x0, sizeof(msg));
                            uint8_t* payload_buf;
                            //printf("[send]1 send_buf size: %ld\n", CD->send_buf.size());
                            St_Header header = messageProcessor::parse_header(CD->send_buf.at(0));

                            if(header.MessageType == MESSAGE_TYPE_SERVER_OR_SIMULATOR_SIGN_ACK){
                                CD->pre_send_seq = 0;
                                payload_buf = (uint8_t*)malloc(PAYLOAD_SIZE_ACK);
                                memcpy(payload_buf, CD->send_buf.at(0) + 9, PAYLOAD_SIZE_ACK);
                                St_Payload_ACK ack = messageProcessor::parse_Payload_ACK(payload_buf);
                                free(payload_buf);
                                payload_buf = nullptr;
                                ret = send(CD->sock, CD->send_buf.at(0), MESSAGEFRAME_SIZE_ACK, MSG_NOSIGNAL);
                            }
                            else if(header.MessageType == MESSAGE_TYPE_SERVER_OR_CLIENT_JSON_DATA){
                                payload_buf = (uint8_t*)malloc(header.PayloadLength);
                                memcpy(payload_buf, CD->send_buf.at(0) + HEADER_SIZE, header.PayloadLength);
                                St_Payload_JSON payload = messageProcessor::parse_json_payload(payload_buf);
                                free(payload_buf);
                                payload_buf = nullptr;

                                int serial_DATA_Length = payload.JsonMessageLength + MESSAGEFRAME_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG;
                                uint8_t* serial_DATA = (uint8_t*)malloc(serial_DATA_Length);
                                memcpy(serial_DATA, &header, HEADER_SIZE);
                                memcpy(serial_DATA + HEADER_SIZE, &payload, PAYLOAD_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG);
                                memcpy(serial_DATA + HEADER_SIZE + PAYLOAD_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG, payload.JsonMessage, payload.JsonMessageLength);

                                ret = send(CD->sock, serial_DATA, serial_DATA_Length, MSG_NOSIGNAL);
                                free(serial_DATA);
                                serial_DATA = nullptr;
                            }

                            vector<uint8_t*>::iterator iter = CD->send_buf.begin();
                            free((uint8_t*)(CD->send_buf.at(0)));
                            CD->send_buf.at(0) = nullptr;
                            CD->send_buf.erase(iter);
                            //int ret = send(CD->sock, &json_data_MAP, sizeof(St_SERVER_OR_CLIENT_JSON_DATA), MSG_NOSIGNAL);
                            if (ret < 0){
                                perror("send error");
                                throw -1;
                                break;
                            }
                            else{
                                //cout << "send: " << msg << endl;
                                CD->send_log.push_back(msg);
                                string log;
                                log = "send-"+(string)(msg);
                                msgLog[CD->ID].push_back(log);
                                string tmp = getUtcTimeStr() + " send: " + CD->ID + " " + (string)msg;
                                //cout << tmp << endl;
                                whole_log.push_back(tmp);
                                CD -> seq++;
                            }
                        }
                        catch(int expn){
                            cout << "send error" << endl;
                            break;
                        }
                    }
                }
            }
            else if((CD->sign == 0) & (CD->conn_state == 0)){
                break;
            }
            
        }
        printf("client %s sending stopped by something\n", (char*)CD->ID);
    }

    void task_recv(clientData* CD){
        uint8_t* msg;

        while(1){
            if (CD->ack == -1){
                break;
            }
            memset(CD->recv_buffer, 0, sizeof(CD->recv_buffer));
            //while(recv(CD->sock, CD->recv_buffer, 50, MSG_DONTWAIT) <= 0){
            //}
            int ret = recv(CD->sock, CD->recv_buffer, HEADER_SIZE, 0);
            //printf("%d\n", ret);
            if (ret > 0){
                //cout << "recv: " << CD->recv_buffer << endl;
                CD->recv_log.push_back(CD->recv_buffer);
                string log;
                log = "recv-"+(string)(CD->recv_buffer);
                msgLog[CD->ID].push_back(log);

                string tmp = getUtcTimeStr() + " recv: " + CD->ID;
                //cout << tmp << endl;
                whole_log.push_back(tmp);

                uint8_t header_buf[HEADER_SIZE];
                memcpy(header_buf, CD->recv_buffer, HEADER_SIZE);

                St_Header header = messageProcessor::parse_header(header_buf);
                //messageProcessor::print_header(header);

                uint8_t* payload_buf = (uint8_t*)malloc(sizeof(uint8_t)*header.PayloadLength);
                ret = recv(CD->sock, payload_buf, header.PayloadLength, 0);
                //printf("payload_buf: %s\n", (char*)payload_buf);

                if(header.MessageType == MESSAGE_TYPE_SERVER_OR_SIMULATOR_SIGN){
                    messageProcessor::print_header(header);
                    St_Payload_SIGN payload_sign = messageProcessor::parse_SIGN(payload_buf);
                    free(payload_buf);
                    payload_buf = nullptr;
                    messageProcessor::print_SIGN(payload_sign);
                    //memcpy(CD->ID, payload_sign.FullDeviceID, PAYLOAD_SIZE_FULL_DEVICE_ID);
                    //if (memcmp(payload_sign.KeyCode, KeyCode, PAYLOAD_SIZE_KEY_CODE) == 0){
                    if (1){
                        uint8_t tmp[PAYLOAD_SIZE_FULL_DEVICE_ID];
                        memcpy(tmp, payload_sign.FullDeviceID, PAYLOAD_SIZE_FULL_DEVICE_ID);
                        sprintf(CD->ID, "%02X%02X%02X%02X%02X%02X", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5]);

                        string access;
                        access = getUtcTimeStr() + " access: " + (string)CD->ID;
                        cout << access << endl;
                        access_log.push_back(access);

                        whole_log.push_back(access);
                        CD->sign = 1;

                        St_SERVER_OR_SIMULATOR_SIGN_ACK ack = messageProcessor::set_ACK(0x00, 0x81);
                        ack.header.Sequence = 0;
                        msg = (uint8_t*)malloc(sizeof(uint8_t)*MESSAGEFRAME_SIZE_ACK);
                        memcpy(msg, &ack, MESSAGEFRAME_SIZE_ACK);
                        CD->send_buf.emplace_back(msg);
                        CD->ack = 1;
                        CD->pre_recv_seq = 0;
                        //clientMap[CD->ID] = *CD;
                        clientMap.insert(make_pair((char*)CD->ID, CD));;

                        printf("%s ack done\n", (char*)CD->ID);

                        printf("clientMap size: %ld\n", clientMap.size());
                    }
                    else{
                        CD->sign = -1;
                        CD->ack = -1;
                        CD->conn_state = 0;
                        St_SERVER_OR_SIMULATOR_SIGN_ACK ack = messageProcessor::set_ACK(0x01, 0x81);
                        memcpy(msg, &ack, MESSAGEFRAME_SIZE_ACK);
                        CD->send_buf.emplace_back(msg);
                        break;
                    }
                }
                else if(header.MessageType == MESSAGE_TYPE_SERVER_OR_CLIENT_JSON_DATA){
                    St_Payload_JSON payload_json = messageProcessor::parse_json_payload(payload_buf);
                    free(payload_buf);
                    payload_buf = nullptr;

                    //messageProcessor::print_json_payload(payload_json);

                    St_Position pos;

                    if (payload_json.MessageID == MESSAGE_ID_VALUE_BSM){
                        DATA_BSM bsm = messageProcessor::parse_json_bsm(payload_json.JsonMessage);
                        printf("client [%s]: bsm\n", CD->ID);
                        pos.lat = bsm.coreData.lat;
                        pos.lon = bsm.coreData.Long;
                        pos.elev = bsm.coreData.elev;
                    }
                    else if (payload_json.MessageID == MESSAGE_ID_VALUE_PVD){
                        DATA_PVD pvd = messageProcessor::parse_json_pvd(payload_json.JsonMessage);
                        printf("client [%s]: pvd\n", CD->ID);
                        pos.lat = pvd.startVector.lat;
                        pos.lon = pvd.startVector.Long;
                        pos.elev = pvd.startVector.elev;
                    }

                    messageProcessor::print_json_payload(payload_json);
                    printf("\n");

                    St_SERVER_TX_J2735_DATA server_tx_data;
                    server_tx_data.payload.UTCTime = payload_json.UTCTime;
                    server_tx_data.payload.Position = pos;
                    server_tx_data.payload.Tx_Result = 1;
                    server_tx_data.payload.PSID = 0xFFFFFFFF;
                    server_tx_data.payload.Channel = 1;
                    server_tx_data.payload.timeslot = 1;
                    server_tx_data.payload.Tx_Power = 1;
                    server_tx_data.payload.Tx_Rate = 1;
                    server_tx_data.payload.priority = 1;
                    server_tx_data.payload.dot2 = 1;
                    server_tx_data.payload.MessageID = payload_json.MessageID;
                    //server_tx_data.payload.EncodedMessageLength = payload_json.JsonMessageLength;
                    server_tx_data.payload.EncodedMessageLength = 334;
                    //server_tx_data.header = messageProcessor::set_header(MESSAGE_TYPE_SERVER_TX_J2735_DATA, 0, PAYLOAD_SIZE_SERVER_TX_WITHOUT_MSG + server_tx_data.payload.EncodedMessageLength, DEVICE_TYPE_SERVER, DEVID);
                    server_tx_data.header = messageProcessor::set_header(MESSAGE_TYPE_SERVER_TX_J2735_DATA, 0, PAYLOAD_SIZE_SERVER_TX_WITHOUT_MSG + 334, DEVICE_TYPE_SERVER, DEVID);
                    //uint8_t* full_buf = (uint8_t*)malloc(MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG + server_tx_data.payload.EncodedMessageLength);
                    uint8_t* full_buf = (uint8_t*)malloc(MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG + 334);
                    memcpy(full_buf, &server_tx_data, MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG);
                    memcpy(full_buf + MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG, encodedMessage_sample, server_tx_data.payload.EncodedMessageLength);



                    //printf("-------------------\n");
                    //for(int i = 0; i< MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG + server_tx_data.payload.EncodedMessageLength; i++){
                        //printf("%02x", full_buf[i]);
                    //}
                    //printf("\n");

                    m_ser_tx_buf.lock();
                    ser_tx_buf.emplace_back(MESSAGEFRAME_SIZE_SERVER_TX_WITHOUT_MSG + 334, full_buf);
                    m_ser_tx_buf.unlock();

                    //DATA_BSM json_bsm = messageProcessor::parse_json_bsm(payload_json.JsonMessage);
                    //messageProcessor::print_json_bsm(json_bsm);
                }

            }
            else if(ret == 0){
                CD->sign = -1;
                break;
            }
            else if(ret < 0){
                break;
            }
        }
        cout << "recv done" << endl;
        //pthread_exit(NULL);
    }
};

class simulatorData{
public:
    int simulator_sock;
    struct sockaddr_in simulator_addr;
    unsigned int simulator_addr_len;
    char header_buf[HEADER_SIZE];
    int sign_code;
    bool sign = false;
    bool denied = false;
    bool conn_state = false;
    bool wait = false;
    char* buf;
    vector<pair<int, uint8_t*>> send_buf;
    vector<pair<int, uint8_t*>> receive_buf;
    mutex m_simulator_denied;
    mutex m_wait;
    uint8_t pre_recv_seq;
    uint8_t pre_send_seq;
};

class simulatorManager{
public:
    simulatorData SD;

    void init(){
        //printf("simulator socket init\n");
        if((SD.simulator_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            perror("simulator_sock ");
        }

        SD.simulator_addr.sin_family = AF_INET;
        //SD.simulator_addr.sin_addr.s_addr = inet_addr(SIMULATOR_IP);
        //SD.simulator_addr.sin_port = htons(SIMULATOR_PORT);
        SD.simulator_addr.sin_addr.s_addr = inet_addr("118.45.183.29");
        SD.simulator_addr.sin_port = htons(9998);

        SD.sign = false;
        SD.denied = false;
        SD.conn_state = false;

    }

    void connectionManager(simulatorData* SD){
        while(1){
            if (SD->conn_state == false){
                connectToSimulator(SD);
            }
            else if((SD->sign == false) & (SD->conn_state == true)){
                signToSimulator(SD);

                SD->m_wait.lock();
                SD->wait = true;
                SD->m_wait.unlock();

                while(SD->wait){
                }

                if(SD->denied == true){
                    SD->m_simulator_denied.lock();
                    SD->denied = false;
                    SD->m_simulator_denied.unlock();
                    try{
                        printf("---\n");
                        close(SD->simulator_sock);
                        printf("---\n");
                        simulatorManager::init();
                    }
                    catch(int expn){}
                    SD->conn_state = false;
                }

            }
        }
        printf("con man end\n");
    }

    void signToSimulator(simulatorData* SD){
        printf("signToSimulator\n");
        St_SERVER_OR_CLIENT_SIGN sign = messageProcessor::set_SIGN();
        //messageProcessor::print_header(sign.header);
        //messageProcessor::print_SIGN(sign.payload);
        //uint8_t serial_sign[MESSAGEFRAME_SIZE_SIGN];
        uint8_t* serial_sign = (uint8_t*)malloc(MESSAGEFRAME_SIZE_SIGN);
        memcpy(serial_sign, &sign, MESSAGEFRAME_SIZE_SIGN);

        m_ser_tx_buf.lock();
        ser_tx_buf.emplace_back(MESSAGEFRAME_SIZE_SIGN, serial_sign);
        m_ser_tx_buf.unlock();
        SD->pre_send_seq = 0;
    }

    void run(){
        thread worker(&simulatorManager::work, this, &SD);
        worker.detach();
    }

    void work(simulatorData* SD){

        simulatorManager::init();
        //simulatorManager::connectToSimulator(SD);
        //simulatorManager::signToSimulator(SD);

        thread connection_manage_thread(&simulatorManager::connectionManager, this, SD);
        thread send_thread(&simulatorManager::sendToSimulator, this, SD);
        thread recv_thread(&simulatorManager::recvFromSimulator, this, SD);

        pthread_t H_conn = connection_manage_thread.native_handle();
        pthread_t H_recv = recv_thread.native_handle();
        pthread_t H_send = send_thread.native_handle();

        send_thread.join();
        cout << "send_thread end" << endl;

        //pthread_cancel(H_recv);
        recv_thread.join();

        string leave;
        leave = getUtcTimeStr() + " leave: " + (string)"simulator";
        cout << leave << endl;

        close(SD->simulator_sock);
    }

    void connectToSimulator(simulatorData* SD){
        int ret;
        printf("connecting to simulator...  \n");
        //while(1){
            ret = connect(SD->simulator_sock, (struct sockaddr *)&(SD->simulator_addr), sizeof(SD->simulator_addr));
            if(ret == -1){
                perror("simulator connect ");
                //close(SD->simulator_sock);
                init();
            }
            else if(ret == 0){
                printf("connect to simulator success\n");
                SD->conn_state = true;
                //break;
            }
            sleep(1);
        //}
    }

    void sendToSimulator(simulatorData* SD){
        printf("sendToSimulator\n");
        struct timespec begin;
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &begin);
        clock_gettime(CLOCK_MONOTONIC, &end);
        while(1){
            //if(((end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1000000000.0)<1){
                //clock_gettime(CLOCK_MONOTONIC, &end);
            //}
            //else{
            if(ser_tx_buf.size()>0){
            //if(SD->send_buf.size()>0){
                vector<pair<int, uint8_t*>>::iterator iter = ser_tx_buf.begin();
                clock_gettime(CLOCK_MONOTONIC, &begin);
                try{
                    m_ser_tx_buf.lock();
                    int msg_len = ser_tx_buf.at(0).first;
                    uint8_t* msg = (uint8_t*)malloc(msg_len);
                    memcpy(msg, ser_tx_buf.at(0).second, msg_len);
                    free((uint8_t*)ser_tx_buf.at(0).second);
                    ser_tx_buf.at(0).second = nullptr;
                    ser_tx_buf.erase(iter);
                    m_ser_tx_buf.unlock();

                    St_Header header = messageProcessor::parse_header(msg);

                    //printf("sendToSimulator\n");
                    //messageProcessor::print_header(header);

                    //for(int i = 0; i<msg_len; i++){
                        //printf("%02x", msg[i]);
                    //}
                    //printf("\n");

                    if(header.MessageType == MESSAGE_TYPE_SERVER_OR_SIMULATOR_SIGN){
                        //for(int i = 0; i<msg_len; i++){
                            //printf("%02X", msg[i]);
                        //}
                        //printf("\n");
                        int ret = send(SD->simulator_sock, msg, msg_len, MSG_NOSIGNAL);
                        free(msg);
                        msg = nullptr;
                    }
                    else if (SD->sign == 1){
                        //int serial_DATA_Length = payload.JsonMessageLength + MESSAGEFRAME_SIZE_SERVER_OR_CLIENT_JSON_DATA_WITHOUT_MSG;
                        int ret = send(SD->simulator_sock, msg, msg_len, MSG_NOSIGNAL);
                        free(msg);
                        msg = nullptr;
                        //int ret = send(CD->sock, &json_data_MAP, sizeof(St_SERVER_OR_CLIENT_JSON_DATA), MSG_NOSIGNAL);
                        if (ret < 0){
                            perror("send error");
                            SD->conn_state = false;
                            SD->sign = false;
                            //throw -1;
                            //break;
                        }
                        else{

                        }
                    }
                    else if(SD->sign == -1){
                        printf("[Simulator Sign failed]\n");
                        //printf("[Simulator Service Code: %s]\n", SIGN_CODE[SD->sign_code]);
                    }

                }
                catch(int expn){
                    //cout << "send error" << endl;
                    //break;
                }
            }   
        }
    }

    void recvFromSimulator(simulatorData* SD){
        printf("recvFromSimulator\n");
        while(1){
            memset(SD->header_buf, 0, sizeof(SD->header_buf));
            //while(recv(CD->sock, header_buf, 50, MSG_DONTWAIT) <= 0){
            //}
            if(SD->conn_state){
                int ret = recv(SD->simulator_sock, SD->header_buf, HEADER_SIZE, MSG_NOSIGNAL);
                if (ret > 0){
                    uint8_t header_buf[HEADER_SIZE];
                    memcpy(header_buf, SD->header_buf, HEADER_SIZE);

                    St_Header header = messageProcessor::parse_header(header_buf);
                    //messageProcessor::print_header(header);

                    uint8_t* payload_buf = (uint8_t*)malloc(sizeof(uint8_t)*header.PayloadLength);
                    ret = recv(SD->simulator_sock, payload_buf, header.PayloadLength, 0);

                    if(header.MessageType == MESSAGE_TYPE_SERVER_OR_SIMULATOR_SIGN_ACK){
                        St_Payload_ACK payload = messageProcessor::parse_Payload_ACK(payload_buf);
                        free(payload_buf);
                        payload_buf = nullptr;
                        messageProcessor::print_ACK(payload);
                        printf("Code: %d\n", (int)payload.Code);
                        if(payload.Code == 0x00){
                            SD->sign = true;
                        }
                        else{
                            SD->sign = false;
                            if(payload.Code == 0x01){
                                SD->sign_code = 1;
                                SD->m_simulator_denied.lock();
                                SD->denied = true;
                                SD->m_simulator_denied.unlock();
                            }
                            else if(payload.Code == 0x01){
                                SD->m_simulator_denied.lock();
                                SD->denied = true;
                                SD->m_simulator_denied.unlock();
                                SD->sign_code = 2;
                            }
                            printf("simulator sign failed: %s\n", SIGN_CODE[SD->sign_code]);
                        }

                        SD->m_wait.lock();
                        SD->wait = false;
                        SD->m_wait.unlock();
                    }
                    else if(header.MessageType == MESSAGE_TYPE_SIMULATOR_TX_J2735_DATA & SD->sign == true){
                        if (header.Sequence == SD->pre_recv_seq){

                        }
                        else{
                            St_Payload_SIMULATOR_TX_J2735_DATA payload = messageProcessor::parse_SIMULATOR_TX_payload(payload_buf);

                            uint8_t* full_buf = (uint8_t*)malloc(HEADER_SIZE + header.PayloadLength);

                            memcpy(full_buf, header_buf, HEADER_SIZE);
                            memcpy(full_buf + HEADER_SIZE, payload_buf, header.PayloadLength);

                            free(payload_buf);
                            payload_buf = nullptr;

                            if (payload.MessageID == MESSAGE_ID_VALUE_MAP){
                                printf("[SERVER]: recv MAP from sim => ");
                            }
                            else if(payload.MessageID == MESSAGE_ID_VALUE_SPAT){
                                printf("[SERVER]: recv SPAT from sim => ");
                            }
                            messageProcessor::print_SIMULATOR_TX_payload(payload);
                            printf("\n");

                            m_sim_tx_buf.lock();
                            sim_tx_buf.emplace_back(HEADER_SIZE + header.PayloadLength, full_buf);
                            m_sim_tx_buf.unlock();

                        }
                    }
                }
                else if(ret == 0){
                    if(SD->sign == 1){
                        printf("simulator connection error\n");
                        SD->conn_state = false;
                        SD->sign = false;
                        //close(SD->simulator_sock);
                        SD->sign = 0;
                    }
                    else{
                        printf("simulator sign denied\n");
                        SD->conn_state = false;
                        SD->sign = false;
                        //close(SD->simulator_sock);
                    }
                    //break;
                }
            }
        }
        cout << "recv done" << endl;
        //pthread_exit(NULL);
    }
};

class commander{
public:
    char command;
    clientManager* CM;
    simulatorManager* SM;

    void run(){
        //thread worker(&commander::work, this);
        //worker.detach();
        work();
    }

    void work(){
        char c;
        while(1){
            cout << endl << "command: (w: whole log, a: access log, r: remove client, c: exit)" << endl;
            cin >> c;
            if(c == 'w'){
                show_whole_log();
            }
            else if(c == 'a'){
                show_access_log();
            }
            else if(c == 'r'){
                string ID;
                cout << "ID: ";
                cin >> ID;
                CM->remove_client(ID);
            }
            else if(c == 'u'){
                show_user_list();
            }
            else if(c == 'c'){

                break;
            }
        }
    }

    void show_user_list(){
        for(auto CD:CM->clientMap){
            cout << CD.first << endl;
        }
    }

    void show_access_log(){
        for(auto log:CM->access_log){
            cout << log << endl;
        }
    }

    void show_whole_log(){
        for(auto log:CM->whole_log){
            cout << log << endl;
        }
    }
};

class server{
public:
    static int total_clnt_cnt;
    int server_sock;
    struct sockaddr_in addr;
    char buffer[1024];
    int len, recv_len;
    unsigned int  addr_len;
    thread server_thread;
    clientManager CM;
    simulatorManager SM;
    commander CDR;

    int simulator_sock;
    struct sockaddr_in simulator_addr;
    unsigned int simulator_addr_len;

    int init(){
        if((server_sock = socket(AF_INET, SOCK_STREAM, 0))<0){
            perror("server socket ");
            return 1;
        }
        memset(&addr, 0x00, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(PORT);

        if(bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
            perror("bind ");
            return 1;
        }    

        if(listen(server_sock, MAXPENDING) < 0){
            perror("listen ");
            return 1;
        }

        //vector<MSG> buf(0);

        CM.server_sock = server_sock;

        CDR.CM = &CM;
        CDR.SM = &SM;

        return 0;
    }

    void start(){
        SM.run();
        CM.run();
                
        /*
        while(1){
            //if (share_data.simulator_buf.size()>0){
            if (sim_tx_buf.size()>0){
                //for(int i = 0; i<malloc_usable_size(sim_tx_buf[0])/sizeof(uint8_t*); i++){
                for(int i = 0; i<sizeof(sim_tx_buf[0]); i++){
                    printf("%02X\n", sim_tx_buf[0][i]);
                }
                printf("\n");
                sim_tx_buf.pop_back();
                //cout << share_data.simulator_buf[0] << endl;
                //share_data.simulator_buf.pop_back();
            }
        }
        */
        CDR.run();
    }
};

int main(){

    size_t siz = messageProcessor::ConvertHexStrToArray(tmp, 334, encodedMessage_sample);
    JO_MAP = json_object_from_file("sample_MAP.json");
    JO_SPAT = json_object_from_file("sample_SPAT.json");


    //for(int i = 0; i<334; i++){
        //printf("%02x", encodedMessage_sample[i]);
    //}
    //printf("\n");
    server MyServer;
    MyServer.init();
    MyServer.start();

    //simulatorManager Simulator;
    //Simulator.init();
    //Simulator.run();

    //DATA_SPAT spat = messageProcessor::set_Json_SPAT();
    //messageProcessor::print_Json_SPAT(spat);
    //messageProcessor::get_str_JSON_SPAT(spat);

    //St_SERVER_OR_CLIENT_SIGN sign = messageProcessor::set_SIGN();
    //messageProcessor::print_SIGN(sign);

    //St_SERVER_OR_CLIENT_JSON_DATA json_data_MAP = messageProcessor::set_JSON_DATA_MAP();
    //uint8_t* serial_Json_DATA = messageProcessor::serialize_JSON_DATA(json_data_MAP);
}
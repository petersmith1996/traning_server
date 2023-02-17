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

#include <json-c/json.h>
#include <json-c/arraylist.h>

#include "tr_MessageFrame.h"

#include <typeinfo>

using namespace std;

std::string getUtcTimeStr(){
    time_t now = time(0);
    tm* now_tm = gmtime(&now);
    char utc_time[64];
    strftime(utc_time, sizeof(utc_time), "%Y%m%d_%X", now_tm);

    return utc_time;
}

class clientData{
public:
    int clnt_cnt;
    char ID[16];
    int seq = 0;
    int sock;
    char recv_buffer[1024];
    int conn_state = 0;
    vector<string> recv_log;
    vector<string> send_log;
    bool sign_succ = false;
    bool sign = false;
    bool denied = false;
};

class clientManager{
public:
    int simulator_sock;
    int total_clnt_cnt = 0;
    int clnt_acpt_idx = 0;
    vector<thread> workers;
    unordered_map<string, clientData> clientMap;
    unordered_map<string, vector<string>> msgLog;
    vector<string> access_log;
    vector<string> whole_log;
    DATA_MAP map;
    DATA_SPAT spat;
    DATA_BSM bsm;
    int cnt = 0;

    void run(){
        thread run(&clientManager::task_accept, this);

        //run.join();
        //printf("accept task done\n");
        run.detach();
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
            printf("waiting for server..\n\n");
            accept_sock = accept(simulator_sock, (struct sockaddr *)&client_addr, &addr_len);
            clientData CD;
            CD.sock = accept_sock;
            CD.clnt_cnt = clnt_acpt_idx;
            clnt_acpt_idx++;
            total_clnt_cnt++;

            memcpy(CD.ID, "server", sizeof("server"));

            clientMap[CD.ID] = CD;

            thread worker(&clientManager::work, this, &clientMap[CD.ID]);
            worker.detach();

            i++;
        }
        
    }

    void remove_client(string ID){
        close(clientMap[ID].sock);
        clientMap.erase(ID);
        cout << "client " << ID << " removed" << endl;
    }

    void work(clientData* CD){
        CD->conn_state = 1;
        int num = CD->clnt_cnt;
        thread recv_thread(&clientManager::task_recv, this, CD);
        thread send_thread(&clientManager::task_send, this, CD);

        pthread_t H_recv = recv_thread.native_handle();
        pthread_t H_send = send_thread.native_handle();

        recv_thread.join();
        cout << "worker kill recv thread" << endl;

        send_thread.join();
        cout << "send_thread end" << endl;


        string leave;
        leave = getUtcTimeStr() + " leave: " + (string)CD->ID;
        cout << leave << endl;
        access_log.push_back(leave);

        whole_log.push_back(leave);

        close(CD->sock);
        clientMap.erase(CD->ID);
    }

    void task_send(clientData* CD){
        struct timespec begin;
        struct timespec end;
        char msg[64];
        clock_gettime(CLOCK_MONOTONIC, &begin);
        clock_gettime(CLOCK_MONOTONIC, &end);
        while(1){
            if (CD->sign_succ){
                if(((end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1000000000.0)<1){
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                else{
                    clock_gettime(CLOCK_MONOTONIC, &begin);
                    try{
                        memset(msg, 0x0, sizeof(msg));
                        /*
                        St_SERVER_OR_CLIENT_JSON_DATA json_data_SPAT = messageProcessor::set_JSON_DATA_SPAT();

                        uint8_t* serial_Json_DATA = (uint8_t*)messageProcessor::serialize_JSON_DATA(json_data_SPAT);
                        int serial_Json_DATA_Length = HEADER_SIZE + json_data_SPAT.header.PayloadLength;
                        */

                        St_SIMULATOR_TX_J2735_DATA data = messageProcessor::set_SIMULATOR_TX_DATA(MESSAGE_ID_VALUE_MAP);
                        int data_size = MESSAGEFRAME_SIZE_SIMULATOR_TX_WITHOUT_MSG + data.payload.EncodedMessageLength;

                        uint8_t* serial_DATA = (uint8_t*)malloc(data_size);

                        memcpy(serial_DATA, &data, MESSAGEFRAME_SIZE_SIMULATOR_TX_WITHOUT_MSG);
                        memcpy(serial_DATA + MESSAGEFRAME_SIZE_SIMULATOR_TX_WITHOUT_MSG, data.payload.EncodedMessage, data.payload.EncodedMessageLength);

                        //printf("send EncodedMessage:\n");
                        //for(int i = 0; i<4; i++){
                            //printf("%02x", *(serial_DATA + MESSAGEFRAME_SIZE_SIMULATOR_TX_WITHOUT_MSG+i) );
                        //}

                        int ret = send(CD->sock, serial_DATA, data_size, MSG_NOSIGNAL);
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
            else{
                if (CD->sign){
                    uint8_t code;
                    uint16_t serviceFlag;
                    code = 0x00;
                    serviceFlag = 0x81;
                    St_SERVER_OR_SIMULATOR_SIGN_ACK ack = messageProcessor::set_ACK(code, serviceFlag);
                    uint8_t* msg = (uint8_t*)malloc(sizeof(MESSAGEFRAME_SIZE_ACK));
                    memcpy(msg, &ack, MESSAGEFRAME_SIZE_ACK);
                    int ret = send(CD->sock, msg, MESSAGEFRAME_SIZE_ACK, MSG_NOSIGNAL);
                    CD->sign_succ = true;
                }
                else if(CD->denied){
                    uint8_t code;
                    uint16_t serviceFlag;
                    code = 0x01;
                    serviceFlag = 0x00;
                    St_SERVER_OR_SIMULATOR_SIGN_ACK ack = messageProcessor::set_ACK(code, serviceFlag);
                    uint8_t* msg = (uint8_t*)malloc(sizeof(MESSAGEFRAME_SIZE_ACK));
                    memcpy(msg, &ack, MESSAGEFRAME_SIZE_ACK);
                    int ret = send(CD->sock, msg, MESSAGEFRAME_SIZE_ACK, MSG_NOSIGNAL);
                    CD->sign_succ = false;
                    printf("server sign deny\n");
                    break;
                }

            }
            
        }
        //printf("client %d sending stopped by something\n", CD->clnt_cnt);
    }

    void task_recv(clientData* CD){
        char msg[64];


        while(1){
            memset(CD->recv_buffer, 0, sizeof(CD->recv_buffer));
            //while(recv(CD->sock, CD->recv_buffer, 50, MSG_DONTWAIT) <= 0){
            //}
            int ret = recv(CD->sock, CD->recv_buffer, HEADER_SIZE, 0);
            //printf("%d\n", ret);
            if (ret > 0){

                uint8_t header_buf[HEADER_SIZE];
                memcpy(header_buf, CD->recv_buffer, HEADER_SIZE);
                //cout << header_buf << endl;

                St_Header header = messageProcessor::parse_header(header_buf);
                messageProcessor::print_header(header);

                uint8_t* payload_buf = (uint8_t*)malloc(sizeof(uint8_t)*header.PayloadLength);
                ret = recv(CD->sock, payload_buf, header.PayloadLength, 0);
                
                //for (int i = 0; i<header.PayloadLength; i++){
                    //printf("%02x", payload_buf[i]);
                //}
                //printf("\n");

                if(header.MessageType == MESSAGE_TYPE_SERVER_OR_SIMULATOR_SIGN){
                    St_Payload_SIGN payload_sign = messageProcessor::parse_SIGN(payload_buf);
                    messageProcessor::print_SIGN(payload_sign);

                    //clientManager::cnt++;
                    //cout << "cnt: " << clientManager::cnt << endl;
                    //if(clientManager::cnt > 3){
                    if(1){
                        CD->sign = true;
                        CD->denied = false;
                    }
                    else{
                        CD->denied = true;
                        printf("server sign unavailable\n");
                        break;
                    }
                }

                else if(header.MessageType == MESSAGE_TYPE_SERVER_OR_CLIENT_JSON_DATA){

                    St_Payload_JSON payload_json = messageProcessor::parse_json_payload(payload_buf);

                    //messageProcessor::print_json_payload(payload_json);

                    if(payload_json.MessageID == MESSAGE_ID_VALUE_BSM){
                        DATA_BSM json_bsm = messageProcessor::parse_json_bsm(payload_json.JsonMessage);
                        //messageProcessor::print_json_bsm(json_bsm);
                    }
                    else if(payload_json.MessageID == MESSAGE_ID_VALUE_PVD){
                        DATA_PVD json_pvd = messageProcessor::parse_json_pvd(payload_json.JsonMessage);
                        //messageProcessor::print_json_bsm(json_bsm);
                    }
                }

                else if(header.MessageType == MESSAGE_TYPE_SERVER_TX_J2735_DATA){
                    
                    St_Payload_SERVER_TX_J2735_DATA payload = messageProcessor::parse_SERVER_TX_payload(payload_buf);
                    messageProcessor::print_SERVER_TX_payload(payload);

                    if(payload.MessageID == MESSAGE_ID_VALUE_BSM){
                        //DATA_BSM json_bsm = messageProcessor::parse_json_bsm(payload.EncodedMessage);
                        printf("MESSAGE_TYPE_SERVER_TX_J2735_DATA: bsm\n");
                    }
                    else if(payload.MessageID == MESSAGE_ID_VALUE_PVD){
                        //DATA_PVD json_pvd = messageProcessor::parse_json_pvd(payload.EncodedMessage);
                        printf("MESSAGE_TYPE_SERVER_TX_J2735_DATA: pvd\n");
                    }

                }

            }
            else if(ret < 0){
                break;
            }
            else{
                break;
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

class simulator{
public:
    static int total_clnt_cnt;
    int simulator_sock;
    struct sockaddr_in addr;
    char buffer[1024];
    int len, recv_len;
    unsigned int  addr_len;
    thread server_thread;
    clientManager CM;
    commander CDR;

    struct sockaddr_in simulator_addr;
    unsigned int simulator_addr_len;

    int init(){
        if((simulator_sock = socket(AF_INET, SOCK_STREAM, 0))<0){
            perror("simulator socket ");
            return 1;
        }
        memset(&addr, 0x00, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(9998);

        if(bind(simulator_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
            perror("bind ");
            return 1;
        }    

        if(listen(simulator_sock, MAXPENDING) < 0){
            perror("listen ");
            return 1;
        }   

        CM.simulator_sock = simulator_sock;

        CDR.CM = &CM;

        return 0;
    }

    void start(){
        CM.run();
        CDR.run();
    }
};

int main(){

    simulator MySimulator;
    MySimulator.init();
    MySimulator.start();

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


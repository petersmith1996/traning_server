#pragma pack(1)
struct St_Header{
    uint16_t MessageType;
    uint8_t Sequence;
    uint16_t PayloadLength;
    uint8_t DeviceType;
    uint8_t DeviceID[3];
};
#pragma pack()
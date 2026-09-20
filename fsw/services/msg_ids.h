#ifndef MSG_IDS_H
#define MSG_IDS_H

/* Software-bus message IDs */
enum {
    MSG_ID_SENSOR_DATA  = 0x0100, /*payload: sensor_sample_t*/
    MSG_ID_EVENT        = 0x0200, /*payload: NUL-terminated string*/
    MSG_ID_CMD_RESET_CTRS   = 0x0301,
    MSG_ID_CMD_SET_TLM_PER   = 0x0302,  /*payload uint32_t*/
    MSG_ID_CMD_SET_TEMP_LIM = 0x0303,  /*payload float*/
    MSG_ID_CMD_ENTER_SAFE = 0x0304,
    MSG_ID_CMD_EXIT_SAFE  = 0x0305,
    MSG_ID_CMD_INJECT_FAULT = 0x0306,  /*payload float*/

};


/* Uplink opcodes (wire)*/
enum {CMD_NOOP=0x00, CMD_RESET_COUNTERS=0x01, CMD_SET_TLM_PERIOD=0x02, CMD_SET_TEMP_LIMIT=0x03,
    CMD_ENTER_SAFE=0x04, CMD_EXIT_SAFE=0x05, CMD_INJECT_FAULT=0x06};

/*Downlink APIDs*/
#define APID_HK 0x001
#define APID_EVENT 0x002


#endif // MSG_ID_H
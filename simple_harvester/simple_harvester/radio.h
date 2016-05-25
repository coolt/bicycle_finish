//#define BLE_ADV_PAYLOAD_BUF_LEN     64

void initRadio(void);
void runRadio(void);
void initRadioInts(void);
void initRadioStructs(void);

void radioPatch(void);
void radioCmdBusRequest(bool enabled);

void radioUpdateAdvData(int size, char* data);

void radioCmdStartRAT(void);
void radioSetupAndTransmit(void);

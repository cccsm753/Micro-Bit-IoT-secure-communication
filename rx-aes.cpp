#include "aes.hpp"
#include "MicroBit.h"
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cstring>
#include "music.h"


// Define variables

MicroBit uBit;
using namespace std; 
unsigned char block[32];
int serial = 0;
const char* secret = "uweIOT2023@Msc!Cyber&Sec";  // defind secret 


/*******import pre-defined variable from sha256.cpp*********/

#ifndef SHA256_BYTES
#define SHA256_BYTES 32
#endif
unsigned char Sha_Key[SHA256_BYTES]; //define variable for store sha256 hash output*/
int salt;

//extern void md5(uint8_t *initial_msg, size_t initial_len);

#ifdef __cplusplus
extern "C" {
#endif

void md5(uint8_t *initial_msg, size_t initial_len);

#ifdef __cplusplus
}
#endif

extern uint32_t h0, h1, h2, h3;   //Define globle variable for md5


// call sha256 function as a external function
extern void sha256(const void *data, size_t len, unsigned char *sha2digest);


MicroBitPin P0(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_ALL); //Initialize Pin 0
MicroBitPin P13(MICROBIT_ID_IO_P13, MICROBIT_PIN_P13, PIN_CAPABILITY_ALL); //Initialize Pin 13
MicroBitPin P14(MICROBIT_ID_IO_P14, MICROBIT_PIN_P14, PIN_CAPABILITY_ALL); //Initialize Pin 14
MicroBitPin P15(MICROBIT_ID_IO_P15, MICROBIT_PIN_P15, PIN_CAPABILITY_ALL); //Initialize Pin 15
MicroBitPin P16(MICROBIT_ID_IO_P16, MICROBIT_PIN_P16, PIN_CAPABILITY_ALL); //Initialize Pin 16


void md5hash(const char* intext, uint8_t* outhash) {  // function for md5 operation
    size_t length = strlen(intext);

    // Call the md5 function
    md5((uint8_t*)intext, length);

    // Copy the raw binary data of the hash into outhash
    memcpy(outhash, &h0, 4);
    memcpy(outhash + 4, &h1, 4);
    memcpy(outhash + 8, &h2, 4);
    memcpy(outhash + 12, &h3, 4);
}


// XOR operation

void xorsha256(uint8_t* shaKey, uint8_t* xorKey) {  // function create for XOR operation
    for (int i = 0; i < 16; ++i) {
        xorKey[i] = shaKey[i] ^ shaKey[i + 16];
    }
}

// call the function to play melody
void audio_virtual_pin_melody()
{
    pin = &uBit.audio.virtualOutputPin;
    playScale();

}

// call this function when recive data from radio
void onDataV2(MicroBitEvent)
{   
    char buffer[32];

    // catch the recived data from radio
    PacketBuffer b = uBit.radio.datagram.recv();

    //extract encrypted msg from receiev buffer (0-15)
    for(char i=0;i<16;i++){
        block[i]=b[i];
    }



    uBit.serial.printf("\r\nReceived Encrypted message is: ");
    for(uint8_t a=0;a<16;a++){
        uBit.serial.printf(" %x ", block[a]);
        uBit.sleep(10);
    }

    uBit.serial.printf("\r\nByte count %d \r\n",b.length());

    //extract salt values from receiev buffer b (16 )
    salt =b[16];
    uBit.serial.printf("\r\nReceived salt is: %d\n", salt); //print the salt

    /*************Perform the hashing*******************************************/
    sha256(secret, strlen(secret), Sha_Key);

    std::stringstream sha_prt;    //create string to apend the sha output

     for (int i = 0; i < SHA256_BYTES; ++i) {
        // Format each byte as a two-digit hexadecimal number
        sha_prt << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(Sha_Key[i]);
    }

    std::string shaKeyStr = sha_prt.str();  // format the output for print
    sha_prt.str(""); // Clear stringstream

    // Print the SHA-256 hash
    uBit.serial.printf("\rSHA-256 hash is: %s\n", shaKeyStr.c_str());
    /**************************************************************************/
     /********************************XOR**************************************/
    uint8_t xorkey[16]; // XOR key will be 16 bytes
    xorsha256(Sha_Key, xorkey);

    // Print the XOR key
    uBit.serial.printf("\r\nXOR Key is: ");
    for (size_t i = 0; i < sizeof(xorkey); i++) {
        uBit.serial.printf("%x ", xorkey[i]);
        uBit.sleep(10);
    }
    uBit.serial.printf("\r\n");
    uBit.serial.printf("Size of XOR Key is: %d ", sizeof(xorkey));

    /*******************************End XOR**************************************/
    ///////////////////////////////////////////////////////////////////////////////

    // call MD5 hash algorithem
    
    uint8_t md5out[16];

    char str[20]; // Buffer to hold the string representation of the integer
    sprintf(str, "%d", salt); // Convert integer to string
    

    md5hash(str, md5out); // call MD5 hash function

    // Print the md5 hash key
    uBit.serial.printf("\r\nMD5 Hash Key is: ");
    for (size_t i = 0; i < sizeof(md5out); i++) {
        uBit.serial.printf("%x ", md5out[i]);
        uBit.sleep(10);
    }
    uBit.serial.printf("\r\n");

    /**********Concatanate xorKey with hashOutput*/

    uint8_t dpk[32]; // = xorKey + hashOutput;
    std::memcpy(dpk, xorkey, 16); // Copy first 16 bytes from xorKey to dpk
    std::memcpy(dpk + 16, md5out, 16); // Copy next 16 bytes from hashOutput to dpk
    uint8_t dpk_prt[32];   // Define an array to print the generated DPK
    strncpy((char*)dpk_prt,(char *)dpk,32);

    /********print the DPK******************/
    uBit.serial.printf("\r\nDpk Key is: ");
    for (size_t i = 0; i < sizeof(dpk); i++) {
        uBit.serial.printf("%x ", dpk_prt[i]);
         uBit.sleep(10);
    }
    uBit.serial.printf("\r\n");
    uBit.serial.printf("\r\nSize of DPK is: %d\r\n",sizeof(dpk));
   
    //call the required AES function from the library
    struct AES_ctx ctx; 
    AES_init_ctx(&ctx,dpk);

    //decript the recived data with genarated AES ctx.
    AES_ECB_decrypt(&ctx, block);
    uBit.serial.printf("\r\nencrypted msg is: ");
    uBit.serial.printf("%s\r\n",&ctx);
    uBit.serial.printf("\r\nDecrepted msg is: ");
    uBit.serial.printf("%s\r\n",block);

    // compare received message with defined values and if its match execute the action.
     
    if (strcmp((char*)block,"LedON") ==0) {
        uBit.display.scroll("LED-ON"); 
        P13.setDigitalValue(1); // P13 is high
        P14.setDigitalValue(1); // P14 is high
        P16.setDigitalValue(1); // P16 is high
    }if (strcmp((char*)block,"LedOFF") ==0) {
        uBit.display.scroll("LED-OFF");
        P16.setDigitalValue(0);  // P16 is low
        P13.setDigitalValue(0); // P13 is low
        P14.setDigitalValue(0); // P14 is low
    }   
    /****************Rotete a FAN*************/
    if (strcmp((char*)block,"FanON") ==0) {
        uBit.display.scroll("FAN-ON");
        P15.setAnalogValue(255);
    }   
    if (strcmp((char*)block,"FanSLOW") ==0) {
        uBit.display.scroll("FAN-SLOW");
        P15.setAnalogValue(150);
    } 
    if (strcmp((char*)block,"FanOFF") ==0) {
        uBit.display.scroll("FAN-OFF");
        P15.setAnalogValue(0);
    }   
    /************Play Music****************/
    if (strcmp((char*)block,"MusicON") ==0) {
        uBit.display.scroll("MUSIC-ON");
        audio_virtual_pin_melody();
        
    }   

    // Output the received encrypted message to the serial port.
    uBit.serial.printf("%s\r\n",buffer);


}


int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();
    uBit.display.print("R"); // Display R to indicate  receiver
    uBit.serial.printf("Radio RX Test\r\n");
    
    // Listen on RADIO and execute function "onData"
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onDataV2);
    // Enable Radio
    uBit.radio.enable();
    //
    while(1)
        uBit.sleep(500);
}
    

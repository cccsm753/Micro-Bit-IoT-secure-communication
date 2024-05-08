#include "MicroBit.h"
#include "aes.hpp"
#include <string>

#include <sstream>
#include <iomanip>
#include <cstring>


MicroBit    uBit;
using namespace std; 
const char* secret = "uweIOT2023@Msc!Cyber&Sec";
std::string cleartext = "";
unsigned char block[16];
int serial = 0;
/*******import prd-defined variable from sha256.cpp*********/
#ifndef SHA256_BYTES
#define SHA256_BYTES 32
#endif
unsigned char Sha_Key[SHA256_BYTES]; //define variable for store sha256 hash output
// Import sha256 function as a external function
extern void sha256(const void *data, size_t len, unsigned char *sha2digest);

/*************************Import md5.c UWE file to use *****************/
#ifdef __cplusplus
extern "C" {
#endif

void md5(uint8_t *initial_msg, size_t initial_len);

#ifdef __cplusplus
}
#endif

extern uint32_t h0, h1, h2, h3;   //Define globle variable for md5

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

/**********************prepaire the key are send message to RX end***************/

void encryptdata(const std::string& cleartext)
{
    /*************Perform the hashing*******************************************/
    sha256(secret, strlen(secret), Sha_Key);

    std::stringstream sha_prt;    //create string to apend the sha output

     for (int i = 0; i < SHA256_BYTES; ++i) {
        // Print each byte as a two-digit hexadecimal number
        sha_prt << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(Sha_Key[i]);
    }

    std::string shaKeyStr = sha_prt.str();  // format the output for print
    
    sha_prt.str(""); // Clear stringstream

    // Print the SHA-256 hash
    uBit.serial.printf("\rSHA-256 hash is: %s\n", shaKeyStr.c_str());
    
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

    int salt = std::rand() % 255 + 1; // Generates a random number 1 byte
    uBit.serial.printf("\rRandom Salt is: %d\n", salt); //print the md5 hash
    uint8_t md5out[16]; // MD5 produces a 16 byte output
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
    std::memcpy(dpk, xorkey, 16); // Copy first 32 bytes from xorKey to dpk
    std::memcpy(dpk + 16, md5out, 16); // Copy next 32 bytes from hashOutput to dpk
    uint8_t dpk_prt[32];
    strncpy((char*)dpk_prt,(char *)dpk,32);

    //print the dpk ********************
    uBit.serial.printf("\r\nDpk Key is: ");
    for (size_t i = 0; i < sizeof(dpk); i++) {
        uBit.serial.printf("%x ", dpk_prt[i]);
         uBit.sleep(10);
    }
    uBit.serial.printf("\r\n");
    uBit.serial.printf("\r\nSize of DPK is: %d\r\n",sizeof(dpk));

   /**************************AES256-ECB Encryption*****************************/
    struct AES_ctx ctx; //define instance of AES_ctx
    AES_init_ctx(&ctx, dpk);
    int num_blocks = cleartext.length() / 16;
    if (cleartext.length() % 16 != 0)
    {
        num_blocks++;
    }
    for (int i = 0; i < num_blocks; i++)
    {
        std::fill(block, block + 16, 0);
        for (int j = 0; j < 16; j++)
        {
            int index = i * 16 + j;
            if (index < cleartext.length())
            {
                block[j] = cleartext[index];
            }
        }
    /*********** Encrypt the command using aes256 ECB**********/
    AES_ECB_encrypt(&ctx, block);
    uBit.serial.printf("\r\nEncrypted msg is: ");
    uBit.serial.printf("%s\r\n",&ctx);
    }

    PacketBuffer ciphertxt(sizeof(block)+1); //Create ciphertxt paketbuffer with size of 16
     
    for(uint8_t a=0;a<16;a++){   // 0 - 15 is encrypted message
        ciphertxt[a]=block[a];
    }
    ciphertxt[16]= salt; //****16th posission contain the salt
    uBit.radio.datagram.send(ciphertxt); //sending message through radio
    uBit.serial.printf("\r\n"); 

}


int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();
    uBit.display.print("T");
    uBit.radio.enable();

    MicroBitPin P0(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_ALL); //Initialize tuch buttons
    srand(std::time(nullptr)); // Use current time as seed for random generator
    bool isLedOn = false; // Flag to toggle between LED ON and OFF

    while(1) {
        if (uBit.buttonA.isPressed()) {
            uBit.sleep(100); // Debounce delay

            if (isLedOn) {
                encryptdata("LedOFF");
                uBit.display.scroll("LED-OFF");
                isLedOn = false;
            } else {
                encryptdata("LedON");
                uBit.display.scroll("LED-ON");
                isLedOn = true;
            }
            while(uBit.buttonA.isPressed()) {
                uBit.sleep(100); // Wait until button A is released
            }
        }
        if(uBit.io.P0.isTouched()){ //Fan ON
            uBit.display.scroll("FAN-ON");
            encryptdata("FanON");

        }if(uBit.io.P1.isTouched()){ //Fan slow
            uBit.display.scroll("FAN-SLOW");
            encryptdata("FanSLOW");

        }if(uBit.io.P2.isTouched()){ //Fan Off
            uBit.display.scroll("FAN-OFF");
            encryptdata("FanOFF");  
        } 

        if(uBit.buttonB.isPressed()){
            encryptdata("MusicON");
            uBit.display.scroll("MUSIC-ON");
        }
        uBit.sleep(500); //min time in ms between two transmissions, 500ms is 0.5sec!
    }
}

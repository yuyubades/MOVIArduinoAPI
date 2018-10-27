/********************************************************************
 This is a library for the Audeme MOVI Voice Control Shield
 ----> http://www.audeme.com/MOVI/
 This code is inspired and maintained by Audeme but open to change
 and organic development on GITHUB:
 ----> https://github.com/audeme/MOVIArduinoAPI
 Written by Gerald Friedland for Audeme LLC.
 Contact: fractor@audeme.com
 BSD license, all text above must be included in any redistribution.
 ********************************************************************/

#include "MOVIShield.h"
#include "MOVIShield.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "WString.h"

#include <pgmspace.h>

MOVI::MOVI()
{
    construct(false);
}

MOVI::MOVI(bool debugonoff)
{
    construct(debugonoff);
}


// Arduino's C++ does not allow for constructor overloading!
void inline MOVI::construct( bool debugonoff)
{
    debug=debugonoff;
    shieldinit=0;
    passstring="";
    response="";
    result="";
    intraining=false;
    firstsentence=true;
    callsigntrainok=true;
    
    mySerial = &Serial;
}

void MOVI::init()
{
    init(true);
}

void MOVI::init(bool waitformovi)
{
    if (shieldinit==0) {
        if (debug) {
            Serial1.begin(DEBUG_SERIAL_BAUDRATE);
        }
        mySerial->begin(SERIAL_BAUDRATE);
        shieldinit=1;
        while (waitformovi && !isReady()) {
            delay(10);
        }
        mySerial->println("INIT");
		if (debug) {
			Serial1.println("INIT");
		}
        String sresponse=getShieldResponse();
        int s=sresponse.indexOf(": ");
        String ver=sresponse.substring(s+2);
        
        firmwareversion=atof(ver.c_str());
       
        s=sresponse.indexOf("@");
        ver=sresponse.substring(s+1);
        hardwareversion=atof(ver.c_str());
		if (debug) {
			Serial1.println("INIT DONE");
		}
    }
    
}

signed int MOVI::poll()
{
    firstsentence=false; // We assume loop() and we can't train in loop.
    intraining=false;
  
    int curchar;
    int eventno;
    if (mySerial->available()) {
        curchar=mySerial->read();
        if (curchar=='\n') {
            if (debug) {
                Serial1.println(response);
            }
            if (response.indexOf("MOVIEvent[")>=0) {
				if (debug) {
					Serial1.print("poll resp MOVI: ");
					Serial1.println(response);
				}
                eventno=response.substring(response.indexOf("[")+1,response.indexOf("]:")).toInt();
                result=response.substring(response.indexOf(" ")+1);
                if (eventno<100) { // then it's a user-read-only event
                    response="";
                    return SHIELD_IDLE;
                }
                if (eventno==202) {
                    result=response.substring(response.indexOf("#")+1);
                    response="";
                    return result.toInt()+1; // Sentences returned start at 0,
                                             // we make it easier for non-programmers and start at 1.
                }
                if (eventno==203) { // this is a password event
                    response="";
                    result.trim();
                    if (passstring.equals(result)) {
                        return PASSWORD_ACCEPT;
                    } else {
                        return PASSWORD_REJECT;
                    }
                }
                response="";
                return -eventno;
            } else {
				if (debug) {
					Serial1.print("other jibberish not belonging to MOVI: ");
					Serial1.println(response);
				}
                // other jibberish not belonging to MOVI
            }
        } else {
			if (debug) {
				//Serial1.print("APPENDING CHAR: ");
				//Serial1.println((char) curchar);
			}
            response+=(char) curchar;
        }
    }
	else {
		//Serial1.println("SHIELD DATA NOT AVAILABLE");
	}
    
    return SHIELD_IDLE;
}

String MOVI::getResult()
{
    return result;
}

String MOVI::getShieldResponse()
{
    String resp="";
    int curchar;
    if (shieldinit==0) {
		if (debug) {
			//Serial1.println("SHIELD NOT READY");
		}
        init();
        return "";
    }
    while (shieldinit>0) {
        delay(10);
        if (mySerial->available()) {
            curchar=mySerial->read();
            if (curchar=='\n') {
				if (debug) {
					Serial1.print("RESP: ");
					Serial1.println(resp);
				}
                if (resp=="") continue;
                return resp;
            } else {
				if (debug) {
					//Serial1.print("APPENDING CHAR: ");
					//Serial1.println((char) curchar);
				}
                resp+=(char) curchar;
            }
        }
		else {
			if (debug) {
				//Serial1.println("SHIELD DATA NOT AVAILABLE");
			}
		}
    }
	
	if (debug) {
		//Serial1.println("SHIELD NOT READY");
	}
    return "";
}

bool MOVI::sendCommand(String command, String parameter, String okresponse)
{
    if (isReady()) {
        mySerial->println(command+" "+parameter+"\n");
        if (debug) {
	        Serial1.println(command+" "+parameter+"\n");
        }
        if (okresponse=="") return true;
        if (getShieldResponse().indexOf(okresponse)>=0) {
            return true;
        } else return false;
    } else return false;
}

#ifdef F // check to see if F() macro is available
bool MOVI::sendCommand(const __FlashStringHelper* command, const __FlashStringHelper* parameter, String okresponse)
{
    if (isReady()) {
        mySerial->print(command);
        mySerial->print(" ");
        mySerial->print(parameter);
        mySerial->println("\n");
		
		if (debug) {
			Serial1.print(command);
			Serial1.print(" ");
			Serial1.print(parameter);
			Serial1.println("\n");
		}
        if (okresponse=="") return true;
        if (getShieldResponse().indexOf(okresponse)>=0) {
            return true;
        } else return false;
    } else return false;
}
#endif


void MOVI::sendCommand(String command, String parameter)
{
    mySerial->println(command+" "+parameter+"\n");
	if (debug) {
		Serial1.println(command+" "+parameter+"\n");
	}
}

#ifdef F // check to see if F() macro is available
void MOVI::sendCommand(const __FlashStringHelper* command, const __FlashStringHelper* parameter)
{
    mySerial->print(command);
    mySerial->print(" ");
    mySerial->print(parameter);
    mySerial->println("\n");
	
	if (debug) {
		Serial1.print(command);
		Serial1.print(" ");
		Serial1.print(parameter);
		Serial1.println("\n");
    }
}
#endif

bool MOVI::isReady()
{
    if (shieldinit==100) {
        return true;
    }
    if (shieldinit==0) {
        init();
    }
    mySerial->println("PING\n");
	if (debug) {
		Serial1.println("PING\n");
	}
    if (getShieldResponse().indexOf("PONG")) {
        shieldinit=100;
        return true;
    }
    shieldinit=1;
    return false;
}

void MOVI::factoryDefault()
{
    sendCommand("FACTORY","");
}

void MOVI::stopDialog()
{
    sendCommand("STOP","");
}

void MOVI::restartDialog()
{
    sendCommand("RESTART","");
}

#ifdef F // check to see if F() macro is available
void MOVI::say(const __FlashStringHelper* sentence)
{
    sendCommand(F("SAY"),sentence);
}
#endif

void MOVI::say(String sentence)
{
    sendCommand("SAY",sentence);
}

void MOVI::setSynthesizer(int synth)
{
    if (synth==SYNTH_PICO) {
        sendCommand(F("SETSYNTH"),F("PICO"));
    } else {
        sendCommand(F("SETSYNTH"),F("ESPEAK"));
    }
}

void MOVI::setSynthesizer(int synth, String commandline)
{
    if (synth==SYNTH_PICO) {
            sendCommand("SETSYNTH","PICO "+commandline);
    } else {
            sendCommand("SETSYNTH","ESPEAK "+commandline);
    }
}


#ifdef F // check to see if F() macro is available
void MOVI::password(const __FlashStringHelper* question, String passkey)
{
    passstring=String(passkey);
    passstring.toUpperCase();
    passstring.trim();
    say(question);
    sendCommand(F("PASSWORD"),F(""));
}
#endif

void MOVI::password(String question, String passkey)
{
    passstring=String(passkey);
    passstring.toUpperCase();
    passstring.trim();
    say(question);
    sendCommand("PASSWORD","");
}

#ifdef F // check to see if F() macro is available
void MOVI::ask(const __FlashStringHelper* question)
{
    say(question);
    sendCommand(F("ASK"),F(""));
}
#endif

void MOVI::ask(String question)
{
    say(question);
    sendCommand("ASK","");
}

void MOVI::callSign(String callsign)
{
    if (callsigntrainok) sendCommand("CALLSIGN",callsign,"callsign");
    callsigntrainok=false;
}

void MOVI::responses(bool on)
{
    String parameter="ON";
    if (!on) parameter="OFF";
    sendCommand("RESPONSES",parameter);
}

void MOVI::welcomeMessage(bool on)
{
    String parameter="ON";
    if (!on) parameter="OFF";
    sendCommand("WELCOMEMESSAGE",parameter);
}

void MOVI::beeps(bool on)
{
    String parameter="ON";
    if (!on) parameter="OFF";
    sendCommand("BEEPS",parameter);
}

void MOVI::setVoiceGender(bool female)
{
    if (female) sendCommand("FEMALE","");
    else sendCommand("MALE","");
}

void MOVI::setVolume(int volume)
{
    sendCommand("VOLUME",String(volume));
}

void MOVI::setThreshold(int threshold)
{
    sendCommand("THRESHOLD",String(threshold));
}

float MOVI::getFirmwareVersion()
{
    return firmwareversion;
}

float MOVI::getAPIVersion()
{
    return API_VERSION;
}

float MOVI::getHardwareVersion()
{
    return hardwareversion;
}

#ifdef F // check to see if F() macro is available
bool MOVI::addSentence(const __FlashStringHelper* sentence)
{
    if (firstsentence) {
        intraining=sendCommand(F("NEWSENTENCES"),F(""),"210");
        firstsentence=false;
    }
    if (!intraining) return false;
    intraining=sendCommand(F("ADDSENTENCE"),sentence,"211");
    return intraining;
}
#endif


bool MOVI::addSentence(String sentence)
{
    // Needs a new MOVI instance (typically restart Arduino). This avoids training only part of the sentence set.
    if (firstsentence) {
        intraining=sendCommand("NEWSENTENCES","","210");
        firstsentence=false;
    }
    if (!intraining) return false;
    intraining=sendCommand("ADDSENTENCE",sentence,"211");
    return intraining;
}

bool MOVI::train()
{
    if (!intraining) return false;
    sendCommand("TRAINSENTENCES","","trained");
    intraining=false;
    return true;
}

MOVI::~MOVI()
{
#ifdef ARDUINO_ARCH_AVR
    if (NULL != mySerial)
    {
        delete mySerial;
    }
#endif
}

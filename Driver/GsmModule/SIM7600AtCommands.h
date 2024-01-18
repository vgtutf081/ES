#pragma once 

namespace ES::Driver {
    const char LF = 0X0A;
    const char CR = 0X0D;
    
    const std::string AtCommandAt = "AT";
    const char Tele2Operator[] = {'"', 'T', 'e', 'l', 'e', '2', ' ', 'T', 'e', 'l', 'e', '2', '"'};
    const char AtCopsData[] = {'+', 'C', 'O', 'P', 'S', ':', ' '}; 
    const char AtCopsRead[] = "AT+COPS?";
    const char TestNumber[] = {'+','7','9','0','8','1','4','6','0','3','5','6'};
    const char ATD[] = {'A', 'T', 'D'}; //make call
    const char ATA[] = {'A', 'T', 'A'}; //answer call
    const char AtCommandGpsEnable[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', '=', '1'};
    const char AtCommandGpsOk[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', '=', '1', CR, CR, LF, 'O', 'K'};
    const char AtCommandGpsInfo[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', 'I', 'N', 'F', 'O'};
    const char AtCommandGpsInfoOk[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', 'I', 'N', 'F', 'O', '?'};
    const char AtCommandGpsDisable[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', '=', '0'};
    const char AtStatusRdy[] = {'R', 'D', 'Y'};
    const char AtCpinReady[] = {'+', 'C', 'P',  'I', 'N', ':', ' ', 'R', 'E', 'A', 'D', 'Y'};
    const char AtSmsDone[] = {'S', 'M', 'S',  ' ', 'D', 'O', 'N', 'E'};
    const char AtPbDone[] = {'P', 'B', ' ', 'D', 'O', 'N', 'E'};
    const char AtStatusReady[] = {'R', 'E', 'A', 'D', 'Y'};
    const char AtStatusOk[] = "OK";
    const char AtCrLf[] = {CR, LF};
    const char CregIsOk[] = "+CREG: 0,1";
    const char CgattRequest[] = "AT+CGATT?";
    const char CgactRequest[] = "AT+CGACT?";
    const char CgattIsOk[] = "+CGATT: 1";
    const char CgactIsOk[] = "+CGACT: 1,1";


    const char AnswerCall[] = "ATA";
    const char DisconnectCall[] = "ATH";
    const char SetAthAvilable[] = "AT+CVHU=0";
    const char AtdTest[] = "ATD+79081460356;";
    const char VoiceCallBegin[] = "VOICE CALL: BEGIN";
    const char VoiceCallEnd[] = "VOICE CALL: END";
    const char NoCarrier[] = "NO CARRIER";
    const char Ring[] = "RING";
    const char MissedCall[] = "MISSED_CALL";
    const char CregReguest[] = "AT+CREG?";
    const char Tele2Apn[] = "internet.tele2.ru";
    const char CgdcontTele2[] = "AT+CGDCONT=1,\"IP\",\"internet.tele2.ru\"";
    const char CgattEnable[] = "AT+CGATT=1";
    const char CgactEnable[] = "AT+CGACT=1,1";

}